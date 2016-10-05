<?php
/**
 * @file
 * lightning_letterbox.default_picture_mapping.inc
 */

/**
 * Implements hook_default_picture_mapping().
 */
function lightning_letterbox_default_picture_mapping() {
  $export = array();

  $picture_mapping = new PictureMapping();
  $picture_mapping->disabled = FALSE; /* Edit this to true to make a default picture_mapping disabled initially */
  $picture_mapping->api_version = 2;
  $picture_mapping->label = 'Letterbox';
  $picture_mapping->machine_name = 'letterbox';
  $picture_mapping->breakpoint_group = 'letterbox';
  $picture_mapping->mapping = array(
    'custom.user.mobile' => array(
      '1x' => array(
        'mapping_type' => 'image_style',
        'image_style' => 'letterbox-mobile',
      ),
    ),
    'custom.user.tablet' => array(
      '1x' => array(
        'mapping_type' => 'image_style',
        'image_style' => 'letterbox-tablet',
      ),
    ),
    'custom.user.narrow' => array(
      '1x' => array(
        'mapping_type' => 'image_style',
        'image_style' => 'letterbox-narrow',
      ),
    ),
    'breakpoints.module.navbar.wide' => array(
      '1x' => array(
        'mapping_type' => 'image_style',
        'image_style' => 'letterbox-wide',
      ),
    ),
  );
  $export['letterbox'] = $picture_mapping;

  return $export;
}
