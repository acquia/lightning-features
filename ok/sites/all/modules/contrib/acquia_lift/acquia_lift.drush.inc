<?php
/**
* @file acquia_lift.drush.inc
 * Provides Acquia Lift drush commands.
*/

/**
 * Implements hook_drush_command().
 */
function acquia_lift_drush_command() {
  $items['acquia-lift-menu-rebuild'] = array (
    'description' => 'Rebuild the menu items and classes',
    'aliases' => array('al-menu'),
  );
  $items['acquia-lift-campaign-sync'] = array (
    'description' => 'Sync all or one Lift campaign(s).',
    'aliases' => array('al-sync'),
  );
  return $items;
}

/**
 * Rebuilds the Drupal menu items.
 *
 * If any menu items get removed from the database, the Lift unified toolbar is
 * not able to properly display or wire up the backbone application
 * functionality.
 *
 * This rebuilds the menu so that it is as the JavaScript code expects.
 */
function drush_acquia_lift_menu_rebuild() {
  module_load_install('acquia_lift');
  // Delete the existing menu.
  menu_delete_links('acquia-lift-controls');
  // Re-generate the menu correctly.
  _acquia_lift_build_menu('acquia-lift-controls');
  drush_log('The menu has been rebuilt.', 'success');
}

/**
 * Syncs the Acquia Lift Campaigns.
 *
 * This command is an alternative to the UI's "Sync with Lift" buttons.
 */
function drush_acquia_lift_campaign_sync() {
  module_load_include('inc', 'acquia_lift', 'acquia_lift.batch');

  // Step 1. Exit if there is no campaign to be synchronized.
  $agents = array();
  foreach (acquia_lift_get_agent_types() as $type => $info) {
    $agents = array_merge($agents, personalize_agent_load_by_type($type));
  }
  if (empty($agents)) {
    drush_log(dt('There is no campaign to be synchronized.'), 'success');
    return;
  }

  // Step 2. Select sync type.
  $options = array(
    'all' => dt('Sync all campaigns'),
    'one' => dt('Sync one campaign'),
  );
  $type_selection = drush_choice($options, dt('Select sync type:'));
  if (!$type_selection) {
    return;
  }

  // Step 3. Based on type, do further selections.
  $agent_names = array_keys($agents);
  switch ($type_selection) {
    // Step 3a. Select all campaigns.
    case 'all':
      $options = array(
        'yes' => dt('Confirm'),
      );
      $options_message = "\n" . dt('Confirm syncing all campaigns:'). "\n- " . implode("\n- ", $agent_names);
      $confirm_selection = drush_choice($options, $options_message);
      if (!$confirm_selection) {
        return;
      }
      break;
    // Step 3b. Select one campaign.
    case 'one':
      $options_message = "\n" . dt('Select a campaign machine name to sync:');
      $name_selection = drush_choice($agent_names, $options_message);
      if (!$name_selection) {
        return;
      }
      $agent_name = $agent_names[$name_selection];
      $agent = $agents[$agent_name];
      $agents = array($agent_name => $agent);
      break;
  }

  // Step 4. Sync selected campaign(s)
  // First delete the queue because we are about to sync everything now.
  acquia_lift_delete_queue();
  // For each agent, set up a batch job including all the required save operations,
  // then process immediately.
  foreach ($agents as $agent_name => $agent) {
    $message = "\n" . dt('Synchronizing campaign ') . $agent_name . " ...";
    drush_print($message);
    $operations = acquia_lift_get_sync_operations_for_agents(array($agent_name => $agent));
    $batch_operations = array();
    foreach ($operations as $operation) {
      $batch_operations[] = array('acquia_lift_batch_process_item', array($operation));
    }
    $batch = array(
      'operations' => $batch_operations,
      'finished' => 'acquia_lift_drush_batch_finished',
      'file' => drupal_get_path('module', 'acquia_lift') . '/acquia_lift.batch.inc',
    );

    // Set and process.
    batch_set($batch);
    $batch =& batch_get();
    $batch['progressive'] = false;
    drush_backend_batch_process();
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
function acquia_lift_drush_batch_finished($success, $results, $operations) {
  // See if an error occurred.
  if (!$success) {
    // $operations contains the operations that remained unprocessed.
    $error_operation = reset($operations);
    $message = '- ' . dt('An error occurred while processing %error_operation with arguments: @arguments', array('%error_operation' => $error_operation[0], '@arguments' => print_r($error_operation[1], TRUE)));
    drush_log($message, 'error');
    return;
  }

  foreach ($results as $result) {
    $message = '- ' . dt($result);
    $message_type = 'ok';
    if (strpos($result, ACQUIA_LIFT_OPERATION_ERROR_PREFIX) === 0) {
      $message_type = 'error';
    }
    drush_log($message, $message_type);
  }
}
