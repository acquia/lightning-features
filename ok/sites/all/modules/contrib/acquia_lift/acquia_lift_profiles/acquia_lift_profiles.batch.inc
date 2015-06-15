<?php
/**
 * @file acquia_lift_profiles.batch.inc
 * Provides Acquia Lift Profiles batch functions.
 */

/**
 * Batch syncs actions to Lift Web.
 *
 * @param array $actions
 *   The actions to sync.
 */
function acquia_lift_batch_sync_actions($actions = array()) {
  if (empty($actions)) {
    $actions = visitor_actions_get_actions();
  }
  $batch_operations = array();
  foreach ($actions as $action_name => $info) {
    $label = acquia_lift_profiles_get_label_for_action($info);
    $operation = array(
      'label' => $label,
      'action_name' => $action_name
    );
    $batch_operations[] = array('acquia_lift_profiles_batch_sync_action', array($operation));
  }

  $batch = array(
    'operations' => $batch_operations,
    'finished' => 'acquia_lift_profiles_batch_finished',
    'title' => t('Syncing visitor actions to Acquia Lift Web'),
    'init_message' => t('Starting action sync.'),
    'progress_message' => t('Processed @current out of @total.'),
    'error_message' => t('Lift Profiles action sync has encountered an error.'),
    'file' => drupal_get_path('module', 'acquia_lift_profiles') . '/acquia_lift_profiles.batch.inc'
  );

  batch_set($batch);
}

/**
 * Batch API callback to sync an action to Lift web
 *
 * @param $item
 *   An array representing the action to sync with keys 'label' and 'action_name'.
 *
 * @param &$context
 *   Parameter passed in by Batch API to allow keeping track of operation
 *   progress.
 */
function acquia_lift_profiles_batch_sync_action($item, &$context) {
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
  }
  if (!isset($context['sandbox']['retries'])) {
    $context['sandbox']['retries'] = 0;
  }
  $errors = array();
  acquia_lift_profiles_put_action($item['action_name'], $item['label'], $errors);

  if (empty($errors)) {
    $context['results'][] = t('Action @action synchronized successfully', array('@action' => $item['label']));
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
function acquia_lift_profiles_batch_finished($success, $results, $operations) {
  if ($success) {
    // See if any of the results contains an error.
    $errors = array();
    foreach ($results as $result) {
      if (strpos($result, ACQUIA_LIFT_OPERATION_ERROR_PREFIX) === 0) {
        $errors[] = substr($result, strlen(ACQUIA_LIFT_OPERATION_ERROR_PREFIX));
      }
    }
    if (empty($errors)) {
      $message = t('Visitor Actions have been synchronized to Lift Web.');
      $message_type = 'status';
    }
    else {
      $message_type = 'error';
      $message = t('Some errors occurred while syncing actions to Acquia Lift Web:');
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
