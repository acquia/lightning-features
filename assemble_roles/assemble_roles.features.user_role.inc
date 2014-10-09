<?php
/**
 * @file
 * assemble_roles.features.user_role.inc
 */

/**
 * Implements hook_user_default_roles().
 */
function assemble_roles_user_default_roles() {
  $roles = array();

  // Exported role: marketer.
  $roles['marketer'] = array(
    'name' => 'marketer',
    'weight' => 5,
    'machine_name' => 'marketer',
  );

  return $roles;
}
