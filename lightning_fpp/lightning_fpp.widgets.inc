<?php
/**
 * @file
 * Widget code for the lightning_fpp feature.
 */

/**
 * Helper function to define our custom Fielable Panels Panes.
 */
function _lightning_fpp_fpp_types() {
  $types = array(
    'raw_html' => array(
      'title' => 'HTML',
      'icon' => 'icon_html.png',
      'path' => 'raw-html',
    ),
    'quick_links' => array(
      'title' => 'links',
      'icon' => 'icon_link.png',
      'path' => 'quick-links',
    ),
    'media_pane' => array(
      'title' => 'media',
      'icon' => 'icon_media.png',
      'path' => 'media',
    ),
    'map' => array(
      'title' => 'map',
      'icon' => 'icon_map.png',
      'path' => 'map',
    ),
    'text' => array(
      'title' => 'text',
      'icon' => 'icon_text.png',
      'path' => 'text',
    ),
  );
  return $types;
}

/**
 * Implementation of hook_form_alter()
 */
function lightning_fpp_form_alter(&$form, &$form_state, $form_id) {

  if ($form_id == 'fieldable_panels_panes_fieldable_panels_pane_content_type_edit_form') {

    // Add fields to the FPP object if we are adding a reusable object for the first time
    if ($form_state['entity']->reusable && $form_state['op'] == 'add' && $form_state['rebuild'] == FALSE) {
      $form = fieldable_panels_panes_entity_edit_form($form, $form_state);
    }

    // Retrieve list of our custom Fieldable Panels Panes bundles.
    $fpp_types = _lightning_fpp_fpp_types();
    $fpp_info = $fpp_types[$form['#bundle']];

    // Add a custom pre-render method to deal with "Content Settings" fieldset.
    $form['#pre_render'][] = 'lightning_fpp_fieldable_panel_pane_pre_render';
    
    // Remove the revisioning information
    $form['revision']['revision']['#type'] = 'value';
    $form['revision']['log']['#access'] = FALSE;

    // Add a widget setting
    $form['widget_settings']['#type'] = 'fieldset';
    $form['widget_settings']['#title'] = t('General Settings');
    $form['title']['#attributes']['placeholder'] = $form['title']['#title'];
    $form['title']['#title_display'] = 'invisible';
    $form['widget_settings']['title'] = $form['title'];
    unset($form['title']);

    // Move the link field into the general settings tab.
    $form['widget_settings']['link'] = $form['link'];
    unset($form['link']);

    // Improve the reusable interface
    $form['reusable']['#title'] = t('Reusable Settings');
    $form['reusable']['#type'] = 'fieldset';
    $form['reusable']['#process'] = array('ctools_dependent_process');
    $form['reusable']['#id'] = 'reusable-settings';
    $form['reusable']['category']['#value'] = t("Reusable @name", array('@name' => $fpp_info['title']));
    $form['reusable']['category']['#type'] = 'value';
    $form['reusable']['admin_description']['#access'] = FALSE;
    // Set the proper dependencies for the reusable markup
    $form['reusable']['warning']['#dependency'] = array('edit-reusable' => array(1));
    $form['reusable']['warning']['#prefix'] = '<div id="reusable-wrapper">';
    $form['reusable']['warning']['#suffix'] = '</div>';
    $form['reusable']['warning']['#type'] = 'item';

    // Update the text for the reusable markup
    $form['reusable']['warning']['#markup'] = '<div class="description">' . t('Note: This item is reusable. Any changes made will be applied globally.') . '</div>';
  }

}

/**
 * Implement the "Content Settings" fieldset in a pre-render. 
 * This fixes issues with image caused by initially doing this in a form_alter.
 *
 * @see http://drupal.org/node/1567704
 */
function lightning_fpp_fieldable_panel_pane_pre_render($element) {
  // Add a content settings fieldset.
  $element['content_settings'] = array(
    '#type' => 'fieldset',
    '#title' => t('Content Settings'),
    '#weight' => 1,
  );

  // Add any remaining fields to the content settings fieldset.
  foreach ($element as $key => $value) {
    if (substr($key, 0, 6) == 'field_' || $key == 'body') {
      $element['content_settings'][$key] = $value;
      unset($element[$key]);
    }
  }

  return $element;
}

