<?php
/**
 * @file
 * assemble_admin.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function assemble_admin_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'view ember support mods'.
  $permissions['view ember support mods'] = array(
    'name' => 'view ember support mods',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
      'marketer' => 'marketer',
      'reviewer' => 'reviewer',
    ),
    'module' => 'ember_support',
  );

  return $permissions;
}
