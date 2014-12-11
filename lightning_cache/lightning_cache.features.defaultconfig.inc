<?php
/**
 * @file
 * lightning_cache.features.defaultconfig.inc
 */

/**
 * Implements hook_defaultconfig_features().
 */
function lightning_cache_defaultconfig_features() {
  return array(
    'lightning_cache' => array(
      'strongarm' => 'strongarm',
    ),
  );
}

/**
 * Implements hook_defaultconfig_strongarm().
 */
function lightning_cache_defaultconfig_strongarm() {
  $export = array();

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'imageinfo_cache_disable_on_demand_generation';
  $strongarm->value = 0;
  $export['imageinfo_cache_disable_on_demand_generation'] = $strongarm;

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'imageinfo_cache_field_add_media';
  $strongarm->value = array(
    'large' => 'large',
    'letterbox-mobile' => 'letterbox-mobile',
    'letterbox-narrow' => 'letterbox-narrow',
    'letterbox-tablet' => 'letterbox-tablet',
    'letterbox-wide' => 'letterbox-wide',
    'linkit_thumb' => 'linkit_thumb',
    'media_thumbnail' => 'media_thumbnail',
    'medium' => 'medium',
    'thumbnail' => 'thumbnail',
  );
  $export['imageinfo_cache_field_add_media'] = $strongarm;

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'imageinfo_cache_field_image';
  $strongarm->value = array(
    'letterbox-mobile' => 'letterbox-mobile',
    'letterbox-narrow' => 'letterbox-narrow',
    'letterbox-tablet' => 'letterbox-tablet',
    'letterbox-wide' => 'letterbox-wide',
    'linkit_thumb' => 'linkit_thumb',
    'media_thumbnail' => 'media_thumbnail',
  );
  $export['imageinfo_cache_field_image'] = $strongarm;

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'imageinfo_cache_getimagesize';
  $strongarm->value = 0;
  $export['imageinfo_cache_getimagesize'] = $strongarm;

  $strongarm = new stdClass();
  $strongarm->disabled = FALSE; /* Edit this to true to make a default strongarm disabled initially */
  $strongarm->api_version = 1;
  $strongarm->name = 'imageinfo_cache_use_httprl';
  $strongarm->value = 1;
  $export['imageinfo_cache_use_httprl'] = $strongarm;

  return $export;
}
