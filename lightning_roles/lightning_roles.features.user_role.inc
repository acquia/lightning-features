<?php
/**
 * @file
 * lightning_roles.features.user_role.inc
 */

/**
 * Implements hook_user_default_roles().
 */
function lightning_roles_user_default_roles() {
  $roles = array();

  // Exported role: curator.
  $roles['curator'] = array(
    'name' => 'curator',
    'weight' => 3,
    'machine_name' => 'curator',
  );

  // Exported role: reviewer.
  $roles['reviewer'] = array(
    'name' => 'reviewer',
    'weight' => 4,
    'machine_name' => 'reviewer',
  );

  // Exported role: marketer.
  $roles['marketer'] = array(
    'name' => 'marketer',
    'weight' => 5,
    'machine_name' => 'marketer',
  );

  return $roles;
}
