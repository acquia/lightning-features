<?php
/**
 * @file
 * lightning_media.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function lightning_media_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "file_entity" && $api == "file_default_displays") {
    return array("version" => "1");
  }
}
