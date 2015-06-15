<?php

/**
 * @file
 * Theme and processing functions for the Personalize module.
 */

/**
 * Themes a button that allows the text to be set independent of the value.
 */
function theme_personalize_html_tag($variables) {
  $element = $variables['element'];
  $attributes = isset($element['#attributes']) ? drupal_attributes($element['#attributes']) : '';
  $output = '<' . $element['#tag'] . $attributes . '>';
  $output .= $element['#text'];
  $output .= '</' . $element['#tag'] . ">\n";
  return $output;
}

/**
 * Theme wrapper for option sets.
 *
 * Wraps an option set in a script tag so that options are not parsed
 * by the browser on page load. Surrounds this with a div containing
 * all the information needed by our js.
 */
function theme_personalize_options_wrapper($variables) {
  // Check for Panels or Panelizer content admin pages.
  $path = current_path();
  $matches = array("admin/structure/pages", "panelizer");
  if (strrpos($path, "content") + strlen("content") === strlen($path)) {
    foreach ($matches as $match) {
      if (strpos($path, $match) !== false) {
        $render = array(
          '#type' => 'container',
          '#attributes' => array('class' => array('content-placeholder')),
          'content' => array(
            '#type' => 'markup',
            '#markup' => t('Placeholder for personalized content.'),
            '#weight' => 0,
          ),
        );
        return drupal_render($render);
      }
    }
  }

  if (!isset($variables['element']['#personalize_option_set'])) {
    return $variables['element']['#children'];
  }

  $option_set = $variables['element']['#personalize_option_set'];
  if (personalize_agent_get_status($option_set->agent) === PERSONALIZE_STATUS_COMPLETED) {
    if (!empty($option_set->winner)) {
      return drupal_render($variables['element']['#personalize_options'][$option_set->winner]);
    }
    elseif (isset($variables['element']['#first_option'])) {
      return drupal_render($variables['element']['#first_option']);
    }
    else {
      $first_option = reset($variables['element']['#personalize_options']);
      return drupal_render($first_option);
    }
  }
  $js_id = personalize_stringify_osid($option_set->osid);
  $prefix = '<div class="' . PERSONALIZE_OPTION_SET_CLASS . '" data-personalize="' . $js_id . '">';
  if (isset($variables['element']['#first_option'])) {
    $no_script_content = drupal_render($variables['element']['#first_option']);
    // Strip any noscript tags from the content so we don't end up with nested
    // noscript tags.
    $pattern = '/\<noscript\>(.*)\<\/noscript\>/';
    $no_script_content = preg_replace($pattern, '', $no_script_content);
    $prefix .= '<noscript>' . $no_script_content . '</noscript>';
  }

  $prefix .= '<script type="text/template" data-personalize-script="' . $js_id . '">';
  $suffix = '</script></div>';

  // Build array of rendered options for JSON encoding.
  $rendered = array();
  $i = 0;
  foreach($variables['element']['#personalize_options'] as $option_name => $option) {
    $rendered[$option_name] = array(
      'html' => drupal_render($option),
      'index' => $i,
    );
    $i++;
  }
  return $prefix . json_encode($rendered) . $suffix;
}

/**
 * Theme wrapper for option sets that are populated via callback.
 *
 * Provides a div to use with Ajax callback population.
 */
function theme_personalize_options_callback_wrapper($variables) {
  $option_set = $variables['element']['#personalize_option_set'];
  $js_id = personalize_stringify_osid($option_set->osid);
  $rendered = '<div class="' . PERSONALIZE_OPTION_SET_CLASS . '" data-personalize="' . $js_id . '"></div>';
  $rendered .= '<noscript>' . drupal_render($variables['element']['#first_option']) . '</noscript>';
  return $rendered;
}

/**
 * Theme wrapper for campaign status alert update message.
 *
 * @param $variables
 *   Associative array of information to display about the status alert.
 *   - agent_name: The machine name of the campaign that is being updated.
 *   - alert_message: An overall alert message to display.
 *   - option_message: A message to show to introduce the winning options.
 *   - option_sets: An associative array of options sets within this campaign.
 *     - label: The label of the option set
 *     - counter: A counter for the current option set
 *     - winner_label: The label of the winning variation for the option set.
 *     - winner_type: The type of winning variation (e.g., winner, control)
 *     - alert: Additional alert to show the end user regarding change.
 */
function theme_personalize_campaign_status_update($variables) {
  $option_message = $variables['option_message'];
  $option_sets = $variables['option_sets'];

  $update_message = empty($variables['alert_message']) ? '' : $variables['alert_message'];
  if (!empty($option_sets)) {
    if (!empty($option_message)) {
      $update_message .= ' ' . $option_message;
    }
    $winner_defaults = array(
      'label' => 'Content variation',
      'counter' => '',
      'winner_label' => t('Variation'),
      'winner_type' => t('Control'),
      'alert' => '',
    );
    $winners = array();
    foreach($option_sets as $option) {
      $option += $winner_defaults;
      $winner_message = "<strong>{$option['label']}:</strong> {$option['counter']} {$option['winner_label']} ({$option['winner_type']})";
      if (!empty($option['alert'])) {
        $winner_message .= ' <span class="personalize-alert">' . $option['alert'] . '</span>';
      }
      $winners[] = $winner_message;
    }
    $update_message .= theme('item_list', array('items' => $winners));
  }
  return $update_message;
}

/**
 * Theme function to display the title of an enumerated list.
 *
 * @param $variables
 *   Associative array of enumerated list item details
 *   - enum: The enumeration text; e.g., Set 1, Goal 2, etc.
 *   - title: The title of the list item.
 *   - title_prefix: An optional prefix for the title.
 *   - title_suffix: An optional suffix for the title.
 */
function theme_personalize_admin_enumerated_item($variables) {
  $variables['attributes']['class'][] = 'personalize-admin-enumerated-item';
  $text = '<div' . drupal_attributes($variables['attributes']) . '><span class="personalize-admin-enumerated-counter">' . $variables['enum'] . '</span> ';
  if (!empty($variables['title_prefix'])) {
    $text .= $variables['title_prefix'];
  }
  $text .= $variables['title'];
  if (!empty($variables['title_suffix'])) {
    $text .= $variables['title_suffix'];
  }
  $text .= '</div>';
  return $text;
}

/**
 * Theme function to output a report for an Option Set.
 *
 * @param $variables
 *   An associative array of variables with a single 'report' key that
 *   holds an array of reporting statistics.
 * @return string
 *   The themed output.
 */
function theme_personalize_option_set_report($variables) {
  if (empty($variables['report'])) {
    return '';
  }
  $output = '<div class="personalize-option-set-report">';
  foreach($variables['report'] as $stat) {
    $output .= '<span class="personalize-stat">' . $stat . '</span>';
  }
  $output .= '</div>';
  return $output;
}

/**
 * Theme function to output header information for an option set.
 *
 * @param $variables
 *   An associative array of variables.
 *   - variation_count: the number of variations within the option set.
 *   - report: an array of reporting statistics for this option set.
 *   - report_link: the link to the report for this option set.
 *   - edit_link: the link to edit the option set.
 *   - delete_link: the link to delete the option set.
 * @return string
 *   The themed output.
 */
function theme_personalize_option_set_header($variables) {
  $output = '<span class="personalize-option-header-item">';
  $output .= format_plural($variables['variation_count'], '1 variation', '@count variations');
  $output .= '</span>';
  if (!empty($variables['report'])) {
    $output .= theme('personalize_option_set_report', array('report' => $variables['report']));
  }
  if (!empty($variables['report_link'])) {
    $output .= '<span class="personalize-option-header-item">' . $variables['report_link'] . '</span>';
  }
  if (!empty($variables['edit_link'])) {
    $output .= '<span class="personalize-option-header-item">' . $variables['edit_link'] . '</span>';
  }
  if (!empty($variables['delete_link'])) {
    $output .= '<span class="personalize-option-header-item">' . $variables['delete_link'] . '</span>';
  }
  $output = '<span class="personalize-option-header">' . $output . '</span>';
  return $output;
}

/**
 * Theme wrapper for an administrative option that displays optional
 * details beyond the base description.
 *
 * @param $variables
 *   Associative array of variables for the theme.
 *   - details: The optional details that should only be shown when requested.
 */
function theme_personalize_admin_info_details($variables) {
  if (!empty($variables['details'])) {
    return '<div class="personalize-admin-optional">' . $variables['details'] . '</div>';
  }
  return '';
}

/**
 * Theme wrapper for administrative section content.
 */
function theme_personalize_admin_content($variables) {
  $element = $variables['element'];
  $attrs = drupal_attributes($element['#attributes']);
  $output = '<fieldset ' . $attrs . '>';

  $output .= '<legend><span class="personalize-fieldset-header"><span class="fieldset-legend personalize-admin-content-title">';
  $output .= $element['#title'] . '</span>';
  $output .= '</span></legend>';
  $output .= '<div class="fieldset-wrapper personalize-admin-content-content">' . $element['#children'] . '</div>';
  $output .= '</fieldset>';

  // Summary content that is shown beneath the title when collapsed.
  if (!empty($element['#summary'])) {
    $output .= '<div class="personalize-admin-content-collapsed">' . $element['#summary'] . '</div>';
  }

  // Wrap the fieldset content.
  $output = '<div class="personalize-admin-content personalize-collapsible">' . $output . '</div>';
  return $output;
}

/**
 * Implements hook_personalize_admin_section().
 */
function personalize_preprocess_personalize_admin_section(&$variables, $hook) {
  // Add general classes.
  $variables['classes_array'][] = 'clearfix';
}

/**
 * Implements hook_preprocess_personalize_content_set_item().
 */
function personalize_preprocess_personalize_content_set_item(&$variables, $hook) {
  // Move elements to the template variables.
  $variables['title'] = $variables['element']['#title'];

  // Add general classes.
  $variables['title_attributes_array']['class'][] = 'personalize-content-set-item-title';
  $variables['content_attributes_array']['class'][] = 'personalize-content-set-item-content';
}
