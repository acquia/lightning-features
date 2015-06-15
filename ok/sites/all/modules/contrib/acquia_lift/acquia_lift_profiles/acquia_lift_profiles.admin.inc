<?php

/**
 * @file acquia_lift_profiles.admin.inc
 * Provides functions needed for the admin UI.
 */

/**
 * Admin form for configuring acquia_lift_profiles behavior.
 */
function acquia_lift_profiles_admin_form($form, &$form_state) {
  $form['#attached']['js'][] = drupal_get_path('module', 'acquia_lift_profiles') . '/js/acquia_lift_profiles.admin.js';
  $form['acquia_lift_profiles_account_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Acquia Lift Profiles Account Name'),
    '#default_value' => variable_get('acquia_lift_profiles_account_name', ''),
    '#required' => TRUE,
  );
  $form['acquia_lift_profiles_site_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Acquia Lift Profiles Customer Site'),
    '#default_value' => variable_get('acquia_lift_profiles_site_name', ''),
  );
  $form['acquia_lift_profiles_api_url'] = array(
    '#type' => 'textfield',
    '#title' => t('Acquia Lift Profiles API URL'),
    '#field_prefix' => 'http(s)://',
    '#default_value' => variable_get('acquia_lift_profiles_api_url', ''),
    '#required' => TRUE,
  );
  $form['acquia_lift_profiles_access_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Acquia Lift Profiles API Access Key'),
    '#default_value' => variable_get('acquia_lift_profiles_access_key', ''),
    '#required' => TRUE,
  );
  $form['acquia_lift_profiles_secret_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Acquia Lift Profiles API Secret Key'),
    '#default_value' => variable_get('acquia_lift_profiles_secret_key', ''),
    '#required' => TRUE,
  );
  $form['acquia_lift_profiles_js_path'] = array(
    '#type' => 'textfield',
    '#title' => t('Acquia Lift Profiles JavaScript path'),
    '#field_prefix' => 'http(s)://',
    '#default_value' => variable_get('acquia_lift_profiles_js_path', ''),
    '#required' => TRUE,
  );

  $identity_parameter_display_value = variable_get('acquia_lift_profiles_identity_param') ?: 'identity';
  $identity_type_parameter_display_value = variable_get('acquia_lift_profiles_identity_type_param') ?: 'identityType';
  $default_identity_type_display_value = variable_get('acquia_lift_profiles_default_identity_type') ?: 'account';
  $default_identity_type_default_value = variable_get('acquia_lift_profiles_default_identity_type') ?: ACQUIA_LIFT_PROFILES_DEFAULT_IDENTITY_TYPE_DEFAULT;
  $form['acquia_lift_profiles_identity'] = array(
    '#title' => t('Identity'),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );
  $form['acquia_lift_profiles_identity']['acquia_lift_profiles_capture_identity'] = array(
    '#type' => 'checkbox',
    '#title' => t('Capture identity on login / register'),
    '#description' => t('Check this if you want Acquia Lift Profiles to capture the identity of the user upon login or registration. This means sending their email address to the Acquia Lift Profile Manager.'),
    '#default_value' => variable_get('acquia_lift_profiles_capture_identity', FALSE),
  );
  $form['acquia_lift_profiles_identity']['acquia_lift_profiles_identity_param'] = array(
    '#type' => 'textfield',
    '#title' => t('Identity parameter'),
    '#description' => t('The query string parameter for specific visitor information, such as an email address or social media username, which is sent to the Acquia Lift service. Example: ?<ins>@identity_parameter_display_value</ins>=jdoe01', array(
      '@identity_parameter_display_value' => $identity_parameter_display_value,
    )),
    '#default_value' => variable_get('acquia_lift_profiles_identity_param', ''),
  );
  $form['acquia_lift_profiles_identity']['acquia_lift_profiles_identity_type_param'] = array(
    '#type' => 'textfield',
    '#title' => t('Identity type parameter'),
    '#description' => t('The query string parameter for the category (standard or custom) of the visitor\'s identity information, as defined in the !help_link. Example: ?@identity_parameter_display_value=jdoe01&<ins>@identity_type_parameter_display_value</ins>=@default_identity_type_default_value', array(
      '@identity_parameter_display_value' => $identity_parameter_display_value,
      '@identity_type_parameter_display_value' => $identity_type_parameter_display_value,
      '@default_identity_type_default_value' => $default_identity_type_default_value,
      '!help_link' => l(t('Acquia Lift service'), 'https://docs.acquia.com/lift/recommend/admin/customer#custom-identifier', array('attributes' => array('target' => '_blank'))),
    )),
    '#default_value' => variable_get('acquia_lift_profiles_identity_type_param', ''),
    '#states' => array(
      'visible' => array(
        ':input[name="acquia_lift_profiles_identity[acquia_lift_profiles_identity_param]"]' => array('!value' => '')
      )
    )
  );
  $form['acquia_lift_profiles_identity']['acquia_lift_profiles_default_identity_type'] = array(
    '#type' => 'textfield',
    '#title' => t('Default identity type'),
    '#attributes' => array(
      'placeholder' => ACQUIA_LIFT_PROFILES_DEFAULT_IDENTITY_TYPE_DEFAULT,
    ),
    '#description' => t('The identity type category to use by default. Leave this field blank to default to <ins>@default</ins>. Example: <ins>@default_identity_type_display_value</ins> is "?@identity_parameter_display_value=jdoe01&@identity_type_parameter_display_value=<ins>@default_identity_type_display_value</ins>", while blank is the same as "?@identity_parameter_display_value=jdoe01&@identity_type_parameter_display_value=<ins>@default</ins>"', array(
      '@default' => ACQUIA_LIFT_PROFILES_DEFAULT_IDENTITY_TYPE_DEFAULT,
      '@identity_parameter_display_value' => $identity_parameter_display_value,
      '@identity_type_parameter_display_value' => $identity_type_parameter_display_value,
      '@default_identity_type_display_value' => $default_identity_type_display_value,
    )),
    '#default_value' => variable_get('acquia_lift_profiles_default_identity_type', ''),
  );

  // Retrieving all context options.
  module_load_include('inc', 'personalize', 'personalize.admin');
  $groups = personalize_get_grouped_context_options(NULL, TRUE, array('acquia_lift_profiles_context', 'acquia_lift_context'));

  // Field mappings.
  $form['acquia_lift_profiles_field_mappings'] = array(
    '#title' => t('Field Mappings'),
    '#description' => t('For the "content section", "content keywords", and "persona" fields in Acquia Lift Profiles, you can map them to different visitor contexts.'),
    '#type' => 'fieldset',
    '#tree' => TRUE,
  );
  $field_list = array(
    'content_section' => 'Content Section',
    'content_keywords' => 'Content Keywords',
    'persona' => 'Persona',
  );
  $field_mappings = variable_get('acquia_lift_profiles_field_mappings', array());
  foreach ($field_list as $field_name => $field_friendly_name) {
    $default_value = isset($field_mappings[$field_name]) ? $field_mappings[$field_name] : NULL;
    $select = acquia_lift_profiles_admin_build_visitor_context_select($groups, $default_value);
    $form['acquia_lift_profiles_field_mappings'][$field_name] = $select;
    $form['acquia_lift_profiles_field_mappings'][$field_name]['#title'] = t($field_friendly_name);
  }

  // Only show the "Tracked actions" selector if acquia_lift_profiles has already been
  // configured for API connections.
  if (acquia_lift_profiles_is_configured(TRUE)) {
    $udf_mappings = variable_get('acquia_lift_profiles_udf_mappings', array());
    $form['acquia_lift_profiles_udf_mappings'] = array(
      '#title' => t('User Defined Field Mappings'),
      '#description' => t('For each user defined field available in Acquia Lift Profiles, you can map a visitor context, whose value will then be sent as the value for that user defined field.'),
      '#type' => 'fieldset',
      '#tree' => TRUE,
    );
    foreach (acquia_lift_profiles_get_udfs() as $type => $udfs) {
      $form['acquia_lift_profiles_udf_mappings'][$type] = array(
        '#title' => t('@type mappings', array('@type' => ucfirst($type))),
        '#type' => 'fieldset',
        '#tree' => TRUE,
      );
      // Start by just showing 3 of each type to map, or if more have already
      // been mapped show that number plus 1.
      $max = count($udfs);
      $min = 3;
      if (isset($udf_mappings[$type])) {
        if (count($udf_mappings[$type]) < $max) {
          $min = max(array(count($udf_mappings[$type]) + 1, 3));
        }
        else {
          $min = $max;
        }
      }
      foreach ($udfs as $i => $udf) {
        $default_value = isset($udf_mappings[$type][$udf]) ? $udf_mappings[$type][$udf] : NULL;
        $select = acquia_lift_profiles_admin_build_visitor_context_select($groups, $default_value);
        $select['#title'] = t('Context to map to the @field field', array('@field' => $udf));
        $form['acquia_lift_profiles_udf_mappings'][$type][$udf] = $select;
        if ($i == $min) {
          $form['acquia_lift_profiles_udf_mappings'][$type][$udf]['#prefix'] = '<div class="acquia-lift-profiles-hidden-udfs">';
        }
        if ($i == $max - 1) {
          $form['acquia_lift_profiles_udf_mappings'][$type][$udf]['#suffix'] = '</div>';
        }
      }
      if ($min < $max) {
        $form['acquia_lift_profiles_udf_mappings'][$type]['button__show_all'] = array(
          '#attributes' => array(
            'class' => array('acquia-lift-profiles-udf-show-all'),
          ),
          '#type' => 'button',
          '#value' => t('Show all')
        );
      }
    }
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  return $form;
}

/**
 * Returns a visitor context single select box.
 */
function acquia_lift_profiles_admin_build_visitor_context_select($groups, $default_value) {
  if (count($groups) == 0) {
    return FALSE;
  }
  $element = array(
    '#type' => 'select',
    '#empty_option' => t('- Not mapped -'),
    '#options' => $groups,
    '#default_value' => $default_value,
  );
  return $element;
}


/**
 * Submit handler for the configuration form.
 */
function acquia_lift_profiles_admin_form_submit($form, &$form_state) {
  // If the "capture identity" setting has changed we need to make sure the visitor
  // actions subscribers are cleared for the login and register actions.
  $capture_identity = variable_get('acquia_lift_profiles_capture_identity', FALSE);
  if ($capture_identity != $form_state['values']['acquia_lift_profiles_identity']['acquia_lift_profiles_capture_identity']) {
    visitor_actions_clear_subscribers('user_login');
    visitor_actions_clear_subscribers('user_register');
  }
  // Credentials.
  $variables = array(
    'acquia_lift_profiles_account_name',
    'acquia_lift_profiles_site_name',
    'acquia_lift_profiles_access_key',
    'acquia_lift_profiles_secret_key',
  );
  foreach ($variables as $key) {
    variable_set($key, $form_state['values'][$key]);
  }
  // Field mappings.
  if (isset($form_state['values']['acquia_lift_profiles_field_mappings'])) {
    // Don't save empty entries.
    $acquia_lift_profiles_field_mappings = array_filter($form_state['values']['acquia_lift_profiles_field_mappings']);
    variable_set('acquia_lift_profiles_field_mappings', $acquia_lift_profiles_field_mappings);
  }
  // Identity parameters.
  $identity_variables = array(
    'acquia_lift_profiles_capture_identity',
    'acquia_lift_profiles_identity_param',
    'acquia_lift_profiles_identity_type_param',
    'acquia_lift_profiles_default_identity_type',
  );
  foreach ($identity_variables as $key) {
    variable_set($key, $form_state['values']['acquia_lift_profiles_identity'][$key]);
  }

  // Strip out any http/https protocol from urls and save.
  foreach (array('acquia_lift_profiles_api_url', 'acquia_lift_profiles_js_path') as $key) {
    variable_set($key, preg_replace('/(^[a-z]+:\/\/)/i', '', $form_state['values'][$key]));
  }

  // Refresh the segments from Acquia Lift Profiles.
  acquia_lift_profiles_refresh_segments_cache();

  if (isset($form_state['values']['acquia_lift_profiles_udf_mappings'])) {
    $mapped_contexts = array();
    foreach ($form_state['values']['acquia_lift_profiles_udf_mappings'] as $type => $mappings) {
      foreach ($mappings as $udf => $mapped_context) {
        if ($udf == 'button__show_all') {
          continue;
        }
        if (!empty($mapped_context)) {
          $mapped_contexts[$type][$udf] = $mapped_context;
        }
      }
    }
    variable_set('acquia_lift_profiles_udf_mappings', $mapped_contexts);
  }

  module_load_include('inc', 'acquia_lift_profiles', 'acquia_lift_profiles.batch');
  acquia_lift_batch_sync_actions();
  drupal_set_message(t('The configuration settings have been saved.'), 'status');
}

/**
 * Administrative form to select the thumbnail image to use per content type/
 * entity bundle.
 *
 * @param $entity_name
 *   The name of the entity type, e.g., node.
 * @param entity
 *   The entity data object.
 * @bundle
 *   The bundle name to select from.
 */
function acquia_lift_profiles_thumbnail_entity_settings_form(&$form, &$form_state, $entity_name, $entity, $bundle) {
  // If the entity is not fieldable, check that there is at least one bundle
  // with an image field.
  $options = acquia_lift_profiles_thumbnail_available_fields($entity_name, $bundle);
  if (!$entity['fieldable']) {
    if (count($options) == 0) {
      return;
    }
  }

  $styles = image_styles();
  $style_options = array();
  foreach($styles as $style) {
    $style_options[$style['name']] = t($style['label']);
  }

  $form['#attached']['js'][] = drupal_get_path('module', 'acquia_lift_profiles') . '/js/acquia_lift_profiles.admin.js';
  $form['acquia_lift_profiles'] = array(
    '#type' => 'fieldset',
    '#title' => t('Acquia Lift Profiles'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
  );
  $form['acquia_lift_profiles']['entity'] = array(
    '#type' => 'value',
    '#value' => $entity_name,
  );
  $form['acquia_lift_profiles']['bundle'] = array(
    '#type' => 'value',
    '#value' => $bundle,
  );
  $form['acquia_lift_profiles']['thumbnail'] = array(
    '#type' => 'select',
    '#title' => t('Select preview image to use for content recommendations.'),
    '#options' => $options,
    '#empty_option' => t('None'),
    '#empty_value' => '',
    '#default_value' =>  acquia_lift_profiles_thumbnail_get_field($entity_name, $bundle),
  );
  $image_style_fields = acquia_lift_profiles_thumbnail_available_fields($entity_name, $bundle, TRUE);
  $form['acquia_lift_profiles']['style'] = array(
    '#type' => 'select',
    '#title' => t('Select the style to use for the thumbnail'),
    '#options' => $style_options,
    '#default_value' => acquia_lift_profiles_thumbnail_get_style_field($entity_name, $bundle),
  );
  // If no available fields support styles then hide the style selector.
  if (empty($image_style_fields)) {
    $form['acquia_lift_profiles']['style']['#type'] = 'value';
    $form['acquia_lift_profiles']['style']['#value'] = '';
  }
  else {
    // Hide the image style field if the thumbnail isn't in the fields that
    // supports image styles.  In order to list multiple negatives we use the
    // workaround: https://api.drupal.org/api/drupal/includes!common.inc/function/drupal_process_states/7#comment-24708
    foreach ($image_style_fields as $key => $value) {
      $form['acquia_lift_profiles']['style']['#states']['invisible'][':input[name="acquia_lift_profiles[thumbnail]"], allow-multiple-' . $key] = array('!value' => $key);
    }
  }

  // If there are no available image fields at all, disable the selection
  // and hide the style selector.
  if (!count($options)) {
    $form['acquia_lift_profiles']['thumbnail']['#disabled'] = TRUE;
    $form['acquia_lift_profiles']['thumbnail']['#description'] = t('You must define at least one image field to enable the content recommendations thumbnail image.');
    // Make the style hidden completely.
    $form['acquia_lift_profiles']['style']['#type'] = 'value';
    $form['acquia_lift_profiles']['style']['#value'] = '';
  }

  $form['#submit'][] = 'acquia_lift_profiles_entity_settings_form_submit';
}

/**
 * Submit handler for acquia_lift_profiles content type settings.
 */
function acquia_lift_profiles_entity_settings_form_submit($form, &$form_state) {
  $entity = $form_state['values']['acquia_lift_profiles']['entity'];
  $bundle = $form_state['values']['acquia_lift_profiles']['bundle'];
  acquia_lift_profiles_thumbnail_set_field($entity, $bundle, $form_state['values']['acquia_lift_profiles']['thumbnail']);
  acquia_lift_profiles_thumbnail_set_style_field($entity, $bundle, $form_state['values']['acquia_lift_profiles']['style']);
}

/**
 * Given an entity type machine name, returns available image fields.
 *
 * @param $entity_type
 *   Type of entity to load fields, e.g., 'node'.
 * @param $bundle
 *   The name of the entity bundle for fields, e.g., 'article'
 * @param $limit_style_support
 *   True if fields should be limited to those with image style support.
 */
function acquia_lift_profiles_thumbnail_available_fields($entity_type, $bundle, $limit_style_support = FALSE) {
  $fields = array();

  foreach (field_info_instances($entity_type, $bundle) as $name => $info) {
    $supported_widget_types = _acquia_lift_profiles_thumbnail_supported_widgets();

    if (isset($info['widget']['type']) && array_key_exists($info['widget']['type'], $supported_widget_types)) {
      $key = $name . ACQUIA_LIFT_PROFILES_THUMBNAIL_WIDGET_SEPARATOR . $info['widget']['type'];
      if (!$limit_style_support || $supported_widget_types[$info['widget']['type']]['use_styles']) {
        $fields[$key] = $info['label'] . ' (' . $name . ')';
      }
    }
  }
  return $fields;
}

/**
 * Gets an array of supported widget types for thumbnail image selection.
 *
 * @return array
 *   An array of widget types keyed by widget name.
 */
function _acquia_lift_profiles_thumbnail_supported_widgets() {
  return array(
    'image_image' => array(
      'use_styles' => TRUE,
    ),
    'media_generic' => array(
      'use_styles' => TRUE,
    ),
  );
}
