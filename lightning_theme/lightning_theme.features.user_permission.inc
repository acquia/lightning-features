<?php
/**
 * @file
 * lightning_theme.features.user_permission.inc
 */

/**
 * Implements hook_defaultconfig_user_default_permissions().
 */
function lightning_theme_defaultconfig_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'view the administration theme'.
  $permissions['view the administration theme'] = array(
    'name' => 'view the administration theme',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'system',
  );

  return $permissions;
}
