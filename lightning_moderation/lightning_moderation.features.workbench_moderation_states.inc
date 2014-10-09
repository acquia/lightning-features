<?php
/**
 * @file
 * lightning_moderation.features.workbench_moderation_states.inc
 */

/**
 * Implements hook_workbench_moderation_export_states().
 */
function lightning_moderation_workbench_moderation_export_states() {
  $items = array(
    'draft' => array(
      'name' => 'draft',
      'label' => 'Draft',
      'description' => 'Save as Draft',
      'weight' => -10,
    ),
    'needs_review' => array(
      'name' => 'needs_review',
      'label' => 'Needs Review',
      'description' => 'Send for Review',
      'weight' => -9,
    ),
    'published' => array(
      'name' => 'published',
      'label' => 'Published',
      'description' => 'Publish',
      'weight' => -8,
    ),
  );
  return $items;
}
