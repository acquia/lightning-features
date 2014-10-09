<?php
/**
 * @file
 * lightning_files.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function lightning_files_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "file_entity" && $api == "file_default_displays") {
    return array("version" => "1");
  }
  if ($module == "strongarm" && $api == "strongarm") {
    return array("version" => "1");
  }
}
