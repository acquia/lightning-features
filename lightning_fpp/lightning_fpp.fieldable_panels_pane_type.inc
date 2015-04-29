<?php
/**
 * @file
 * lightning_fpp.fieldable_panels_pane_type.inc
 */

/**
 * Implements hook_default_fieldable_panels_pane_type().
 */
function lightning_fpp_default_fieldable_panels_pane_type() {
  $export = array();

  $fieldable_panels_pane_type = new stdClass();
  $fieldable_panels_pane_type->disabled = FALSE; /* Edit this to true to make a default fieldable_panels_pane_type disabled initially */
  $fieldable_panels_pane_type->api_version = 1;
  $fieldable_panels_pane_type->name = 'content';
  $fieldable_panels_pane_type->title = 'Content';
  $fieldable_panels_pane_type->description = 'A widget that inserts one or more pieces of content embeded in order of list weight.';
  $export['content'] = $fieldable_panels_pane_type;

  $fieldable_panels_pane_type = new stdClass();
  $fieldable_panels_pane_type->disabled = FALSE; /* Edit this to true to make a default fieldable_panels_pane_type disabled initially */
  $fieldable_panels_pane_type->api_version = 1;
  $fieldable_panels_pane_type->name = 'map';
  $fieldable_panels_pane_type->title = 'Map';
  $fieldable_panels_pane_type->description = 'A widget that easily allows insertion of a Google Map embed based on an address.';
  $export['map'] = $fieldable_panels_pane_type;

  $fieldable_panels_pane_type = new stdClass();
  $fieldable_panels_pane_type->disabled = FALSE; /* Edit this to true to make a default fieldable_panels_pane_type disabled initially */
  $fieldable_panels_pane_type->api_version = 1;
  $fieldable_panels_pane_type->name = 'media_pane';
  $fieldable_panels_pane_type->title = 'Media';
  $fieldable_panels_pane_type->description = 'Media widget allowing image, video and audio embedding.';
  $export['media_pane'] = $fieldable_panels_pane_type;

  $fieldable_panels_pane_type = new stdClass();
  $fieldable_panels_pane_type->disabled = FALSE; /* Edit this to true to make a default fieldable_panels_pane_type disabled initially */
  $fieldable_panels_pane_type->api_version = 1;
  $fieldable_panels_pane_type->name = 'quick_links';
  $fieldable_panels_pane_type->title = 'Links';
  $fieldable_panels_pane_type->description = 'A quick links widget allowing simple list of links to be added.';
  $export['quick_links'] = $fieldable_panels_pane_type;

  $fieldable_panels_pane_type = new stdClass();
  $fieldable_panels_pane_type->disabled = FALSE; /* Edit this to true to make a default fieldable_panels_pane_type disabled initially */
  $fieldable_panels_pane_type->api_version = 1;
  $fieldable_panels_pane_type->name = 'raw_html';
  $fieldable_panels_pane_type->title = 'HTML';
  $fieldable_panels_pane_type->description = 'Smart HTML widget allowing snippets from external resources to be added.';
  $export['raw_html'] = $fieldable_panels_pane_type;

  $fieldable_panels_pane_type = new stdClass();
  $fieldable_panels_pane_type->disabled = FALSE; /* Edit this to true to make a default fieldable_panels_pane_type disabled initially */
  $fieldable_panels_pane_type->api_version = 1;
  $fieldable_panels_pane_type->name = 'text';
  $fieldable_panels_pane_type->title = 'Text';
  $fieldable_panels_pane_type->description = 'The filtered text widget provides a WYSIWYG text editor for adding content.';
  $export['text'] = $fieldable_panels_pane_type;

  return $export;
}
