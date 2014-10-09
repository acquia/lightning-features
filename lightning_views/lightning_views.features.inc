<?php
/**
 * @file
 * lightning_views.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function lightning_views_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

