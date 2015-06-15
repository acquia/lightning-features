<?php
/**
 * @file acquia_lift.admin.unibar.inc
 *
 * Contains the administrative functionality callbacks from the unified
 * navigation bar.
 */

/***************************************************************
 *
 * C A M P A I G N  C R E A T E
 *
 ***************************************************************/

/**
 * Page callback to start the create a campaign process.  The first step is to select
 * the type of
 * @param bool $ajax
 *   Indicates if form is called with ajax support.
 */
function acquia_lift_campaign_create_modal_callback($ajax) {
  if (!$ajax) {
    drupal_goto('admin/structure/personalize/add');
    return;
  }
  // Show the campaign type selection form in a modal.
  ctools_include('modal');
  ctools_modal_add_js();

  $path = drupal_get_path('module', 'acquia_lift');
  $output = theme('acquia_lift_type_list', array(
    'id' => 'acquia-lift-modal-campaign-type-list',
    'items' => array(
      'ab' => array(
        'title' => t('A/B test'),
        'description' => t('Create and compare content variations.'),
        'path' => 'admin/structure/acquia_lift/add/ab',
        'modal' => TRUE,
        'ctools_style' => 'ctools-modal-acquia-lift-style',
        'logo' => theme('image', array(
          'path' => $path . '/images/campaign-type-ab.png',
          'alt' => t('A/B test'),
          'title' => t('Select this option to create an A/B test.'),
        )),
      ),
      'custom' => array(
        'title' => t('Custom Lift campaign'),
        'description' => t('Create a content targeting or multivariate campaign using the Drupal-integrated campaign tools.'),
        'path' => 'admin/structure/personalize/add/',
        'url_options' => array(
          'query' => array(
            'personalize_agent_type' => 'acquia_lift'
          )
        ),
        'modal' => FALSE,
        'logo' => theme('image', array(
          'path' => $path . '/images/campaign-type-custom.png',
          'alt' => t('Custom Lift campaign'),
          'title' => t('Select this option to create a custom Lift campaign.'),
        ))
      ),
    ),
  ));
  return ctools_modal_render(t('Create a campaign'), $output);
}

/**
 * Page callback to generate the ctools modal form to create a campaign
 * of a specific type.
 *
 * @param $type
 *   The type of campaign to create.  Current supported choices:  'ab'
 * @param $ajax
 *   Indicates if AJAX is supported
 */
function acquia_lift_campaign_type_create_modal_callback($type, $ajax) {
  // If the campaign type is not supported or AJAX is not supported then go to the full campaign
  // creation process.
  if (!$ajax || !in_array($type, array('ab'))) {
    drupal_goto('admin/structure/personalize/add');
    return;
  }
  acquia_lift_create_ctools_form(
    t('Create a campaign'),
    'acquia_lift_campaign_type_create_form',
    array('type' => $type),
    'acquia_lift_campaign_type_create_complete_callback'
  );
}

/**
 * Ctools form processing complete handler for the creation of a new campaign
 * by type.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_campaign_type_create_complete_callback($form, $form_state) {
  $agent_name = personalize_get_campaign_context();
  return array(
    ctools_modal_command_dismiss(),
    ajax_command_settings(array(
      'acquia_lift' => array(
        'campaigns' => acquia_lift_get_campaign_details(variable_get('acquia_lift_unibar_allow_status_change', TRUE)),
        'agent_map' => array(
          $agent_name => personalize_agent_get_map_settings($agent_name),
        ),
      ),
      'personalize' => array(
        'activeCampaign' => $agent_name,
      ),
    ), TRUE),
    acquia_lift_command_messagebox(t('Click the element you want to change in %variation.', array(
      '%campaign' => $form_state['values']['agent_basic_info']['title'],
      '%variation' => 'Variation #1',
    )), 10),
    acquia_lift_command_page_variation_toggle(),
    // @todo add this back in when the campaigns can be processed along with
    // variation set creation.
    // acquia_lift_command_process_queue(),
  );
}

/**
 * Form builder function to create a new campaign in the modal process
 * of a specific type.
 *
 * @param $type
 *   The type of campaign to create.  Current supported choices: 'ab'
 */
function acquia_lift_campaign_type_create_form($form, &$form_state, $type) {
  ctools_include('modal');
  ctools_include('ajax');

  ctools_add_js('ajax-responder');

  $change_link = ctools_modal_text_button(t('Change type of test'), 'admin/structure/personalize/add/nojs', t('Change type of test'), 'ctools-modal-acquia-lift-style');
  // Add individual forms specific to the type of campaign being created.
  switch ($type) {
    case 'ab':
      $form['campaign_type'] = array(
        '#markup' => theme('acquia_lift_create_type_change', array(
          'type' => t('A/B test'),
          'change_link' => $change_link,
        )),
      );
      $form['agent'] = _acquia_lift_simplified_campaign_create_form('acquia_lift_simple_ab');
      break;
  }
  if (empty($form['agent'])) {
    // There isn't any agent data to act upon.
    return $form;
  }
  // Add common actions.
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Create campaign'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Loads the simplified campaign creation form for an A/B test with an
 * Acquia Lift agent.
 */
function _acquia_lift_simplified_campaign_create_form($agent_type) {
  ctools_include('plugins');

  if ($agent_class = ctools_plugin_load_class('personalize', 'agent_type', $agent_type, 'handler')) {
    $method = is_subclass_of($agent_class, 'AcquiaLiftSimplifiedAgentInterface') ? 'simplifiedForm' : 'optionsForm';
    $options_form = call_user_func_array(array($agent_class, $method), array(array()));
  }
  else {
    drupal_set_message(t('There is a problem accessing the Acquia Lift agent. Acquia Lift campaigns can not be created until you configure your account info !here', array('!here' => l('here', 'admin/config/content/personalize/acquia_lift'))), 'error');
    return array();
  }
  // Recreate a basic campaign form with the appropriate AB options.
  // @see personalize_agent_build_basic_form.
  $form['agent_basic_info'] = array(
    '#tree' => TRUE,
  );
  $form['agent_basic_info']['title'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
  );
  $form['agent_basic_info']['agent_type'] = array(
    '#type' => 'value',
    '#value' => $agent_type,
  );
  $form['agent_basic_info']['options'] = array(
    '#tree' => TRUE,
  );
  $form['agent_basic_info']['options'][$agent_type] = $options_form;
  // Defaults to caching decisions.
  $form['cache_decisions'] = array(
    '#type' => 'value',
    '#value' => 1,
  );
  return $form;
}

function acquia_lift_campaign_type_create_form_validate($form, &$form_state) {
  $form_state['values']['agent_basic_info']['machine_name'] = personalize_generate_machine_name($form_state['values']['agent_basic_info']['title'], 'personalize_agent_machine_name_exists');
  module_load_include('inc', 'personalize', 'personalize.admin');
  personalize_agent_form_validate($form, $form_state);
}

/**
 * Submit handler to create a campaign from the Acquia Lift UI flow.
 */
function acquia_lift_campaign_type_create_form_submit($form, &$form_state) {
  module_load_include('inc', 'personalize', 'personalize.admin');
  $agent_data = _personalize_agent_from_form_values($form_state['values']['agent_basic_info']);
  $agent_data->data['cache_decisions'] = $form_state['values']['cache_decisions'];
  $agent = personalize_agent_save($agent_data);
  personalize_set_campaign_context($agent->machine_name);
  // @todo find out how to remove this in favor of asynchronous processing.
  acquia_lift_process_queue(FALSE);
}

/***************************************************************
 *
 * V A R I A T I O N  C R E A T E / E D I T
 *
 ***************************************************************/

/**
 * Form handler for an element variation details form.
 *
 * This is a personalize elements, essentially, that can either be a page
 * variation (for simple A/B campaigns) or a regular personalize elements option
 * set for other agent types.
 *
 * @param string $variation_type
 *   The variation type that defines the type of details to include.  This
 *   is a key defined as a personalize_elements variation.
 * @param string $osid
 *   (Optional) The option set id when adding variations to an existing option
 *   set.  If null, then create a new option set with the control and variation.
 * @param string $option_id
 *   (Optional) The option id for the specific variation to be edited.  NOTE:
 *   Only element variations can be edited (not page variations).
 *
 * @see hook_personalize_elements_variation_types
 */
function acquia_lift_element_variation_details_form($form, &$form_state, $variation_type, $osid = NULL, $option_id = NULL) {
  module_load_include('inc', 'personalize_elements', 'personalize_elements.admin');
  $types = module_invoke_all('personalize_elements_variation_types');
  if (!isset($types[$variation_type]['contextual']['formitem'])) {
    $form['error'] = array(
      '#markup' => t('Invalid page variation type.  Please close this message and create a new variation.'),
    );
    return $form;
  }
  if (!empty($osid)) {
    $osid = personalize_resolve_stringified_osid($osid);
    $option_set = personalize_option_set_load($osid);
    $edit_option = NULL;
    if (!empty($option_id)) {
      if ($option_id === PERSONALIZE_CONTROL_OPTION_ID) {
        $form['error'] = array(
          '#markup' => t('The control variation cannot be edited.'),
        );
        return $form;
      }
      foreach ($option_set->options as $option) {
        if ($option['option_id'] === $option_id) {
          $edit_option = $option;
          $form['option'] = array(
            '#type' => 'value',
            '#value' => $edit_option,
          );
          break;
        }
      }
    }
    $form['osid'] = array(
      '#type' => 'value',
      '#value' => $osid,
    );
  }
  $form['variation_type'] = array(
    '#type' => 'value',
    '#value' => $variation_type,
  );
  // These form item will be populated by the front-end code.
  $form['selector'] = array(
    '#type' => 'textfield',
    '#title' => t('Selector'),
    '#required' => TRUE,
    '#maxlength' => 500,
    '#access' => empty($osid),
  );
  $form['pages'] = array(
    '#type' => 'hidden',
  );
  $form['agent'] = array(
    '#type' => 'hidden',
  );
  // Variation number applicable to page variations.
  $form['variation_number'] = array(
    '#type' => 'hidden',
  );
  // Option set title applicable to element variations.
  $form['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Variation set name'),
    '#description' => t('Descriptive name for the variation set, used by Acquia Lift for administration and reporting.'),
    '#required' => TRUE,
    '#maxlength' => 255,
    '#access' => empty($osid),
  );
  // Option label if editing.
  $option_label = isset($option['option_label']) ? $option['option_label'] : '';
  $form['option_label'] = array(
    '#type' => 'textfield',
    '#title' => t('Variation name'),
    '#description' => t('Descriptive name for the variation, used by Acquia Lift for administration and reporting.'),
    '#required' => TRUE,
    '#maxlength' => PERSONALIZE_MACHINE_NAME_MAXLENGTH,
    '#access' => !empty($option),
    '#default_value' => filter_xss($option_label),
  );
  // Now include the details specific to this variation type.
  $form['personalize_elements_content'] = $types[$variation_type]['contextual']['formitem'];
  if (isset($edit_option)) {
    $form['personalize_elements_content']['#default_value'] = personalize_elements_filter_personalize_element_option($variation_type, $option['personalize_elements_content']);
    $form['variation_number']['#type'] = 'value';
    $form['variation_number']['#value'] = $edit_option['option_id'];
  }

  $form['actions'] = array(
    '#type' => 'actions',
  );
  // Form actions.
  // TRICKY: These must have ids that are different from our CTools loaded
  // modal forms or else their submit buttons will be processed via Drupal ajax
  // rather than the main URL callbacks leading to a 200 response with an
  // empty response body.
  $form['actions']['variation_type_submit_form'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#ajax' => array(
      'callback' => 'acquia_lift_element_variation_details_form_ajax_callback',
      'wrapper' => 'acquia-lift-element-variation-details-form',
      'progress' => array(
        'message' => NULL,
        'type' => 'throbber',
      ),
    ),
  );
  // Add a cancel button for AJAX-loaded forms.
  $form['actions']['variation_type_cancel_form'] = array(
    '#type' => 'button',
    '#button_type' => 'cancel',
    '#value' => t('Cancel'),
    '#attributes' => array(
      'class' => array('acquia-lift-cancel-button'),
    ),
    '#ajax' => array(
      'callback' => 'acquia_lift_element_variation_details_form_ajax_callback',
      'wrapper' => 'acquia-lift-element-variation-details-form',
      'progress' => array(
        'message' => NULL,
        'type' => 'throbber',
      ),
    ),
    // Turn off validation.
    '#limit_validation_errors' => array(),
    '#submit' => array(),
  );

  return $form;
}

/**
 * Form validation handler for acquia_lift_element_variation_details_form.
 */
function acquia_lift_element_variation_details_form_validate($form, &$form_state) {
  module_load_include('inc', 'personalize_elements', 'personalize_elements.admin');
  if (!personalize_elements_selector_validate($form_state['values']['selector'])) {
    form_set_error('selector', t('The selector cannot contain tags.'));
  }
  $filtered = personalize_elements_filter_personalize_element_option($form_state['values']['variation_type'], $form_state['values']['personalize_elements_content']);
  if ($filtered !== $form_state['values']['personalize_elements_content']) {
    $form_state['values']['personalize_elements_content'] = $filtered;
    drupal_set_message(t('Disallowed HTML tags were removed from the variation.'), 'warning');
  }
}

/**
 * Form submit handler for acquia_lift_element_variation_details_form.
 *
 * Calls the appropriate handler for the creation of the personalize elements
 * variation based on the type of campaign.
 */
function acquia_lift_element_variation_details_form_submit($form, &$form_state) {
  $agent = personalize_agent_load_agent($form_state['values']['agent']);
  if ($agent instanceof AcquiaLiftSimpleAB) {
    _acquia_lift_page_variation_details_form_submit($form, $form_state);
  }
  else {
    _acquia_lift_personalize_element_variation_details_form_submit($form, $form_state);
  }
}

/**
 * Submit handler to create a new page variation for an AB agent.
 *
 * This creates a page variation set if it does not yet exist and the variation
 * for that page.  For a simple A/B test, there is only a single variation set
 * allowed per campaign and therefore it is named for the campaign/agent.
 *
 * A variation set is similar to a decision name concept and groups together
 * option sets for a particular agent to be considered as a group.
 * A variation is comprised of all of the options of the same index across
 * all option sets in a variation set (decision name).
 * In other words, an option set handles the changes for a single element of
 * the variation and includes how that element is shown for each variation as
 * its options.
 */
function _acquia_lift_page_variation_details_form_submit($form, &$form_state) {
  $agent_name = $variation_set_name = $form_state['values']['agent'];
  $option_set = personalize_elements_get_option_set_for_variation($variation_set_name, $agent_name, $form_state['values']['selector'], $form_state['values']['variation_type'], $form_state['values']['pages']);

  $variation_number = NULL;
  if (is_numeric($form_state['values']['variation_number']) && $form_state['values']['variation_number'] >= 0) {
    $variation_number = $form_state['values']['variation_number'];
  }
  module_load_include('inc', 'acquia_lift', 'acquia_lift.page_variations');
  $variation_number = acquia_lift_page_variation_create($variation_set_name, $option_set, array('personalize_elements_content' => $form_state['values']['personalize_elements_content']), $variation_number);
  // Save the updated option sets for use within the Ajax callback.
  $form_state['storage']['option_sets'] = personalize_option_set_load_multiple(FALSE, array('decision_name' => $variation_set_name));
  $form_state['storage']['variation_number'] = $variation_number;
  $form_state['storage']['confirmation_message'] = t('The variation has been created.');
}

/**
 * Submit handler to create a new personalize elements option set or edit
 * and existing option..
 */
function _acquia_lift_personalize_element_variation_details_form_submit($form, &$form_state) {
  // Prepare the values to those expected by the personalize elements module.
  $values = $form_state['values'];
  $message = '';

  if (empty($values['osid'])) {
    // Creating a new option set and a new option within it.
    $values['options'] = array(
      array(
        'option_label' => t('Variation #@num', array('@num' => 1)),
        'personalize_elements_content' => $values['personalize_elements_content'],
      ),
    );
    $values['add_control_option'] = TRUE;
    $values['agent_select'] = $values['agent'];

    module_load_include('inc', 'personalize_elements', 'personalize_elements.admin');
    $option_set = personalize_elements_convert_form_values_to_option_set($values);
    $message = t('The variation set has been created.');
  }
  else {
    $option_set = personalize_option_set_load($values['osid']);
    if (empty($values['variation_number'])) {
      // Adding a variation to an existing option set.
      $option_set->options[] = array(
        'option_label' => t('Variation #@num', array('@num' => count($option_set->options))),
        'personalize_elements_content' => $values['personalize_elements_content'],
      );
      $message = t('The variation has been created.');
    }
    else {
      // Editing an existing option set option.
      foreach ($option_set->options as &$option) {
        if ($option['option_id'] === $values['variation_number']) {
          $option['personalize_elements_content'] = $values['personalize_elements_content'];
          $option['option_label'] = $values['option_label'];
          break;
        }
      }
      $message = t('The variation has been updated.');
    }
  }
  $option_set = personalize_option_set_save($option_set);
  // Now re-load the option set so that all hooks get run.
  // This is particularly important to set the selector property.
  $option_set = personalize_option_set_load($option_set->osid);
  // Save the updated option sets for use within the Ajax callback.
  $form_state['storage']['option_sets'] = array($option_set);
  $form_state['storage']['osid'] = $option_set->osid;
  // Determine the option set to preview after save.
  if (!empty($values['variation_number'])) {
    // Preview the edited variation.
    $form_state['storage']['option_id'] = $values['variation_number'];
  }
  else {
    // It was a new option, so preview the last one.
    // The extra parenthesis over array_values is for avoiding PHP strict warning.
    $updated_option = end((array_values($option_set->options)));
    $form_state['storage']['option_id'] = $updated_option['option_id'];
  }
  $form_state['storage']['confirmation_message'] = $message;
}

/**
 * Responds to AJAX submission of a variation type details page.
 */
function acquia_lift_element_variation_details_form_ajax_callback($form, &$form_state) {
  $op = isset($form_state['values']['op']) ? $form_state['values']['op'] : '';
  $commands = array();
  if ($op == t('Save')) {
    $errors = form_get_errors();
    // If validation errors exist, return the form.
    if (!empty($errors)) {
      return $form;
    }
    // Get the updated option sets for the page variation.
    $agent_name = $form_state['values']['agent'];
    $option_sets = $form_state['storage']['option_sets'];
    $option_set_settings = array_map('_personalize_convert_option_set_to_js_setting', $option_sets);
    foreach($option_set_settings as $option_set_setting) {
      $settings['personalize']['option_sets'][key($option_set_setting)] = reset($option_set_setting);
    }
    // Also get the updated personalize elements options.
    personalize_elements_limit_option_sets_by_path($option_sets, $form_state['values']['pages']);
    $settings['personalize_elements']['elements'] = personalize_elements_element_settings($option_sets);
    // We need to update the campaigns as well in case a campaign was paused by this action.
    $settings['acquia_lift']['campaigns'] = acquia_lift_get_campaign_details(variable_get('acquia_lift_unibar_allow_status_change', TRUE));
    // And the agent map.
    $settings['personalize']['agent_map'][$agent_name] = personalize_agent_get_map_settings($agent_name);
    // Note: JavaScript assets for personalize elements are included in
    // personalize_elements_page_build when the module is enabled and therefore
    // do not need to be explicitly added here.
    $commands[] = ajax_command_settings($settings, TRUE);
    // Determine the index to preview.  If creating a new variation, then the
    // first index is 1 because 0 is the control variation.
    if (isset($form_state['storage']['variation_number'])) {
      $preview_variation_index = (!is_numeric($form_state['storage']['variation_number']) || $form_state['storage']['variation_number'] < 1) ? 1 : $form_state['storage']['variation_number'];
      $commands[] = acquia_lift_command_page_variation_preview($form_state['values']['agent'], $preview_variation_index);
    }
    else {
      $commands[] = acquia_lift_command_variation_preview($form_state['values']['agent'], $form_state['storage']['osid'], $form_state['storage']['option_id']);
    }
    // End the editing mode.
    $commands[] = acquia_lift_command_page_variation_toggle(FALSE);
    // Show a confirmation message.
    $current_status = personalize_agent_get_status($agent_name);
    $message = $form_state['storage']['confirmation_message'];
    if ($current_status === PERSONALIZE_STATUS_NOT_STARTED) {
      if ($settings['acquia_lift']['campaigns'][$agent_name]['verified']) {
        $message .= ' ' . t('You can now start the campaign.');
      }
      else {
        $message .= ' ' . t('Add one or more goals by clicking Goals &gt; Add goal.');
      }
    }
    $commands[] = acquia_lift_command_messagebox($message, 10);
    $commands[] = acquia_lift_command_process_queue();
  }
  else if ($op === t('Cancel')) {
    // Turn off edit mode and redirect them.
    $commands[] = acquia_lift_command_page_variation_toggle(FALSE);
  }

  return personalize_ajax_commands_deliver($commands, TRUE);
}

/***************************************************************
 *
 * O P T I O N  S E T  C R E A T E
 *
 ***************************************************************/

/**
 * Page callback to create a new option set.
 *
 * @param string $agent_name
 *   The machine name of the current camaign.
 * @param string $ajax
 *   Indicates if request is from 'ajax' or 'nojs' support.
 */
function acquia_lift_option_set_add_modal_callback($ajax) {
  if (!$ajax) {
    drupal_goto('admin/structure/personalize/variations');
    return;
  }

  // Show the option set type selection form in a modal.
  ctools_include('modal');
  ctools_modal_add_js();

  $path = drupal_get_path('module', 'acquia_lift');
  $output = theme('acquia_lift_type_list', array(
    'id' => 'acquia-lift-option-set-type-list',
    'items' => array(
      'personalize-element' => array(
        'title' => t('Webpage elements'),
        'description' => t('Create a variation set of personalized webpage elements, including a base and control version. Useful for quick tests and personalization on individual webpages.'),
        'path' => 'admin/structure/personalize/variations/personalize-elements/add',
        'modal' => FALSE,
        'logo' => theme('image', array(
          'path' => $path . '/images/variation-type-element.png',
          'alt' => t('Webpage element'),
          'title' => t('Select this option to create a variation set from web page elements.'),
        )),
      ),
      'personalize-block' => array(
        'title' => t('Drupal blocks'),
        'description' => t('Create a variation set of personalized Drupal blocks of content. Useful for personalization and testing across your website where this block appears.'),
        'path' => 'admin/structure/personalize/variations/personalize-blocks/add',
        'modal' => FALSE,
        'logo' => theme('image', array(
          'path' => $path . '/images/variation-type-block.png',
          'alt' => t('Drupal block'),
          'title' => t('Select this option to create a variation set from Drupal content blocks.'),
        )),
      ),
    ),
  ));
  return ctools_modal_render(t('Add a variation set'), $output);
}

/***************************************************************
 *
 * P A G E  V A R I A T I O N  R E N A M E
 *
 ***************************************************************/

/**
 * Page callback to generate a page variation rename modal window.
 *
 * @param string $agent_name
 *   The name of the campaign to edit.
 * @param string $variation_index
 *   The index of the variation to rename.
 */
function acquia_lift_page_variation_rename_modal_callback($agent_name, $variation_index) {
  acquia_lift_create_ctools_form(
    t('Rename variation'),
    'acquia_lift_page_variation_rename_form',
    array(
      'agent_name' => $agent_name,
      'variation_index' => $variation_index,
    ),
    'acquia_lift_page_variation_rename_complete_callback'
  );
}

/**
 * Ctools form processing complete handler for the renaming of a page
 * variation.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_page_variation_rename_complete_callback($form, $form_state) {
  $settings = array();
  $option_sets = personalize_option_set_load_multiple(FALSE, array('decision_name' => $form_state['values']['variation_set_name']));
  $option_set_settings = array_map('_personalize_convert_option_set_to_js_setting', $option_sets);
  foreach($option_set_settings as $option_set_setting) {
    $settings[key($option_set_setting)] = reset($option_set_setting);
  }
  // We need to update the campaigns as well in case a campaign was paused by this action.
  $campaign_settings['acquia_lift']['campaigns'] = acquia_lift_get_campaign_details(variable_get('acquia_lift_unibar_allow_status_change', TRUE));
  $commands = array(
    ctools_modal_command_dismiss(),
    acquia_lift_command_messagebox(t('The variation has been renamed.'), 10),
    acquia_lift_command_option_set_updates($settings),
    ajax_command_settings($campaign_settings, TRUE),
    acquia_lift_command_process_queue(),
  );
  return $commands;
}

/**
 * Form handler to generate a form to change a variation name.
 *
 * This form is called within a ctools modal window.
 *
 * @param string $agent_name
 *   The name of the campaign to edit.
 * @param string $variation_index
 *   The index of the variation to rename.
 */
function acquia_lift_page_variation_rename_form($form, &$form_state, $agent_name, $variation_index) {
  $variation_set_name = $agent_name;

  ctools_include('modal');
  ctools_include('ajax');
  ctools_add_js('ajax-responder');

  module_load_include('inc', 'acquia_lift', 'acquia_lift.page_variations');

  $form = array();
  $form['agent_name'] = array(
    '#type' => 'value',
    '#value' => $agent_name,
  );
  $form['variation_set_name'] = array(
    '#type' => 'value',
    '#value' => $variation_set_name,
  );
  $form['variation_index'] = array(
    '#type' => 'value',
    '#value' => $variation_index,
  );
  if ($variation_index == 0) {
    $form['message'] = array(
      '#markup' => t('The control variation cannot be renamed.'),
    );
    return $form;
  }
  $form['variation_name'] = array(
    '#type' => 'textfield',
    '#title' => t('New name'),
    '#default_value' => acquia_lift_page_variation_get_name($agent_name, $agent_name, $variation_index),
    '#required' => TRUE,
    '#maxlength' => PERSONALIZE_MACHINE_NAME_MAXLENGTH,
  );
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Rename'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Submit handler for acquia_lift_page_variation_rename_form().
 */
function acquia_lift_page_variation_rename_form_submit($form, &$form_state) {
  module_load_include('inc', 'acquia_lift', 'acquia_lift.page_variations');
  acquia_lift_page_variation_rename($form_state['values']['variation_set_name'], $form_state['values']['agent_name'], $form_state['values']['variation_index'], $form_state['values']['variation_name']);
}

/***************************************************************
 *
 * P A G E  V A R I A T I O N  D E L E T E
 *
 ***************************************************************/

/**
 * Page callback to generate a page variation delete confirm modal window.
 *
 * @param string $agent_name
 *   The name of the campaign to edit.
 * @param string $variation_index
 *   The index of the variation to rename.
 */
function acquia_lift_page_variation_delete_modal_callback($agent_name, $variation_index) {
  acquia_lift_create_ctools_form(
    t('Delete variation'),
    'acquia_lift_page_variation_delete_form',
    array(
      'agent_name' => $agent_name,
      'variation_index' => $variation_index,
    ),
    'acquia_lift_page_variation_delete_complete_callback'
  );
}

/**
 * Ctools form processing complete handler for deletion of a page variation.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_page_variation_delete_complete_callback($form, $form_state) {
  $settings = array();
  $option_sets = personalize_option_set_load_multiple(FALSE, array('decision_name' => $form_state['values']['variation_set_name']));
  if (empty($option_sets)) {
    $settings['empty'] = $form_state['values']['agent_name'];
  }
  else {
    $option_set_settings = array_map('_personalize_convert_option_set_to_js_setting', $option_sets);
    foreach ($option_set_settings as $option_set_setting) {
      $settings[key($option_set_setting)] = reset($option_set_setting);
    }
  }
  $campaign_settings['acquia_lift']['campaigns'] = acquia_lift_get_campaign_details(variable_get('acquia_lift_unibar_allow_status_change', TRUE));
  $commands = array(
    ajax_command_settings($campaign_settings, TRUE),
    ctools_modal_command_dismiss(),
    acquia_lift_command_messagebox(t('The variation has been deleted.'), 10),
    acquia_lift_command_option_set_updates($settings),
    acquia_lift_command_process_queue(),
  );
  return $commands;
}

/**
 * Form handler to generate a confirmation form to delete a variation.
 *
 * This form is called within a ctools modal window.
 *
 * @param string $agent_name
 *   The name of the campaign to edit.
 * @param string $variation_index
 *   The index of the variation to rename.
 */
function acquia_lift_page_variation_delete_form($form, &$form_state, $agent_name, $variation_index) {
  module_load_include('inc', 'acquia_lift', 'acquia_lift.page_variations');
  $variation_set_name = $agent_name;
  $variation_name = acquia_lift_page_variation_get_name($variation_set_name, $agent_name, $variation_index);

  ctools_include('modal');
  ctools_include('ajax');
  ctools_add_js('ajax-responder');

  $form = array();
  $form['agent_name'] = array(
    '#type' => 'value',
    '#value' => $agent_name,
  );
  $form['variation_set_name'] = array(
    '#type' => 'value',
    '#value' => $variation_set_name,
  );
  $form['variation_index'] = array(
    '#type' => 'value',
    '#value' => $variation_index,
  );
  if ($variation_index == 0) {
    $form['message'] = array(
      '#markup' => t('The control variation cannot be renamed.'),
    );
    return $form;
  }
  $form['confirm'] = array(
    '#prefix' => '<div>',
    '#suffix' => '</div>',
    '#markup' => t('Are you sure you want to delete %variation?', array(
      '%variation' => $variation_name,
    )),
  );
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Delete'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Submit handler for acquia_lift_page_variation_delete_form().
 */
function acquia_lift_page_variation_delete_form_submit($form, &$form_state) {
  module_load_include('inc', 'acquia_lift', 'acquia_lift.page_variations');
  acquia_lift_page_variation_delete($form_state['values']['variation_set_name'], $form_state['values']['agent_name'], $form_state['values']['variation_index']);
}

/***************************************************************
 *
 * E L E M E N T  V A R I A T I O N  D E L E T E
 *
 ***************************************************************/

/**
 * Page callback to generate an element variation delete confirm modal window.
 *
 * @param string $osid
 *   The stringified option set id
 * @param string $option_id
 *   The id of the option to be deleted.
 */
function acquia_lift_element_variation_delete_modal_callback($osid, $option_id) {
  acquia_lift_create_ctools_form(
    t('Delete variation'),
    'acquia_lift_element_variation_delete_form',
    array(
      'osid' => personalize_resolve_stringified_osid($osid),
      'option_id' => $option_id,
    ),
    'acquia_lift_element_variation_delete_complete_callback'
  );
}

/**
 * Ctools form processing complete handler for deletion of an element variation.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_element_variation_delete_complete_callback($form, $form_state) {
  $settings = array();
  $form_option_set = $form_state['values']['option_set'];
  $osid = $form_option_set->osid;

  $option_sets = personalize_option_set_load_by_agent($form_state['values']['agent_name']);
  $updated = NULL;
  foreach ($option_sets as $option_set) {
    if ($option_set->osid === $osid) {
      $updated = $option_set;
      break;
    }
  }
  if ($updated) {
    // The variation has been deleted, but the option set still has more.
    $message = t('The variation has been deleted.');
    $settings = _personalize_convert_option_set_to_js_setting($updated);
  }
  else {
    $message = t('The variation set has been deleted.');
    if (empty($option_sets)) {
      // There are no more option sets in the agent.
      $settings['empty'] = $form_state['values']['agent_name'];
    }
    else {
      // The last variation was removed from the option set, but the agent
      // still has other option sets.
      $settings[personalize_stringify_osid($osid)] = 'empty';
    }
  }
  $campaign_settings['acquia_lift']['campaigns'] = acquia_lift_get_campaign_details(variable_get('acquia_lift_unibar_allow_status_change', TRUE));
  $commands = array(
    ajax_command_settings($campaign_settings, TRUE),
    ctools_modal_command_dismiss(),
    acquia_lift_command_messagebox($message, 10),
    acquia_lift_command_option_set_updates($settings),
    acquia_lift_command_process_queue(),
  );
  return $commands;
}

/**
 * Form handler to generate a confirmation form to delete an element variation.
 *
 * This form is called within a ctools modal window.
 *
 * @param string $agent_name
 *   The name of the campaign to edit.
 * @param int $osid
 *   The id of the option set
 * @param string $option_id
 *   The id of the option/variation.
 */
function acquia_lift_element_variation_delete_form($form, &$form_state, $osid, $option_id) {
  ctools_include('modal');
  ctools_include('ajax');
  ctools_add_js('ajax-responder');

  $option_set = personalize_option_set_load($osid);
  $variation_name = $option_id;
  foreach ($option_set->options as $option) {
    if ($option['option_id'] === $option_id) {
      $variation_name = $option['option_label'];
      break;
    }
  }

  $form = array();
  $form['agent_name'] = array(
    '#type' => 'value',
    '#value' => $option_set->agent,
  );
  $form['option_set'] = array(
    '#type' => 'value',
    '#value' => $option_set,
  );
  $form['option_id'] = array(
    '#type' => 'value',
    '#value' => $option_id,
  );
  if ($option_id == PERSONALIZE_CONTROL_OPTION_ID) {
    $form['message'] = array(
      '#markup' => t('The control variation cannot be renamed.'),
    );
    return $form;
  }
  $form['confirm'] = array(
    '#prefix' => '<div>',
    '#suffix' => '</div>',
    '#markup' => t('Are you sure you want to delete %variation?', array(
      '%variation' => $variation_name,
    )),
  );
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Delete'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Submit handler for acquia_lift_page_variation_delete_form().
 */
function acquia_lift_element_variation_delete_form_submit($form, &$form_state) {
  $option_set = $form_state['values']['option_set'];
  foreach ($option_set->options as $key => $option) {
    if ($option['option_id'] === $form_state['values']['option_id']) {
      unset($option_set->options[$key]);
      break;
    }
  }
  // @todo: This logic should be moved into personalize.
  if (count($option_set->options) < 2) {
    personalize_option_set_delete($option_set->osid);
  }
  else {
    personalize_option_set_save($option_set);
  }
}

/***************************************************************
 *
 * G O A L  C R E A T E
 *
 ***************************************************************/

/**
 * Page callback to create a new goal by selecting the type of goal to create.
 *
 * @param bool $ajax
 *   Indicates if the form is called with ajax support.
 */
function acquia_lift_goal_create_modal_callback($ajax) {
  if (!$ajax) {
    drupal_goto('admin/structure/personalize/goals');
    return;
  }

  // Show the goal type selection form in a modal.
  ctools_include('modal');
  ctools_modal_add_js();

  $path = drupal_get_path('module', 'acquia_lift');
  $output = theme('acquia_lift_type_list', array(
    'id' => 'acquia-lift-modal-goal-type-list',
    'items' => array(
      'existing' => array(
        'title' => t('Predefined goal'),
        'description' => t('Visitor actions already defined by Acquia Lift'),
        'path' => 'admin/structure/acquia_lift/goal/add/existing',
        'modal' => TRUE,
        'ctools_style' => 'ctools-modal-acquia-lift-style',
        'logo' => theme('image', array(
          'path' => $path . '/images/goal-type-predefined.png',
          'alt' => t('Pre-existing goal'),
          'title' => t('Select this option to create a goal from a pre-existing visitor action.'),
        )),
      ),
      'element' => array(
        'title' => t('New element goal'),
        'description' => t('Element-level visitor action, such as clicking a link or hovering over an image'),
        'path' => 'admin/structure/visitor_actions',
        'modal' => FALSE,
        'logo' => theme('image', array(
          'path' => $path . '/images/goal-type-element.png',
          'alt' => t('Element goal'),
          'title' => t('Select this option to create an element goal.'),
        ))
      ),
      'page' => array(
        'title' => t('New page goal'),
        'description' => t('Page-level visitor action, such as scrolling to the bottom of the page'),
        'path' => 'admin/structure/acquia_lift/goal/add/page',
        'modal' => TRUE,
        'ctools_style' => 'ctools-modal-acquia-lift-style',
        'logo' => theme('image', array(
          'path' => $path . '/images/goal-type-page.png',
          'alt' => t('Page goal'),
          'title' => t('Select this option to create a page goal.'),
        ))
      ),
    ),
  ));
  return ctools_modal_render(t('Add a goal'), $output);
}

/**
 * Page callback to generate the ctools modal form to create a goal of a
 * specific type.
 *
 * @param $type
 *   The type of goal to create.  Current supported choices: 'existing', 'page'
 *   Note that element is handled via JavaScript.
 * @param $ajax
 *   Indicates if AJAX is supported.
 */
function acquia_lift_goal_type_create_modal_callback($type, $ajax) {
  acquia_lift_create_ctools_form(
    t('Add a goal'),
    'acquia_lift_goal_type_create_form',
    array('type' => $type),
    'acquia_lift_goal_type_create_completed_callback'
  );
}

/**
 * Ctools form processing complete handler for the creation of a new goal by
 * type.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_goal_type_create_completed_callback($form, &$form_state) {
  $agent = $form_state['values']['agent'];
  $include_status = variable_get('acquia_lift_unibar_allow_status_change', TRUE);
  $settings = acquia_lift_get_campaign_details($include_status);
  if ($settings[$agent->machine_name]['verified'] && personalize_agent_get_status($agent->machine_name) === PERSONALIZE_STATUS_NOT_STARTED) {
    $message = t('Goal added.  You can now !start the campaign.', array('!start' => $include_status ? t('start') : l(t('start'), 'admin/structure/personalize/' . $agent->machine_name . '/edit')));
  }
  else {
    $message = t('!goal goal added to campaign.', array('!goal' => filter_xss($form_state['acquia_lift']['goal_name'])));
  }
  // Load updated data about custom defined visitor actions.
  $actions = visitor_actions_custom_load_multiple();

  $commands = array(
    ctools_modal_command_dismiss(),
    ajax_command_settings(array(
      'acquia_lift' => array(
        'campaigns' => $settings,
        'customActions' => $actions,
      )
    ), TRUE),
    acquia_lift_command_messagebox($message, 10),
    acquia_lift_command_process_queue(),
  );
  return $commands;
}

/**
 * Form builder function to create a new goal in the modal process of a
 * specific type.
 *
 * @param type
 *   The type of goal to create.  Current supported choices: 'page', 'existing'
 */
function acquia_lift_goal_type_create_form($form, &$form_state, $type) {
  ctools_include('modal');
  ctools_include('ajax');
  ctools_add_js('ajax-responder');

  $agent_name = personalize_get_campaign_context();
  $agent = personalize_agent_load($agent_name);
  if (empty($agent)) {
    $form['message'] = array(
      '#markup' => t('You must select a campaign before adding goals.'),
    );
    return $form;
  }

  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent,
  );
  $form['goal_type'] = array(
    '#type' => 'value',
    '#value' => $type,
  );
  $change_link = ctools_modal_text_button(t('Change type of goal'), 'admin/structure/acquia_lift/goal/add/nojs', t('Change type of test'), 'ctools-modal-acquia-lift-style');
  // Add individual forms specific to the type of goal being created.
  switch ($type) {
    case 'existing':
      $form['goal_type_change'] = array(
        '#markup' => theme('acquia_lift_create_type_change', array(
          'type' => t('Pre-existing goal'),
          'id' => 'acquia-lift-create-goal-existing',
          'change_link' => $change_link,
        )),
      );
      $form['goal'] = _acquia_lift_existing_goal_create_form($agent);
      break;
    case 'page':
      $form['goal_type_change'] = array(
        '#markup' => theme('acquia_lift_create_type_change', array(
          'type' => t('Page goal'),
          'id' => 'acquia-lift-create-goal-page',
          'change_link' => $change_link,
        )),
      );
      $form['goal'] = _acquia_lift_page_goal_create_form($agent);
      break;
  }
  // Common actions.
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Add goal'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Loads the existing goal selection form for a campaign.
 *
 * @param $agent
 *   The agent data for the campaign that this goal will belong to.
 */
function _acquia_lift_existing_goal_create_form($agent) {
  // Exclude any actions already used in this campaign.
  $exclude = array();
  $goals = personalize_goal_load_by_conditions(array('agent' => $agent->machine_name));
  foreach ($goals as $goal) {
    $exclude[$goal->action] = $goal->action;
  }
  module_load_include('inc', 'personalize', 'personalize.admin');
  $all_goal_options = personalize_goals_options(TRUE);
  $goal_options = array_diff_key($all_goal_options, $exclude);
  $form['action_name'] = array(
    '#type' => 'select',
    '#title' => t('Goal'),
    '#options' => $goal_options,
    '#empty_option' => t('-Select-'),
    '#required' => TRUE,
  );
  return $form;
}

/**
 * Loads the page goals selection form for a campaign.
 *
 * @param $agent
 *   The agent data for the campaign that this goal will belong to.
 */
function _acquia_lift_page_goal_create_form($agent) {
  $form['actionable_element'] = array(
    '#type' => 'value',
    '#value' => 'page',
  );
  $form['title'] = array(
    '#title' => t('Title'),
    '#description' => t('Name'),
    '#type' => 'textfield',
    '#required' => TRUE,
  );
  // To be filled in on the client-side.
  $form['pages'] = array(
    '#type' => 'hidden',
  );
  // Needed by the visitor actions form for structure.
  $form['event'] = array(
    '#tree' => TRUE
  );
  $form['options'] = array(
    '#tree' => TRUE,
  );

  $form['#validate'][] = '_acquia_lift_page_goal_create_form_validate';
  module_load_include('inc', 'visitor_actions', 'visitor_actions.admin');
  visitor_actions_get_form_options('page', $form, FALSE);
  return $form;
}

/**
 * Form validation for creating a new page goal simplified form.
 */
function _acquia_lift_page_goal_create_form_validate($form, &$form_state) {
  // Allow the plugin to validate the form values.
  ctools_include('plugins');
  if ($class = ctools_plugin_load_class('visitor_actions', 'actionable_element', $form_state['values']['actionable_element'], 'handler')) {
    call_user_func_array(array($class, 'validate'), array($form_state['values']));
  }
}

/**
 * Submit handler to create a new goal for a campaign.
 */
function acquia_lift_goal_type_create_form_submit($form, &$form_state) {
  $action_name = $goal_name = '';
  $agent_data = $form_state['values']['agent'];

  switch ($form_state['values']['goal_type']) {
    case 'existing':
      $action_name = $form_state['values']['action_name'];
      $goal_name = $form['goal']['action_name']['#options'][$action_name];
      break;
    case 'page':
      $plugin = $form_state['values']['actionable_element'];
      list($type, $event) = explode('::', $form_state['values']['event'][$plugin]);
      $goal_name = $form_state['values']['title'];
      $action_name = personalize_generate_machine_name($goal_name, 'visitor_actions_action_name_exists', '_');
      $action = array(
        'label' => $goal_name,
        'machine_name' => $action_name,
        'plugin' => $plugin,
        'client_side' => $type == 'client' ? 1 : 0,
        'identifier' => '',
        'event' => $event,
        'pages' => $form_state['values']['pages'],
        // The data array gets populated with the plugin-specific
        // info for the action.
        'data' => isset($form_state['values']['options'][$plugin]) ? $form_state['values']['options'][$plugin] : array(),
      );
      // Allow the plugin to modify the action before saving.
      ctools_include('plugins');
      if ($class = ctools_plugin_load_class('visitor_actions', 'actionable_element', $action['plugin'], 'handler')) {
        $action = call_user_func_array(array($class, 'actionPresave'), array($action));
      }
      visitor_actions_save_action($action);
      break;
  }
  try {
    personalize_goal_save($agent_data->machine_name, $action_name, 1);
  } catch (Exception $e) {
    drupal_set_message($e->getMessage(), 'error');
  }
  $form_state['acquia_lift']['goal_name'] = $goal_name;
}

/***************************************************************
 *
 * G O A L  R E N A M E
 *
 ***************************************************************/

/**
 * Page callback to generate a goal rename modal window.
 *
 * @param string $action_name
 *   The machine name for the visitor action for the goal.
 */
function acquia_lift_goal_rename_modal_callback($action_name) {
  $action = visitor_actions_custom_load($action_name);
  acquia_lift_create_ctools_form(
    t('Rename goal'),
    'acquia_lift_goal_rename_form',
    array(
      'action' => $action,
    ),
    'acquia_lift_goal_rename_complete_callback'
  );
}

/**
 * Ctools form processing complete handler for the renaming of a goal.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_goal_rename_complete_callback($form, $form_state) {
  $settings = acquia_lift_get_campaign_details(variable_get('acquia_lift_unibar_allow_status_change', TRUE));
  $message = $form_state['acquia_lift']['goal_rename_success'] ? t('The goal has been renamed.') : t('There was a problem renaming the goal.');
  $commands = array(
    ctools_modal_command_dismiss(),
    ajax_command_settings(array('acquia_lift' => array(
      'campaigns' => $settings,
    )), TRUE),
    acquia_lift_command_messagebox($message, 10),
  );
  return $commands;
}

/**
 * Form handler to generate a form to change a goal name.
 *
 * This form is called within a ctools modal window.
 *
 * @param array $action
 *   The goal's underlying visitor action.
 */
function acquia_lift_goal_rename_form($form, &$form_state, $action) {
  ctools_include('modal');
  ctools_include('ajax');
  ctools_add_js('ajax-responder');

  $form = array();
  $form['action'] = array(
    '#type' => 'value',
    '#value' => $action,
  );
  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('New name'),
    '#default_value' => $action['label'],
    '#required' => TRUE,
    '#maxlength' => PERSONALIZE_MACHINE_NAME_MAXLENGTH,
  );
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Rename'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Submit handler for acquia_lift_goal_rename().
 */
function acquia_lift_goal_rename_form_submit($form, &$form_state) {
  $action = $form_state['values']['action'];
  $action['label'] = $form_state['values']['label'];
  // Limit those fields to save based on those available in the custom table.
  $schema = drupal_get_schema('visitor_actions_actions');
  $action = array_intersect_key($action, $schema['fields']);
  $form_state['acquia_lift']['goal_rename_success'] = visitor_actions_save_action($action);
}

/***************************************************************
 *
 * G O A L  D E L E T E
 *
 ***************************************************************/


/**
 * Page callback to generate a goal delete confirm modal window.
 *
 * @param string $agent_name
 *   The name of the campaign to edit.
 * @param string $action_name
 *   The machine name for the visitor action for the goal.
 */
function acquia_lift_goal_delete_modal_callback($agent_name, $action_name) {
  acquia_lift_create_ctools_form(
    t('Delete goal'),
    'acquia_lift_goal_delete_form',
    array(
      'agent_name' => $agent_name,
      'action_name' => $action_name,
    ),
    'acquia_lift_goal_delete_complete_callback'
  );
}

/**
 * Ctools form processing complete handler for deletion of a goal.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_goal_delete_complete_callback($form, $form_state) {
  $message = $form_state['acquia_lift']['goal_delete_success'] ? t('The goal has been deleted.') : t('There was a problem deleting the goal.');
  $commands = array(
    ctools_modal_command_dismiss(),
    acquia_lift_command_messagebox($message, 10),
    acquia_lift_command_goal_updates($form_state['values']['agent_name']),
    acquia_lift_command_process_queue(),
  );
  return $commands;
}

/**
 * Form handler to generate a confirmation form to delete a goal.
 *
 * This form is called within a ctools modal window.
 *
 * @param string $agent_name
 *   The name of the campaign to edit.
 * @param string $action_name
 *   The machine name for the visitor action for the goal.
 */
function acquia_lift_goal_delete_form($form, &$form_state, $agent_name, $action_name) {
  ctools_include('modal');
  ctools_include('ajax');
  ctools_add_js('ajax-responder');

  $form = array();
  // Will only load if this is a custom action (not defined in code).
  $action = visitor_actions_custom_load($action_name);
  $form['agent_name'] = array(
    '#type' => 'value',
    '#value' => $agent_name,
  );
  $form['action_name'] = array(
    '#type' => 'value',
    '#value' => $action_name,
  );
  $form['confirm'] = array(
    '#prefix' => '<div>',
    '#suffix' => '</div>',
    '#markup' => t('Are you sure you want to delete %goal?', array(
      '%goal' => isset($action['label']) ? $action['label'] : $action_name,
    )),
  );
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Delete'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Submit handler for acquia_lift_goal_delete_form().
 */
function acquia_lift_goal_delete_form_submit($form, &$form_state) {
  $goals = personalize_goal_load_by_conditions(array(
    'agent' => $form_state['values']['agent_name'],
    'action' => $form_state['values']['action_name'],
  ));
  if (count($goals) === 1) {
    $goal = reset($goals);
    personalize_goal_delete($goal->id);
    $form_state['acquia_lift']['goal_delete_success'] = TRUE;
  }
  else {
    $form_state['acquia_lift']['goal_delete_success'] = FALSE;
  }
}

/***************************************************************
 *
 * C A M P A I G N  S T A R T  F O R M
 *
 ***************************************************************/


/**
 * Page callback to generate a start campaign screen.
 */
function acquia_lift_campaign_start_modal_callback($agent_name) {
  $agent_data = personalize_agent_load($agent_name);
  acquia_lift_create_ctools_form(
    t('Start campaign %campaign', array('%campaign' => $agent_data->label)),
    'acquia_lift_campaign_start_form',
    array('agent_data' => $agent_data),
    'acquia_lift_campaign_start_complete_callback'
  );
}

/**
 * Form handler to start a new campaign.
 *
 * @param stdClass $agent_data
 *   The data for the agent to start.
 */
function acquia_lift_campaign_start_form($form, &$form_state, $agent_data) {
  ctools_include('modal');
  ctools_include('ajax');
  ctools_add_js('ajax-responder');

  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent_data,
  );
  $form['adaptive'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically determine a winner'),
    '#description' => t('Acquia Lift will adapt to visitors to choose the best option over time.'),
    '#default_value' => $agent_data->data['decision_style'] == 'adaptive',
  );
  module_load_include('inc', 'personalize', 'personalize.admin');
  $form['campaign_dates'] = personalize_agent_date_form($agent_data->machine_name);
  // Rather than "Start manually", the option is really to start it now since
  // the user has triggered this via a "start campaign" form.
  $form['campaign_dates']['campaign_start']['#options']['none'] = t('Start now');

  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'acquia-lift-submit-button'),
    ),
    '#value' => t('Start campaign'),
  );
  $form['actions']['reset'] = array(
    '#markup' => ctools_ajax_text_button(t('Cancel'), 'admin/structure/acquia_lift/cancel/nojs', t('Cancel'), 'acquia-lift-cancel-button'),
  );
  return $form;
}

/**
 * Form validator for acquia_lift_campaign_start_form().
 */
function acquia_lift_campaign_start_form_validate($form, &$form_state) {
  module_load_include('inc', 'personalize', 'personalize.admin');
  personalize_agent_date_form_validate($form_state['values']);
}

/**
 * Form submit handler for acquia_lift_campaign_start_form().
 */
function acquia_lift_campaign_start_form_submit($form, &$form_state) {
  // Set the decision style.
  $agent_data = $form_state['values']['agent'];
  $agent_data->data['decision_style'] = empty($form_state['values']['adaptive']) ? 'random' : 'adaptive';
  personalize_agent_save($agent_data);

  // Set the end date.
  module_load_include('inc', 'personalize', 'personalize.admin');
  personalize_agent_date_form_submit($agent_data->machine_name, $form_state['values']);

  // Start the campaign.
  $form_state['acquia_lift']['started'] = FALSE;
  if ($form_state['values']['campaign_start'] == 'none') {
    $form_state['acquia_lift']['started'] = personalize_agent_set_status($agent_data->machine_name, PERSONALIZE_STATUS_RUNNING);
  }
}

/**
 * Form complete callback for the acquia_lift_campaign_start_form form.
 * @see acquia_lift_create_ctools_form().
 */
function acquia_lift_campaign_start_complete_callback($form, $form_state) {
  $agent_data = $form_state['values']['agent'];

  // Determine the message to display.
  if ($form_state['acquia_lift']['started']) {
    $message = t('The %campaign campaign has been started', array(
      '%campaign' => $agent_data->label,
    ));
  }
  else {
    if ($form_state['values']['campaign_start'] == 'specified') {
      $start_variable = _personalize_agent_get_starttime_variable($agent_data->machine_name);
      $start_date = variable_get($start_variable, 0);
      $message = t('The %campaign campaign will be started on %date', array(
        '%campaign' => $agent_data->label,
        '%date' => format_date($start_date, 'custom', 'M d, Y'),
      ));
    }
    else {
      $message = t('There was a problem starting your campaign.');
    }
  }
  return array(
    ctools_modal_command_dismiss(),
    ajax_command_settings(array('acquia_lift' => array(
      'campaigns' => acquia_lift_get_campaign_details(variable_get('acquia_lift_unibar_allow_status_change', TRUE)),
    )), TRUE),
    acquia_lift_command_messagebox($message, 10),
  );
}

/***************************************************************
 *
 * G E N E R A L  D I A L O G  F U N C T I O N S
 *
 ***************************************************************/

/**
 * Handles generating a CTools form window for a particular form.

 * @param string $title
 *   The title for the modal window.
 * @param $form_name
 *   The name of the form to retrieve.
 * @param null $args
 *   An array of form arguments to pass to the form.
 * @param null $callback
 *   A callback to call when the form has been executed.
 *   The callback will receive the $form and $form_state arguments and should
 *   return the array of AJAX commands to execute.
 *   If empty then the modal will simply be dismissed.
 */
function acquia_lift_create_ctools_form($title, $form_name, $args = array(), $callback = NULL) {
  ctools_include('modal');
  ctools_include('ajax');
  ctools_modal_add_js();
  ctools_js_load('ajax-responder');

  $form_state = array(
    'ajax' => TRUE,
    'title' => $title,
    'build_info' => array(
      'args' => $args,
    )
  );

  $output = ctools_modal_form_wrapper($form_name, $form_state);

  if ($form_state['executed']) {
    // Form has been processed to generate the output commands.
    if (is_callable($callback)) {
      $output = $callback($output, $form_state);
    } else {
      $output = array(
        ctools_modal_command_dismiss(),
      );
    }
  }
  personalize_ajax_commands_deliver($output);
}

/**
 * AJAX callback to cancel the campaign creation flow.
 *
 * @param $ajax
 *   Indicates if AJAX is supported.
 */
function acquia_lift_campaign_flow_cancel($ajax = NULL) {
  if (!$ajax) {
    // We should never be here out of ajax context.
    return MENU_NOT_FOUND;
  }

  ctools_include('modal');
  ctools_include('ajax');

  $commands = array(ctools_modal_command_dismiss());
  print ajax_render($commands);
}

/**
 * Ajax callback to update an agent's status.
 *
 * @param $agent
 *   The agent to update.
 * @param $status
 *   The new status to set.
 */
function acquia_lift_agent_set_status_ajax($agent, $status) {
  $updated = personalize_agent_set_status($agent->machine_name, $status);
  $return = array('success' => $updated);
  if ($updated) {
    list($next_status, $text) = _personalize_status_toggle_next($status);
    $return['nextStatus'] = array(
      'status' => $next_status,
      'text' => $text,
    );
    $return['currentStatus'] = $status;
  }
  return drupal_json_output($return);
}
