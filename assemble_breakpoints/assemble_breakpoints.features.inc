<?php
/**
 * @file
 * assemble_breakpoints.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function assemble_breakpoints_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "breakpoints" && $api == "default_breakpoints") {
    return array("version" => "1");
  }
}

