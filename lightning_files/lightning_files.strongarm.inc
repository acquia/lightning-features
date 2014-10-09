<?php
/**
 * @file
 * lightning_files.strongarm.inc
 */

/**
 * Implements hook_strongarm().
 */
function lightning_files_strongarm() {
  $export = array();

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'file_entity_link_settings';
  $strongarm->value = array(
    'video' => array(
      'alt_preview' => 1,
      'default' => 0,
      'preview' => 0,
      'teaser' => 0,
    ),
  );
  $export['file_entity_link_settings'] = $strongarm;

  return $export;
}
