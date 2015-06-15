<?php
/**
 * @file acquia_lift.admin.page_variations.inc
 *
 * API to handle the administration of page variations within an agent.
 */

/**
 * Creates an option set option for a page variation based on the passed in option info.
 *
 * @param $variation_set_name
 *   The variation set this belongs to.
 * @param $option_set
 *   The option set object to add the option to
 * @param $option_info
 *   The information about the individual option to be added for this variation.
 * @param $variation_number
 *   The variation number to add this option to. If unspecified it will be created as a
 *   new variation.
 * @return number
 *   The variation number that was created or added to.
 */
function acquia_lift_page_variation_create($variation_set_name, $option_set, $option_info, $variation_number = NULL) {
  // Find all other option sets in this variation set (i.e. with the same
  // decision name and agent.)
  $option_sets = personalize_option_set_load_multiple(FALSE, array('decision_name' => $variation_set_name, 'agent' => $option_set->agent));

  // To figure out the variation number, check the number of options in the first
  // existing option set and the number of options in the passed in option set and
  // use whichever is higher.
  if (!empty($option_sets)) {
    $first_os = reset($option_sets);
    $max_first = max(array_keys($first_os->options));
  }
  else {
    $max_first = 0;
  }
  // The maximum new variation number must take into account the number of
  // options for both the passed in option set and the original first option set
  // as well as the indexes on those options which may no longer be
  // sequential.
  $max_variation_number  = max(array_merge(array(0, $max_first), array_keys($option_set->options)));
  // The above would give us the maximum existing number so increment for new.
  $max_variation_number++;
  if ($variation_number === NULL || $variation_number > $max_variation_number) {
    $variation_number = $max_variation_number;
  }

  $option_set->decision_name = $variation_set_name;
  $has_control_option = FALSE;
  $new_option_id = 'variation-' . $variation_number;
  $new_option_label = t('Variation #@num', array('@num' => $variation_number));
  $i = 0;
  foreach ($option_set->options as $i => $option) {
    // Check for the control variation.
    if ($option['option_id'] == PERSONALIZE_CONTROL_OPTION_ID) {
      if ($i !== 0) {
        // The control option has to be index 0.
        unset($option_set->options[$i]);
      }
      else {
        $has_control_option = TRUE;
      }
    }
  }
  $next_option = $i + 1;
  // Add any missing options before adding the one for this variation.
  for ($j = $next_option; $j < $variation_number; $j++) {
    $option_label = empty($first_os) || !isset($first_os->options[$j]) ? t('Variation #@num', array('@num' => $j)) : $first_os->options[$j]['option_label'];
    $option_set->options[$j] = personalize_elements_get_option_for_variation('variation-' . $j, $option_label);
  }
  // Now add the option for this variation.
  $option_set->options[$variation_number] = array('option_id' => $new_option_id, 'option_label' => $new_option_label) + $option_info;
  $control_option = personalize_elements_get_option_for_variation(PERSONALIZE_CONTROL_OPTION_ID, PERSONALIZE_CONTROL_OPTION_LABEL);
  if (!$has_control_option) {
    array_unshift($option_set->options, $control_option);
  }
  // If the variation set contains variations with higher numbers than this, make
  // sure options exist in this option set for those variation numbers.
  for ($k = $variation_number + 1; $k < $max_variation_number; $k++) {
    if (!isset($option_set->options[$k])) {
      $option_label = empty($first_os) || !isset($first_os->options[$k]) ? t('Variation #@num', array('@num' => $k)) : $first_os->options[$k]['option_label'];
      $option_set->options[$k] = personalize_elements_get_option_for_variation('variation-' . $k, $option_label);
    }
  }

  // Save the option set, specifying *not* to enforce decision matching as we know
  // the options will not match at this point.
  $option_set = personalize_option_set_save($option_set, FALSE);
  // Go through the other option sets in the variation set to update their options.
  foreach ($option_sets as $osid => $os) {
    if (isset($option_set->osid) && $osid == $option_set->osid) {
      continue;
    }
    // Make sure we have a control option.
    if (!isset($os->options[0])) {
      $os->options[0] = array();
    }
    $os->options[0] = array_merge($os->options[0], $control_option);
    // Add empty options for any missing indexes up to and including the variation number.
    for ($i = 1; $i <= $variation_number; $i++) {
      if (!isset($os->options[$i])) {
        $op_id = 'variation-' . $i;
        $op_label = empty($first_os) || !isset($first_os->options[$i]) ? t('Variation #@num', array('@num' => $i)) : $first_os->options[$i]['option_label'];
        $os->options[$i] = personalize_elements_get_option_for_variation($op_id, $op_label);
      }
    }
    personalize_option_set_save($os, FALSE);
  }
  return $variation_number;
}

/**
 * Rename a page variation.
 *
 * To rename a page variation, re-label all options that match the variation.
 *
 * @param string $variation_set_name
 *   The name of the variation set that this variation belongs to.
 * @param string $agent_name
 *   The name of the campaign/agent for this page variation.
 * @param number $variation_number
 *   The number of variation within the variation set to rename
 * @param string $variation_name
 *   The new variation name.
 */
function acquia_lift_page_variation_rename($variation_set_name, $agent_name, $variation_number, $variation_name) {
  // Cannot rename the control variation.
  if ($variation_number == 0) {
    return;
  }
  $option_sets = personalize_option_set_load_multiple(FALSE, array('decision_name' => $variation_set_name, 'agent' => $agent_name));
  foreach ($option_sets as $option_set) {
    if (isset($option_set->options[$variation_number])) {
      $option_set->options[$variation_number]['option_label'] = $variation_name;
      personalize_option_set_save($option_set);
    }
  }
}

/**
 * Gets the name of a specific page variation set variation.
 *
 * @param string $variation_set_name
 *   The name of the variation set that this variation belongs to.
 * @param string $agent_name
 *   The name of the campaign/agent for this page variation.
 * @param number $variation_number
 *   The number of variation within the variation set to retrieve.
 */
function acquia_lift_page_variation_get_name($variation_set_name, $agent_name, $variation_number) {
  $option_sets = personalize_option_set_load_multiple(FALSE, array('decision_name' => $variation_set_name, 'agent' => $agent_name));
  $option_set = current($option_sets);
  if ($option_set) {
    if (isset($option_set->options[$variation_number])) {
      return filter_xss($option_set->options[$variation_number]['option_label']);
    }
  }
  return '';
}

/**
 * Gets the page for the variations within a variation set.
 *
 * Assumption: All page variations are on the same path for a single variation set.
 */
function acquia_lift_page_variation_get_path($variation_set_name, $agent_name) {
  $option_sets = personalize_option_set_load_multiple(FALSE, array('decision_name' => $variation_set_name, 'agent' => $agent_name));
  $option_set = current($option_sets);
  if ($option_set && isset($option_set->data)) {
    return isset($option_set->data['pages']) ? $option_set->data['pages'] : '';
  }
  return '';
}

/**
 * Delete a page variation.
 *
 * @param string $variation_set_name
 *   The name of the variation set that contains the variation to be removed.
 * @param string $agent_name
 *   The name of the campaign/agent for the page variation.
 * @param number $variation_number
 *   The number for the variation to delete.
 */
function acquia_lift_page_variation_delete($variation_set_name, $agent_name, $variation_number) {
  $option_sets = personalize_option_set_load_multiple(FALSE, array('decision_name' => $variation_set_name, 'agent' => $agent_name));
  foreach ($option_sets as $option_set) {
    if (isset($option_set->options[$variation_number])) {
      unset($option_set->options[$variation_number]);
      personalize_option_set_save($option_set, FALSE);
    }
  }
  // If the only variation left is the control variation, then delete
  // all of the option sets for this variation.
  if (count($option_set->options) === 1) {
    $last_option = current($option_set->options);
    if ($last_option['option_id'] === PERSONALIZE_CONTROL_OPTION_ID) {
      reset($option_sets);
      foreach ($option_sets as $option_set) {
        personalize_option_set_delete($option_set->osid);
      }
    }
  }
}

