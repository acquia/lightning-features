<?php

/**
 * @file
 * Drush hook implementations for the Demo Framework Administation Screens
 * module.
 */

/**
 * Implements hook_drush_command().
 */
function lightning_admin_drush_command() {
  $items = array();

  $items['lightning-enable'] = array(
    'description' => 'Enables the Lightning feature',
    'arguments' => array(
      'module' => 'The name of the features module',
    ),
    'aliases' => array('le'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  $items['lightning-disable'] = array(
    'description' => 'Disables the Lightning feature',
    'arguments' => array(
      'module' => 'The name of the features module',
    ),
    'aliases' => array('ld'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL,
  );

  return $items;
}

/**
 * Enables a Lightning Feature.
 *
 * @param string $module
 *   The machine name of the feature.
 */
function drush_lightning_admin_lightning_enable($module) {
  // Check if module is not enabled.
  if (module_exists($module)) {
    drupal_set_message(t('@module is already enabled.', array('@module' => $module)), 'error');
    return;
  }
  else {
    // Enable the module.
    if (module_enable(array($module))) {
      drupal_set_message(t('@module enabled.', array('@module' => $module)));
      // Clear Drupal cache.
      drupal_flush_all_caches();
    }
  }

  // Revert the module's Features components.
  if (($feature = feature_load($module, TRUE)) && module_exists($module)) {
    // Clear & rebuild Features key caches.
    features_get_info(NULL, NULL, TRUE);
    features_rebuild();
    // Revert the feature (cleanup install).
    $components = array();
    foreach (array_keys($feature->info['features']) as $component) {
      if (features_hook($component, 'features_revert')) {
        $components[] = $component;
      }
    }
    foreach ($components as $component) {
      features_revert(array($module => array($component)));
    }
  }
}

/**
 * Disables a Lightning Feature.
 *
 * @param string $module
 *   The machine name of the feature.
 */
function drush_lightning_admin_lightning_disable($module) {
  // Check if module is enabled.
  if (!module_exists($module)) {
    drupal_set_message(t('@module cannot be disabled as is not currently enabled.', array('@module' => $module)), 'error');
    return;
  }
  // Disable the module.
  module_disable(array($module), FALSE);

  // Uninstall the module.
  drupal_uninstall_modules(array($module));

  // Return status.
  drupal_set_message(t('@module disabled and uninstalled.', array('@module' => $module)));
}

