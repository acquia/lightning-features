<?php
/**
 * @file
 * lightning_blocks.strongarm.inc
 */

/**
 * Implements hook_strongarm().
 */
function lightning_blocks_strongarm() {
  $export = array();

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bean_ctools_prefix';
  $strongarm->value = TRUE;
  $export['bean_ctools_prefix'] = $strongarm;

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'bean_ctools_separate';
  $strongarm->value = FALSE;
  $export['bean_ctools_separate'] = $strongarm;

  return $export;
}
