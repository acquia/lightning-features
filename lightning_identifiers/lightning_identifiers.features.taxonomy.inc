<?php
/**
 * @file
 * lightning_identifiers.features.taxonomy.inc
 */

/**
 * Implements hook_taxonomy_default_vocabularies().
 */
function lightning_identifiers_taxonomy_default_vocabularies() {
  return array(
    'identifiers' => array(
      'name' => 'Identifiers',
      'machine_name' => 'identifiers',
      'description' => 'Campaign identifiers used by marketers on Landing Pages.',
      'hierarchy' => 0,
      'module' => 'taxonomy',
      'weight' => 0,
    ),
  );
}
