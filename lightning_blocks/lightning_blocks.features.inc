<?php
/**
 * @file
 * lightning_blocks.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function lightning_blocks_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}

