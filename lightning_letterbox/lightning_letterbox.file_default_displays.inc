<?php
/**
 * @file
 * lightning_letterbox.file_default_displays.inc
 */

/**
 * Implements hook_file_default_displays().
 */
function lightning_letterbox_file_default_displays() {
  $export = array();

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'video__default__file_picture';
  $file_display->weight = 0;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'picture_group' => 'letterbox',
    'fallback_image_style' => '',
    'alt' => '',
    'title' => '',
  );
  $export['video__default__file_picture'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'video__preview__file_picture';
  $file_display->weight = 0;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'picture_group' => 'letterbox',
    'fallback_image_style' => '',
    'alt' => '',
    'title' => '',
  );
  $export['video__preview__file_picture'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'video__teaser__file_picture';
  $file_display->weight = 0;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'picture_group' => 'letterbox',
    'fallback_image_style' => '',
    'alt' => '',
    'title' => '',
  );
  $export['video__teaser__file_picture'] = $file_display;

  return $export;
}
