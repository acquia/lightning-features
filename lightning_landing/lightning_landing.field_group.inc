<?php
/**
 * @file
 * lightning_landing.field_group.inc
 */

/**
 * Implements hook_field_group_info().
 */
function lightning_landing_field_group_info() {
  $export = array();

  $field_group = new stdClass();
  $field_group->disabled = FALSE; /* Edit this to true to make a default field_group disabled initially */
  $field_group->api_version = 1;
  $field_group->identifier = 'group_landing_campaign|node|landing|form';
  $field_group->group_name = 'group_landing_campaign';
  $field_group->entity_type = 'node';
  $field_group->bundle = 'landing';
  $field_group->mode = 'form';
  $field_group->parent_name = '';
  $field_group->data = array(
    'label' => 'Campaign',
    'weight' => '1',
    'children' => array(
      0 => 'field_landing_identifiers',
      1 => 'field_landing_tags',
    ),
    'format_type' => 'tab',
    'format_settings' => array(
      'label' => 'Campaign',
      'instance_settings' => array(
        'required_fields' => 0,
        'classes' => 'group-landing-campaign field-group-tab ',
        'description' => '',
      ),
      'formatter' => 'closed',
    ),
  );
  $export['group_landing_campaign|node|landing|form'] = $field_group;

  return $export;
}
