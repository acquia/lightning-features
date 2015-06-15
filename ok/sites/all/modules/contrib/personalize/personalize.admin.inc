<?php

/**
 * @file personalize.admin.inc
 * Provides functions needed for the admin UI.
 */

/**
 * Admin form for configuring personalization backends.
 */
function personalize_admin_form($form, &$form_state) {
  $form['#attached']['css'][] = drupal_get_path('module', 'personalize') . '/css/personalize.admin.css';
  $form['cache'] = array(
    '#type' => 'fieldset',
    '#title' => t('Cache storage'),
    '#tree' => FALSE,
    '#attributes' => array(
      'class' => array('personalize-admin-cache-settings'),
    ),
  );
  $form['cache']['personalize_local_caching_storage'] = array(
    '#type' => 'select',
    '#title' => t('Clear decision cache'),
    '#options' => array(
      'session' => t('at end of session'),
      'local' => t('after ...'),
    ),
    '#default_value' => variable_get('personalize_local_caching_storage', 'session'),
  );
  $form['cache']['personalize_local_caching_expiration'] = array(
    '#type' => 'textfield',
    '#default_value' => variable_get('personalize_local_caching_expiration', 30),
    '#field_suffix' => t('minutes'),
    '#size' => 5,
    '#states' => array(
      'visible' => array(
        ':input[name="personalize_local_caching_storage"]' => array('value' => 'local'),
      ),
    ),
  );
  $form['personalize_use_admin_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use admin mode'),
    '#description' => t('When this box is checked, users with the "Manage personalized content" permission will not trigger any personalization activity such as requests for decisions or the sending of goals.'),
    '#default_value' => variable_get('personalize_use_admin_mode', TRUE),
  );
  $form['personalize_enable_debug_mode'] = array(
    '#type' => 'checkbox',
    '#title' => t('Debug mode'),
    '#description' => t('Select this check box to display Acquia Lift diagnostic information in certain browser consoles. !learnmore.', array('!learnmore' => l(t('Learn more'), 'https://docs.acquia.com/lift/drupal/debug', array('attributes' => array('target' => '_blank'))))),
    '#default_value' => variable_get('personalize_enable_debug_mode', FALSE),
  );
  // Add visitor context configuration.
  $settings = variable_get('personalize_visitor_context_disabled', array());
  $form['personalize_visitor_context_disabled'] = personalize_admin_build_visitor_context_select($settings, FALSE);
  $form['personalize_visitor_context_disabled']['#title'] = t('Disallowed visitor context items');
  $form['personalize_visitor_context_disabled']['#description'] = t('Select which visitor context items should never show up when configuring visitor context for campaigns.');

  // Add our own submit handler to run BEFORE settings form handler.
  $form['#submit'][] = 'personalize_admin_form_submit';

  return system_settings_form($form);
}

/**
 * Validation callback for the settings form.
 */
function personalize_admin_form_validate($form, &$form_state) {
  // Perform some slight re-organization of the values because the visitor context
  // settings need to be shuffled around.
  if (isset($form_state['values']['personalize_visitor_context_disabled'])) {
    $form_state['values']['personalize_visitor_context_disabled'] = personalize_admin_convert_visitor_context_form_values($form_state['values']['personalize_visitor_context_disabled']);
  }
}

/**
 * Submit callback for settings form.
 */
function personalize_admin_form_submit($form, &$form_state) {
  // Clear the visitor context cache settings if the allowed contexts changed.
  // This submit callback is called BEFORE the system_settings_form callback.
  if (isset($form_state['values']['personalize_visitor_context_disabled']) && $form_state['values']['personalize_visitor_context_disabled'] != variable_get('personalize_visitor_context_disabled')) {
    personalize_visitor_context_expiration_clear();
  }
}

/**
 * Form for changing the status of an agent.
 *
 * @param $agent_name
 *   THe name of the agent the form is for.
 * @param array $agent_status
 *   The current status of the agent.
 * @return array
 *   Array representing the status change form.
 */
function personalize_status_change_form($form, &$form_state, $agent_name, $agent_status) {
  $friendly_statuses = personalize_get_agent_status_map();
  $form = array();
  $form['agent_name'] = array(
    '#type' => 'value',
    '#value' => $agent_name,
  );
  switch($agent_status) {
    case PERSONALIZE_STATUS_NOT_STARTED:
      // Just present a start button, no need for a dropdown.
      $form['status'] = array(
        '#type' => 'value',
        '#value' => PERSONALIZE_STATUS_RUNNING
      );
      $submit_text = t('Start');
      break;
    case PERSONALIZE_STATUS_RUNNING:
      // Just present  Pause button, not need for a dropdown.
      $form['status'] = array(
        '#type' => 'value',
        '#value' => PERSONALIZE_STATUS_PAUSED
      );
      $submit_text = t('Pause');
      break;
    default:
      // Present a dropdown of all available next statuses.
      $allowed_statuses = personalize_allowed_status_transitions($agent_status);
      $options = array(
        '' => t('--Select--')
      );
      foreach ($allowed_statuses as $status) {
        $options[$status] = $friendly_statuses[$status];
      }
      $form['status'] = array(
        '#prefix' => '<div class="personalize-status-change-select">',
        '#suffix' => '</div>',
        '#type' => 'select',
        '#options' => $options,
        '#default_value' => '',
      );
      $submit_text = t('Change');
      break;
  }

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => $submit_text
  );
  return $form;
}

/**
 * Submit callback for the status change form.
 */
function personalize_status_change_form_submit(&$form, &$form_state) {
  $agent_name = $form_state['values']['agent_name'];
  if (!empty($form_state['values']['status'])) {
    $status = $form_state['values']['status'];
    if ($status == PERSONALIZE_STATUS_COMPLETED) {
      // Redirect to a confirm form as this has more serious implications than
      // other status changes.
      $form_state['redirect'] = array('admin/structure/personalize/manage/'. $agent_name .'/complete');
    }
    else {
      // Otherwise just change the status as requested.
      $status_map = personalize_get_agent_status_map();
      if (personalize_agent_set_status($agent_name, $status)) {
        drupal_set_message(t('Set the status of campaign @campaign to @status', array('@campaign' => $agent_name, '@status' => $status_map[$status])));
      }
    }
  }
}

/**
 * Returns a visitor context select box.
 *
 * @param $settings
 *   The existing visitor context settings, needed for default values.
 * @param $exclude_disallowed
 *   (optional) Exclude context options disallowed by admin settings.
 * @param $agent_data
 *   (optional) Object representing an existing agent if this is an edit form.
 * @return mixed
 *   An array representing a visitor_context dropdown form element or FALSE
 *     if no context options are available to show
 */
function personalize_admin_build_visitor_context_select($settings, $exclude_disallowed = TRUE, $agent_data = NULL) {
  $groups = personalize_get_grouped_context_options($agent_data, $exclude_disallowed);
  // We'll need an array of default values.
  $selected = array();
  // Count the number of items in total so we can determine the correct size of
  // the multi-select element.
  $group_count = 0;
  foreach ($groups as $group) {
    $group_count++;
    if (is_array($group)) {
      $group_count = $group_count + count($group);
      foreach ($group as $option_name => $friendly_name) {
        list($plugin_name, $code) = explode(PERSONALIZE_TARGETING_ADMIN_SEPARATOR, $option_name);
        // Build up our default values based on the settings passed in.
        if (isset($settings[$plugin_name][$code])) {
          $selected[$option_name] = $option_name;
        }
      }
    }
  }
  if ($group_count === 0) {
    return FALSE;
  }
  $element = array(
    '#type' => 'select',
    '#title' => t('Visitor contexts to use for automatic targeting'),
    '#options' => $groups,
    '#multiple' => TRUE,
    '#default_value' => $selected,
    '#size' => $group_count > 0 ? $group_count : 1,
  );
  return $element;
}

/**
 * Returns the available context options, sorted into groups.
 *
 * The array returned is suitable for use in a select element.
 *
 * @param null $agent
 *   An object representing an agent, if these settings apply to an agent.
 * @param bool $exclude_disallowed
 *   Whether to exclude any contexts that have been set as disallowed.
 * @param array $exclude_plugins
 *   An array of plugins to be excluded from the list.
 * @return array
 *   An associative array of available contexts where the keys are group names
 *   and each value is an associative array  of contexts with context name and
 *   friendly name as key and value respectively.
 */
function personalize_get_grouped_context_options($agent = NULL, $exclude_disallowed = TRUE, $exclude_plugins = array()) {
  ctools_include('plugins');
  $disallowed = $exclude_disallowed ? variable_get('personalize_visitor_context_disabled', array()) : array();
  $groups = array('Miscellaneous' => array());
  $contexts = personalize_get_visitor_contexts();
  foreach ($contexts as $plugin_name => $plugin_info) {
    if (in_array($plugin_name, $exclude_plugins)) {
      continue;
    }
    if ($class = ctools_plugin_load_class('personalize', 'visitor_context', $plugin_name, 'handler')) {
      if (!empty($agent)) {
        if (!call_user_func(array($class, 'allowedFromAgent'), $agent)) {
          continue;
        }
      }
      $context_options = call_user_func(array($class, 'getOptions'));
      // Organize the elements according to groups.
      foreach ($context_options as $code => $info) {
        if (isset($disallowed[$plugin_name][$code])) {
          continue;
        }
        $option_name = $plugin_name . PERSONALIZE_TARGETING_ADMIN_SEPARATOR . $code;
        if (isset($info['group'])) {
          $group = $info['group'];
          if (!isset($groups[$group])) {
            $groups[$group] = array();
          }
          $groups[$group][$option_name] = $info['name'];
        }
        else {
          $groups['Miscellaneous'][$option_name] = $info['name'];
        }
      }
    }
  }
  // Move the Miscellaneous group to the end.
  $misc = $groups['Miscellaneous'];
  unset($groups['Miscellaneous']);
  if (!empty($misc)) {
    $groups['Miscellaneous'] = $misc;
  }
  return $groups;
}

/**
 * Converts visitor context form values into the correct structure.
 *
 * @param $values
 *   The values that were submitted in the form.
 * @return array
 *   An array of properly structure visitor context settings.
 */
function personalize_admin_convert_visitor_context_form_values($values) {
  $visitor_context = array();
  // Visitor context form values are grouped by category. We want to
  // save them grouped by the plugin that provided them.
  $selected = array_filter($values);
  foreach ($selected as $name => $value) {
    // Element names are in the form {$plugin_name}__{$element_name}
    list($plugin_name, $element_name) = explode(PERSONALIZE_TARGETING_ADMIN_SEPARATOR, $name);
    if (!isset($visitor_context[$plugin_name])) {
      $visitor_context[$plugin_name] = array();
    }
    $visitor_context[$plugin_name][$element_name] = $element_name;
  }

  return $visitor_context;
}

/**
 * Menu callback for displaying a list of all agents.
 */
function personalize_agent_list() {
  $status_map = personalize_get_agent_status_map();

  $header = array(
    array('data' => t('Campaign name')),
    array('data' => t('Type')),
    array('data' => t('Change status')),
    array('data' => t('Operations'), 'colspan' => 4),
  );
  $agents = personalize_agent_load_multiple(array(), array(), FALSE, TRUE, 'label');
  if (empty($agents)) {
    return array(
      'no_agents' => array(
        '#markup' => '<div>' . t('No campaigns available.') . '</div>'
      )
    );
  }
  $sorted_agents = array(
    PERSONALIZE_STATUS_NOT_STARTED => array(),
    PERSONALIZE_STATUS_RUNNING => array(),
    PERSONALIZE_STATUS_PAUSED => array(),
    PERSONALIZE_STATUS_COMPLETED => array()
  );

  foreach ($agents as $agent) {
    $agent_status = personalize_agent_get_status($agent->machine_name);
    $sorted_agents[$agent_status][] = $agent;
  }

  $build = array();
  foreach ($sorted_agents as $status => $agents) {
    if (empty($agents)) {
      continue;
    }
    $description = '';
    switch($status) {
      case PERSONALIZE_STATUS_NOT_STARTED:
        $description = t('This table lists campaigns that have never been started.');
        break;
      case PERSONALIZE_STATUS_RUNNING:
        $description = t('This table lists campaigns that are displaying personalized content to website visitors for each variation set.');
        break;
      case PERSONALIZE_STATUS_PAUSED:
        $description = t('This table lists campaigns that use personalization JavaScript to display the fallback/winner variation for each variation set.');
        break;
      case PERSONALIZE_STATUS_COMPLETED:
        $description = t('This table lists campaigns that use PHP (whenever possible) to display the fallback/winner variation for each variation set, which can cause variations to display more quickly than those in paused campaigns.');
        break;
    }
    $build['status_' . $status] = array(
      '#markup' => '<h2>' . $status_map[$status] . '</h2><p>' . $description . '</p>'
    );
    $rows = array();
    foreach ($agents as $agent) {
      // The ability to delete depends on the storage type and on whether the campaign has started
      // or contains option sets.
      $can_delete = personalize_delete_agent_access($agent);
      $delete_link = '';
      // Determine storage
      switch ($agent->export_type) {
        case EXPORT_IN_DATABASE | EXPORT_IN_CODE:
          $storage = t('Overridden');
          if ($can_delete) {
            $delete_link = l(t('Revert'), 'admin/structure/personalize/manage/'. $agent->machine_name .'/delete');
          }
          break;
        case EXPORT_IN_DATABASE:
          $storage = t('Normal');
          if ($can_delete) {
            $delete_link = l(t('Delete'), 'admin/structure/personalize/manage/'. $agent->machine_name .'/delete');
          }
          break;
        case EXPORT_IN_CODE:
          $storage = t('Default');
          break;
      }

      $status_change_form = drupal_get_form("personalize_change_status_{$agent->machine_name}_form", $agent->machine_name, $status);
      // @todo We are ignoring the "storage" info and the clone/export functionality
      //   for now. Revisit once we can provide full support for clone/export.
      $plugin = personalize_agent_load_agent($agent->machine_name);
      $tablerow = array(
        array('data' => check_plain($agent->label)),
        array('data' => $agent->plugin),
        array('data' => drupal_render($status_change_form)),
        //array('data' => $storage),
        array('data' => l(t('Edit'), 'admin/structure/personalize/manage/'. $agent->machine_name .'/edit')),
        array('data' => $plugin instanceof PersonalizeAgentReportInterface && ($status > PERSONALIZE_STATUS_NOT_STARTED) ? l(t('View reports'), 'admin/structure/personalize/manage/'. $agent->machine_name .'/report') : ''),
        //array('data' => l(t('Export'), 'admin/structure/personalize/manage/'. $agent->machine_name .'/export')),
        //array('data' => l(t('Clone'), 'admin/structure/personalize/manage/'. $agent->machine_name .'/clone')),
        array('data' => $delete_link),
      );
      // Show the scheduled start date for campaigns that aren't running yet.
      if ($status === PERSONALIZE_STATUS_NOT_STARTED) {
        $start_variable = _personalize_agent_get_starttime_variable($agent->machine_name);
        $start_date = variable_get($start_variable, 0);
        array_splice($tablerow, 1, 0, array('data' => $start_date > 0 ? format_date($start_date, 'custom', 'M d, Y') : ''));
      }

      $rows[] = $tablerow;
    }
    // Add the scheduled start date in to the "not started" table headers.
    $build_header = $header;
    if ($status === PERSONALIZE_STATUS_NOT_STARTED) {
      array_splice($build_header, 1, 0, array('data' => t('Scheduled start')));
    }

    $build['#attached']['css'][] = drupal_get_path('module', 'personalize') . '/css/personalize.admin.css';
    $build['table_' . $status] = array(
      '#theme' => 'table',
      '#header' => $build_header,
      '#rows' => $rows,
      '#attributes' => array('id' => 'personalize'),
    );
  }

  return $build;
}

/**
 * Form for setting the status of an agent to "Completed".
 *
 * @todo This duplicates code in personalize_status_toggle_message - refactor
 *   it out to its own function.
 */
function personalize_agent_complete_form($form, &$form_state, $agent) {
  // Display information about the "winner" that will be shown for each option
  // set in this agent.
  $option_sets = personalize_option_set_load_by_agent($agent->machine_name);
  $theme_variables = array(
    'option_sets' => array(),
    'option_message' => '',
    'alert_message' => '',
    'agent_name' => $agent->machine_name,
  );
  foreach($option_sets as $option_set) {
    $v = 1;

    // No winner selected, so first item is shown as the control item.
    if (empty($option_set->winner)) {
      $winner = $option_set->options[0];
      $winner_type = t('Control');
    }
    // Show the selected winning option.
    else {
      foreach($option_set->options as $option) {
        if ($option_set->winner == $option['option_id']) {
          $winner = $option;
          $winner_type = t('Winner');
          break;
        }
        $v++;
      }
    }
    // Generate the message shown for each 'winning' option.
    $theme_variables['option_sets'][] = array(
      'label' => (isset($option_set->label) ? check_plain($option_set->label) : t('Option @id', array('@id' => $option_set->osid))),
      'counter' => 'V' . $v,
      'winner_label' => $winner['option_label'],
      'alert' => (!empty($winner['targeting_features']) ? t('Show @v to specific users will not apply when paused.', array('@v' => 'V' . $v)) : ''),
      'winner_type' => $winner_type,
    );
  }
  if (!empty($theme_variables['option_sets'])) {
    $theme_variables['option_message'] = t('All visitors will see the following @variations:', array('@variations' => format_plural(count($option_sets), 'variation', 'variations')));
  }
  $form['message'] = array(
    '#markup' => theme('personalize_campaign_status_update', $theme_variables)
  );
  $form['machine_name'] = array('#type' => 'hidden', '#value' => $agent->machine_name);
  $form['title'] = array('#type' => 'hidden', '#value' => $agent->label);
  return confirm_form($form, t('Are you sure you want to stop the agent %title?', array('%title' => $agent->label)), 'admin/structure/personalize', '', t('Complete'), t('Cancel'));
}

/**
 * Submit handler for agent completion form.
 */
function personalize_agent_complete_form_submit($form, &$form_state) {
  personalize_agent_set_status($form_state['values']['machine_name'], PERSONALIZE_STATUS_COMPLETED);
  drupal_set_message(t('The campaign %name has been stopped.', array('%name' => $form_state['values']['title'])));
  $form_state['redirect'] = 'admin/structure/personalize';
}

/**
 * Menu callback for displaying a list of content variations.
 */
function personalize_option_set_list($form, &$form_state) {
  $option_sets = personalize_option_set_load_multiple(FALSE, array(), FALSE, 'label');

  $header = array(
    array('data' => t('Variation set name')),
    array('data' => t('Type')),
    array('data' => t('Campaign')),
    array('data' => t('Edit')),
    array('data' => t('Delete')),
  );

  $types = array();
  $rows = array();
  $filter = isset($form_state['values']['filter']) ? $form_state['values']['filter'] : '';
  foreach($option_sets as $option_set) {
    $types[$option_set->plugin] = $option_set->plugin;
    if (!empty($filter) && $filter != $option_set->plugin) {
      continue;
    }
    $campaign = personalize_agent_load($option_set->agent);
    if (empty($campaign)) {
      continue;
    }
    $option_set_plugin = personalize_get_option_set_type($option_set->plugin);
    $edit_link = module_hook($option_set_plugin['module'], 'personalize_edit_link') ? module_invoke($option_set_plugin['module'], 'personalize_edit_link', $option_set) : '';
    $delete_link = module_hook($option_set_plugin['module'], 'personalize_delete_link') ? module_invoke($option_set_plugin['module'], 'personalize_delete_link', $option_set) : '';
    $rows[] = array(
      check_plain($option_set->label),
      $option_set->plugin,
      l($campaign->label, 'admin/structure/personalize/manage/' . $option_set->agent),
      !empty($edit_link) ? l(t('edit'), $edit_link) : '',
      !empty($delete_link) ? l(t('delete'), $delete_link) : '',
    );
  }

  // Links to create new option sets of each type.
  $form['add'] = array(
    '#type' => 'markup',
    '#markup' => personalize_get_create_new_links_dropbutton('admin/structure/personalize/variations'),
  );

  // Include a filter by variation set type.
  if (count($types) > 0) {
    $types = array_merge(array('' => t('All')), $types);
    $form['filter'] = array(
      '#type' => 'select',
      '#title' => t('Filter by type'),
      '#options' => $types,
      '#ajax' => array(
        'callback' => 'personalize_option_set_list_ajax_callback',
        'wrapper' => 'personalize-variation-sets',
      )
    );
  }

  $form['results'] = array(
    '#markup' => theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'personalize-variation-sets',
      ),
    )),
  );
  return $form;
}

/**
 * Ajax submit handler used to filter the content variation list page.
 */
function personalize_option_set_list_ajax_callback($form, &$form_state) {
  return $form['results'];
}

/**
 * Menu callback for displaying a list of campaign goals.
 */
function personalize_goals_list($form, &$form_state) {
  $goals = personalize_goal_load_multiple();
  $actions = personalize_goals_options();

  $header = array(
    array('data' => t('Goal')),
    array('data' => t('Campaign')),
    array('data' => t('Value')),
    array('data' => t('Edit')),
    array('data' => t('Delete')),
  );

  $rows = array();
  foreach($goals as $goal) {
    $campaign = personalize_agent_load($goal->agent);
    $rows[] = array(
      $actions[$goal->action],
      check_plain($campaign->label),
      $goal->value,
      l(t('edit'), 'admin/structure/personalize/manage/' . $goal->agent, array('query' => array('goal' => $goal->id))),
      l(t('delete'), 'admin/structure/personalize/goals/' . $goal->id . '/delete'),
    );
  }

  $form['results'] = array(
    '#markup' => theme('table', array(
      'header' => $header,
      'rows' => $rows,
      'attributes' => array(
        'id' => 'personalize-goals',
      ),
    )),
  );
  return $form;
}

/**
 * Form creation function to delete a specified goal.
 *
 * @param array $goal
 *   The goal to be deleted.
 */
function personalize_admin_goal_delete($form, &$form_state, $goal) {
  $options = personalize_goals_options();
  $campaign = personalize_agent_load($goal->agent);
  if (personalize_agent_get_status($campaign->machine_name) == PERSONALIZE_STATUS_RUNNING) {
    $description = t('The %goal goal will no longer be tracked in the currently running %campaign campaign.', array(
      '%campaign' => $campaign->label,
      '%goal' => $options[$goal->action]
    ));
  }
  else {
    $description = t('The %goal goal will no longer be tracked when the %campaign campaign is started.', array(
      '%campaign' => $campaign->label,
      '%goal' => $options[$goal->action]
    ));
  }
  $form['goal'] = array(
    '#type' => 'value',
    '#value' => $goal,
  );
  return confirm_form($form, t('Are you sure you want to delete %goal from %campaign?', array(
    '%goal' => $options[$goal->action],
    '%campaign' => $campaign->label,
  )), 'admin/structure/personalize/goals', $description);
}

/**
 * Form submission handler to delete a specific goal.
 */
function personalize_admin_goal_delete_submit($form, &$form_state) {
  personalize_goal_delete($form_state['values']['goal']->id);
  $form_state['redirect'] = 'admin/structure/personalize/goals';
}

/**
 * Returns a form with just a submit button for changing the status of an agent.
 *
 * @param $agent_name
 *   The name of the campaign for which to toggle status.
 * @param $current_status
 *   The current status of the campaign, should match defined constant values.
 * @param $ajax_callback
 *   A callback function to apply Ajax settings to the form.
 */
function personalize_status_toggle_form($form, &$form_state, $agent_name, $current_status, $ajax_callback = NULL) {
  // If called via Ajax, then retrieve the new status values from the previous submit.
  if (!empty($form_state['values']['agent'])) {
    $agent_data = personalize_agent_load($agent_name);
    $current_status = personalize_agent_get_status($agent_data->machine_name);
  }
  $form = personalize_status_toggle_form_definition($agent_name, $current_status, FALSE, $ajax_callback);
  return $form;
}

/**
 * Gets the form fields that define the status toggle form.
 *
 * @param $agent_name
 *   The name of the campaign for which to toggle status.
 * @param $current_status
 *   The current status of the campaign, should match defined constant values.
 * @param $add_handlers
 *   Indicates if submit/validate handlers need to be added to the submit.
 *   These are only needed if the form is included within another form.
 * @param $ajax_callback
 *   A callback function to apply Ajax settings to the form.
 */
function personalize_status_toggle_form_definition($agent_name, $current_status, $add_handlers = FALSE, $ajax_callback = NULL) {
  $form = array();
  $form['#prefix'] = '<div id="personalize-status-toggle-form">';
  $form['#suffix'] = '</div>';

  list($new_status, $button_text) = _personalize_status_toggle_next($current_status);
  if (!$new_status) {
    return array();
  }
  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent_name,
  );
  $form['status'] = array(
    '#type' => 'value',
    '#value' => $new_status,
  );
  $form['actions']['toggle_form'] = array(
    '#type' => 'submit',
    '#name' => 'toggle_submit',
    '#value' => $button_text,
    '#attributes' => array(
      'class' => array('action-item-primary-active'),
    ),
  );
  if ($add_handlers) {
    $form['actions']['toggle_form']['#submit'] = array('personalize_status_toggle_form_submit');
  }
  if (is_callable($ajax_callback)) {
    $form = $ajax_callback($form);
  }
  return $form;
}

/**
 * Callback function to add Ajax settings to the personalize_status_toggle_form
 * form in the context of the personalize_agent_page() builder function.
 *
 * @param $form
 *   The form for ajax changes.
 * @return
 *   The updated form structure.
 */
function personalize_agent_page_status_toggle_ajax_callback(&$form) {
  $form['actions']['toggle_form']['#ajax'] = array(
    'callback' => 'personalize_agent_form_ajax_submit',
    'wrapper' => 'personalize-status-toggle-form',
    'effect' => 'fade',
  );
  return $form;
}

/**
 * Submit handler for the agent status toggle form.
 */
function personalize_status_toggle_form_submit($form, &$form_state) {
  $status_map = personalize_get_agent_status_map();
  if (personalize_agent_set_status($form_state['values']['agent'], $form_state['values']['status'])) {
    switch ($form_state['values']['status']) {
      case PERSONALIZE_STATUS_RUNNING:
        drupal_set_message(t('The campaign "@agent" is now live.', array(
          '@agent' => $form_state['values']['agent'],
        )));
        break;
      case PERSONALIZE_STATUS_PAUSED:
      case PERSONALIZE_STATUS_COMPLETED:
        drupal_set_message(t('The campaign "@agent" is @status.', array(
          '@agent' => $form_state['values']['agent'],
          '@status' => drupal_strtolower($status_map[$form_state['values']['status']]),
        )));
        break;
      default:
        drupal_set_message(t('The status of agent @agent has been set to @status', array(
          '@agent' => $form_state['values']['agent'],
          '@status' => drupal_strtolower($status_map[$status_map[$form_state['values']['status']]]),
        )));
    }
  }
  else {
    drupal_set_message(t('There was a problem setting the status of agent @agent to @status', array(
      '@agent' => $form_state['values']['agent'],
      '@status' => drupal_strtolower($status_map[$form_state['values']['status']]))
    ), 'error');
  }
  personalize_status_toggle_message($form_state['values']['agent'], $form_state['values']['status']);
  $form_state['redirect'] = 'admin/structure/personalize';
}

/**
 * Update messaging when the user changes the status of a campaign.
 */
function personalize_status_toggle_message($agent_name, $current_status) {
  if ($current_status == PERSONALIZE_STATUS_RUNNING || $current_status == PERSONALIZE_STATUS_NOT_STARTED) {
    return;
  }
  $theme_variables = array(
    'option_sets' => array(),
    'option_message' => '',
    'alert_message' => '',
    'agent_name' => $agent_name,
  );
  $option_sets = personalize_option_set_load_by_agent($agent_name);
  foreach($option_sets as $option_set) {
    $v = 1;

    // No winner selected, so first item is shown as the control item.
    if (empty($option_set->winner)) {
      $winner = $option_set->options[0];
      $winner_type = t('Control');
    }
    // Show the selected winning option.
    else {
      foreach($option_set->options as $option) {
        if ($option_set->winner == $option['option_id']) {
          $winner = $option;
          $winner_type = t('Winner');
        }
        $v++;
      }
    }
    // Generate the message shown for each 'winning' option.
    $theme_variables['option_sets'][] = array(
      'label' => (isset($option_set->label) ? check_plain($option_set->label) : t('Option @id', array('@id' => $option_set->osid))),
      'counter' => 'V' . $v,
      'winner_label' => $winner['option_label'],
      'alert' => (!empty($winner['targeting_features']) ? t('Show @v to specific users will not apply when paused.', array('@v' => 'V' . $v)) : ''),
      'winner_type' => $winner_type,
    );
  }
  if (!empty($theme_variables['option_sets'])) {
    $theme_variables['option_message'] = t('All visitors will see the following variations:');
  }
  drupal_set_message(theme('personalize_campaign_status_update', $theme_variables));
}

/**
 * Menu callback for the campaign edit page.
 */
function personalize_agent_page($agent_data) {
  // The page title of the simple campaign name is taken from
  // personalize_campaign_title_callback().  In order to give the edit tab
  // a specific title, explicitly update the title.
  $edit_tab_title = t('Edit @campaign', array('@campaign' => drupal_get_title()));
  drupal_set_title($edit_tab_title);

  // Set the campaign context to this campaign.
  personalize_set_campaign_context($agent_data->machine_name);
  $build = array();
  $build['personalize_messages'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'personalize-agent-page-messages',
    ),
  );
  $agent_instance = personalize_agent_load_agent($agent_data->machine_name);
  $build['agent'] = drupal_get_form('personalize_agent_form', $agent_data);
  $build['option_sets'] = drupal_get_form('personalize_agent_option_sets_form', $agent_data, $agent_instance);
  if ($agent_instance instanceof PersonalizeAgentGoalInterface) {
    $build['goals'] = drupal_get_form('personalize_agent_goals_form', $agent_data);
  }
  if ($agent_instance->supportsMVTs()) {
    $build['mvt'] = drupal_get_form('personalize_agent_mvt_form', $agent_data);
  }

  drupal_alter('personalize_agent_page', $build, $agent_data);
  return $build;
}

/**
 * Agent add/edit form.
 */
function personalize_agent_form($form, &$form_state, $agent_data = NULL) {
  if (empty($agent_data)) {
    $agent_data = new stdClass();
    if (isset($_GET['personalize_agent_type']) && $plugin = personalize_get_agent_type($_GET['personalize_agent_type'])) {
      $agent_data->plugin = $plugin['name'];
    }
  }
  if (isset($form_state['values']['agent'])) {
    $agent_data = personalize_agent_load($form_state['values']['agent']);
  }

  $form['#attached']['library'][] = array('personalize', 'admin.campaign');
  if (isset($agent_data->machine_name)) {
    $form['#attached']['js'][] = array(
      'type' => 'setting',
      'data' => array(
        'personalize' => array(
          'campaigns' => array(
            $agent_data->machine_name => array(
              'status' => personalize_agent_get_status($agent_data->machine_name),
            )
          )
        )
      )
    );
  }

  $form['agent_form'] = array(
    '#tree' => TRUE,
    '#type' => 'personalize_admin_section',
  );

  $form['agent_form']['agent_fieldset'] = array(
    '#type' => 'fieldset',
    '#collapsible' => !empty($agent_data->machine_name),
    '#collapsed' => !empty($agent_data->machine_name),
    '#tree' => FALSE,
    '#title' => !empty($agent_data->label) ? check_plain($agent_data->label) : t('New Campaign'),
    '#theme_wrappers' => array('personalize_admin_content'),
  );

  if (!empty($agent_data->machine_name)) {
    $status = personalize_agent_get_status($agent_data->machine_name);
    if ($status == PERSONALIZE_STATUS_COMPLETED) {
      drupal_set_message(t('You are editing a completed campaign.'), 'warning');
    }
    else {
      $toggle_form = personalize_status_toggle_form_definition($agent_data->machine_name, $status, TRUE, 'personalize_agent_page_status_toggle_ajax_callback');
      $form['agent_form']['agent_fieldset']['header'] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('personalize-admin-content-header'),
        ),
        'toggle_form' => $toggle_form,
      );
    }
  }
  $basic_form = personalize_agent_build_basic_form($agent_data, TRUE, array('agent_form', 'agent_fieldset', 'agent_basic'));
  if ($basic_form === FALSE) {
    // Campaigns cannot be added or edited at this time.
    return array();
  }
  $form['agent_form']['agent_fieldset']['agent_basic'] = $basic_form;

  // Add visitor context items to the form for agents that support automatic
  // targeting.
  if (!empty($agent_data->machine_name)) {
    $agent_instance = personalize_agent_load_agent($agent_data->machine_name);
    if ($agent_instance instanceof PersonalizeAutoTargetingInterface) {
      $visitor_context = isset($agent_data->data['visitor_context']) ? $agent_data->data['visitor_context'] : array();
      if ($context_element = personalize_admin_build_visitor_context_select($visitor_context, TRUE, $agent_data)) {
        $form['agent_form']['agent_fieldset']['visitor_context'] = $context_element;
      }
    }
  }

  // Allow the campaign end date to be set.
  $form['agent_form']['agent_fieldset']['campaign_dates'] = personalize_agent_date_form(!empty($agent_data->machine_name) ? $agent_data->machine_name : NULL);

  $form['agent_form']['agent_fieldset']['advanced'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => FALSE,
  );
  $form['agent_form']['agent_fieldset']['advanced']['cache_decisions'] = array(
    '#type' => 'checkbox',
    '#title' => 'Cache decisions made by this agent?',
    '#description' => t('Check this if decisions made by this agent should stick, so that they don\'t have to be made again every time variation sets are rendered. This depends on the availability of local storage in the visitor\'s browser'),
    '#default_value' => isset($agent_data->data['cache_decisions']) ? $agent_data->data['cache_decisions'] : 1
  );

  $form['#prefix'] = !empty($form['#prefix']) ? '<div id="personalize-agent-form">' . $form['#prefix'] : '<div id="personalize-agent-form">';
  $form['#suffix'] = !empty($form['#suffix']) ? $form['#suffix'] . '</div>' : '</div>';

  $form['agent_form']['agent_fieldset']['actions'] = array('#type' => 'actions');
  $classes = array('action-item-primary-active');
  if (!empty($agent_data->machine_name)) {
    // Triggers warning when editing an active campaign.
    $classes[] = 'personalize-admin-warn';
  }
  $form['agent_form']['agent_fieldset']['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#attributes' => array(
      'class' => $classes,
    ),
    '#value' => t('Save campaign settings'),
  );
  $form['agent_form']['agent_fieldset']['actions']['reset'] =  _personalize_agent_get_cancel('personalize-agent-form');
  if (!empty($agent_data->machine_name)) {
    $form['agent_form']['agent_fieldset']['actions']['submit_form']['#ajax'] = array(
      'callback' => 'personalize_agent_form_ajax_submit',
      'wrapper' => 'personalize-agent-form',
      'effect' => 'fade',
    );
  }

  // We have to specify the include file so as not to lose it during rendering from ajax.
  // @see personalize_agent_form_ajax_submit()
  // @see drupal_retrieve_form():734
  $form_state['build_info']['files'] = array(
    drupal_get_path('module', 'personalize') . '/personalize.admin.inc',
  );

  return $form;
}

/**
 * Generate the form array to set campaign dates.
 *
 * @param $agent_name
 *   (optional) The machine name for the campaign.
 * @return array
 *   The renderable form array for the end date form fields.
 */
function personalize_agent_date_form($agent_name = NULL) {
  if (!empty($agent_name)) {
    $end_variable = _personalize_agent_get_stoptime_variable($agent_name);
    $agent_end = $default_end = variable_get($end_variable, 0);
    $start_variable = _personalize_agent_get_starttime_variable($agent_name);
    $agent_start = $default_start = variable_get($start_variable, 0);
  }
  else {
    $agent_end = 0;
    $agent_start = 0;
  }
  // Default the start date to one month and the end date to one month if the
  // campaign is new or does not yet have an end date set.
  if ($agent_start == 0) {
    $default_start = strtotime('+1 month');
  }
  if ($agent_end == 0) {
    $default_end = strtotime('+1 month');
  }

  $agent_start_date = array('year' => date('Y', $default_start), 'month' => intval(date('m', $default_start)), 'day' => date('d', $default_start));
  $form['campaign_start'] = array(
    '#type' => 'radios',
    '#title' => t('Campaign start date'),
    '#options' => array(
      'none' => t('Start manually'),
      'specified' => t('Start on selected date'),
    ),
    '#default_value' => $agent_start == 0 ? 'none' : 'specified',
  );
  $form['campaign_start_date'] = array(
    '#type' => 'date',
    '#default_value' => $agent_start_date,
    '#states' => array(
      'visible' => array(
        ':input[name="campaign_start"]' => array('value' => 'specified'),
      ),
    ),
  );
  $agent_end_date = array('year' => date('Y', $default_end), 'month' => intval(date('m', $default_end)), 'day' => date('d', $default_end));
  $form['campaign_end'] = array(
    '#type' => 'radios',
    '#title' => t('Campaign end date'),
    '#options' => array(
      'none' => t('End manually'),
      'specified' => t('End on selected date'),
    ),
    '#default_value' => $agent_end == 0 ? 'none' : 'specified',
  );
  $form['campaign_end_date'] = array(
    '#type' => 'date',
    '#default_value' => $agent_end_date,
    '#states' => array(
      'visible' => array(
        ':input[name="campaign_end"]' => array('value' => 'specified'),
      ),
    ),
  );
  drupal_alter('personalize_agent_date_form', $form, $agent_name);
  return $form;
}

/**
 * Validate the campaign date form values.
 *
 * @param $values
 *   An array of values to validate.
 */
function personalize_agent_date_form_validate($values) {
  $start_date = 0;
  $end_date = 0;
  if (isset($values['campaign_start']) && $values['campaign_start'] === 'specified') {
    $start_date = strtotime($values['campaign_start_date']['year'] . '-' . $values['campaign_start_date']['month'] . '-' . $values['campaign_start_date']['day']);
    if ($start_date < time()) {
      form_set_error('campaign_start_date', t('When provided, the start date for the campaign must be in the future.'));
    }
  }
  if (isset($values['campaign_end']) && $values['campaign_end'] === 'specified') {
    $end_date = strtotime($values['campaign_end_date']['year'] . '-' . $values['campaign_end_date']['month'] . '-' . $values['campaign_end_date']['day']);
    if ($end_date < time()) {
      form_set_error('campaign_end_date', t('When provided, the end date for the campaign must be in the future.'));
    }
  }
  if ($start_date > 0 && $end_date > 0) {
    if ($end_date <= $start_date) {
      form_set_error('campaign_end_date', t('When provided, the end date for the campaign must be after the start date for the campaign.'));
    }
  }
}

/**
 * Save an agent campaign start and end date values.
 *
 * @param $agent_name
 *   The name of the campaign.
 * @param $values
 *   An array of campaign end date values.
 */
function personalize_agent_date_form_submit($agent_name, $values) {
  if (!empty($values['campaign_start'])) {
    // Set the start date.
    if ($values['campaign_start'] === 'specified') {
      $start_date = strtotime($values['campaign_start_date']['year'] . '-' . $values['campaign_start_date']['month'] . '-' . $values['campaign_start_date']['day']);
    }
    else {
      $start_date = 0;
    }
    $varname = _personalize_agent_get_starttime_variable($agent_name);
    variable_set($varname, $start_date);
  }
  if (!empty($values['campaign_end'])) {
    // Set the end date.
    if ($values['campaign_end'] === 'specified') {
      $end_date = strtotime($values['campaign_end_date']['year'] . '-' . $values['campaign_end_date']['month'] . '-' . $values['campaign_end_date']['day']);
    }
    else {
      $end_date = 0;
    }
    $varname = _personalize_agent_get_stoptime_variable($agent_name);
    variable_set($varname, $end_date);
  }
}

/**
 * Ajax callback for agent form to reset the current subform.
 */
function personalize_agent_form_ajax_reset($form, &$form_state) {
  // Clear custom storage.
  unset($form_state['to_remove']);
  unset($form_state['num_goals']);
  // Clear user input.
  unset($form_state['complete form']);
  $form_state['input'] = array();
  $form_state['values'] = array();
  // Rebuild the form.
  return drupal_rebuild_form($form['#form_id'], $form_state, $form);
}

/**
 * Ajax callback for agent edit page form ajax submissions
 */
function personalize_agent_form_ajax_submit($form, $form_state) {
  $commands = array();
  $agent_data = NULL;
  if (isset($form_state['values']['agent'])) {
    $agent_data = personalize_agent_load($form_state['values']['agent'], TRUE);
  }

  if ($form['#form_id'] == 'personalize_agent_form') {
    // Configuration the agent pauses the agent status so toggle and
    // option sets must also be updated.
    $agent_data = personalize_agent_load($form_state['values']['agent_basic_info']['machine_name'], TRUE);
    $agent_instance = personalize_agent_load_agent($agent_data->machine_name);
    $option_sets_form = drupal_get_form('personalize_agent_option_sets_form', $agent_data, $agent_instance);
    $commands[] = ajax_command_replace('#personalize-agent-option-sets-form', drupal_render($option_sets_form));
    // This could be triggered by the status toggle so enforce reloading the
    // full personalize-agent-form regardless of the submit used.
    $form_div_id = 'personalize-agent-form';
  }
  else if ($form['#form_id'] == 'personalize_agent_option_sets_form') {
    // Updating the option sets causes the campaign to pause so we need to
    // update display of the status toggle.
    $campaign_form = drupal_get_form('personalize_agent_form', $agent_data);
    $commands[] = ajax_command_replace('#personalize-agent-form', drupal_render($campaign_form));
  }

  // Update the current agent status.
  if (!empty($agent_data)) {
    $current_status = personalize_agent_get_status($agent_data->machine_name);
    $settings = array(
      'personalize' => array(
        'campaigns' => array(
          $agent_data->machine_name => array(
            'status' => $current_status,
          ),
        ),
      ),
    );
    $commands[] = ajax_command_settings($settings, TRUE);
  }

  // Show status messages relative to the personalize page.
  $commands[] = ajax_command_replace('#personalize-agent-page-messages', '<div id="personalize-agent-page-messages">' . theme('status_messages') . '</div>');

  // Return the updated build of the submitted form.
  $form = drupal_rebuild_form($form['#form_id'], $form_state, $form);
  if (!isset($form_div_id)) {
    $form_div_id = $form_state['triggering_element']['#ajax']['wrapper'];
  }
  $commands[] = ajax_command_replace('#' . $form_div_id, drupal_render($form));

  // Any agent type that needs to sync changes to a 3rd party system will probably
  // do so asynchronously.
  // Third party agents can modify the commands returned to synchronize
  // from the client-side.
  drupal_alter('personalize_form_ajax_commands', $commands);
  // Third party agents can implements this hook to synchronize from the server
  // side during ajax submit of these forms.
  module_invoke_all('personalize_form_ajax_submit', $agent_data);

  return personalize_ajax_commands_deliver($commands, TRUE);
}

/**
 * Returns the markup for a summary report for the specified agent.
 *
 * @param $agent_name
 *   The name of the agent to return a report for.
 * @return string
 *   A string of markup containing the report.
 */
function personalize_agent_get_summary_report($agent_name) {
  // Try to load a summary report for this agent.
  try {
    $agent = personalize_agent_load_agent($agent_name);
    $report = $agent->getSummaryReport();
    if (!empty($report)) {
      return $report;
    }
  }
  catch(Exception $e) {
    drupal_set_message("There is not yet any data available for reports.");
  }
  return '';
}

/**
 * Builds the basic form for agent creation.
 *
 * @param $agent_data
 *   Object representing an existing agent if this is an edit form.
 * @param $include_machine_name_field
 *   Whether to include the machine name field in the form. (This form can be
 *   embedded, with #states controlling whether it is visible or not, in which
 *   case the machine_name field doesn't work.)
 * @param $parent_array
 *   The #array_parents of the new form if nested.
 */
function personalize_agent_build_basic_form($agent_data = NULL, $include_machine_name_field = TRUE, $parent_array = NULL) {
  $form = array();
  $form['agent_basic_info'] = array(
    '#tree' => TRUE
  );
  if ($agent_data === NULL) {
    $agent_data = new stdClass();
  }
  $form['#agent'] = $agent_data;
  // Make sure we have at least one agent type available.
  $agent_types = personalize_get_agent_types();
  $agent_type_options = $agent_type_form_options = array();
  foreach ($agent_types as $name => $info) {
    if ($class = ctools_plugin_load_class('personalize', 'agent_type', $name, 'handler')) {
      // Add this option to the options for the "agent type" dropdown.
      $agent_type_options[$name] = $name;
      // Form structure for option parents in the current form.
      $option_parents = array('agent_basic_info', 'options', $name);
      // Get the agent type's options form elements
      $agent_type_form_options[$name] = call_user_func_array(array($class, 'optionsForm'), array($agent_data, $option_parents));
    }
  }
  if (empty($agent_type_options)) {
    drupal_set_message(t('You don\'t have any agent types enabled. Please enable the personalize_target module or another module that provides an agent type.'), 'error');
    return FALSE;
  }
  ksort($agent_type_options);

  $form['agent_basic_info']['title'] = array(
    '#title' => t('Name'),
    '#type' => 'textfield',
    '#default_value' => isset($agent_data->label) ? $agent_data->label : '',
    '#weight' => -9,
    '#required' => TRUE,
  );
  if ($include_machine_name_field) {
    // Define the parent array to the source input.
    if (empty($parent_array)) {
      $parent_array = array();
    }
    $parent_array[] = 'agent_basic_info';
    $parent_array[] = 'title';
    $form['agent_basic_info']['machine_name'] = array(
      '#type' => 'machine_name',
      '#maxlength' => PERSONALIZE_MACHINE_NAME_MAXLENGTH,
      '#machine_name' => array(
        'exists' => 'personalize_agent_machine_name_exists',
        'source' => $parent_array,
        'replace_pattern' => '[^a-z0-9-]+',
        'replace' => '-',
      ),
      '#description' => t('A unique machine-readable name for this agent. It must only contain lowercase letters, numbers, and hyphens.'),
      '#weight' => -8,
    );
    if (!empty($agent_data->machine_name)) {
      $form['agent_basic_info']['machine_name']['#default_value'] = $agent_data->machine_name;
      $form['agent_basic_info']['machine_name']['#disabled'] = TRUE;
      $form['agent_basic_info']['machine_name']['#value'] = $agent_data->machine_name;
    }
  }

  if (!empty($agent_data->plugin)) {
    // It is not possible to change the type of an agent.
    $form['agent_basic_info']['agent_type'] = array(
      '#type' => 'value',
      '#value' => $agent_data->plugin,
    );
  }
  elseif (count($agent_type_options) < 2) {
    // No need to show a dropdown if there's only one available plugin.
    $form['agent_basic_info']['agent_type'] = array(
      '#type' => 'hidden',
      '#value' => key($agent_type_options),
    );
  }
  else {
    $form['agent_basic_info']['agent_type'] = array(
      '#type' => 'select',
      '#title' => t('Agent Type'),
      '#options' => $agent_type_options,
      '#default_value' => '',
      '#description' => t('Choose which type of agent to create.'),
      '#weight' => -7,
    );
  }

  // Add the agent-type-specific form elements to the form, to be shown only if the
  // agent type in question is selected.
  $form['agent_basic_info']['options'] = array('#tree' => TRUE, '#weight' => -6);
  if (!empty($agent_data->plugin)) {
    // If in edit mode, then only show the options for the selected agent.
    $form['agent_basic_info']['options'][$agent_data->plugin] = $agent_type_form_options[$agent_data->plugin];
  }
  else {
    // If in add mode, then show options dynamically using states.
    foreach ($agent_type_form_options as $agent_type => $options) {
      foreach ($options as &$option) {
        $option['#states'] = array('visible' => array(':input[name="agent_basic_info[agent_type]"]' => array('value' => $agent_type)));
      }
      $form['agent_basic_info']['options'][$agent_type] = $options;
    }
  }
  return $form;
}

/**
 * Builds a form for editing the goals of an agent.
 */
function personalize_agent_goals_form($form, &$form_state, $agent_data) {
  // Determine goals for inclusion.
  $goals = $exclude = array();
  $all_actions = visitor_actions_get_actions();
  if (isset($form_state['values']['goals'])) {
    foreach ($form_state['values']['goals'] as $delta => $goal) {
      if (!empty($goal['action_name'])) {
        $goals[$delta] = array(
          'action' => $goal['action_name'],
          'value' => $goal['value']
        );
      }
    }
  }
  else {
    $goal_entities = personalize_goal_load_by_conditions(array('agent' => $agent_data->machine_name));
    foreach ($goal_entities as $obj) {
      $goals[] = (array) $obj;
    }
  }

  // If the "Remove" button was clicked for a goal, we need to remove that goal
  // from the form.
  if (isset($form_state['to_remove'])) {
    unset($goals[$form_state['to_remove']]);
    unset($form_state['to_remove']);
    $form_state['num_goals']--;
  }

  foreach ($goals as $i => $goal) {
    if (!isset($all_actions[$goal['action']])) {
      // The action for this goal no longer exists - don't include it in the form
      // and set a warning that it will be removed upon save.
      drupal_set_message(t('At least one of your goals no longer has an action associated with it. Saving this form will cause the goal to be removed from the campaign.'), 'warning', FALSE);
      unset($goals[$i]);
    }
    else {
      // Any action already set as a goal on this campaign should be excluded
      // from the action dropdown.
      $exclude[$goal['action']] = $goal['action'];
    }
  }

  if (empty($goals)) {
    $goals[] = array('action' => '', 'value' => 1);
  }

  // If the "Add another" button was clicked, we need to add goals to get up
  // to the number indicated.
  $num_goals = count($goals);
  $clicked_add = isset($form_state['clicked_button']['#submit']) && in_array('personalize_agent_add_goal_submit', $form_state['clicked_button']['#submit']);
  if (isset($form_state['num_goals']) && $form_state['num_goals'] > $num_goals) {
    while ($num_goals < $form_state['num_goals']) {
      $goals[] = array('action' => '', 'value' => 1, 'collapsed' => FALSE, 'classes' => array('personalize-goal-add'));
      $num_goals++;
    }
  }
  $form_state['num_goals'] = count($goals);

  $goal_options = personalize_goals_options(TRUE);
  // For new goals, we exclude actions already used.
  $new_goal_options = array_diff_key($goal_options, $exclude);
  // Add the "add new action in context" option if visitor_actions_ui
  // is enabled.
  if (module_exists('visitor_actions_ui')) {
    $link = url('admin/structure/visitor_actions/add-in-context', array(
      'query' => array('destination' => "admin/structure/personalize/manage/{$agent_data->machine_name}/edit"),
      'attributes' => array(
        'class' => array('personalize-add-context', 'button'),
      ),
    ));
    $new_goal_options[$link] = t('[Add new action in context...]');
  }

  // Begin form generation.
  $form = array();
  $form['#agent'] = $agent_data;
  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent_data->machine_name
  );
  // Assemble the form in nested containers to allow for easier styling.
  $form['variations'] = array(
    '#type' => 'personalize_admin_section',
    '#tree' => FALSE,
    'primary' => array(
      '#theme_wrappers' => array('container'),
    ),
  );
  // Determine the collapsed content.
  $collapsed = '';
  $goal_labels = array();
  // Use a counter variable as the delta is affected by adding/removing goals.
  $counter = 1;
  foreach($goals as $delta => $goal) {
    // Save the display goal title for use later
    $goal_labels[$delta] = theme('personalize_admin_enumerated_item', array(
      'enum' => t('Goal @delta', array('@delta' => ($counter))),
      'title' => check_plain($goal['action']),
    ));
    $collapsed .= $goal_labels[$delta];
    $counter++;
  }
  $form['variations']['primary']['goals_fieldset'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => !$clicked_add,
    '#tree' => FALSE,
    '#title' => t('Goals'),
    '#summary' => $collapsed,
    '#theme_wrappers' => array('personalize_admin_content'),
  );
  // Header content for goals.
  $form['variations']['primary']['goals_fieldset']['header'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('personalize-admin-content-header'),
    ),
  );
  $form['variations']['primary']['goals_fieldset']['header']['add_goal'] = array(
    '#prefix' => '<span class="personalize-add-link-prefix"></span>',
    '#type' => 'submit',
    '#value' => t('Add goal'),
    '#attributes' => array(
      'class' => array('personalize-add-link'),
      'title' => t('Click here to add more goals.')
    ),
    '#submit' => array('personalize_agent_add_goal_submit'),
    '#ajax' => array(
      'callback' => 'personalize_agent_goals_ajax_callback',
      'wrapper' => 'personalize-goals',
      'effect' => 'fade',
    ),
    '#limit_validation_errors' => array(array('goals')),
  );

  // Now handle the goals input display.
  $section = &$form['variations']['primary']['goals_fieldset'];
  $section['goals'] = array(
    '#tree' => TRUE,
    '#theme_wrappers' => array('container'),
    '#attributes' => array(
      'id' => 'personalize-goals',
      'class' => array('personalize-goals-wrapper'),
    ),
  );

  foreach ($goals as $delta => $goal) {
    $goal_fieldset_classes = array('personalize-admin-collapsed-content', 'personalize-goal');
    if (isset($goal['classes']) && is_array($goal['classes'])) {
      $goal_fieldset_classes = array_merge($goal_fieldset_classes, $goal['classes']);
    }
    // The goal id should match loaded goals from the database for jumping to
    // a specific goal on page load.  This will not be available after an AJAX
    // call, but that doesn't matter in the use case of initial page linkages.
    $section['goals'][$delta] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => isset($goal['collapsed']) ? $goal['collapsed'] : TRUE,
      '#title' => $goal_labels[$delta],
      '#tree' => TRUE,
      '#attributes' => array(
        'class' => $goal_fieldset_classes,
        'id' => isset($goal['id']) ? "personalize-goal-{$goal['id']}" : "personalize-goal-$delta",
      )
    );
    $options = $goal['action'] == '' ? $new_goal_options : $goal_options;

    if (!empty($goal['action']) && empty($options[$goal['action']])) {
      if (isset($all_actions[$goal['action']])) {
        // The current goal was excluded from the list of options because it is
        // a limited use visitor action, just add it separately here.
        $options = array_merge(array($goal['action'] => $all_actions[$goal['action']]['label']), $options);
      }
    }
    $section['goals'][$delta]['action_name'] = array(
      '#type' => 'select',
      '#title' => t('Action'),
      '#options' => $options,
      '#default_value' => $goal['action'],
      '#empty_option' => t('Select an action'),
      '#attributes' => array(
        'class' => array('personalize-goal-action'),
      ),
    );
    $section['goals'][$delta]['value'] = array(
      '#type' => 'textfield',
      '#title' => t('Value'),
      '#size' => 5,
      '#default_value' => $goal['value']
    );
    $section['goals'][$delta]['remove'] = array(
      '#type' => 'submit',
      '#tag' => 'button',
      '#text' => t('Remove'),
      '#value' => 'remove_' . $delta,
      '#theme_wrappers' => array('personalize_html_tag'),
      '#attributes' => array(
        // The ID is necessary for the AJAX replace to function correctly. It
        // is fragile to declare it like this, essentially hard-coding the
        // #parents, but I know of no other way to do this programmatically.
        'id' => 'edit-goals-' . $delta . '-remove',
        'class' => array('personalize-delete-goal', 'form-submit',),
        'title' => t('Delete the goal.'),
        // Adding the name and value attributes allows for targeting within
        // simpletest.
        'name' => 'edit-goals-' . $delta . '-remove',
        'value' => t('Remove'),
      ),
      '#submit' => array('personalize_agent_remove_goal_submit'),
      '#ajax' => array(
        'callback' => 'personalize_agent_goals_ajax_callback',
        'wrapper' => 'personalize-goals',
        'method' => 'replace',
        'effect' => 'fade',
      ),
      '#limit_validation_errors' => array(array('goals')),
    );
  }

  $form['#prefix'] = !empty($form['#prefix']) ? '<div id="personalize-agent-goals-form">' . $form['#prefix'] : '<div id="personalize-agent-goals-form">';
  $form['#suffix'] = !empty($form['#suffix']) ? $form['#suffix'] . '</div>' : '</div>';

  $section['actions'] = array('#type' => 'actions');
  $section['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#value' => t('Save goals'),
    '#ajax' => array(
      'callback' => 'personalize_agent_form_ajax_submit',
      'wrapper' => 'personalize-agent-goals-form',
      'effect' => 'fade',
    ),
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'personalize-admin-warn'),
    ),
  );
  $section['actions']['reset'] = _personalize_agent_get_cancel('personalize-agent-goals-form');
  return $form;
}

/**
 * Builds a form for editing the Option Sets of an agent.
 */
function personalize_agent_option_sets_form($form, &$form_state, $agent_data, $agent_instance) {
  // Load the data used throughout form.
  $option_sets = personalize_option_set_load_by_agent($agent_data->machine_name);
  $campaign_page = "admin/structure/personalize/manage/{$agent_data->machine_name}/edit";
  $is_running = personalize_agent_get_status($agent_data->machine_name) == PERSONALIZE_STATUS_RUNNING;

  $form = array();
  $form['#agent'] = $agent_data;
  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent_data->machine_name
  );

  $form['#prefix'] = !empty($form['#prefix']) ? '<div id="personalize-agent-option-sets-form">' . $form['#prefix']
    : '<div id="personalize-agent-option-sets-form">';
  $form['#suffix'] = !empty($form['#suffix']) ? $form['#suffix'] . '</div>' : '</div>';

  // Assemble the form in nested containers to allow for easier styling.
  $form['variations'] = array(
    '#type' => 'personalize_admin_section',
    '#tree' => FALSE,
    'primary' => array(
      '#theme_wrappers' => array('container'),
    ),
  );
  // Determine collapsed content.
  $collapsed = '';
  $counter = 1;
  // Tricky: key is not necessarily from 0.
  // Setting value on existing object causes unwanted recursion so labels
  // are saved within their own object.
  $option_sets_labels = array();
  foreach($option_sets as $option_set) {
    $option_sets_labels[$option_set->osid] = theme('personalize_admin_enumerated_item', array(
      'enum' => t('Set @delta', array('@delta' => $counter)),
      'title' => isset($option_set->label) ? check_plain($option_set->label) : t('Option Set @id', array('@id' => $option_set->osid)),
    ));
    $collapsed .= $option_sets_labels[$option_set->osid];
    $counter++;
  }
  $form['variations']['primary']['option_sets'] = array(
    '#tree' => TRUE,
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Content variation sets'),
    '#theme_wrappers' => array('personalize_admin_content'),
    '#summary' => $collapsed,
  );
  $form['variations']['primary']['option_sets']['header'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('personalize-admin-content-header'),
    ),
  );
  $form['variations']['primary']['option_sets']['header']['add'] = array(
    '#markup' => personalize_get_create_new_links_dropbutton($campaign_page),
  );

  $section = &$form['variations']['primary']['option_sets'];
  // For any option sets that have been created for this campaign, display
  // advanced options for specifying decision name etc.
  if (empty($option_sets)) {
    return $form;
  }

  $targeting_support = FALSE;
  if ($agent_instance instanceof PersonalizeExplicitTargetingInterface) {
    $targeting_support = $agent_instance->explicitTargetingSupportMultiple();
    // Build up a list of targeting values so we can map options to them.
    $targeting_values = personalize_get_targeting_values_for_agent($agent_data);
  }

  foreach ($option_sets as $option_set) {
    $option_set_plugin = personalize_get_option_set_type($option_set->plugin);
    $replace_option_set_id = drupal_strtolower('personalize-option-set-' . $option_set->osid);

    // Show this open if it was just refreshed via Ajax.
    $section['option_set_' . $option_set->osid] = array(
      '#tree' => TRUE,
      '#type' => 'fieldset',
      '#collapsible' => 1,
      '#collapsed' => !isset($form_state['triggering_element']['#ajax']['wrapper']) || $form_state['triggering_element']['#ajax']['wrapper'] != $replace_option_set_id,
      '#title' => $option_sets_labels[$option_set->osid],
      '#attributes' => array(
        'class' => array('personalize-option-set'),
        'id' => $replace_option_set_id, // Used for AJAX replace.
      ),
    );
    $option_set_winner = isset($form_state['winners']['option_set_' . $option_set->osid]) ? $form_state['winners']['option_set_' . $option_set->osid] : $option_set->winner;
    $section['option_set_' . $option_set->osid]['winner'] = array(
      '#type' => 'value',
      '#value' => $option_set_winner,
    );
    // Header information to be shown in the fieldset summary.
    $edit_link = module_invoke($option_set_plugin['module'], 'personalize_edit_link', $option_set);
    $delete_link = module_invoke($option_set_plugin['module'], 'personalize_delete_link', $option_set);
    $section['option_set_' . $option_set->osid]['summary'] = array(
      '#type' => 'markup',
      '#markup' => theme('personalize_option_set_header', array(
        'variation_count' => count($option_set->options),
        'report' => $agent_instance instanceof PersonalizeAgentReportInterface ? $agent_instance->renderStatsForOptionSet($option_set, $agent_data->started) : array(),
        'report_link' => $agent_instance instanceof PersonalizeAgentReportInterface ? l(t('report'), "admin/structure/personalize/manage/{$agent_data->machine_name}/report/{$option_set->osid}") : '',
        'edit_link' => !empty($edit_link) ? l('edit', $edit_link, array('query' => array('destination' => $campaign_page))) : '',
        'delete_link' => !empty($delete_link) ? l('delete', $delete_link, array('query' => array('destination' => $campaign_page))) : '',
      )),
      '#theme_wrappers' => array('container'),
      '#attributes' => array(
        'class' => array('personalize-summary'),
      ),
    );

    $variant_number = 1;
    $targeting = personalize_get_targeting_for_options($option_set);
    foreach ($option_set->options as $option) {
      $is_winner = FALSE;
      $is_control = $variant_number == 1;
      // Determine container classes based on campaign status and winner.
      $classes = array('personalize-admin-content-item', 'personalize-content-variation', 'clearfix');
      if ($is_running) {
        $classes[] = 'personalize-content-variation-running';
      }
      else {
        $classes[] = 'personalize-content-variation-stopped';
        // The winner is either previously selected, or the control option.
        if ((!empty($option_set_winner) && $option_set_winner == $option['option_id']) ||
          (empty($option_set_winner) && $is_control)) {
          $is_winner = TRUE;
          $classes[] = 'personalize-content-variation-winner';
        }
      }
      $section['option_set_' . $option_set->osid]['options'][$option['option_id']] = array(
        '#tree' => TRUE,
        '#type' => 'container',
        '#attributes' => array(
          'class' => $classes,
        ),
      );
      $section['option_set_' . $option_set->osid]['options'][$option['option_id']]['basic'] = array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('personalize-variation-row'),
        ),
      );
      $preview_link = '';
      if (!empty($option_set->preview_link)) {
        $preview_link = l('Preview', $option_set->preview_link, array(
          'attributes' => array('target' => 'preview'),
          'query' => array(PERSONALIZE_PRESELECTION_PARAM => personalize_stringify_osid($option_set->osid) . '--' . $option['option_id']),
        ));
      }
      $suffix = !empty($preview_link) ? '" ' . $preview_link : '"';
      if ($is_control) {
        $suffix .= '<span class="personalize-content-variation-control">' . t('Control') . '</span>';
      }
      if ($is_winner) {
        $suffix .= '<span class="personalize-content-variation-winner">' . t('Fallback/Winner') . '</span>';
      }

      $heading = theme('personalize_admin_enumerated_item', array(
        'enum' => t('V@delta', array('@delta' => $variant_number)),
        'title' => check_plain($option['option_label']),
        'title_prefix' => '"',
        'title_suffix' => $suffix,
      ));
      $section['option_set_' . $option_set->osid]['options'][$option['option_id']]['basic']['heading'] = array(
        '#markup' => $heading,
      );
      if (!$is_running & !$is_winner) {
        $section['option_set_' . $option_set->osid]['options'][$option['option_id']]['basic']['winner'] = array(
          '#prefix' => '<div class="personalize-option-set-winner">',
          '#suffix' => '</div>',
          '#type' => 'submit',
          '#tag' => 'button',
          '#text' => t('Set as Fallback/Winner'),
          '#value' => 'winner_' . $option_set->osid . '_' . $option['option_id'],
          '#theme_wrappers' => array('personalize_html_tag'),
          '#attributes' => array(
            // The ID is necessary for the AJAX replace to function correctly.
            'id' => drupal_strtolower('edit-option-sets-option-set-' . trim($option_set->osid, ' -') . '-options-' . trim($option['option_id'], ' -') . '-basic-winner'),
            'class' => array('personalize-add-link'),
            'title' => t('Click here to set as winning variation.')
          ),
          '#submit' => array('personalize_agent_option_sets_winner_submit'),
          '#ajax' => array(
            'callback' => 'personalize_agent_option_sets_ajax_callback',
            'wrapper' => $replace_option_set_id,
            'effect' => 'fade',
            'progress' => array(
              'message' => ' ',
              'type' => 'throbber',
            )
          ),
        );
      }
      if ($targeting_support && !empty($targeting_values)) {
        $section['option_set_' . $option_set->osid]['options'][$option['option_id']]['targeting'] = array(
          '#type' => 'container',
          '#attributes' => array(
            'class' => array('personalize-variation-row'),
          ),
        );
        $section['option_set_' . $option_set->osid]['options'][$option['option_id']]['targeting']['enable_explicit_targeting'] = array(
          '#title' => t('Show to visitors with specific traits'),
          '#type' => 'checkbox',
          '#default_value' => !empty($targeting[$option['option_id']]) ? 1 : 0,
          '#parents' => array('option_sets', 'option_set_' . $option_set->osid, 'options', $option['option_id'],'enable_explicit_targeting'),
        );
        $states = array(
          'visible' => array(
            ':input[name="option_sets[option_set_' . $option_set->osid . '][options][' . $option['option_id'] . '][enable_explicit_targeting]"]' => array('checked' => TRUE),
          ),
        );
        $parents = array('option_sets', 'option_set_' . $option_set->osid, 'options', $option['option_id']);
        $section['option_set_' . $option_set->osid]['options'][$option['option_id']]['explicit_targeting'] = personalize_targeting_support_form_elements($option_set, $option, $targeting_values, $targeting_support, $states, $parents, $form_state);
      }
      $variant_number++;
    } // end of loop through variations.

    switch ($targeting_support) {
      case PersonalizeExplicitTargetingInterface::EXPLICIT_TARGETING_MULTIPLE_AND:
        $section['option_set_' . $option_set->osid]['explicit_targeting_explanation'] = array(
          '#type' => 'markup',
          '#markup' => t('You can add explicit targeting for this content variation. If you choose multiple contexts for an option, then the visitor must have all specified contexts for the rule to apply.')

        );
        break;
      case PersonalizeExplicitTargetingInterface::EXPLICIT_TARGETING_MULTIPLE_OR:
        $section['option_set_' . $option_set->osid]['explicit_targeting_explanation'] = array(
          '#type' => 'markup',
          '#markup' => t('You can add explicit targeting for this content variation. If you choose multiple contexts for an option, then the rule will apply if the visitor has any of the specified contexts.')
        );
        break;
    }

    // Advanced area per option set.
    $section['option_set_' . $option_set->osid]['advanced'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#title' => t('Advanced'),
    );
    $section['option_set_' . $option_set->osid]['advanced']['label'] = array(
      '#type' => 'textfield',
      '#title' => 'Name',
      '#description' => 'Shown in the menu when you are viewing content variations on a page.',
      '#default_value' => isset($option_set->label) ? $option_set->label : '',
    );
    $section['option_set_' . $option_set->osid]['advanced']['decision_name'] = array(
      '#type' => 'textfield',
      '#title' => 'Decision Name',
      '#description' => 'By default, this decision will be named after the content variation ID. You can use this if you want to make one conceptual decision across multiple content variations such as a special offer which manifests in different places on the site.',
      '#default_value' => isset($option_set->decision_name) ? $option_set->decision_name : '',
    );
    $section['option_set_' . $option_set->osid]['advanced']['stateful'] = array(
      '#type' => 'checkbox',
      '#title' => t('Shareable'),
      '#description' => 'Will display what the original visitor saw, not another variation.',
      '#default_value' => isset($option_set->stateful) ? $option_set->stateful : 0,
    );
    $section['option_set_' . $option_set->osid]['advanced']['preview_link'] = array(
      '#type' => 'textfield',
      '#title' => 'Preview link',
      '#description' => t('Enter the internal Drupal path on your site to a page containing this content variation set. Enter &lt;front&gt; to link to the front page.'),
      '#default_value' => !empty($option_set->preview_link) ? $option_set->preview_link : '',
    );
    // Display options to select the executor if supported.
    $executors = personalize_get_executors();
    $supported_executors = module_invoke($option_set_plugin['module'], 'personalize_get_executor_options');
    if (count($supported_executors) == 1) {
      $section['option_set_' . $option_set->osid]['advanced']['executor'] = array(
        '#type' => 'value',
        '#value' => array_shift(array_keys($supported_executors)),
      );
    }
    else if (count($supported_executors) > 1) {
      $default_executor = '';
      $options = array();
      foreach($supported_executors as $executor_name => &$supported_options) {
        // Allow the supported declarations to overwrite the default displays.
        $supported_options += $executors[$executor_name];
        $options[$executor_name] = $supported_options['title'];
        if (isset($supported_options['default']) && $supported_options['default'] === TRUE) {
          $default_executor = $executor_name;
        }
      }

      if (isset($option_set->executor) && isset($options[$option_set->executor])) {
        $default_executor = $option_set->executor;
      }
      if (empty($default_executor)) {
        reset($supported_executors);
        $default_executor = key($supported_executors);
      }

      $section['option_set_' . $option_set->osid]['advanced']['executor'] = array(
        '#type' => 'radios',
        '#title' => t('Rendering'),
        '#options' => $options,
        '#default_value' => $default_executor,
        '#title_display' => 'invisible',
      );
      // Add descriptions to the executor options
      foreach($supported_executors as $executor_name => $options) {
        $section['option_set_' . $option_set -> osid]['advanced']['executor'][$executor_name] = array(
          '#description' => $options['description'] . theme('personalize_admin_info_details', $options),
        );
      }
    }
  } // End loop through option sets.

  $section['actions'] = array('#type' => 'actions');
  $section['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#value' => t('Save variation sets'),
    '#ajax' => array(
      'callback' => 'personalize_agent_form_ajax_submit',
      'wrapper' => 'personalize-agent-option-sets-form',
      'effect' => 'fade',
    ),
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'personalize-admin-warn'),
    ),
  );
  $section['actions']['reset'] = _personalize_agent_get_cancel('personalize-agent-option-sets-form');

  // We have to specify the include file so as not to lose it during rendering from ajax.
  // @see personalize_agent_form_ajax_submit()
  // @see drupal_retrieve_form():734
  $form_state['build_info']['files'] = array(
    drupal_get_path('module', 'personalize') . '/personalize.admin.inc',
  );

  return $form;
}

/**
 * Builds the form elements needed for adding explicit targeting to an option.
 *
 * @param $option_set
 *   The option set this explicit targeting config is for.
 * @param $option
 *   The individual option this explicit targeting config is for.
 * @param $targeting_values
 *   The available context values.
 * @param $targeting_support
 *   How multiple targeting features can be handled, i.e. using AND, OR or either
 *   of these, in which case a dropdown is provided for the user to select.
 * @param $states
 *   An array to use as the states property for these elements.
 * @param $parents
 *   The array of parents of these elements within the larger form
 *
 * @return array
 *   An array representing the explicit targeting portion of the form.
 */
function personalize_targeting_support_form_elements($option_set, $option, $targeting_values, $targeting_support, $states, $parents, &$form_state) {
  $attributes = array(
    'class' => array('personalize-variation-row'),
  );

  $osid = $option_set->osid;
  $form = array(
    '#tree' => TRUE,
    '#type' => 'container',
    '#attributes' => $attributes,
    '#states' => $states,
  );

  $context_options = array('' => '-- ' . t('Select a context') . ' --');
  $value_types = array();
  $operator_options = array(
    'string' => array(
      'equals' => t('equals'),
      'contains' => t('contains'),
      'starts' => t('starts with'),
      'ends' => t('ends with')
    ),
    'number' => array(
      'equals' => t('equals'),
      'numgt' => t('greater than'),
      'numlt' => t('less than')
    )
  );
  foreach ($targeting_values as  $key => $info) {
    $option_key = $info['visitor_context'] . PERSONALIZE_TARGETING_ADMIN_SEPARATOR . $key;
    $key_label = isset($info['friendly name']) ? $info['friendly name'] : $key;
    $context_options[$option_key] = $key_label;
    $value_types[$option_key] = $info['value type'];
  }

  // This is the portion of the form that will be replace when the "add new" or "remove
  // context" links are clicked.
  $main_wrapper_id = drupal_strtolower('personalize-targeting-osid-' . $osid . '-' . $option['option_id']);

  $form['mapping'] = array(
    '#tree' => TRUE,
    '#theme_wrappers' => array('container'),
    '#attributes' => $attributes,
  );
  $form['mapping']['contexts'] = array(
    '#type' => 'container',
    '#states' => $states,
    '#attributes' => array(
      'id' => $main_wrapper_id,
    ),
  );

  // Get an array of targeting rules per option id.
  $targeting_for_options = personalize_get_targeting_for_options($option_set);
  // Load from the existing form if passed first, otherwise, load from option
  // set.
  $mappings = array();
  if (isset($form_state['values']['option_sets']['option_set_' . $osid]['options'][$option['option_id']]['explicit_targeting']['mapping']['contexts'])) {
    foreach($form_state['values']['option_sets']['option_set_' . $osid]['options'][$option['option_id']]['explicit_targeting']['mapping']['contexts'] as $delta => $context) {
      list($plugin_name, $context_option) = explode(PERSONALIZE_TARGETING_ADMIN_SEPARATOR, $context['context']);
      // Important: preserve the delta passed through the form as it is used
      // to determine the item to delete when "remove" is clicked.
      $mappings[$delta] = array(
        'plugin' => $plugin_name,
        'context' => $context_option,
        'operator' => $context['value']['operator'],
        'match' => $context['value']['match'],
      );
    }
  }
  else if (isset($targeting_for_options[$option['option_id']])) {
    $rule = $targeting_for_options[$option['option_id']];
    foreach ($rule['targeting_features'] as $feature) {
      if (isset($rule['targeting_rules'][$feature])) {
        $mappings[] = $rule['targeting_rules'][$feature];
      }
    }
  }

  // If the "Remove" button was clicked for a context, we need to remove that context
  // from the form.
  if (isset($form_state['to_remove']['option_set_' . $osid][$option['option_id']])) {
    unset($mappings[$form_state['to_remove']['option_set_' . $osid][$option['option_id']]]);
    unset($form_state['to_remove']['option_set_' . $osid][$option['option_id']]);
    $form_state['num_contexts']['option_set_' . $osid][$option['option_id']]--;
  }

  // Make sure there is at least an empty context.
  if (empty($mappings)) {
    $mappings[] = array(
      'context' => '',
      'operator' => 'equals',
      'match' => '',
      'plugin' => '',
    );
  }

  // If the "Add another" button was clicked, we need to add contexts to get up
  // to the number indicated.
  $num_contexts = count($mappings);
  if (isset($form_state['num_contexts']['option_set_' . $osid][$option['option_id']]) && $form_state['num_contexts']['option_set_' . $osid][$option['option_id']] > $num_contexts) {
    while ($num_contexts < $form_state['num_contexts']['option_set_' . $osid][$option['option_id']]) {
      $mappings[] = array(
        'context' => '',
        'operator' => 'equals',
        'match' => '',
        'plugin' => '',
      );
      $num_contexts++;
    }
  }

  $form_state['num_contexts']['option_set_' . $osid][$option['option_id']] = count($mappings);
  foreach ($mappings as $delta => $mapping) {
    $wrapper_id = 'operator-dropdown-replace-' . $osid . '-' . $option['option_id'] . '-' . $delta;
    $selected_plugin_context = $mapping['plugin'] . PERSONALIZE_TARGETING_ADMIN_SEPARATOR . $mapping['context'];
    $selected_context = $mapping['context'];
    $selected_operator = $mapping['operator'];
    $selected_match = $mapping['match'];
    $form['mapping']['contexts'][$delta] = array(
      '#prefix' => '<div class="personalize-target-wrapper">',
      '#suffix' => '</div>',
      'context' => array(
        '#type' => 'select',
        '#title' => t('Context'),
        '#options' => $context_options,
        '#default_value' => empty($selected_plugin_context) ? '' : $selected_plugin_context,
        '#ajax' => array(
          'event' => 'change',
          'callback' => 'personalize_explicit_targeting_context_callback',
          'wrapper' => $wrapper_id,
        ),
        '#attributes' => array(
          'class' => array('personalize-targeting-context'),
        ),
      ),
    );
    $form['mapping']['contexts'][$delta]['value'] = array(
      '#tree' => TRUE,
      '#prefix' => '<div id="' . $wrapper_id. '">',
      '#suffix' => '</div>'
    );
    $value_type = isset($value_types[$selected_plugin_context]) ? $value_types[$selected_plugin_context] : 'string';
    // We use different form elements depending on whether there is a predefined list
    // of possible values or not.
    switch ($value_type) {
      case 'predefined':
        $form['mapping']['contexts'][$delta]['value']['operator'] = array(
          '#type' => 'value',
          '#value' => 'equals',
        );
        $form['mapping']['contexts'][$delta]['value']['match'] = array(
          '#type' => 'select',
          '#options' => $targeting_values[$selected_context]['values'],
          '#title' => 'Value',
          '#default_value' => $selected_match,
        );
       break;
      case 'boolean':
        $form['mapping']['contexts'][$delta]['value']['operator'] = array(
          '#type' => 'value',
          '#value' => 'equals',
        );
        $off_label = isset($targeting_values[$selected_context]['off_label']) ? $targeting_values[$selected_context]['off_label'] : 'No';
        $on_label = isset($targeting_values[$selected_context]['on_label']) ? $targeting_values[$selected_context]['on_label'] : 'Yes';
        $form['mapping']['contexts'][$delta]['value']['match'] = array(
          '#type' => 'select',
          '#options' => array(0 => $off_label, 1 => $on_label),
          '#title' => 'Value',
          '#default_value' => $selected_match,
        );
        break;
      default:
        $form['mapping']['contexts'][$delta]['value']['operator'] = array(
          '#type' => 'select',
          '#title' => t('Operator'),
          '#options' => $operator_options[$value_type],
          '#default_value' => $selected_operator
        );
        $form['mapping']['contexts'][$delta]['value']['match'] = array(
          '#type' => 'textfield',
          '#title' => 'Value',
          '#size' => $value_type == 'number' ? 6 : 30,
          '#default_value' => $selected_match,
        );
        break;
    }

    // Add a "remove" button for this context.
    // NOTE: ajax.js expects the ID of the element to match the element's name
    // even when a different selector is passed.
    $form['mapping']['contexts'][$delta]['remove'] = array(
      '#prefix' => '<div class="personalize-remove-context">',
      '#suffix' => '</div>',
      '#type' => 'submit',
      '#tag' => 'button',
      '#text' => t('Remove'),
      '#value' => 'remove_' . $osid . '_' . $option['option_id'] . '_' . $delta,
      '#theme_wrappers' => array('personalize_html_tag'),
      '#attributes' => array(
        'class' => array('personalize-delete-context', 'form-submit',),
        'title' => t('Delete this context.'),
        'id' => drupal_strtolower('edit-option-sets-option-set-' . trim($osid, '- ') . '-options-' . trim($option['option_id'], ' -') . '-explicit-targeting-mapping-contexts-' . $delta . '-remove'),
      ),
      '#submit' => array('personalize_targeting_remove_context_submit'),
      '#ajax' => array(
        'callback' => 'personalize_targeting_context_ajax_callback',
        'wrapper' => $main_wrapper_id,
        'effect' => 'fade',
      ),
    );
  }
  // Create an "add new context" link.
  $form['add_new'] = array(
    '#prefix' => '<span class="personalize-add-link-prefix"></span>',
    '#type' => 'submit',
    '#tag' => 'button',
    '#text' => t('Add context'),
    '#value' => 'add_context_' . $osid . '_' . $option['option_id'],
    '#theme_wrappers' => array('personalize_html_tag'),
    '#attributes' => array(
      'id' => drupal_strtolower('edit-option-sets-option-set-' . trim($osid, ' -') . '-options-' . trim($option['option_id'], ' -') . '-explicit-targeting-add-new'),
      'class' => array('personalize-add-link'),
      'title' => t('Click here to add more contexts.')
    ),
    '#submit' => array('personalize_targeting_add_context_submit'),
    '#ajax' => array(
      'callback' => 'personalize_targeting_context_ajax_callback',
      'wrapper' => $main_wrapper_id,
      'effect' => 'fade',
    ),
  );
  $strategy_parents = $parents;
  foreach (array('explicit_targeting', 'strategy') as $parent) {
    $strategy_parents[] = $parent;
  }
  // And/Or targeting selection.
  if ($targeting_support == PersonalizeExplicitTargetingInterface::EXPLICIT_TARGETING_MULTIPLE_BOTH) {
    $form['strategies'] = array(
      '#type' => 'container',
      '#attributes' => $attributes,
      '#states' => $states,
    );

    // Add radio buttons so the user can select how multiple features for an option
    // should be treated.
    $default_strategy = 'OR';
    if (isset($targeting_for_options[$option['option_id']]) && isset($targeting_for_options[$option['option_id']]['targeting_strategy'])) {
      $default_strategy = $targeting_for_options[$option['option_id']]['targeting_strategy'];
    }
    $form['strategies']['strategy'] = array(
      '#type' => 'select',
      '#multiple' => FALSE,
      '#field_prefix' => t('Visitor must have '),
      '#field_suffix' => t(' of the specified contexts'),
      '#description' => t('Choose how multiple contexts should be applied to options. Choose "any" if the rule should apply if the user has any of the contexts. Choose "all" if the rule should apply only if the user has all of the contexts.'),
      '#options' => array(
        'OR' => 'any',
        'AND' => 'all'
      ),
      '#default_value' => $default_strategy,
      '#parents' => $strategy_parents,
    );
  }
  else {
    // Send the strategy as a value so it can be used when checking for features
    // that have been assigned to multiple options.
    $form['strategy'] = array(
      '#type' => 'value',
      '#value' => PersonalizeExplicitTargetingInterface::EXPLICIT_TARGETING_MULTIPLE_OR ? 'OR' : 'AND',
      '#parents' => $strategy_parents,
    );
  }
  return $form;
}

/**
 * Ajax callback for the add context and remove context buttons.
 */
function personalize_targeting_context_ajax_callback($form, $form_state) {
  $parents = $form_state['triggering_element']['#array_parents'];
  $section = $form;
  $pos = array_search('explicit_targeting', $parents);
  for ($i = 0; $i <= $pos; $i++) {
    $section = $section[$parents[$i]];
  }
  return $section['mapping']['contexts'];
}

/**
 * Submit handler for the "Add Context" button.
 */
function personalize_targeting_add_context_submit($form, &$form_state) {
  $parents = $form_state['clicked_button']['#parents'];
  $option_set = $parents[1];
  $option_id = $parents[3];
  // Increment the number of goal elements to be rendered.
  $form_state['num_contexts'][$option_set][$option_id]++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "Remove Context" button.
 */
function personalize_targeting_remove_context_submit($form, &$form_state) {
  $parents = $form_state['clicked_button']['#parents'];
  // The parents are option_sets, option_set_[osid], options, [option_id],
  // explicit_targeting, mapping, contexts, [delta]. At least if we only depend
  // on the position of the immediate parent of each element we need, that makes
  // this a little less vulnerable to future chnages made to the form structure.
  $option_set_pos = array_search('option_sets', $parents) + 1;
  $option_id_pos = array_search('options', $parents) + 1;
  $delta_pos = array_search('contexts', $parents) + 1;
  $option_set = $parents[$option_set_pos];
  $option_id = $parents[$option_id_pos];
  $delta = $parents[$delta_pos];
  $form_state['to_remove'][$option_set][$option_id] = $delta;
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback for the explicit targeting context dropdown.
 *
 * Returns the operator and value portion of the form, which are different
 * depending on what was selected in the context dropdown.
 */
function personalize_explicit_targeting_context_callback($form, &$form_state) {
  // We need to return the section of the form corresponding to one level up from
  // the dropdown that triggered the ajax, so use the array parents to find that
  // section.
  $parents = $form_state['triggering_element']['#array_parents'];
  $section = $form;
  for ($i = 0; $i < count($parents) -1; $i++) {
    $section = $section[$parents[$i]];
  }
  return $section['value'];
}

/**
 * Helper function to return targeting values for a particular visitor context
 * plugin and agent.
 *
 * This function returns the results in an array suitable for fixed targeting.
 *
 * @param string $plugin_name
 *   The name of the plugin to get targeting values from.
 * @param stdClass $agent_data
 *   The agent data for the current campaign.
 * @param array $selected_context
 *   (Optional) An array of pre-selected contexts for the agent in order to
 *   limit the returned values.  Only pass if limit is desired.
 * @return array
 *   An associative array keyed by context name, with an array of information about the
 *   possible values for that context. The 'value type' key in this array indicates what
 *   type of value this field can hold and is one of 'predefined', 'string' or 'number'. If
 *   the value type is 'predefined', then there must be a 'values' array with the list of
 *   predefined possible values. For example:
 *     array(
 *       'some_context' => array(
 *         'label' => 'Some Context',
 *         'type' => 'string',
 *         'visitor_context' => 'Plugin name',
 *        ),
 *       'some_other_context' => array(
 *         'label' => 'Some Other Context',
 *         'type' => 'predefined',
 *         'visitor_context' => 'Plugin name',
 *         'values' => array(
 *           'some_value' => 'Some Value',
 *           'some_other_value' => 'Some Other Value',
 *         ),
 *       ),
 *     )
 */
function _personalize_get_targeting_values($plugin_name, $agent_data, $limit_to_context = NULL) {
  ctools_include('plugins');
  $values = array();
  $agent_instance = personalize_agent_load_agent($agent_data->machine_name);
  $limit = TRUE;
  if (!isset($limit_to_context)) {
    $limit = FALSE;
    $limit_to_context = array();
  }

  if ($class = ctools_plugin_load_class('personalize', 'visitor_context', $plugin_name, 'handler')) {
    // Make sure this agent is allowed to use this context.
    if (!call_user_func(array($class, 'allowedFromAgent'), $agent_data)) {
      return $values;
    }
    if ($plugin = call_user_func_array(array($class, 'create'), array($agent_instance, array_keys($limit_to_context)))) {
      // Get the values and add in the source visitor context plugin name.
      $values = $plugin->getPossibleValues($limit);

      array_walk($values, function (&$item, $key, $visitor_context_plugin) {
        $item['visitor_context'] = $visitor_context_plugin;
      }, $plugin_name);
    }
  }
  return $values;
}

/**
 * Extracts targeting values from those set up on the agent available for fixed
 * targeting.
 *
 * @param stdClass $agent_data
 *   The personalize agent plugin data.
 * @return array
 *   An associative array keyed by context name, with an array of information
 *   about the possible values for that context.
 */
function personalize_get_targeting_values_for_agent($agent_data) {
  $agent_instance = personalize_agent_load_agent($agent_data->machine_name);
  if ($agent_instance instanceof PersonalizeAutoTargetingInterface && $agent_instance::constrainExplicitTargetingContexts()) {
    $value_options = array();
    if (!empty($agent_data->data['visitor_context'])) {
      foreach ($agent_data->data['visitor_context'] as $name => $contexts) {
        $selected_context = array_filter($contexts);
        if (!empty($selected_context)) {
          $value_options = array_merge($value_options, _personalize_get_targeting_values($name, $agent_data, $selected_context));
        }
      }
    }
    return $value_options;
  }
  // Otherwise the targeting values for an agent are the same as the options
  // available.
  return personalize_get_targeting_options_for_agent($agent_data);
}

/**
 * Gets all targeting options that can be selected for an agent.
 *
 * @param stdClass $agent_data
 *   The personalize agent plugin data.
 * @return array
 *   An associative array keyed by context name, with an array of information
 *   about the possible values for that context.
 */
function personalize_get_targeting_options_for_agent($agent_data) {
  $contexts = personalize_get_visitor_contexts();
  $value_options = array();
  foreach ($contexts as $plugin_name => $plugin_info) {
    $value_options = array_merge($value_options, _personalize_get_targeting_values($plugin_name, $agent_data));
  }
  return $value_options;
}

/**
 * Form for creating a MVT for an agent.
 */
function personalize_agent_mvt_form($form, &$form_state, $agent_data) {
  // Only return an MVT creation form if the agent has at least 2 option sets.
  $option_sets = personalize_option_set_load_by_agent($agent_data->machine_name);
  if (count($option_sets) < 2) {
    return array();
  }
  $mvts = personalize_mvt_load_all_by_agent($agent_data->machine_name);

  $form = array();
  $form['mvt_wrapper'] = array(
    '#tree' => FALSE,
    '#type' => 'personalize_admin_section',
    'primary' => array(
      '#theme_wrappers' => array('container'),
    ),
  );
  $collapsed = '';
  foreach($mvts as $delta => &$mvt) {
    $mvt->display_label = theme('personalize_admin_enumerated_item', array(
      'enum' => t('Test @delta', array('@delta' => ($delta + 1))),
      'title' => check_plain($mvt->label),
    ));
    $collapsed .= $mvt->display_label;
  }
  $form['mvt_wrapper']['mvt'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#tree' => TRUE,
    '#title' => t('Multivariate tests'),
    '#summary' => $collapsed,
    '#theme_wrappers' => array('personalize_admin_content'),
  );
  $form['mvt_wrapper']['mvt']['agent'] = array(
    '#type' => 'value',
    '#value' => $agent_data->machine_name,
  );
  // Show a list of existing MVTs with links to edit them.
  if (!empty($mvts)) {
    foreach ($mvts as $delta => $existing) {
      $os_names = array();
      foreach ($existing->option_sets as $os) {
        $os_names[] = check_plain($os->label);
      }
      $label = $existing->display_label;
      if (!empty($os_names)) {
        $label .= ': ' . implode('/', $os_names);
      }
      $form['mvt_wrapper']['mvt'][$delta] = array(
        '#markup' => $label . ' - ' . l(t('edit'), "admin/structure/personalize/mvt/manage/{$existing->machine_name}/edit"),
        '#attributes' => array(
          'class' => array('personalize-mvt-edit'),
        ),
        '#theme_wrappers' => array('container'),
      );
    }
  }
  // Adds the form to add a new MVT.
  $form['mvt_wrapper']['mvt']['add'] = array(
    '#type' => 'fieldset',
    '#collapsed' => TRUE,
    '#collapsible' => TRUE,
    '#title' => t('Add multivariate test'),
  );
  $elements = personalize_mvt_form_elements(NULL, $agent_data->machine_name, FALSE);
  $form['mvt_wrapper']['mvt']['add'] += $elements;

  $form['#prefix'] = !empty($form['#prefix']) ? '<div id="personalize-agent-mvt-form">' . $form['#prefix']
    : '<div id="personalize-agent-mvt-form">';
  $form['#suffix'] = !empty($form['#suffix']) ? $form['#suffix'] . '</div>' : '</div>';

  $form['mvt_wrapper']['mvt']['add']['actions'] = array('#type' => 'actions');
  $form['mvt_wrapper']['mvt']['add']['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#value' => t('Save test'),
    '#ajax' => array(
      'callback' => 'personalize_agent_form_ajax_submit',
      'wrapper' => 'personalize-agent-mvt-form',
      'effect' => 'fade',
    ),
    '#attributes' => array(
      'class' => array('action-item-primary-active', 'personalize-admin-warn'),
    ),
  );
  $form['mvt_wrapper']['mvt']['add']['actions']['reset'] = _personalize_agent_get_cancel('personalize-agent-mvt-form');

  return $form;
}

/**
 * Generate the render array for a cancel button to be included on the agent
 * edit form.
 *
 * @param string $wrapper
 *   The wrapping div id for Ajax callback.
 * @param string $button_text
 *   (optional) The text to display on the button (already translated).
 */
function _personalize_agent_get_cancel($wrapper, $button_text = NULL) {
  if (empty($button_text)) {
    $button_text = t('Cancel');
  }
  return array(
    '#type' => 'button',
    '#button_type' => 'reset',
    '#value' => $button_text,
    '#ajax' => array(
      'callback' => 'personalize_agent_form_ajax_reset',
      'wrapper' => $wrapper,
      'effect' => 'fade',
    ),
    '#validate' => array(),
    '#limit_validation_errors' => array(),
    '#attributes' => array(
      'class' => array('form-submit'),
    ),
  );
}

/**
 * Ajax callback for the add goal and remove goal buttons.
 */
function personalize_agent_goals_ajax_callback($form, $form_state) {
  return $form['variations']['primary']['goals_fieldset']['goals'];
}

/**
 * Submit handler for the "Add Goal" button.
 */
function personalize_agent_add_goal_submit($form, &$form_state) {
  // Increment the number of goal elements to be rendered.
  $form_state['num_goals']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "Remove Goal" button.
 */
function personalize_agent_remove_goal_submit($form, &$form_state) {
  // Get the goal delta for the clicked button.
  $delta = $form_state['clicked_button']['#parents'][1];
  $form_state['to_remove'] = $delta;
  $form_state['rebuild'] = TRUE;
}

/**
 * Validation callback for agent add/edit form.
 */
function personalize_agent_form_validate($form, &$form_state) {
  $agent_type = $form_state['values']['agent_basic_info']['agent_type'];
  ctools_include('plugins');
  if ($class = ctools_plugin_load_class('personalize', 'agent_type', $agent_type, 'handler')) {
    // Call the validation callback for the agent type.
    $class::optionsFormValidate($form, $form_state, array('agent_basic_info', 'options', $agent_type));
  }
  personalize_agent_date_form_validate($form_state['values']);
}

/**
 * Submit handler for agent add/edit form.
 */
function personalize_agent_form_submit($form, &$form_state) {
  $agent = _personalize_agent_from_form_values($form_state['values']['agent_basic_info']);

  // Now save the agent itself, along with visitor context configuration.
  $visitor_context = array();
  if (isset($form_state['values']['visitor_context'])) {
    $visitor_context = personalize_admin_convert_visitor_context_form_values($form_state['values']['visitor_context']);
  }
  $agent->data['visitor_context'] = $visitor_context;
  $agent->data['cache_decisions'] = $form_state['values']['cache_decisions'];
  $exists = personalize_agent_load($agent->machine_name);
  if (personalize_agent_save($agent)) {
    drupal_set_message(t('The campaign has been @saved.', array('@saved' => $exists ? 'updated' : 'created')));
    personalize_set_campaign_context($agent->machine_name);
    if (personalize_agent_get_status($agent->machine_name) == PERSONALIZE_STATUS_RUNNING) {
      personalize_agent_set_status($agent->machine_name, PERSONALIZE_STATUS_PAUSED);
      personalize_status_toggle_message($agent->machine_name, PERSONALIZE_STATUS_PAUSED);
    }
    personalize_agent_date_form_submit($agent->machine_name, $form_state['values']);
    $form_state['redirect'] = "admin/structure/personalize/manage/{$agent->machine_name}/edit";
  }
  else {
    drupal_set_message(t('There was a problem saving the campaign.'));
    $form_state['redirect'] = "admin/structure/personalize";
  }
}

/**
 * Submit callback for the goal edit form.
 */
function personalize_agent_goals_form_submit($form, &$form_state) {
  $agent_name = $form_state['values']['agent'];

  // Update the goals for this agent.
  $form_values = $form_state['values']['goals'];

  // Check if any goals have been removed during editing.
  $saved_goals = $existing_goals = array();
  foreach (personalize_goal_load_by_conditions(array('agent' => $agent_name)) as $goal) {
    // We can only have one goal with a given action per agent, so
    // we key the array of existing goals by action name.
    $existing_goals[$goal->action] = $goal->id;
  }
  foreach ($form_values as $goal) {
    if (!empty($goal['action_name'])) {
      try {
        personalize_goal_save($agent_name, $goal['action_name'], $goal['value']);

        // Add this to the array of saved goals so that at the end
        // we know that anything in $existing_goals but not in $saved_goals
        // needs to be deleted.
        $saved_goals[$goal['action_name']] = isset($existing_goals[$goal['action_name']]) ? $existing_goals[$goal['action_name']] : 'new';
      }
      catch (Exception $e) {
        drupal_set_message($e->getMessage(), 'error');
      }
    }
  }
  // The difference between existing goals and saved goals are the ones
  // that need to be deleted.
  $to_delete = array_diff_key($existing_goals, $saved_goals);
  foreach ($to_delete as $goal_id) {
    personalize_goal_delete($goal_id);
  }
  // Redraw the form without any additional new goals included.
  unset($form_state['num_goals']);
  $form_state['rebuild'] = TRUE;
}

/**
 * Custom validation callback for Option sets form.
 *
 * Clean Explicit targeting if "Enable explicit targeting" is unchecked.
 * Ensure that preview links are internal and valid.
 */
function personalize_agent_option_sets_form_validate($form, &$form_state) {
  $form_values = &$form_state['values']['option_sets'];

  $need_form_state_cache_clear = FALSE;

  foreach ($form_values as $option_set_id => $values) {
    if (!empty($form_values[$option_set_id]['advanced']['preview_link'])) {
      if ($form_values[$option_set_id]['advanced']['preview_link'] != '<front>' && drupal_lookup_path('source', $form_values[$option_set_id]['advanced']['preview_link']) === FALSE) {
        form_set_error('option_sets][' . $option_set_id . '][advanced][preview_link', t('The preview link for "@option_set" must be a valid internal Drupal path.', array(
          '@option_set' => $form_values[$option_set_id]['advanced']['label'],
        )));
      }
    }
    if (!empty($form_values[$option_set_id]['options'])) {
      foreach ($form_values[$option_set_id]['options'] as $option_name => $option) {
        if (isset($option['enable_explicit_targeting']) && $option['enable_explicit_targeting'] == 0) {
          $form_values[$option_set_id]['options'][$option_name]['explicit_targeting']['mapping'] = array();
          $need_form_state_cache_clear = TRUE;
        }
      }
    }
  }

  // Drupal caches form_state before validators are triggered, so flush it
  if ($need_form_state_cache_clear) {
    cache_clear_all('form_state_' . $form_state['values']['form_build_id'], 'cache_form');
  }
}

/**
 * Ajax submit callback to set the winning variation within an option set.
 */
function personalize_agent_option_sets_winner_submit($form, &$form_state) {
  list($prefix, $osid, $option_id) = explode('_', $form_state['triggering_element']['#value']);
  $form_state['winners']['option_set_' . $osid] = $option_id;
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback to return a portion of the option set form.
 */
function personalize_agent_option_sets_ajax_callback($form, &$form_state) {
  list($prefix, $osid, $option_id) = explode('_', $form_state['triggering_element']['#value']);
  return $form['variations']['primary']['option_sets']['option_set_' . $osid];
}

/**
 * Submit callback for the option set edit form.
 */
function personalize_agent_option_sets_form_submit($form, &$form_state) {
  $agent_name = $form_state['values']['agent'];
  $form_values = $form_state['values']['option_sets'];
  foreach ($form_values as $option_set_id => $values) {
    $osid = (int) str_replace('option_set_', '', $option_set_id);
    if ($option_set = personalize_option_set_load($osid)) {
      $option_set->label = $values['advanced']['label'];
      $option_set->stateful = $values['advanced']['stateful'];
      if (isset($values['advanced']['executor'])) {
        $option_set->executor = $values['advanced']['executor'];
      }
      if (isset($values['advanced']['decision_name'])) {
        $option_set->decision_name = personalize_generate_machine_name($values['advanced']['decision_name']);
      }
      if (isset($values['advanced']['preview_link'])) {
        $option_set->preview_link = $values['advanced']['preview_link'];
      }
      if (isset($values['winner'])) {
        $option_set->winner = $values['winner'];
      }
      if (!isset($values['options'])) {
        personalize_option_set_save($option_set);
        continue;
      }

      // Keep track of which explicit targeting features have already been
      // designated as each can only be specified for one option.
      $agent = personalize_agent_load_agent($agent_name);
      // Keep track of feature strings that have already been designated to options,
      // which options they have been designated to, and whether there's a rule
      // associated with each feature.
      $designated_features = $targeting = array();
      $target_weight = 0;
      foreach ($values['options'] as $option_id => $settings) {
        $option_features[$option_id] = $feature_rules[$option_id] = array();
        $feature_values = $rules = array();
        if (isset($settings['explicit_targeting']) && !empty($settings['explicit_targeting']['mapping'])) {
          foreach ($settings['explicit_targeting']['mapping']['contexts'] as $delta => $context_values) {
            if ($context_values['context'] == '') {
              continue;
            }
            list($plugin_name, $context_name) = explode(PERSONALIZE_TARGETING_ADMIN_SEPARATOR, $context_values['context']);
            $context_values['match'] = $context_values['value']['match'];
            $context_values['operator'] = $context_values['value']['operator'];
            unset($context_values['value'], $context_values['remove']);
            // Generate a value code based on the operator used.
            $value = personalize_targeting_generate_value_code($context_values['match'], $context_values['operator']);
            // Create a feature string for this context value that can be consumed
            // by the agent that will be using it.
            $feature_string = $agent->convertContextToFeatureString($context_name, $value);
            $feature_values[] = $feature_string;
            // Save the actual rule information as this is what will be used
            // for evaluating it.
            $rules[$feature_string] = $context_values;
            // Override the context to split it into plugin and context parts.
            $rules[$feature_string]['context'] = $context_name;
            $rules[$feature_string]['plugin'] = $plugin_name;
          }
          foreach ($feature_values as $feature) {
            // Mark this feature string as already designated.
            if (!in_array($feature, $designated_features)) {
              $designated_features[] = $feature;
            }
            else {
              drupal_set_message(t('You have set the same targeting feature for more than one option. Depending on whether features are AND\'d or OR\'d together for the different options, and on how the decision agent evaluates explicit targeting rules, this may not produce the desired effect.'), 'warning');
            }
          }
          $rule = array(
            'option_id' => $option_id,
            'targeting_features' => $feature_values,
            'targeting_rules' => $rules,
            'weight' => $target_weight,
          );
          // If a strategy has been specified for how to use the targeting features,
          // then store this as well.
          if (isset($values['options'][$option_id]['explicit_targeting']['strategy'])) {
            $rule['targeting_strategy'] = $values['options'][$option_id]['explicit_targeting']['strategy'];
          }
          $targeting['target_' . ($target_weight+1)] = $rule;
          $target_weight++;
        }
      }

      $option_set->targeting = $targeting;
      personalize_option_set_save($option_set);
    }
  }
  $form_state['redirect'] = "admin/structure/personalize/manage/$agent_name/edit";
}

/**
 * Submit callback for the MVT creation form.
 */
function personalize_agent_mvt_form_submit($form, &$form_state) {
  $agent_name = $form_state['values']['mvt']['add']['mvt_basic_info']['agent_select'];
  personalize_mvt_save_from_form_values($form_state['values']['mvt']['add']['mvt_basic_info']);
  $form_state['redirect'] = "admin/structure/personalize/manage/$agent_name/edit";
}


/**
 * Validation callback for the MVT creation form.
 */
function personalize_agent_mvt_form_validate($form, &$form_state) {
  if (!empty($form_state['values']['mvt']['add']) && count($form_state['values']['mvt']['add']['mvt_basic_info']['option_sets']) < 2) {
    form_set_error('mvt[create_new]', t('You must choose at least two option sets to create a new multivariate test.'));
  }
}

/**
 * Saves an MVT for an agent.
 *
 * @param $agent
 *   The agent the MVT belongs to.
 * @param $form_values
 *   The form values corresponding to the MVT part of the agent form.
 */
function personalize_mvt_save_from_form_values($form_values) {
  if (isset($form_values['machine_name'])) {
    $mvt = personalize_mvt_load($form_values['machine_name']);
  }
  else {
    $mvt = new stdClass();
    $mvt->agent = $form_values['agent_select'];
  }
  $mvt->label = $form_values['label'];
  $mvt->stateful = $form_values['stateful'];
  $mvt->option_sets = $form_values['option_sets'];

  personalize_mvt_save($mvt);

}

/**
 * Menu callback for displaying a list of campaign results.
 *
 * @param stdClass $agent_data
 *   The campaign class for the report.
 * @param int $osid
 *   (optional) Content variation set id to use for initial display.
 */
function personalize_campaign_report($agent_data, $osid = NULL) {
  // The page title of the simple campaign name is taken from
  // personalize_campaign_title_callback().  In order to give the reports tab
  // a specific title, explicitly update the title.
  $report_tab_title = t('@campaign Campaign Report', array('@campaign' => drupal_get_title()));
  drupal_set_title($report_tab_title);

  $plugins = personalize_get_agent_types();
  if (isset($plugins[$agent_data->plugin])) {
    $module = $plugins[$agent_data->plugin]['module'];
    $option_set = !empty($osid) ? personalize_option_set_load($osid) : NULL;
    $report = module_invoke($module, 'personalize_campaign_report', $agent_data, $option_set);
    if (!empty($report)) {
      return $report;
    }
  }
  return t('No reports available for the specified campaign');
}

/**
 * Clone agent.
 */
function personalize_agent_clone($agent) {
  unset($agent->machine_name);
  $agent->label = '';
  return drupal_get_form('personalize_agent_form', 'clone', $agent);
}

/**
 * Form for deleting an agent.
 */
function personalize_agent_delete_form($form, $form_state, $agent) {
  $form['machine_name'] = array('#type' => 'hidden', '#value' => $agent->machine_name);
  $form['title'] = array('#type' => 'hidden', '#value' => $agent->label);
  return confirm_form($form, t('Are you sure you want to delete the agent %title?', array('%title' => $agent->label)), 'admin/structure/personalize', '', t('Delete'), t('Cancel'));
}

/**
 * Submit handler for agent deletion form.
 */
function personalize_agent_delete_form_submit($form, &$form_state) {
  personalize_agent_delete($form_state['values']['machine_name']);
  drupal_set_message(t('The agent %name has been removed.', array('%name' => $form_state['values']['title'])));
  $form_state['redirect'] = 'admin/structure/personalize';
}

/**
 * Export form for agents.
 */
function personalize_agent_export_form($form, &$form_state, $agent) {
  ctools_include('export');
  // Generate export code
  $code = '$items = array();' ."\n";
  $code .= ctools_export_object('personalize_agent', $agent, '');
  $code .= '$items["'. $agent->machine_name .'"] = $personalize_agent;' ."\n";
  $code .= 'return $items;';

  // Create form
  $form = array();
  $form['export'] = array(
    '#type' => 'textarea',
    '#default_value' => $code,
    '#rows' => substr_count($code, "\n") + 1,
    '#resizable' => FALSE,
    '#description' => t('Place this code in your module\'s implementation of <code>hook_personalize_default_agents()</code> to provide it as a default agent.'),
  );
  $form['done'] = array(
    '#type' => 'submit',
    '#value' => t('Done'),
  );
  $form['#redirect'] = 'admin/structure/personalize';

  return $form;
}

/**
 * Helper function to convert the admin form data into an agent object.
 */
function _personalize_agent_from_form_values($values) {
  $agent_type = $values['agent_type'];
  $agent = new stdClass();
  $agent->label = $values['title'];
  $agent->plugin = $agent_type;
  $agent->data = isset($values['options'][$agent_type]) ? $values['options'][$agent_type] : array();
  if (isset($values['machine_name'])) {
    $agent->machine_name = $values['machine_name'];
  }
  return $agent;
}

/**
 * =======================================================================
 *  M V T  M A N A G E M E N T
 * =======================================================================
 */

/**
 * Lists all MVTs
 */
function personalize_mvt_list() {

  $header = array(
    array('data' => t('Test')),
    array('data' => t('Operations'), 'colspan' => 3),
  );
  $rows = array();

  foreach (personalize_mvt_load_all() as $mvt) {

    $tablerow = array(
      array('data' => check_plain($mvt->label)),
      array('data' => l(t('Edit'), 'admin/structure/personalize/mvt/manage/'. $mvt->machine_name .'/edit')),
      array('data' => l(t('Export'), 'admin/structure/personalize/mvt/manage/'. $mvt->machine_name .'/export')),
      array('data' => l(t('Delete'), 'admin/structure/personalize/mvt/manage/'. $mvt->machine_name .'/delete')),
    );
    $rows[] = $tablerow;
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No MVTS available.'), 'colspan' => 4));
  }

  $build = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
    '#attributes' => array('id' => 'personalize'),
  );
  return $build;
}

/**
 * Form for editing an existing MVT.
 */
function personalize_mvt_form($form, &$form_state, $mvt) {
  $form = personalize_mvt_form_elements($mvt, $mvt->agent);
  $form['actions']['submit_form'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#attributes' => array(
      'class' => array('action-item-primary-active'),
    ),
  );
  return $form;
}

/**
 * Returns the actual form elements using for creating or editing an MVT.
 *
 * See the above two forms.
 *
 * @param $mvt
 *   An object representing the existing MVT or null if this is a creation form.
 * @param $agent
 *   The name of the agent for which this MVT is being added / edited.
 * @param bool $include_machine_name_field
 *   Whether to include the machine name field.
 * @return array
 *   An array of form elements.
 */
function personalize_mvt_form_elements($mvt = NULL, $agent) {
  if (empty($mvt)) {
    $mvt = new stdClass();
  }
  $form = array();
  $form['mvt_basic_info'] = array(
    '#tree' => TRUE
  );
  $form['mvt_basic_info']['label'] = array(
    '#title' => t('Administrative title'),
    '#type' => 'textfield',
    '#default_value' => isset($mvt->label) ? check_plain($mvt->label) : '',
    '#weight' => -9,
    '#required' => TRUE,
  );

  if (!empty($mvt->machine_name)) {
    $form['mvt_basic_info']['machine_name'] = array(
      '#type' => 'value',
      '#value' => $mvt->machine_name,
    );
  }

  $form['mvt_basic_info']['agent_select'] = array(
    '#type' => 'value',
    '#value' => $agent,
  );

  $option_sets = personalize_option_set_load_by_agent($agent);
  $option_set_options = array();
  foreach ($option_sets as $osid => $option_set) {
    $option_set_options[$osid] = check_plain($option_set->label);
  }
  $default = isset($mvt->option_sets) ? array_keys($mvt->option_sets) : array();
  $form['mvt_basic_info']['option_sets'] = array(
    '#type' => 'select',
    '#title' => t('Option Sets'),
    '#multiple' => TRUE,
    '#options' => $option_set_options,
    '#default_value' => $default,
  );
  $form['mvt_basic_info']['stateful'] = array(
    '#type' => 'checkbox',
    '#title' => t('Shareable'),
    '#description' => t('If a visitor shares the URL the receiver will see what the sharer saw, not a personalized variation.'),
    '#default_value' => isset($mvt->stateful) ? $mvt->stateful : 0,
  );
  return $form;
}

/**
 * Submit handler for the MVT add/edit form.
 */
function personalize_mvt_form_submit($form, &$form_state) {
  personalize_mvt_save_from_form_values($form_state['values']['mvt_basic_info']);
}

/**
 * Form for deleting an MVT.
 */
function personalize_mvt_delete_form($form, $form_state, $mvt) {
  $form['machine_name'] = array('#type' => 'hidden', '#value' => $mvt->machine_name);
  $form['title'] = array('#type' => 'hidden', '#value' => $mvt->label);
  return confirm_form($form, t('Are you sure you want to delete the multivariate test %title?', array('%title' => $mvt->label)), 'admin/structure/personalize/mvt', '', t('Delete'), t('Cancel'));
}

/**
 * Submit handler for MVT deletion form.
 */
function personalize_mvt_delete_form_submit($form, &$form_state) {
  personalize_mvt_delete($form_state['values']['machine_name']);
  drupal_set_message(t('The MVT %name has been removed.', array('%name' => $form_state['values']['title'])));
  $form_state['redirect'] = 'admin/structure/personalize/mvt';
}

/**
 * =======================================================================
 *  E N D  M V T  R E L A T E D
 * =======================================================================
 */

/**
 * =======================================================================
 *  A J A X   C A L L B A C K S
 * =======================================================================
 */

/**
 * AJAX callback for getting the active campaign context.
 */
function personalize_get_campaign_context_callback() {
  $ret = array(
    'personalize_campaign' => personalize_get_campaign_context(),
  );
  drupal_json_output($ret);
}

/**
 * AJAX callback for setting the active campaign context.
 *
 * @param object $agent_data
 *   A Personalize Agent object.
 */
function personalize_set_campaign_context_callback($agent_data) {
  if (isset($agent_data)) {
    $name = $agent_data->machine_name;
    // Set the SESSION variable personalize_campaign.
    personalize_set_campaign_context($name);
    // Return a response with this campaign name to the caller.
    $ret = array(
      'personalize_campaign' => $name,
    );
    drupal_json_output($ret);
  }
  else {
    throw new Exception('No campaign was specified.');
  }
}

/**
 * =======================================================================
 *  E N D   A J A X   C A L L B A C K S
 * =======================================================================
 */

/**
 * Landing page for configuring agents.
 */
function personalize_admin_page() {
  return l(t('manage agents'), 'admin/content/personalize/agents');
}

/**
 * Retrieves a list of existing actions as goal options.
 *
 * @param boolean $exclude_limited_use
 *   Whether to exclude those goals that have been defined as limited use.
 * @return array
 *   An associative array with goal names as keys and labels as values.
 */
function personalize_goals_options($exclude_limited_use = FALSE) {
  $actions = visitor_actions_get_actions($exclude_limited_use);
  $options = array();
  foreach($actions as $name => $info) {
    $options[$name] = isset($info['label']) ? check_plain($info['label']) : $name;
  }
  return $options;
}

/**
 * Maps operators onto 2-character codes.
 *
 * These are only used to differentiate a string that needs to be
 * evaluated from a string holding a value to be matched exactly.
 *
 * @return array
 *
 */
function personalize_targeting_operator_map() {
  return array(
    'contains' => 'sc',
    'starts' => 'ss',
    'ends' => 'se',
    'numgt' => 'ng',
    'numlt' => 'nl',
  );
}

/**
 * Generates the 'value' string to be used when creating the feature
 * string for explicit targeting.
 *
 * If an operator other than 'equals' is passed in, the value is prefixed
 * with a string representing the operator.
 *
 * @param $value_match
 * @param $operator
 * @return string
 */
function personalize_targeting_generate_value_code($value_match, $operator) {
  if ($operator == 'equals') {
    return $value_match;
  }
  $operator_map = personalize_targeting_operator_map();
  if (!isset($operator_map[$operator])) {
    return $value_match;
  }
  return $operator_map[$operator] . PERSONALIZE_TARGETING_OP_SEPARATOR . $value_match;
}

/**
 * Returns the targeting that has been defined for the options in an option set.
 *
 * @param $option_set
 *   The option set to get targeting info for.
 * @return array
 *   An associative array, keyed by option_id, whose values are arrays of target-
 *   ing info for the option in question. If any option does not have targeting
 *   set up, it will not be included in the array.
 */
function personalize_get_targeting_for_options($option_set) {
  // Get an array of targeting rules per option id.
  $targeting_for_options = array();
  if (!empty($option_set->targeting)) {
    foreach ($option_set->targeting as $name => $rule) {
      if (isset($rule['option_id'])) {
        $targeting_for_options[$rule['option_id']] = $rule;
        $targeting_for_options[$rule['option_id']]['name'] = $name;
      }
    }
  }
  return $targeting_for_options;
}
