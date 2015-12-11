<?php
/**
 * @file
 * lightning_filter.features.filter.inc
 */

/**
 * Implements hook_filter_default_formats().
 */
function lightning_filter_filter_default_formats() {
  $formats = array();

  // Exported format: Filtered HTML.
  $formats['filtered_html'] = array(
    'format' => 'filtered_html',
    'name' => 'Filtered HTML',
    'cache' => 0,
    'status' => 1,
    'weight' => -10,
    'editor' => 'ckeditor',
    'editor_settings' => 'a:3:{s:7:"toolbar";a:1:{i:0;a:9:{i:0;a:2:{s:4:"name";s:6:"Format";s:5:"items";a:1:{i:0;s:6:"Format";}}i:1;a:2:{s:4:"name";s:4:"Text";s:5:"items";a:3:{i:0;s:4:"Bold";i:1;s:6:"Italic";i:2;s:9:"Underline";}}i:2;a:2:{s:4:"name";s:9:"Alignment";s:5:"items";a:3:{i:0;s:11:"JustifyLeft";i:1;s:13:"JustifyCenter";i:2;s:12:"JustifyRight";}}i:3;a:2:{s:4:"name";s:5:"Lists";s:5:"items";a:2:{i:0;s:12:"BulletedList";i:1;s:12:"NumberedList";}}i:4;a:2:{s:4:"name";s:5:"Links";s:5:"items";a:2:{i:0;s:10:"DrupalLink";i:1;s:12:"DrupalUnlink";}}i:5;a:2:{s:4:"name";s:10:"Formatting";s:5:"items";a:3:{i:0;s:12:"RemoveFormat";i:1;s:10:"Blockquote";i:2;s:5:"Table";}}i:6;a:2:{s:4:"name";s:5:"Media";s:5:"items";a:1:{i:0;s:4:"file";}}i:7;a:2:{s:4:"name";s:7:"Widgets";s:5:"items";a:2:{i:0;s:7:"leaflet";i:1;s:19:"foundationAccordion";}}i:8;a:2:{s:4:"name";s:5:"Tools";s:5:"items";a:2:{i:0;s:6:"Source";i:1;s:8:"Maximize";}}}}s:12:"image_upload";a:5:{s:6:"status";i:1;s:6:"scheme";s:6:"public";s:9:"directory";s:13:"inline-images";s:8:"max_size";s:0:"";s:14:"max_dimensions";a:2:{s:5:"width";s:0:"";s:6:"height";s:0:"";}}s:7:"plugins";a:1:{s:5:"style";a:1:{s:10:"style_list";a:0:{}}}}',
    'filters' => array(
      'filter_html' => array(
        'weight' => 0,
        'status' => 1,
        'settings' => array(
          'allowed_html' => '<a> <em> <strong> <cite> <blockquote> <code> <ul> <ol> <li> <dl> <dt> <dd> <h3> <h4> <h5> <p> <img> <figure> <figcaption> <drupal-entity> <div> <iframe> <span> <table> <thead> <tbody> <tr> <td> <th>',
          'filter_html_help' => 1,
          'filter_html_nofollow' => 0,
        ),
      ),
      'entity_embed' => array(
        'weight' => 1,
        'status' => 1,
        'settings' => array(),
      ),
      'editor_align' => array(
        'weight' => 2,
        'status' => 1,
        'settings' => array(),
      ),
      'editor_caption' => array(
        'weight' => 3,
        'status' => 1,
        'settings' => array(),
      ),
      'filter_htmlcorrector' => array(
        'weight' => 4,
        'status' => 1,
        'settings' => array(),
      ),
    ),
  );

  // Exported format: Full HTML.
  $formats['full_html'] = array(
    'format' => 'full_html',
    'name' => 'Full HTML',
    'cache' => 1,
    'status' => 1,
    'weight' => -9,
    'editor' => '',
    'editor_settings' => 'a:0:{}',
    'filters' => array(
      'filter_url' => array(
        'weight' => 0,
        'status' => 1,
        'settings' => array(
          'filter_url_length' => 72,
        ),
      ),
      'filter_autop' => array(
        'weight' => 1,
        'status' => 1,
        'settings' => array(),
      ),
      'filter_htmlcorrector' => array(
        'weight' => 10,
        'status' => 1,
        'settings' => array(),
      ),
    ),
  );

  // Exported format: Plain text.
  $formats['plain_text'] = array(
    'format' => 'plain_text',
    'name' => 'Plain text',
    'cache' => 1,
    'status' => 1,
    'weight' => -8,
    'editor' => '',
    'editor_settings' => 'a:0:{}',
    'filters' => array(
      'filter_html_escape' => array(
        'weight' => 0,
        'status' => 1,
        'settings' => array(),
      ),
      'filter_url' => array(
        'weight' => 1,
        'status' => 1,
        'settings' => array(
          'filter_url_length' => 72,
        ),
      ),
      'filter_autop' => array(
        'weight' => 2,
        'status' => 1,
        'settings' => array(),
      ),
    ),
  );

  // Exported format: User.
  $formats['userbase'] = array(
    'format' => 'userbase',
    'name' => 'User',
    'cache' => 1,
    'status' => 1,
    'weight' => -7,
    'editor' => 'ckeditor',
    'editor_settings' => 'a:3:{s:7:"toolbar";a:1:{i:0;a:5:{i:0;a:2:{s:4:"name";s:10:"Formatting";s:5:"items";a:2:{i:0;s:4:"Bold";i:1;s:6:"Italic";}}i:1;a:2:{s:4:"name";s:9:"Alignment";s:5:"items";a:3:{i:0;s:10:"Blockquote";i:1;s:12:"NumberedList";i:2;s:12:"BulletedList";}}i:2;a:2:{s:4:"name";s:5:"Lists";s:5:"items";a:0:{}}i:3;a:2:{s:4:"name";s:5:"Media";s:5:"items";a:0:{}}i:4;a:2:{s:4:"name";s:5:"Tools";s:5:"items";a:0:{}}}}s:12:"image_upload";a:5:{s:6:"status";i:0;s:6:"scheme";s:6:"public";s:9:"directory";s:13:"inline-images";s:8:"max_size";s:0:"";s:14:"max_dimensions";a:2:{s:5:"width";s:0:"";s:6:"height";s:0:"";}}s:7:"plugins";a:1:{s:5:"style";a:1:{s:10:"style_list";a:0:{}}}}',
    'filters' => array(
      'filter_html' => array(
        'weight' => -10,
        'status' => 1,
        'settings' => array(
          'allowed_html' => '<a> <em> <strong> <cite> <blockquote> <code> <ul> <ol> <li> <dl> <dt> <dd> <p> <br>',
          'filter_html_help' => 1,
          'filter_html_nofollow' => 1,
        ),
      ),
      'filter_autop' => array(
        'weight' => 0,
        'status' => 1,
        'settings' => array(),
      ),
      'filter_htmlcorrector' => array(
        'weight' => 10,
        'status' => 1,
        'settings' => array(),
      ),
    ),
  );

  return $formats;
}
