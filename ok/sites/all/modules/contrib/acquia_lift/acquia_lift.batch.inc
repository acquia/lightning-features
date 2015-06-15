<?php
/**
 * @file acquia_lift.batch.inc
 *
 * Provides functions for batch syncing agents to Lift
 */


/**
 * Batch syncs all campaigns to Lift.
 */
function acquia_lift_batch_sync_campaigns($agents = array()) {
  // First delete the queue because we are about to sync everything now.
  acquia_lift_delete_queue();
  // Now get all sync operations for all existing Acquia Lift agents.
  if (empty($agents)) {
    foreach (acquia_lift_get_agent_types() as $type => $info) {
      $agents = array_merge($agents, personalize_agent_load_by_type($type));
    }
  }

  $operations = acquia_lift_get_sync_operations_for_agents($agents);

  $batch_operations = array();
  foreach ($operations as $operation) {
    $batch_operations[] = array('acquia_lift_batch_process_item', array($operation));
  }
  $batch = array(
    'operations' => $batch_operations,
    'finished' => 'acquia_lift_batch_finished',
    'title' => t('Syncing campaign components to Acquia Lift'),
    'init_message' => t('Starting campaign sync.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Lift campaign sync has encountered an error.'),
    'file' => drupal_get_path('module', 'acquia_lift') . '/acquia_lift.batch.inc'
  );

  batch_set($batch);
}

/**
 * Batch API callback to process an API call to Lift.
 *
 * @param $item
 *   An array representing an API call that can be passed directly to
 *   acquia_lift_sync_item().
 *
 * @param &$context
 *   Parameter passed in by Batch API to allow keeping track of operation
 *   progress.
 */
function acquia_lift_batch_process_item($item, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
  }
  if (!isset($context['sandbox']['retries'])) {
    $context['sandbox']['retries'] = 0;
  }
  $errors = array();
  acquia_lift_batch_sync_item($item, $errors);
  if (empty($errors)) {
    $context['results'][] = t('Method @method called successfully', array('@method' => $item['method']));
    $context['sandbox']['progress']++;
  }
  else {
    // We allow one retry per operation.
    if (!$context['sandbox']['retries']) {
      $context['finished'] = 0.5;
      $context['sandbox']['retries'] = 1;
    }
    else {
      // Is there a better way to signal that a particular operation was
      // unsuccessful during batch processing?
      $context['results'][] = ACQUIA_LIFT_OPERATION_ERROR_PREFIX . implode(',', $errors);
      $context['sandbox']['progress']++;
      // Reset retries for the next operation.
      $context['sandbox']['retries'] = 0;
    }
  }
}

/**
 * Batch API callback for when processing of all items is complete.
 *
 * @param $success
 *   Whether or not the batch completed successfully.
 * @param $results
 *   An array holding the results of each operation.
 * @param $operations
 *   An array of unprocessed operations.
 */
function acquia_lift_batch_finished($success, $results, $operations) {
  if ($success) {
    // See if any of the results contains an error.
    $errors = array();
    foreach ($results as $result) {
      if (strpos($result, ACQUIA_LIFT_OPERATION_ERROR_PREFIX) === 0) {
        $errors[] = substr($result, strlen(ACQUIA_LIFT_OPERATION_ERROR_PREFIX));
      }
    }
    if (empty($errors)) {
      $message = t('Sync operation completed successfully.');
      $message_type = 'status';
    }
    else {
      $message_type = 'error';
      $message = t('Some errors occurred while syncing campaigns to Acquia Lift:');
      $message .= theme('item_list', array('items' => $errors));
    }
    drupal_set_message($message, $message_type);
  }
  else {
    // An error occurred.
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = t('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drupal_set_message($message, 'error');
  }
}

/**
 * Gets the required operations for syncing the specified agents to Lift.
 *
 * @param $agents
 *   An array of agents with agent names as keys and stdClass objects rep-
 *   resenting the agents as values.
 * @return array
 *   An array of operation arrays, each with a 'method' key and an 'args'
 *   key, that can be processed by acquia_lift_sync_item().
 */
function acquia_lift_get_sync_operations_for_agents($agents) {
  $operations = array();
  $lift_api = AcquiaLiftAPI::getInstance(variable_get('acquia_lift_account_info', array()));
  foreach ($agents as $agent_name => $agent) {
    if (!acquia_lift_is_testing_agent($agent)) {
      continue;
    }
    if ($agent_instance = personalize_agent_load_agent($agent_name)) {
      if (!$agent_instance instanceof AcquiaLiftAgent) {
        continue;
      }
      // If we're dealing with an agent that already exists in Lift
      // then we may need to delete some of its components.
      try {
        $existing_agent = $lift_api->getAgent($agent_name);
      }
      catch (AcquiaLiftException $e) {
        if ($e instanceof AcquiaLiftNotFoundException) {
          $existing_agent = FALSE;
        }
        else {
          // Any other exception means we can't communicate with Lift so just
          // return an empty operations array.
          return array();
        }
      }

      // First sync the agent itself. We need to know if Lift already has
      // a targeting rule set up for an agent with this name. It will need
      // to be deleted if the version of the agent we are now pushing does
      // not have one.
      $targeting_rule_exists = $lift_api->hasAutoTargetingRule($agent_name);
      $operations = array_merge($operations, $agent_instance->getAgentSyncOperations($targeting_rule_exists));

      // Now go through all option sets and sync those.
      $option_sets = personalize_option_set_load_by_agent($agent_name);
      $decisions = empty($option_sets) ? array() : AcquiaLiftAgent::convertOptionSetsToDecisions($option_sets);
      $old_decisions = array();
      if ($existing_agent) {
        // If this is an existing agent then the decision structure
        // may be different and we'll need to perform some DELETEs
        // as well as PUTs.
        if ($existing_points = $lift_api->getPointsForAgent($agent_name)) {
          // Build up the old decision tree for comparison with the
          // new tree.
          foreach ($existing_points as $point) {
            $old_decisions[$point] = array();
            if ($existing_decisions = $lift_api->getDecisionsForPoint($agent_name, $point)) {
              foreach ($existing_decisions as $decision) {
                $old_decisions[$point][$decision] = array();
                if ($existing_choices = $lift_api->getChoicesForDecision($agent_name, $point, $decision)) {
                  foreach ($existing_choices as $choice) {
                    $old_decisions[$point][$decision][] = $choice;
                  }
                }
              }
            }
          }
        }
      }
      $operations = array_merge($operations, $agent_instance->getDecisionSyncOperations($old_decisions, $decisions));

      if (!empty($option_sets)) {
        // Tell Acquia Lift about the fixed targeting for the option sets.
        $operations = array_merge($operations, $agent_instance->getFixedTargetingSyncOperations($option_sets));
      }
      $goals = personalize_goal_load_by_conditions(array('agent' => $agent_name));
      $new_goals = $old_goals = array();
      foreach ($goals as $goal) {
        $new_goals[$goal->action] = $goal->value;
      }
      if ($existing_agent && ($existing_goals = $lift_api->getGoalsForAgent($agent_name))) {
        // If it's an existing agent there may be goals that need
        // to be deleted.
        foreach ($existing_goals as $goal) {
          // The array of goals has goal names as keys and goal values
          // as values, but the values are not used so we can pass
          // any value for each old goal.
          $old_goals[$goal] = 1;
        }
      }
      $operations = array_merge($operations, $agent_instance->getGoalSyncOperations($old_goals, $new_goals));
    }
  }
  return $operations;
}
