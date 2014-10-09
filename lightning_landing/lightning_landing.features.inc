<?php
/**
 * @file
 * lightning_landing.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function lightning_landing_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "field_group" && $api == "field_group") {
    return array("version" => "1");
  }
  if ($module == "panelizer" && $api == "panelizer") {
    return array("version" => "1");
  }
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_node_info().
 */
function lightning_landing_node_info() {
  $items = array(
    'landing' => array(
      'name' => t('Landing Page'),
      'base' => 'node_content',
      'description' => t('Build <em>landing pages</em> to display content using custom layouts.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
