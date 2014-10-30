<?php
/**
 * @file
 * lightning_blocks.features.user_permission.inc
 */

/**
 * Implements hook_defaultconfig_user_default_permissions().
 */
function lightning_blocks_defaultconfig_user_default_permissions() {
  $permissions = array();

  // Exported permission: 'access bean overview'.
  $permissions['access bean overview'] = array(
    'name' => 'access bean overview',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'administer bean settings'.
  $permissions['administer bean settings'] = array(
    'name' => 'administer bean settings',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'administer bean types'.
  $permissions['administer bean types'] = array(
    'name' => 'administer bean types',
    'roles' => array(
      'administrator' => 'administrator',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'administer beans'.
  $permissions['administer beans'] = array(
    'name' => 'administer beans',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'administer blocks'.
  $permissions['administer blocks'] = array(
    'name' => 'administer blocks',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'block',
  );

  // Exported permission: 'create any bean_tax_listing bean'.
  $permissions['create any bean_tax_listing bean'] = array(
    'name' => 'create any bean_tax_listing bean',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'create any bean_tax_related bean'.
  $permissions['create any bean_tax_related bean'] = array(
    'name' => 'create any bean_tax_related bean',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'delete any bean_tax_listing bean'.
  $permissions['delete any bean_tax_listing bean'] = array(
    'name' => 'delete any bean_tax_listing bean',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'delete any bean_tax_related bean'.
  $permissions['delete any bean_tax_related bean'] = array(
    'name' => 'delete any bean_tax_related bean',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'edit any bean_tax_listing bean'.
  $permissions['edit any bean_tax_listing bean'] = array(
    'name' => 'edit any bean_tax_listing bean',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'edit any bean_tax_related bean'.
  $permissions['edit any bean_tax_related bean'] = array(
    'name' => 'edit any bean_tax_related bean',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'edit bean view mode'.
  $permissions['edit bean view mode'] = array(
    'name' => 'edit bean view mode',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'view any bean_tax_listing bean'.
  $permissions['view any bean_tax_listing bean'] = array(
    'name' => 'view any bean_tax_listing bean',
    'roles' => array(
      'administrator' => 'administrator',
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'view any bean_tax_related bean'.
  $permissions['view any bean_tax_related bean'] = array(
    'name' => 'view any bean_tax_related bean',
    'roles' => array(
      'administrator' => 'administrator',
      'anonymous user' => 'anonymous user',
      'authenticated user' => 'authenticated user',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'view bean page'.
  $permissions['view bean page'] = array(
    'name' => 'view bean page',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  // Exported permission: 'view bean revisions'.
  $permissions['view bean revisions'] = array(
    'name' => 'view bean revisions',
    'roles' => array(
      'administrator' => 'administrator',
      'marketer' => 'marketer',
    ),
    'module' => 'bean',
  );

  return $permissions;
}
