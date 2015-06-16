<?php
/**
 * @file
 * lightning_fpp.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function lightning_fpp_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "fieldable_panels_panes" && $api == "fieldable_panels_pane_type") {
    return array("version" => "1");
  }
}

/**
 * Implements hook_views_api().
 */
function lightning_fpp_views_api($module = NULL, $api = NULL) {
  return array("api" => "3.0");
}
