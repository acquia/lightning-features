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

/**
 * Implements hook_image_default_styles().
 */
function lightning_media_image_default_styles() {
  $styles = array();

  // Exported image style: media_browser.
  $styles['media_browser'] = array(
    'effects' => array(
      array(
        'name' => 'image_scale_and_crop',
        'data' => array('width' => 300, 'height' => 300),
        'weight' => 0,
      ),
    ),
    'label' => 'Media browser',
  );

  return $styles;
}
