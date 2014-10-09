<?php
/**
 * @file
 * lightning_sps.features.field_base.inc
 */

/**
 * Implements hook_field_default_field_bases().
 */
function lightning_sps_field_default_field_bases() {
  $field_bases = array();

  // Exported field_base: 'field_collection'
  $field_bases['field_collection'] = array(
    'active' => 1,
    'cardinality' => 1,
    'deleted' => 0,
    'entity_types' => array(),
    'field_name' => 'field_collection',
    'field_permissions' => array(
      'type' => 0,
    ),
    'foreign keys' => array(),
    'indexes' => array(
      'collection' => array(
        0 => 'value',
      ),
    ),
    'locked' => 0,
    'module' => 'collection_field',
    'settings' => array(),
    'translatable' => 0,
    'type' => 'collection',
  );

  return $field_bases;
}
