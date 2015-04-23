<?php
/**
 * @file
 * lightning_lift.features.user_permission.inc
 */

/**
 * Implements hook_user_default_permissions().
 */
function lightning_lift_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'manage personalized content'.
  $permissions['manage personalized content'] = array(
    'name' => 'manage personalized content',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'personalize',
  );

  return $permissions;
}
