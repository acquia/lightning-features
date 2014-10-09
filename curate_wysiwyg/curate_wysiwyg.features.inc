<?php
/**
 * @file
 * curate_wysiwyg.features.inc
 */

/**
 * Implements hook_ctools_plugin_api().
 */
function curate_wysiwyg_ctools_plugin_api($module = NULL, $api = NULL) {
  if ($module == "linkit" && $api == "linkit_profiles") {
    return array("version" => "1");
  }
}
