<?php
/**
 * @file
 * lightning_landing.features.user_permission.inc
 */

/**
 * Implements hook_defaultconfig_user_default_permissions().
 */
function lightning_landing_defaultconfig_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'administer panelizer node landing content'.
  $permissions['administer panelizer node landing content'] = array(
    'name' => 'administer panelizer node landing content',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'panelizer',
  );

  // Exported permission: 'administer panelizer node landing context'.
  $permissions['administer panelizer node landing context'] = array(
    'name' => 'administer panelizer node landing context',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'panelizer',
  );

  // Exported permission: 'administer panelizer node landing defaults'.
  $permissions['administer panelizer node landing defaults'] = array(
    'name' => 'administer panelizer node landing defaults',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'panelizer',
  );

  // Exported permission: 'administer panelizer node landing layout'.
  $permissions['administer panelizer node landing layout'] = array(
    'name' => 'administer panelizer node landing layout',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'panelizer',
  );

  // Exported permission: 'administer panelizer node landing overview'.
  $permissions['administer panelizer node landing overview'] = array(
    'name' => 'administer panelizer node landing overview',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'panelizer',
  );

  // Exported permission: 'administer panelizer node landing settings'.
  $permissions['administer panelizer node landing settings'] = array(
    'name' => 'administer panelizer node landing settings',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'panelizer',
  );

  // Exported permission: 'create landing content'.
  $permissions['create landing content'] = array(
    'name' => 'create landing content',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'node',
  );

  // Exported permission: 'delete any landing content'.
  $permissions['delete any landing content'] = array(
    'name' => 'delete any landing content',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'node',
  );

  // Exported permission: 'delete own landing content'.
  $permissions['delete own landing content'] = array(
    'name' => 'delete own landing content',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'node',
  );

  // Exported permission: 'edit any landing content'.
  $permissions['edit any landing content'] = array(
    'name' => 'edit any landing content',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'node',
  );

  // Exported permission: 'edit own landing content'.
  $permissions['edit own landing content'] = array(
    'name' => 'edit own landing content',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'node',
  );

  return $permissions;
}
