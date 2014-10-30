<?php
/**
 * @file
 * lightning_edit.features.user_permission.inc
 */

/**
 * Implements hook_defaultconfig_user_default_permissions().
 */
function lightning_edit_defaultconfig_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'access in-place editing'.
  $permissions['access in-place editing'] = array(
    'name' => 'access in-place editing',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'quickedit',
  );

  return $permissions;
}
