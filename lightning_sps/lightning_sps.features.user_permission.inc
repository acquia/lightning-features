<?php
/**
 * @file
 * lightning_sps.features.user_permission.inc
 */

/**
 * Implements hook_defaultconfig_user_default_permissions().
 */
function lightning_sps_defaultconfig_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'access collection list'.
  $permissions['access collection list'] = array(
    'name' => 'access collection list',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'collection',
  );

  // Exported permission: 'administer collections'.
  $permissions['administer collections'] = array(
    'name' => 'administer collections',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'collection',
  );

  // Exported permission: 'create collections'.
  $permissions['create collections'] = array(
    'name' => 'create collections',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'collection',
  );

  // Exported permission: 'delete collections'.
  $permissions['delete collections'] = array(
    'name' => 'delete collections',
    'roles' => array(
      'administrator' => 'administrator',
      'reviewer' => 'reviewer',
    ),
    'module' => 'collection',
  );

  // Exported permission: 'edit collections'.
  $permissions['edit collections'] = array(
    'name' => 'edit collections',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'collection',
  );

  // Exported permission: 'view collections'.
  $permissions['view collections'] = array(
    'name' => 'view collections',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'collection',
  );

  // Exported permission: 'view interactive information bar'.
  $permissions['view interactive information bar'] = array(
    'name' => 'view interactive information bar',
    'roles' => array(
      'administrator' => 'administrator',
      'curator' => 'curator',
    ),
    'module' => 'iib',
  );

  return $permissions;
}
