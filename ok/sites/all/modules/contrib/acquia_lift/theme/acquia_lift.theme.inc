<?php

/**
 * @file acquia_lift.theme.inc
 * Provides theme functions for Acquia Lift.
 */


/**
 * Returns HTML for a personalized form element.
 *
 * @param $variables
 *   An associative array containing:
 *   - element: A render element representing the form element.
 *
 * @ingroup themeable
 */
function theme_acquia_lift_personalizable_field_form($variables) {

  $element = $variables['element'];

  // Sort items according to '_weight' (needed when the form comes back after
  // preview or failed validation).
  $items = array();
  foreach (element_children($element) as $key) {
    if ($key === 'add_more') {
      $add_more_button = &$element[$key];
      // Convert the form-submit to a button so that it is more styleable with // css. If markup changes are still needed by a theme, allow for a more
      // specific theme_html_tag__personalize_button() override.
      $add_more_button['#theme'] = 'html_tag__personalize_button';
      $add_more_button['#tag'] = 'button';
      $add_more_button['#attributes']['id'] = $add_more_button['#id'];
      $add_more_button['#attributes']['class'][] = 'form-submit';
      $add_more_button['#attributes']['class'][] = 'personalize-field-add';
      $add_more_button['#attributes']['title'] = t('Add an option');
      $add_more_button['#value'] = t('Add an option');
      unset($add_more_button['#theme_wrappers']);
    }
    else {
      $element[$key]['_weight']['#theme_wrappers'][] = 'acquia_lift_personalize_field_weight_field_wrapper';
      $items[] = &$element[$key];
    }
  }
  usort($items, '_field_sort_items_value_helper');

  $rendered_items = '<div class="personalize-field-options">';

  foreach ($items as $key => $item) {
    $class = 'personalize-field' . ($key == 0 ? ' personalize-field-visible' : '');
    $rendered_items .= '<div class="' . $class . '">' . drupal_render($item) . '</div>';
  }
  $rendered_items .= '</div>';

  $children = drupal_render($add_more_button);
  $children .= $rendered_items;

  $output = '<div class="form-item">';

  // If #title is not set, we don't display any label or required marker.
  if (!isset($element['#title'])) {
    $element['#title_display'] = 'none';
  }
  $prefix = isset($element['#field_prefix']) ? '<span class="field-prefix">' . $element['#prefix'] . '</span> ' : '';
  $suffix = isset($element['#field_suffix']) ? ' <span class="field-suffix">' . $element['#suffix'] . '</span>' : '';

  switch ($element['#title_display']) {
    case 'before':
    case 'invisible':
      $output .= ' ' . theme('form_element_label', $variables);
      $output .= ' ' . $prefix . $children . $suffix . "\n";
      break;

    case 'after':
      $output .= ' ' . $prefix . $children . $suffix;
      $output .= ' ' . theme('form_element_label', $variables) . "\n";
      break;

    case 'none':
    case 'attribute':
      // Output no label and no required marker, only the children.
      $output .= ' ' . $prefix . $children . $suffix . "\n";
      break;
  }

  $output .= $element['#description'] ? '<div class="description">' . $element['#description'] . '</div>' : '';
  $output .= '</div>';

  return $output;
}

/**
 * Theme wrapper for the personalize option weight field.
 */
function theme_acquia_lift_personalize_field_weight_field_wrapper($variables) {
  $attributes = array(
    'class' => array('element-invisible'),
    'aria-hidden' => 'true',
  );
  return '<div'. drupal_attributes($attributes) . '>' . $variables['element']['#children'] . '</div>';
}

/**
 * Theme function for the in-context personalization controls.
 */
function theme_acquia_lift_edit_mode_personalize_in_context_links($variables) {
  $elements = $variables['elements'];
  $launch = drupal_render($elements['launch']);
  $cancel = drupal_render($elements['cancel']);
  return '<div' . drupal_attributes($elements['#attributes']) . '>' . $launch . ' ' . t('or') . ' ' . $cancel . '</div>';
}

/**
 * Theme function to output links for filtering by feature,
 *
 * @ingroup themeable
 */
function theme_acquia_lift_feature_filter_links($variables) {
  $element = $variables['element'];
  $output = '<div>Filter by feature: ';

  $rendered = array();
  foreach ($element['links'] as $link) {
    $rendered[] = drupal_render($link);
  }
  $output .= implode (' | ', $rendered);

  $output .= '</div>';
  return $output;
}

/**
 * Theme function to output a control for entering percentage values.
 *
 * @ingroup themeable
 */
function theme_acquia_lift_percentage($variables) {
  $element = $variables['element'];

  // Add classes for percentage control and collapsible details.
  $element['#attributes']['class'][] = 'acquia-lift-percentage-control';
  if (isset($element['#collapsible']) && $element['#collapsible']) {
    $element['#attributes']['class'][] = 'acquia-lift-collapsible';
    if (isset($element['#collapsed']) && $element['#collapsed']) {
      $element['#attributes']['class'][] = 'acquia-lift-collapsed';
    }
  }
  // Output a wrapping div.
  $output = '<div ' . drupal_attributes($element['#attributes']) . '>';

  // Render the initial control as a textfield within a form element.
  if (!isset($element['#autocomplete_path'])) {
    $element['#autocomplete_path'] = '';
  }
  $textfield = theme('textfield', array('element' => $element));
  $element['#children'] = $textfield;
  $output .= theme('form_element', array('element' => $element));

  // Add the "rest" percentage indicator if metadata available.
  if (isset($element['#rest_title'])) {
    $output .= '<div class="form-item acquia-lift-percentage-rest">';
    $output .= '<span class="acquia-lift-percentage-rest-label">' . $element['#rest_title'] . '</span>';
    $output .= '<span class="acquia-lift-percentage-rest-display"></span>';
    if (isset($element['#rest_description'])) {
      $output .= '<div class="description">' . $element['#rest_description'] . '</div>';
    }
    $output .= '</div>';
  }
  // Close the wrapping div.
  $output .= '</div>';
  return $output;
}

/**
 * Theme function to show a percentage difference as a label.
 *
 * The percentage is shown as a line filled to the set percentage with optional
 * labels for the percentage indicated (and rest of percentage).
 *
 * @param $variables
 *   The theme variables that will include:
 *   - percent_label: The label for the main percentage selected
 *   - rest_label: The label for the rest of the percentage selected
 *   - percent: The required percentage selected as a float.
 */
function theme_acquia_lift_percentage_label($variables) {
  $percent = round($variables['percent'], 2);
  $rest_percent = 100 - $percent;

  $output = '<div class="acquia-lift-distribution form-item">';
  if (!empty($variables['percent_label'])) {
    $output .= '<div class="distribution-set">';
    $output .= $variables['percent_label'] . ' (' . $variables['percent'] . '%)';
    $output .= '</div>';
  }
  if (!empty($variables['rest_label'])) {
    $output .= '<div class="distribution-rest">';
    $output .= $variables['rest_label'] . ' (' . $rest_percent . '%)';
    $output .= '</div>';
  }
  $output .= '<div class="distribution-graphic">';
  $output .= '<div class="fill" data-acquia-lift-fill="' . $percent . '"></div>';
  $output .= '</div>';
  $output .= '</div>';
  return $output;
}

/**
 * Theme function for generating the mark-up to display a high-low value.
 *
 * @param $variables
 *   The theme variables that will include
 *   - value: The mean value to display
 *   - high: The high value to display
 *   - low: The low value to display
 */
function theme_acquia_lift_high_low($variables) {
  $high = is_numeric($variables['high']) ? $variables['high'] : 0;
  $low = is_numeric($variables['low']) ? $variables['low'] : 0;
  $title = t('It is predicted that the goal value using this option will be between @low and @high.', array(
    '@low' => $low,
    '@high' => $high,
  ));
  $output = '<div class="acquia-lift-hilo-estimate" title="' . $title . '">';
  $output .= '<div class="acquia-lift-hilo-background">';
  $output .= '<div class="acquia-lift-hilo-bounds" data-acquia-lift-high="' . $high . '" data-acquia-lift-low="' . $low . '">';
  $output .= '<div class="acquia-lift-estimate">';
  $output .= '<span class="acquia-lift-badge">' . $variables['value'] . '</span>';
  $output .= '</div></div></div></div>';
  return $output;
}

/**
 * @param $variables
 *   An associative array of variables including:
 *   - goal_total: The total goal value
 * @return the rendered output
 */
function theme_acquia_lift_goal_total($variables) {
  $output = '<div class="acquia-lift-experiment-goal-total-wrapper">';
  $output .= t('Total possible goal value: ') . '<span class="acquia-lift-experiment-goal-total-value">' . $variables['goal_total'] . '</span>';
  $output .= '</div>';
  return $output;
}

/**
 * Theme function for formatting a report overview item
 */
function theme_acquia_lift_report_overview($variables) {
  $element = $variables['element'];
  $output = '<div class="acquia-lift-overview-item">' . $element['#title'] . '</div>';
  if (!empty($element['#description'])) {
    $output .= '<div class="acquia-lift-overview-caption">' . $element['#description'] . '</div>';
  }
  return $output;
}

/**
 * Theme function for a type selection list.
 *
 * This is used by the campaign A/B flow to allow a wizard-like selection of
 * campaign types and goal types.
 *
 * @param $variables
 *   An associative array of variables including:
 *   - items: An array of types with the following keys
 *     - type:  What to display for the type
 *     - description: What to display as the description of the type
 *     - url: The url to create a new item of that type.
 *     - logo: Logo to denote the type.
 *     - modal: true if url should continue in a modal, false if a new page
 *     - ctools_style: The ctools modal style to use for the next url (if
 *       modal = true).
 *   - id: An optional id to use for the wrapping container element.
 */
function theme_acquia_lift_type_list($variables) {
  ctools_include('modal');
  ctools_include('ajax');
  ctools_modal_add_js();
  drupal_add_library('system', 'drupal.ajax');

  $items = $variables['items'];
  $html = '<div';
  if (!empty($variables['id'])) {
    $html .= ' id="' . $variables['id'] . '"';
  }
  $html .= ' class="acquia-lift-type-list">';
  foreach ($items as $typestr => $type) {
    $html .= '<div class="acquia-lift-type clearfix">';
    $html .= '<div class="acquia-lift-type-logo">' . $type['logo'] . '</div>';
    $html .= '<div class="acquia-lift-type-title">';
    $url_options = isset($type['url_options']) ? $type['url_options'] : array();
    if ($type['modal']) {
      $path_parts = explode('/', $type['path']);
      $last = end($path_parts);
      if ($last != 'nojs') {
        $path_parts[] = 'nojs';
      }
      $type['path'] = implode('/', $path_parts);
      // We can't use ctools_modal_text_button here because it doesn't give us control
      // over the options array and we need to be able to add query parameters to the link.
      $url_options = array_merge_recursive($url_options, array(
        'html' => TRUE,
        'attributes' => array(
          'class' => array(
            'ctools-use-modal',
            'acquia-lift-type-select',
            $type['ctools_style']
          ),
          'title' => $type['title']
        ),
      ));
    }
    else {
      $url_options = array_merge_recursive($url_options, array('attributes' => array('class' => array('acquia-lift-type-select'))));
    }
    $html .= l($type['title'], $type['path'], $url_options);
    $html .= '</div>';
    $html .= '<p>' . $type['description'] . '</p>';
    $html .= '</div>';
  }
  $html .= '</div>';
  return $html;
}

/**
 * Theme function to change the type of form shown in the campaign creation
 * process.
 *
 * @param $variables
 *   An associative array of variables including:
 *   - type:  The title of the type of form to display
 *   - change_link: A change link to change the type shown
 *   - id: Optional an id to include in the markup.
 */
function theme_acquia_lift_create_type_change($variables) {
  $html = '<span';
  if (!empty($variables['id'])) {
    $html .= ' id="' . $variables['id'] . '"';
  }
  $html .= ' class="acquia-lift-form-type">' . $variables['type'] . '</span>&nbsp;' . $variables['change_link'];
  return $html;
}
