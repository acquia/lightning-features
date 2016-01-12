<?php
/**
 * @file
 * lightning_identifiers.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function lightning_identifiers_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}
