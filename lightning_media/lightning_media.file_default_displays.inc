<?php
/**
 * @file
 * lightning_media.file_default_displays.inc
 */

/**
 * Implements hook_file_default_displays().
 */
function lightning_media_file_default_displays() {
  $file_displays = array();

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'audio__preview__media_oembed_thumbnail';
  $file_display->weight = 0;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_thumbnail',
  );
  $file_displays['audio__preview__media_oembed_thumbnail'] = $file_display;

  // Audio previews should be displayed using a large filetype icon.
  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'audio__media_browser__file_field_media_large_icon';
  $file_display->weight = 49;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_browser',
  );
  $file_displays['audio__media_browser__file_field_media_large_icon'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'audio__media_browser__file_field_file_default';
  $file_display->weight = 50;
  $file_display->status = TRUE;
  $file_display->settings = '';
  $file_displays['audio__media_browser__file_field_file_default'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'document__preview__media_oembed_thumbnail';
  $file_display->weight = 0;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_thumbnail',
  );
  $file_displays['document__preview__media_oembed_thumbnail'] = $file_display;

  // Document previews should be displayed using a large filetype icon.
  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'document__media_browser__file_field_media_large_icon';
  $file_display->weight = 49;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_browser',
  );
  $file_displays['document__media_browser__file_field_media_large_icon'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'document__media_browser__file_field_file_default';
  $file_display->weight = 50;
  $file_display->status = TRUE;
  $file_display->settings = '';
  $file_displays['document__media_browser__file_field_file_default'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'image__preview__media_oembed_thumbnail';
  $file_display->weight = 0;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_thumbnail',
  );
  $file_displays['image__preview__media_oembed_thumbnail'] = $file_display;

  // Image previews should be displayed as image thumbnails by default.
  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'image__media_browser__file_field_image';
  $file_display->weight = 48;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_browser',
    'image_link' => '',
  );
  $file_displays['image__media_browser__file_field_image'] = $file_display;

  // Image previews should be displayed using a large filetype icon.
  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'image__media_browser__file_field_media_large_icon';
  $file_display->weight = 49;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_browser',
  );
  $file_displays['image__media_browser__file_field_media_large_icon'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'image__media_browser__file_field_file_default';
  $file_display->weight = 50;
  $file_display->status = TRUE;
  $file_display->settings = '';
  $file_displays['image__media_browser__file_field_file_default'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'video__preview__media_oembed_thumbnail';
  $file_display->weight = 0;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_thumbnail',
  );
  $file_displays['video__preview__media_oembed_thumbnail'] = $file_display;

  // Video previews should be displayed using a large filetype icon.
  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'video__media_browser__file_field_media_large_icon';
  $file_display->weight = 49;
  $file_display->status = TRUE;
  $file_display->settings = array(
    'image_style' => 'media_browser',
  );
  $file_displays['video__media_browser__file_field_media_large_icon'] = $file_display;

  $file_display = new stdClass();
  $file_display->api_version = 1;
  $file_display->name = 'video__media_browser__file_field_file_default';
  $file_display->weight = 50;
  $file_display->status = TRUE;
  $file_display->settings = '';
  $file_displays['video__media_browser__file_field_file_default'] = $file_display;

  return $file_displays;
}
