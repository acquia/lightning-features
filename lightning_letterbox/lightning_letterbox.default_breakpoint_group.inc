<?php
/**
 * @file
 * lightning_letterbox.default_breakpoint_group.inc
 */

/**
 * Implements hook_default_breakpoint_group().
 */
function lightning_letterbox_default_breakpoint_group() {
  $export = array();

  $breakpoint_group = new stdClass();
  $breakpoint_group->disabled = FALSE; /* Edit this to true to make a default breakpoint_group disabled initially */
  $breakpoint_group->api_version = 1;
  $breakpoint_group->machine_name = 'letterbox';
  $breakpoint_group->name = 'Letterbox';
  $breakpoint_group->breakpoints = array(
    0 => 'custom.user.mobile',
    1 => 'custom.user.tablet',
    2 => 'custom.user.narrow',
    3 => 'breakpoints.module.navbar.wide',
  );
  $breakpoint_group->type = 'custom';
  $breakpoint_group->overridden = 0;
  $export['letterbox'] = $breakpoint_group;

  return $export;
}
