<?php
/**
 * @file
 * lightning_fpp.features.filter.inc
 */

/**
 * Implements hook_filter_default_formats().
 */
function lightning_fpp_filter_default_formats() {
  $formats = array();

  // Exported format: HTML.
  $formats['raw_html'] = array(
    'format' => 'raw_html',
    'name' => 'HTML',
    'cache' => 1,
    'status' => 1,
    'weight' => -6,
    'filters' => array(
      'filter_htmlcorrector' => array(
        'weight' => 10,
        'status' => 1,
        'settings' => array(),
      ),
    ),
  );

  return $formats;
}
