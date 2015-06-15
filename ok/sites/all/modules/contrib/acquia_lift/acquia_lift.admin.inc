<?php

/**
 * @file acquia_lift.admin.inc
 * Provides functions needed for the admin UI.
 */

/**
 * Menu callback for the Acquia Lift settings page.
 *
 * Consists of multiple forms.
 *
 * @return array
 *   A render array for the page.
 */
function acquia_lift_configuration_page() {
  $build['main_config'] = drupal_get_form('acquia_lift_admin_form');
  $build['batch_sync'] = drupal_get_form('acquia_lift_batch_sync_form');
  return $build;
}

/**
 * Admin form for configuring personalization backends.
 */
function acquia_lift_admin_form($form, &$form_state) {
  $form = array(
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'acquia_lift') . '/css/acquia_lift.admin.css',
      ),
    ),
  );
  $account_info = variable_get('acquia_lift_account_info', array());
  $account_info_provided = !empty($account_info['owner_code']) && !empty($account_info['api_key']);
  if ($account_info_provided) {
    // Add a button for checking the connection.
    $form['ping_test_wrapper'] = array(
      '#theme_wrappers' => array('container'),
      '#attributes' => array('id' => 'acquia-lift-config-messages'),
    );
    $form['ping_test'] = array(
      '#type' => 'submit',
      '#value' => t('Test connection to Acquia Lift'),
      '#attributes' => array('title' => t('Click here to check your Acquia Lift connection.')),
      '#submit' => array('acquia_lift_ping_test_submit'),
      '#ajax' => array(
        'callback' => 'acquia_lift_ping_test_ajax_callback',
        'wrapper' => 'acquia-lift-ping-test',
        'effect' => 'fade',
      ),
      '#limit_validation_errors' => array(),
    );
    // Add info about number of API calls made last month and current month
    // to date.
    try {
      $api = AcquiaLiftAPI::getInstance(variable_get('acquia_lift_account_info', array()));
      $ts = time();
      $calls_last_month = $api->getTotalRuntimeCallsForPreviousMonth($ts);
      $form['calls_last_month'] = array(
        '#type' => 'markup',
        '#markup' => '<div>' . t('Number of API calls made last month: ') . $calls_last_month . '</div>',
      );
      $calls_this_month = $api->getTotalRuntimeCallsForMonthToDate($ts);
      $form['calls_this_month'] = array(
        '#type' => 'markup',
        '#markup' => '<div>' . t('Number of API calls made so far this month: ') . $calls_this_month . '</div>',
      );
    }
    catch (Exception $e) {
      drupal_set_message($e->getMessage());
    }
  }

  $form['acquia_lift_account_info'] = array(
    '#type' => 'fieldset',
    '#title' => 'Acquia Lift Account Settings',
    '#tree' => TRUE,
    '#collapsible' => TRUE,
    '#collapsed' => $account_info_provided
  );

  $form['acquia_lift_account_info']['msg'] = array(
    '#markup' => t("<p>This information is used to link your !acquialift account to Drupal.</p><p>If you are already an Acquia Lift customer, contact Acquia Support to obtain your credentials. Otherwise, contact !advocacyemail to purchase a subscription to the Acquia Lift service.</p>", array('!acquialift' => l(t('Acquia Lift'), 'http://www.acquia.com/products-services/website-personalization', array('attributes' => array('target' => '_blank'))), '!advocacyemail' => l('advocacy@acquia.com', 'mailto:advocacy@acquia.com'))),
  );

  $form['acquia_lift_account_info']['owner_code'] = array(
    '#type' => 'textfield',
    '#title' => t('Owner Code'),
    '#default_value' => !empty($account_info['owner_code']) ? $account_info['owner_code'] : '',
    '#size' => 35,
    '#maxlength' => 50,
    '#description' => t("Paste in your Acquia Lift owner code"),
    '#required' => TRUE,
  );

  $form['acquia_lift_account_info']['api_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Runtime API Key'),
    '#default_value' => !empty($account_info['api_key']) ? $account_info['api_key'] : '',
    '#size' => 35,
    '#maxlength' => 50,
    '#description' => t("Paste in your Acquia Lift api key"),
    '#required' => TRUE,
  );

  $form['acquia_lift_account_info']['admin_key'] = array(
    '#type' => 'textfield',
    '#title' => t('Admin API Key'),
    '#default_value' => !empty($account_info['admin_key']) ? $account_info['admin_key'] : '',
    '#size' => 35,
    '#maxlength' => 50,
    '#description' => t("Paste in your Acquia Lift admin key"),
    '#required' => TRUE,
  );

  $form['acquia_lift_account_info']['api_url'] = array(
    '#type' => 'textfield',
    '#title' => t('API Server URL'),
    '#default_value' => !empty($account_info['api_url']) ? $account_info['api_url'] : '',
    '#field_prefix' => 'http(s)://',
    '#size' => 35,
    '#maxlength' => 50,
    '#description' => t("Paste in your Acquia Lift API URL"),
    '#required' => TRUE,
  );
  $form['acquia_lift_batch_decisions'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('acquia_lift_batch_decisions', FALSE),
    '#title' => t('Make all decisions on a page in one call'),
    '#description' => t('If this is enabled, Lift will look at every decision to be made on the page and make it in a single request. If disabled, there will be a separate HTTP request per decision.')
  );
  $form['acquia_lift_confidence_measure'] = array(
    '#type' => 'textfield',
    '#title' => t('Confidence measure'),
    '#size' => 3,
    '#field_suffix' => '%',
    '#required' => TRUE,
    '#default_value' => variable_get('acquia_lift_confidence_measure', 95),
    '#description' => t('The confidence percentage at which a test is considered statistically significant.'),
    '#element_validate' => array('element_validate_number'),
  );

  $form['minimum_runtime'] = array(
    '#type' => 'container',
    '#tree' => FALSE,
    '#attributes' => array(
      'class' => array('acquia-lift-config-minimum-runtime'),
    ),

  );
  $form['minimum_runtime']['acquia_lift_min_runtime_title'] = array(
    '#type' => 'markup',
    '#markup' => '<label for="acquia_lift_min_runtime_num">' . t('Duration') . '<span class="form-required" title="This field is required.">*</span></label>',
  );
  $form['minimum_runtime']['acquia_lift_min_runtime_num'] = array(
    '#type' => 'textfield',
    '#size' => 3,
    '#required' => TRUE,
    '#default_value' => variable_get('acquia_lift_min_runtime_num', 2),
    '#element_validate' => array('element_validate_number'),
  );
  $form['minimum_runtime']['acquia_lift_min_runtime_unit'] = array(
    '#type' => 'select',
    '#required' => TRUE,
    '#default_value' => variable_get('acquia_lift_min_runtime_unit', 'week'),
    '#options' => array(
      'minute' => t('minutes'),
      'hour' => t('hours'),
      'day' => t('days'),
      'week' => t('weeks')
    ),
  );
  $form['minimum_runtime']['acquia_lift_min_runtime_desc'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="description">' . t('The amount of time a campaign will run before it can be completed. Applies only to campaigns with the "End when campaign thresholds are reached" option selected.') . '</div>',
  );
  $form['acquia_lift_min_decisions'] = array(
    '#type' => 'textfield',
    '#title' => t('Decisions'),
    '#size' => 3,
    '#required' => TRUE,
    '#default_value' => variable_get('acquia_lift_min_decisions', 1000),
    '#description' => t('The number of variations selected by Acquia Lift for display to visitors before the campaign can be completed. Applies only to campaigns with the "End when campaign thresholds are reached" option selected.'),
    '#element_validate' => array('element_validate_number'),
  );
  $form['acquia_lift_unibar_allow_status_change'] = array(
    '#type' => 'checkbox',
    '#default_value' => variable_get('acquia_lift_unibar_allow_status_change', TRUE),
    '#title' => t('Campaigns can be started, paused, or resumed from the Acquia Lift menu bar.'),
    '#description' => t('Disabling this option hides the status change link on the menu bar, which can increase the performance of the menu bar if the website has many defined campaigns.')
  );
  $form = system_settings_form($form);
  $form['#submit'][] = 'acquia_lift_admin_form_submit';
  return $form;
}

/**
 * Simple form for initiating batch syncing of agents to Lift.
 */
function acquia_lift_batch_sync_form($form, &$form_state) {
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Synchronize with Acquia Lift service')
  );
  $form['explanation'] = array(
    '#type' => 'markup',
    '#markup' => '<div>' . t('Sends your local campaign information to the hosted Acquia Lift service. Use this feature if you change your Acquia Lift credentials after creating one or more campaigns.') . '</div>'
  );
  return $form;
}

/**
 * Submit callback for the batch sync form.
 */
function acquia_lift_batch_sync_form_submit($form, &$form_state) {
  module_load_include('inc', 'acquia_lift', 'acquia_lift.batch');
  acquia_lift_batch_sync_campaigns();
}

/**
 * Submit callback for the ping test button.
 */
function acquia_lift_ping_test_submit($form, &$form_state) {
  $api = AcquiaLiftAPI::getInstance(variable_get('acquia_lift_account_info', array()));
  if ($api->pingTest()) {
    drupal_set_message(t('Successfully connected to the Acquia Lift service'));
  }
  else {
    drupal_set_message(t('There was a problem connecting to the Acquia Lift service. Please check your credentials'), 'error');
  }
}

/**
 * Ajax callback for the ping test button.
 */
function acquia_lift_ping_test_ajax_callback($form, &$form_state) {
  $commands = array();
  // Show status messages.
  $commands[] = ajax_command_replace('#acquia-lift-config-messages', '<div id="acquia-lift-config-messages">' . theme('status_messages') . '</div>');
  return array(
    '#type' => 'ajax',
    '#commands' => $commands,
  );
}

/**
 * Validation callback for the Acquia Lift admin form.
 */
function acquia_lift_admin_form_validate($form, &$form_state) {
  if (!AcquiaLiftAPI::codeIsValid($form_state['values']['acquia_lift_account_info']['owner_code'])) {
    form_set_error('acquia_lift_account_info][owner_code', 'You must enter a valid owner code');
  }
  if (!valid_url($form_state['values']['acquia_lift_account_info']['api_url'])) {
    form_set_error('acquia_lift_account_info][api_url', t('You must enter a valid URL'));
  }
  // Strip any scheme from the API URL.
  $form_state['values']['acquia_lift_account_info']['api_url'] = preg_replace('/(^[a-z]+:\/\/)/i', '', $form_state['values']['acquia_lift_account_info']['api_url']);
  if ($form_state['values']['acquia_lift_confidence_measure'] <= 0 || $form_state['values']['acquia_lift_confidence_measure'] >= 100) {
    form_set_error('acquia_lift_confidence_measure', t('Confidence measure must be a value between 0 and 100.'));
  }
}

/**
 * Submit handler for the Acquia Lift admin form.
 *
 * Creates a default Acquia Lift agent if one does not yet exist.
 */
function acquia_lift_admin_form_submit($form, &$form_state) {
  acquia_lift_ensure_default_agent($form_state['values']['acquia_lift_account_info']);

  if ($form_state['values']['acquia_lift_confidence_measure'] < 95) {
    drupal_set_message(t('A minimum confidence measure of 95% is recommended to ensure proper evaluation of test results.'), 'warning');
  }

  // Clear the ctools plugin "agent_type" cache for personalize, clear loaded
  // files cache, and rebuild the autoloader class definitions.
  cache_clear_all('plugins:personalize:agent_type', 'cache', TRUE);
  cache_clear_all('ctools_plugin_files:personalize:agent_type', 'cache', TRUE);
  registry_rebuild();
}

/**
 * =======================================================================
 *  A C Q U I A  L I F T  A G E N T  R E P O R T I N G
 * =======================================================================
 */

/**
 * Form build function for the Acquia Lift report, which has filters.
 *
 * @param stdClass $agent_data
 *   The campaign agent data for this report.
 * @param stdClass $option_set
 *   (optional) The content variation to show in the default view.
 */
function acquia_lift_report($form, &$form_state, $agent_data, $option_set) {
  if (!$agent = personalize_agent_load_agent($agent_data->machine_name)) {
    return array();
  }
  if (!$agent instanceof AcquiaLiftAgent) {
    return array();
  }
  if ($agent_data->started == 0) {
    return array(
      'no_report' => array(
        '#markup'=> t('This agent has not started running yet, no reports to show.')
      )
    );
  }

  // If this agent is not currently enabled in Acquia Lift, there are no reports
  // to show.
  $errors = $agent->errors();
  if (!empty($errors)) {
    return array(
      'no_report' => array(
        '#markup'=> t('This agent is not properly configured, no reports to show.')
      )
    );
  }
  // If this agent doesn't implement the reporting interface then there are no
  // reports to show.
  if (!$agent instanceof PersonalizeAgentReportInterface) {
    return array(
      'no_report' => array(
        '#markup' => t('This agent does not support reporting.')
      )
    );
  }

  if ($agent instanceof AcquiaLiftSimpleAB) {
    return acquia_lift_report_ab($form, $form_state, $agent, $agent_data);
  }
  else {
    return acquia_lift_report_custom($form, $form_state, $agent, $agent_data, $option_set);
  }
}

/**
 * Submit handler for Acquia Lift reports.
 */
function acquia_lift_report_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Form build function for a custom Acquia Lift agent report.
 *
 * @param AcquiaLiftAgentInterface $agent
 *   The plugin agent for the selected campaign.
 * @param stdClass $agent_data
 *   The campaign agent data for this report.
 * @param stdClass $option_set
 *   (optional) The content variation to show in the default view.
 */
function acquia_lift_report_custom($form, &$form_state, $agent, $agent_data, $option_set) {
  // Check for Rickshaw and D3 libraries and alert users if not exist.
  if (_acquia_lift_missing_library_warning(array('rickshaw', 'd3'), t('The following libraries are required in order to view the Acquia Lift reports:'))) {
    return array();
  }

  // Generate report filters.
  $data = $agent->getData();
  $form = array(
    '#prefix' => '<div id="acquia-lift-reports">',
    '#suffix' => '</div>',
    '#attached' => array(
      'css' => array(
        drupal_get_path('module', 'acquia_lift') . '/css/acquia_lift.admin.css',
      ),
      'js' => array(
        drupal_get_path('module', 'acquia_lift') . '/js/acquia_lift.admin.js',
      ),
      'library' => array(
        array('acquia_lift', 'acquia_lift.reports'),
      ),
    ),
  );

  $form['report_filters'] = array(
    '#type' => 'container',
    '#tree' => FALSE,
    '#attributes' => array(
      'class' => array('acquia-lift-report-filters', 'clearfix'),
    ),
  );

  // Get the decision points for this agent so we can provide a filter on this.
  $decisions = AcquiaLiftAgent::convertOptionSetsToDecisions($data['decisions']);
  $decision_options = array();
  foreach ($decisions as $name => $decision) {
    $decision_options[$name] = personalize_get_label_for_decision_point_name($name);
  }

  // Decision point filters.
  if (isset($form_state['values']['decision_point'])) {
    $decision_point = $form_state['values']['decision_point'];
  }
  else {
    $decision_point = !empty($option_set) ? personalize_get_decision_name_for_option_set($option_set) : key($decisions);
  }
  $form['report_filters']['decision_point'] = acquia_lift_report_decision_point_dropdown($decision_options, $decision_point);

  // Default to showing the complete history of the campaign.
  $date_start_report = date('Y-m-d', $agent_data->started);
  $date_end_report = $date_end_targeting = date('Y-m-d');

  // If the campaign's status is "completed" then we need to use the date it ended.
  $agent_name = $agent->getMachineName();
  $status = variable_get(_personalize_agent_get_status_variable($agent_name));
  if ($status == PERSONALIZE_STATUS_COMPLETED && $end_time = variable_get(_personalize_agent_get_stoptime_variable($agent_name), 0)) {
    $date_end_report = $date_end_targeting = date('Y-m-d', $end_time);
  }
  // Conversion report filters.
  $selected_goal = empty($form_state['values']['goal']) ? NULL : $form_state['values']['goal'];
  $selected_metric = empty($form_state['values']['metric']) ? 'rate' : $form_state['values']['metric'];

  $options = array(
    'decision' => $decision_point,
    'start' => $date_start_report,
    'end' => $date_end_report,
    'goal' => $selected_goal,
    'conversion_metric' => $selected_metric,
  );

  $reports = $agent->buildCampaignReports($options);
  if (!($reports['#has_data'])) {
    // This campaign hasn't been shown yet, so there is no data for reporting.
    return array(
      'no_report' => array(
        '#markup' => t('This campaign does not yet contain information about your visitors\' website interactions. This situation generally occurs for campaigns that have either just been started or do not generate much website traffic.'),
      ),
    );
  }

  // Generate mark-up for adaptive style report labels.
  $report_title_additional = '';
  if ($data['decision_style'] === 'adaptive') {
    $report_title_additional = theme('acquia_lift_percentage_label', array(
      'percent_label' => t('Random'),
      'rest_label' => t('Personalized'),
      'percent' => $data['explore_rate']
    ));
  }

  // Overview report section.
  $form['overview_report'] = array(
    'overview_report_title' => array(
      '#markup' => '<h2>' . t('Overview') . '</h2>',
    ),
    '#theme_wrappers' => array('container'),
    '#attributes' => array(
      'id' => 'acquia-lift-overview-report',
      'class' => array('acquia-lift-report-section', 'clearfix'),
    ),
  );
  $form['overview_report']['report'] = array(
    '#markup' => drupal_render($reports['overview']),
    '#theme_wrappers' => array('container'),
    '#id' => 'acquia-lift-overview-report-data',
  );

  // Conversion details section.
  $form['experiment_report'] = array(
    '#type' => 'container',
    'header' => array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('acquia-lift-report-section-header', 'clearfix'),
      ),
      'title' => array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('acquia-lift-report-section-title'),
        ),
        'report_title' => array(
          '#markup' => '<h2>' . t('Experiment') . '</h2>',
        ),
      ),
    ),
    'summary' => array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('acquia-lift-report-header-summary'),
      )
    ),
    '#attributes' => array(
      'id' => 'acquia-lift-experiment-report',
      'class' => array('acquia-lift-report-section'),
    ),
  );
  if (!empty($report_title_additional)) {
    $form['experiment_report']['header']['title']['groups'] = array(
      '#markup' => t('random group'),
    );
    $form['experiment_report']['header']['summary']['distribution'] = array(
      '#markup' => $report_title_additional,
      '#theme_wrappers' => array('container'),
    );
  }
  // Get the conversion report options.
  $form['experiment_report']['header']['options'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('acquia-lift-report-section-options'),
    ),
    '#tree' => FALSE,
    'goal' => acquia_lift_report_goal_dropdown($agent->getMachineName(), $selected_goal),
    'metric' => acquia_lift_report_conversion_metric_dropdown($selected_metric),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Filter'),
    ),
  );
  $form['experiment_report']['header']['summary']['report_summary'] = array(
    '#theme_wrappers' => array('container'),
    '#markup' => t('See which content variations are winning'),
    '#attributes' => array(
      'class' => array('acquia-lift-report-summary'),
    ),
  );
  $form['experiment_report']['report'] = array(
    '#markup' => drupal_render($reports['experiment']),
    '#theme_wrappers' => array('container'),
    '#id' => 'acquia-lift-experiment-report-data',
  );

  // The context and stability reports are only relevant if there is context
  // targeting in place for this campaign.
  if (empty($reports['targeting'])) {
    return $form;
  }

  // Context report section.
  $context_select = $reports['targeting'];
  acquia_lift_chosenify_element($context_select, array('acquia-lift-chosen-select-half', 'acquia-lift-report-context-select'));
  $form['context_report'] = array(
    '#type' => 'container',
    'header' => array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('acquia-lift-report-section-header', 'clearfix'),
      ),
      'title' => array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('acquia-lift-report-section-title'),
        ),
        'report_title' => array(
          '#markup' => '<h2>' . t('Context') . '</h2>',
        ),
      ),
      'summary' => array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('acquia-lift-report-header-summary'),
        ),
      ),
    ),
    '#attributes' => array(
      'id' => 'acquia-lift-context-report',
      'class' => array('acquia-lift-report-section'),
    )
  );
  if (!empty($report_title_additional)) {
    $form['context_report']['header']['title']['groups'] = array(
      '#markup' => t('random and personalized groups'),
    );
    $form['context_report']['header']['summary']['distribution'] = array(
      '#markup' => $report_title_additional,
      '#theme_wrappers' => array('container'),
    );
  }
  $form['context_report']['header']['summary']['report_summary'] = array(
    '#theme_wrappers' => array('container'),
    '#markup' => t('See who converts best for each content variation'),
    '#attributes' => array(
      'class' => array('acquia-lift-report-summary'),
    ),
  );
  $form['context_report']['header']['controls'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('acquia-lift-report-controls'),
    ),
    'context' => $context_select,
  );
  $form['context_report']['report'] = array(
    '#markup' => drupal_render($reports['context']),
    '#theme_wrappers' => array('container'),
    '#id' => 'acquia-lift-context-report-data',
  );

  // Stability report section.
  $context_select = $reports['targeting'];
  acquia_lift_chosenify_element($context_select, array('acquia-lift-chosen-select-half', 'acquia-lift-report-context-select'));
  $form['advanced_reports'] = array(
    '#type' => 'fieldset',
    '#title' => t('Advanced reporting'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );
  $form['advanced_reports']['stability_report'] = array(
    '#type' => 'container',
    'header' => array(
      '#type' => 'container',
      '#attributes' => array('acquia-lift-report-section-header', 'clearfix'),
      'title' => array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('acquia-lift-report-section-title'),
        ),
        'report_title' => array(
          '#markup' => '<h2>' . t('Context Stability') . '</h2>',
        ),
      ),
      'summary' => array(
        '#type' => 'container',
        '#attributes' => array(
          'class' => array('acquia-lift-report-header-summary'),
        ),
      ),
    ),
    '#attributes' => array(
      'id' => 'acquia-lift-stability-report',
      'class' => array('acquia-lift-report-section'),
    ),
  );
  if (!empty($report_title_additional)) {
    $form['advanced_reports']['stability_report']['header']['title']['groups'] = array(
      '#markup' => t('random and personalized groups'),
    );
    $form['advanced_reports']['stability_report']['header']['summary']['distribution'] = array(
      '#markup' => $report_title_additional,
      '#theme_wrappers' => array('container'),
    );
  }
  $form['advanced_reports']['stability_report']['header']['controls'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('acquia-lift-report-controls'),
    ),
    'context' => $context_select,
  );
  $form['advanced_reports']['stability_report']['report'] = array(
    '#markup' => drupal_render($reports['stability']),
    '#theme_wrappers' => array('container'),
    '#id' => 'acquia-lift-stability-report-data',
  );

  // We have to specify the include file so as not to lose it during rendering from ajax.
  // @see drupal_retrieve_form():734
  $form_state['build_info']['files'] = array(
    drupal_get_path('module', 'acquia_lift') . '/acquia_lift.admin.inc',
    drupal_get_path('module', 'acquia_lift') . '/acquia_lift.ui.inc',
  );
  return $form;
}

/**
 * Returns a dropdown for filtering by decision point.
 *
 * @param array $options
 *   The different decision point options.
 * @param $selected
 *   The decision point to set as the default value.
 * @return array
 *   An array representing a dropdown select list.
 */
function acquia_lift_report_decision_point_dropdown($options, $selected) {
  if (empty($options)) {
    return array();
  }
  if (count($options) == 1) {
    $option = key($options);
    return array(
      '#type' => 'hidden',
      '#value' => $option,
    );
  }
  return array(
    '#title' => t('Variation Set'),
    '#type' => 'select',
    '#options' => $options,
    '#default_value' => $selected,
    '#ajax' => array(
      'callback' => "acquia_lift_report_ajax_callback",
      'wrapper' => "acquia-lift-reports",
    ),
    '#id' => 'acquia-lift-report-decision-point-filter',
  );
}

/**
 * Returns the drop-down for filtering reports by goal.
 *
 * @param $agent_name
 *   The machine name of the campaign.
 * @param $selected
 *   The selected goal action.
 * @return array
 *   A form element array to be used as the dropdown.
 */
function acquia_lift_report_goal_dropdown($agent_name, $selected = NULL) {
  $goals = personalize_goal_load_by_conditions(array('agent' => $agent_name));
  // There should always be at least one goal in an Acquia Lift report.
  if (empty($goals)) {
    return array();
  }
  if (count($goals) == 1) {
    $goal = current($goals);
    return array(
      '#type' => 'hidden',
      '#value' => $goal->action,
    );
  }
  else {
    $actions = visitor_actions_get_actions();
    foreach ($goals as $goal) {
      $options[$goal->action] = $actions[$goal->action]['label'];
    }
    return array(
      '#title' => t('Goals'),
      '#type' => 'select',
      '#options' => $options,
      '#default_value' => $selected,
      '#empty_option' => t('All goals'),
    );
  }
}

/**
 * Returns the drop-down for filtering reports by metric.
 *
 * @param $selected
 *   The currently display metric.
 * @return array
 *   A form element array to be used as the dropdown.
 */
function acquia_lift_report_conversion_metric_dropdown($selected) {
  return array(
    '#type' => 'select',
    '#title' => t('Metrics'),
    '#options' => array(
      'rate' => t('Conversion rate %'),
      'value' => t('Conversion value')
    ),
    '#default_value' => $selected,
    '#required' => TRUE,
  );
}

/**
 * Ajax callback for filtering options.
 */
function acquia_lift_report_ajax_callback($form, &$form_state) {
  return $form;
}

/**
 * Form build function for an A/B Acquia Lift agent report.
 *
 * @param AcquiaLiftAgentInterface $agent
 *   The plugin agent for the selected campaign.
 * @param stdClass $agent_data
 *   The campaign agent data for this report.
 */
function acquia_lift_report_ab($form, &$form_state, $agent, $agent_data) {
  $agent_name = $agent->getMachineName();
  // Check for Rickshaw and D3 libraries and alert users if not exist.
  if (_acquia_lift_missing_library_warning(array('rickshaw', 'd3'), t('The following libraries are required in order to view the Acquia Lift reports:'))) {
    return array();
  }
  $selected_goal = empty($form_state['values']['goal']) ? NULL : $form_state['values']['goal'];
  $selected_metric = empty($form_state['values']['metric']) ? 'rate' : $form_state['values']['metric'];

  $build = array();
  $build['options'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('acquia-lift-report-section-options'),
    ),
    '#tree' => FALSE,
    'goal' => acquia_lift_report_goal_dropdown($agent_name, $selected_goal),
    'metric' => acquia_lift_report_conversion_metric_dropdown($selected_metric),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Filter'),
    ),
  );

  // Default to showing the complete history of the campaign.
  $date_start_report = date('Y-m-d', $agent_data->started);
  $date_end_report = $date_end_targeting = date('Y-m-d');

  // If the campaign's status is "completed" then we need use the date it ended.
  $status = variable_get(_personalize_agent_get_status_variable($agent_name));
  if ($status == PERSONALIZE_STATUS_COMPLETED && $end_time = variable_get(_personalize_agent_get_stoptime_variable($agent_name), 0)) {
    $date_end_report = $date_end_targeting = date('Y-m-d', $end_time);
  }
  // Acquia Lift A/B reports have a decision name that is the same as the
  // campaign machine name.
  $options = array(
    'decision' => $agent_data->machine_name,
    'start' => $date_start_report,
    'end' => $date_end_report,
  );
  $build['reports'] = $agent->buildCampaignReports($options);

  if (!$build['reports']['#has_data']) {
    // This campaign hasn't been shown yet, so there is no data for reporting.
    return array(
      'no_report' => array(
        '#markup' => t('This campaign does not yet contain information about your visitors\' website interactions. This situation generally occurs for campaigns that have either just been started or do not generate much website traffic.'),
      ),
    );
  }

  $build['reports']['#attached']['library'][] = array('acquia_lift', 'acquia_lift.reports');

  return $build;
}

/**
 * AJAX callback to return the tabular HTML for conversion reports.
 *
 * The following parameters are supplied within the query string:
 *   - campaign: the machine name of the campaign
 *   - decision: the decision name to get data
 *   - start: (optional) the timestamp to use as start date
 *   - end: (optional) the timestamp to use as the end date
 *   - goal: (optional) a goal name to limit results (defaults to all goals)
 */
function acquia_lift_report_conversion() {
  $params = drupal_get_query_parameters();
  $agent_name = empty($params['campaign']) ? NULL : filter_xss($params['campaign']);
  $options = array();
  $options['decision'] = empty($params['decision']) ? NULL : filter_xss($params['decision']);
  $options['goal'] = empty($params['goal']) ? NULL : filter_xss($params['goal']);
  $options['start'] = empty($params['start']) ? NULL : check_plain($params['start']);
  $options['end'] = empty($params['end']) ? NULL : check_plain($params['end']);
  $agent = personalize_agent_load($agent_name);

  if (!acquia_lift_is_testing_agent($agent)) {
    return array();
  }
  if (is_numeric($options['start'])) {
    $options['start'] = date('Y-m-d', $options['start']);
  }
  if (is_numeric($options['end'])) {
    $options['end'] = date('Y-m-d', $options['end']);
  }

  $reports = array();
  if ($plugin = personalize_agent_load_agent($agent_name)) {
    $reports = $plugin->buildConversionReport($options);
  }
  $reports['#type'] = 'ajax';
  return drupal_json_output(drupal_render($reports));
}


/**
 * =======================================================================
 *  A C Q U I A  L I F T  T A R G E T I N G  W O R K F L O W
 * =======================================================================
 */

/**
 * Form for adding a targeting audience to an agent.
 */
function acquia_lift_new_target_audience_form($form, &$form_state, $agent) {
  // Build up a list of available context values for targeting.
  module_load_include('inc', 'personalize', 'personalize.admin');
  $targeting_values = personalize_get_targeting_values_for_agent($agent);
  $attributes = array(
    'class' => array('personalize-variation-row'),
  );
  $form = array(
    '#tree' => TRUE,
    '#type' => 'container',
    '#attributes' => $attributes,
  );
  $form['#attached']['css'][] = drupal_get_path('module', 'personalize') . '/css/personalize.admin.css';
  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent->machine_name
  );
  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Audience name'),
    '#required' => TRUE
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
  $main_wrapper_id = 'acquia-lift-targeting';

  $form['mapping'] = array(
    '#tree' => TRUE,
    '#theme_wrappers' => array('container'),
    '#attributes' => $attributes,
  );
  $form['mapping']['contexts'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => $main_wrapper_id,
    ),
  );

  // Load from the existing form if passed first, otherwise, load from option
  // set.
  $mappings = array();
  if (isset($form_state['values']['mapping']['contexts'])) {
    foreach($form_state['values']['mapping']['contexts'] as $delta => $context) {
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

  // If the "Remove" button was clicked for a context, we need to remove that context
  // from the form.
  if (isset($form_state['to_remove'])) {
    unset($mappings[$form_state['to_remove']]);
    unset($form_state['to_remove']);
    $form_state['num_contexts']--;
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
  if (isset($form_state['num_contexts']) && $form_state['num_contexts'] > $num_contexts) {
    while ($num_contexts < $form_state['num_contexts']) {
      $mappings[] = array(
        'context' => '',
        'operator' => 'equals',
        'match' => '',
        'plugin' => '',
      );
      $num_contexts++;
    }
  }

  $form_state['num_contexts'] = count($mappings);
  foreach ($mappings as $delta => $mapping) {
    $wrapper_id = 'operator-dropdown-replace-' . $delta;
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
          'class' => array('acquia-lift-targeting-context'),
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
      '#prefix' => '<div class="acquia-lift-remove-context">',
      '#suffix' => '</div>',
      '#type' => 'submit',
      '#tag' => 'button',
      '#text' => t('Remove'),
      '#value' => 'remove_' . $delta,
      '#theme_wrappers' => array('personalize_html_tag'),
      '#attributes' => array(
        'class' => array('personalize-delete-context', 'form-submit',),
        'title' => t('Delete this context.'),
        'id' => 'edit-targeting-mapping-contexts-' . $delta . '-remove',
      ),
      '#submit' => array('acquia_lift_targeting_remove_context_submit'),
      '#ajax' => array(
        'callback' => 'acquia_lift_targeting_context_ajax_callback',
        'wrapper' => $main_wrapper_id,
        'effect' => 'fade',
      ),
    );
  }
  // Create an "add new context" link.
  $form['add_new'] = array(
    '#prefix' => '<span class="personalize-add-link-prefix"></span>',
    '#type' => 'submit',
    '#value' => t('Add context'),
    '#submit' => array('acquia_lift_targeting_add_context_submit'),
    '#ajax' => array(
      'callback' => 'acquia_lift_targeting_context_ajax_callback',
      'wrapper' => $main_wrapper_id,
      'effect' => 'fade',
    ),
  );
  $form['strategies'] = array(
    '#type' => 'container',
    '#attributes' => $attributes,
  );

  // Add radio buttons so the user can select how multiple features for an option
  // should be treated.
  $default_strategy = 'OR';
  $form['strategy'] = array(
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
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Create new audience')
  );
  return $form;
}

/**
 * Validation callback for the new target audience form.
 */
function acquia_lift_new_target_audience_form_validate($form, &$form_state) {
  $machine_name = personalize_generate_machine_name($form_state['values']['name'], NULL, '-');
  $agent_name = $form_state['values']['agent'];
  $option_set = acquia_lift_get_option_set_for_targeting($agent_name);
  if (isset($option_set->targeting[$machine_name])) {
    form_set_error('name', t('Please choose a different name for your audience as this one is already taken'));
  }
}

/**
 * Submit callback for the new target audience form.
 */
function acquia_lift_new_target_audience_form_submit($form, &$form_state) {
  $values = $form_state['values'];
  $contexts = array();
  // We need to massage the context information as submitted in the form into
  // an array of contexts that can be consumed by the
  // acquia_lift_target_audience_save() function.
  foreach ($values['mapping']['contexts'] as $context_values) {
    if ($context_values['context'] == '') {
      continue;
    }
    $context_values['match'] = $context_values['value']['match'];
    $context_values['operator'] = $context_values['value']['operator'];
    unset($context_values['value'], $context_values['remove']);
    $contexts[] = $context_values;
  }
  if (acquia_lift_target_audience_save($values['name'], $values['agent'], $contexts, $values['strategy'])) {
    drupal_set_message(t('The new target audience was saved successfully'));
  }
  else {
    drupal_set_message(t('There was a problem saving the new target audience.'), 'error');
  }
}

/**
 * Saves a new target audience for an agent.
 *
 * @param $label
 *   The human-readable name of the audience to create.
 * @param $agent_name
 *   THe name of the agent to add the audience to.
 * @param $contexts
 *   The contexts that make up the audience definition.
 * @param $strategy
 *   The strategy to use for multiple contexts, i.e. 'AND' or 'OR'
 */
function acquia_lift_target_audience_save($label, $agent_name, $contexts, $strategy) {
  module_load_include('inc', 'personalize', 'personalize.admin');
  $machine_name = personalize_generate_machine_name($label, NULL, '-');
  // Find the option set to use for targeting and add the audience.
  $option_set = acquia_lift_get_option_set_for_targeting($agent_name);
  if (empty($option_set)) {
    return FALSE;
  }
  $option_set->targeting[$machine_name] = array(
    'label' => $label,
    'weight' => count($option_set->targeting)
  );
  // Generate the feature strings and rules for the contexts.
  $agent = personalize_agent_load_agent($agent_name);
  $feature_strings = $feature_rules = array();
  foreach ($contexts as $context_values) {
    list($plugin_name, $context_name) = explode(PERSONALIZE_TARGETING_ADMIN_SEPARATOR, $context_values['context']);
    // Generate a value code based on the operator used.
    $value = personalize_targeting_generate_value_code($context_values['match'], $context_values['operator']);
    // Create a feature string for this context value that can be consumed
    // by the agent that will be using it.
    $feature_string = $agent->convertContextToFeatureString($context_name, $value);
    $feature_strings[] = $feature_string;
    // Save the actual rule information as this is what will be used
    // for evaluating it.
    $feature_rules[$feature_string] = $context_values;
    // Override the context to split it into plugin and context parts.
    $feature_rules[$feature_string]['context'] = $context_name;
    $feature_rules[$feature_string]['plugin'] = $plugin_name;
  }
  $option_set->targeting[$machine_name]['targeting_features'] = $feature_strings;
  $option_set->targeting[$machine_name]['targeting_rules'] = $feature_rules;
  $option_set->targeting[$machine_name]['targeting_strategy'] = $strategy;
  try {
    personalize_option_set_save($option_set);
    return TRUE;
  }
  catch (PersonalizeException $e) {
    return FALSE;
  }
}

/**
 * Submit handler for the "Add Context" button.
 */
function acquia_lift_targeting_add_context_submit($form, &$form_state) {
  // Increment the number of contexts to be rendered.
  $form_state['num_contexts']++;
  $form_state['rebuild'] = TRUE;
}

/**
 * Submit handler for the "Remove Context" button.
 */
function acquia_lift_targeting_remove_context_submit($form, &$form_state) {
  $parents = $form_state['clicked_button']['#parents'];
  $delta_pos = array_search('contexts', $parents) + 1;
  $delta = $parents[$delta_pos];
  $form_state['to_remove'] = $delta;
  $form_state['rebuild'] = TRUE;
}

/**
 * Ajax callback for the add context and remove context buttons.
 */
function acquia_lift_targeting_context_ajax_callback($form, $form_state) {
  return $form['mapping']['contexts'];
}

/**
 * Form for assigning options or tests to audiences.
 *
 * @param $agent
 *   A stdClass object representing the agent.
 */
function acquia_lift_targeting_form($form, &$form_state, $agent) {
  if ($agent->plugin != 'acquia_lift_target') {
    drupal_goto('admin/structure/personalize');
  }
  // Check if we should display the confirm form, if they are reverting changes.
  if (isset($form_state['confirm_revert_changes'])) {
    return acquia_lift_confirm_revert_changes($form, $form_state, $agent->machine_name);
  }
  $option_set = acquia_lift_get_option_set_for_targeting($agent->machine_name);
  $variation_options = $saved_targeting = array();
  $exising_targeting = acquia_lift_get_structure_from_targeting($option_set);
  // The settings reflected in the form will either be what is currently running
  // or any saved overrides from when this form was last submitted (if the
  // review step was never completed to apply them.)
  if (!empty($agent->data['lift_targeting'])) {
    $saved_targeting = $agent->data['lift_targeting'];
    if ($saved_targeting != $exising_targeting) {
      $message = t('The targeting settings shown here do not match what is currently running for this campaign.');
      $message_type = 'warning';
    }
  }
  else {
    if (personalize_agent_get_status($agent->machine_name) != PERSONALIZE_STATUS_NOT_STARTED) {
      $message = t('The targeting settings shown here represent what is currently running for this campaign.');
      $message_type = 'ok';
    }
    $saved_targeting = $exising_targeting;
  }

  if (isset($message)) {
    $form['message'] = array(
      '#type' => 'markup',
      '#markup' => '<div class="acquia-lift-targeting-message acquia-lift-targeting-message-' . $message_type . '">' . $message . '</div>',
      '#weight' => -1
    );
  }
  foreach ($option_set->options as $option) {
    $variation_options[$option['option_id']] = $option['option_label'];
  }
  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent->machine_name
  );
  $form['targeting'] = array(
    '#type' => 'fieldset',
    '#tree' => TRUE
  );
  foreach (array_keys($saved_targeting) as $audience) {
    $form['targeting'][$audience] = array(
      '#title' => $audience,
      '#type' => 'select',
      '#multiple' => TRUE,
      '#options' => array('' => t('Select...')) + $variation_options,
      '#default_value' => isset($saved_targeting[$audience]) ? $saved_targeting[$audience] : ''
    );
  }
  $form['actions'] = array('#type' => 'actions', '#tree' => FALSE);
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save')
  );
  $form['actions']['revert_changes'] = array(
    '#type' => 'submit',
    '#value' => t('Revert changes')
  );
  return $form;
}

/**
 * Submit callback for the targeting form.
 */
function acquia_lift_targeting_form_submit($form, &$form_state) {
  $agent = personalize_agent_load($form_state['values']['agent']);
  if ($form_state['triggering_element']['#value'] == t('Revert changes')) {
    // Execute the revert action.
    if ($form_state['values']['revert_changes'] === TRUE) {
      return acquia_lift_confirm_revert_changes_submit($form, $form_state);
    }
    // Rebuild the form to confirm the revert action.
    $form_state['rebuild'] = TRUE;
    $form_state['confirm_revert_changes'] = TRUE;
    return;
  }
  $targeting = array();
  foreach ($form_state['values']['targeting'] as $audience => $options) {
    $targeting[$audience] = array();
    foreach ($options as $option) {
      if (!empty($option)) {
        $targeting[$audience][] = $option;
      }
    }
  }
  acquia_lift_save_targeting_structure($agent, $targeting);
}

/**
 * Confirm form for reverting the changes to targeting.
 *
 * @param $agent_name
 *   The name of hte agent whose targeting changes are being reverted.
 */
function acquia_lift_confirm_revert_changes($form, &$form_state, $agent_name) {
  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent_name
  );
  $form['revert_changes'] = array('#type' => 'value', '#value' => TRUE);
  return confirm_form($form,
    t('Are you sure you want to revert the targeting changes for the %agent campaign?',
      array('%agent' => $agent_name)),
    'admin/structure/personalize/' . $agent_name,
    t('Reverting the targeting will discard all changes since the campaign was set to running.'),
    t('Revert changes'),
    t('Cancel'));
}

/**
 * Submit callback for the "Revert changes" confirm form.
 */
function acquia_lift_confirm_revert_changes_submit($form, &$form_state) {
  if ($agent = personalize_agent_load($form_state['values']['agent'])) {
    $agent->data['lift_targeting'] = array();
    personalize_agent_save($agent);
  }
}

/**
 * Final review form for applying targeting changes.
 *
 * @param $agent
 *   A stdClass object representing the agent.
 */
function acquia_lift_review_form($form, &$form_state, $agent) {
  if ($agent->plugin != 'acquia_lift_target') {
    drupal_goto('admin/structure/personalize');
  }
  $form = array();
  $form['agent'] = array(
    '#type' => 'value',
    '#value' => $agent->machine_name
  );
  $saved_targeting = array();
  if (!empty($agent->data['lift_targeting'])) {
    $saved_targeting = $agent->data['lift_targeting'];
  }

  $option_set = acquia_lift_get_option_set_for_targeting($agent->machine_name);
  $running_targeting = acquia_lift_get_structure_from_targeting($option_set);

  // Only show a submit button if there are actual changes to apply.
  if (!empty($saved_targeting) && $saved_targeting != $running_targeting) {
    drupal_set_message(t('Saving this form will restructure your personalization and could result in loss of data about existing tests.'), 'warning');
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Confirm changes')
    );
  }
  else {
    $form['all_good'] = array(
      '#type' => 'markup',
      '#markup' => t('No changes to apply. !change', array('!change' => l(t('Make changes to the targeting for this campaign.'), 'admin/structure/personalize/manage/' . $agent->machine_name . '/lift-target')))
    );
  }

  return $form;
}

/**
 * Submit callback for the campaign review form.
 */
function acquia_lift_review_form_submit($form, &$form_state) {
  $agent_name = $form_state['values']['agent'];
  $agent = personalize_agent_load($agent_name);
  acquia_lift_update_targeting($agent);
}

/**
 * Takes whatever is in the 'lift_targeting' data property and converts it into
 * the required campaign structure (including nested tests where needed).
 *
 * @param $agent
 *   The agent to create the targeting structure for.
 */
function acquia_lift_update_targeting($agent) {
  if (empty($agent->data['lift_targeting'])) {
    return;
  }
  // First we need to figure out what existing tests we have running as nested
  // tests for this agent.
  $option_set = acquia_lift_get_option_set_for_targeting($agent->machine_name);
  $existing_structure = acquia_lift_get_structure_from_targeting($option_set);
  // Any target audience that has multiple variations in it has a test running.
  $existing_tests = $existing_options = array();
  foreach ($existing_structure as $audience => $variations) {
    if (count($variations) > 1) {
      $existing_tests[$audience] = $variations;
    }
    else {
      $existing_options[$audience] = reset($variations);
    }
  }
  // Now look at the tests and options required by the new structure.
  $rules = $new_tests = $new_options = array();
  foreach ($agent->data['lift_targeting'] as $new_audience => $variations) {
    if (count($variations) == 1) {
      $new_options[$new_audience] = reset($variations);
      continue;
    }
    // See if this variation combination already exists as a test.
    foreach ($existing_tests as $existing_audience => $vars) {
      if ($variations == $vars) {
        $rules[$new_audience] = $existing_audience;
      }
    }
    // If we didn't find an existing test corresponding to this combination,
    // we'll need to create a new one.
    if (!isset($rules[$new_audience])) {
      $new_tests[$new_audience] = $variations;
    }
  }

  foreach (array_keys($existing_tests) as $audience) {
    if (!in_array($audience, $rules)) {
      // Delete the old test if it's no longer being used.
      acquia_lift_delete_old_nested_test($agent, $audience);
    }
    else {
      // Change the audience for the test if necessary.
      $new_audience = array_search($audience, $rules);
      if ($new_audience != $audience) {
        acquia_lift_set_audience_for_test($agent, $audience, $new_audience);
      }
    }
  }
  foreach ($existing_options as $audience => $option) {
    if (!isset($agent->data['lift_targeting'][$audience])) {
      acquia_lift_remove_option_for_audience($agent, $audience);
    }
  }

  // Now create any new tests required by the new structure.
  foreach ($new_tests as $audience => $variations) {
    acquia_lift_add_new_test_for_audience($agent, $variations, $audience);
  }
  foreach ($new_options as $audience => $option) {
    acquia_lift_set_option_for_audience($agent, $option, $audience);
  }
  // Now clobber the "lift_targeting" data as we only use that to store *changes*
  // to the running targeting.
  $agent->data['lift_targeting'] = array();
  personalize_agent_save($agent);
}

/**
 * Assigns an option to a target audience.
 *
 * @param $agent
 *   A stdClass object representing the agent.
 * @param $option_id
 *   The option ID
 * @param $audience
 *   The name of hte targeting audience to assign the option to.
 * @throws \PersonalizeException
 */
function acquia_lift_set_option_for_audience($agent, $option_id, $audience) {
  $option_set = acquia_lift_get_option_set_for_targeting($agent->machine_name);
  if (isset($option_set->targeting[$audience])) {
    // If there was a test assigned to this audience, remove it.
    if (isset($option_set->targeting[$audience]['osid'])) {
      unset($option_set->targeting[$audience]['osid']);
    }
    $option_set->targeting[$audience]['option_id'] = $option_id;
    personalize_option_set_save($option_set);
  }
}

/**
 * Unsets the option_id property for a given target audience.
 * @param $agent
 * @param $audience
 * @throws \PersonalizeException
 */
function acquia_lift_remove_option_for_audience($agent, $audience) {
  $option_set = acquia_lift_get_option_set_for_targeting($agent->machine_name);
  if (isset($option_set->targeting[$audience])) {
    // If there was a test assigned to this audience, remove it.
    if (isset($option_set->targeting[$audience]['option_id'])) {
      unset($option_set->targeting[$audience]['option_id']);
      personalize_option_set_save($option_set);
    }
  }
}

/**
 * Creates a new nested test of the specified variations for the specified agent.
 *
 * @param $parent_agent
 *   The parent agent to add the test to.
 * @param $variations
 *   The variations to test.
 * @param $audience
 *   The target audience for the test.
 * @param string $test_agent_type
 *   The agent plugin to use for the nested test.
 * @throws \PersonalizeException
 */
function acquia_lift_add_new_test_for_audience($parent_agent, $variations, $audience, $test_agent_type = 'acquia_lift') {
  module_load_include('inc', 'personalize', 'personalize.admin');
  $option_set = acquia_lift_get_option_set_for_targeting($parent_agent->machine_name);
  if (isset($option_set->targeting[$audience])) {
    // First we need to create a new agent.
    $agent = new stdClass();
    $agent->label = 'Sub-test for ' . $parent_agent->label;
    $agent->plugin = $test_agent_type;
    $agent->data = array();
    $agent->machine_name = personalize_generate_machine_name($parent_agent->machine_name . '-test', 'personalize_agent_machine_name_exists');
    try {
      $agent = personalize_agent_save($agent);
      $nested_os = new stdClass();
      $nested_os->agent = $agent->machine_name;
      $nested_os->is_new = TRUE;
      $nested_os->data = array();
      $nested_os->options = array();
      // This is basically a non-renderable option set - just set the plugin to
      // 'options'.
      $nested_os->plugin = 'options';
      foreach ($variations as $option_id) {
        $nested_os->options[] = array(
          'option_id' => $option_id
        );
      }
      $nested_os = personalize_option_set_save($nested_os);
    }
    catch (PersonalizeException $e) {
      // If anything goes wrong savign the new agent or option set, just print
      // the error message and bail.
      drupal_set_message($e->getMessage());
      return;
    }
    if (isset($option_set->targeting[$audience]['option_id'])) {
      unset($option_set->targeting[$audience]['option_id']);
    }
    $option_set->targeting[$audience]['osid'] = $nested_os->osid;
    personalize_option_set_save($option_set);
  }
}

/**
 * Saves a test to a particular targeting audience.
 *
 * @param $parent_agent
 *   The machine name of the parent agent.
 * @param $old_audience
 *   The target audience that currently has this test running.
 * @param $new_audience
 *   The target audience to move the test to.
 */
function acquia_lift_set_audience_for_test($parent_agent, $old_audience, $new_audience) {
  if ($old_audience == $new_audience) {
    return;
  }
  $option_set = acquia_lift_get_option_set_for_targeting($parent_agent->machine_name);
  if (isset($option_set->targeting[$old_audience]) && isset($option_set->targeting[$old_audience]['osid'])) {
    $nested_osid = $option_set->targeting[$old_audience]['osid'];
    unset($option_set->targeting[$old_audience]['osid']);
  }
  else {
    return;
  }

  // Now add the test to the new audience.
  if (isset($option_set->targeting[$new_audience])) {
    // If this audience previously had a single option assigned to it, remove
    // that option.
    if (isset($option_set->targeting[$new_audience]['option_id'])) {
      unset($option_set->targeting[$new_audience]['option_id']);
    }
    $option_set->targeting[$new_audience]['osid'] = $nested_osid;
  }
  personalize_option_set_save($option_set);
}

/**
 * Deletes a nested test that is no longer needed.
 *
 * @param $parent_agent
 *   The name of the parent agent.
 * @param $audience
 *   THe name of the target audience the test was assigned to.
 * @throws \PersonalizeException
 */
function acquia_lift_delete_old_nested_test($parent_agent, $audience) {
  $option_set = acquia_lift_get_option_set_for_targeting($parent_agent->machine_name);
  // Find the nested option set for the specified targeting rule.
  if (isset($option_set->targeting[$audience]) && isset($option_set->targeting[$audience]['osid'])) {
    $nested_os = personalize_option_set_load($option_set->targeting[$audience]['osid']);
    // Delete the option set, then delete the agent.
    personalize_option_set_delete($option_set->targeting[$audience]['osid']);
    personalize_agent_delete($nested_os->agent);
    unset($option_set->targeting[$audience]['osid']);
  }

  personalize_option_set_save($option_set);
}

/**
 * Saves the targeting structure defined via the UI.
 *
 * This does not actually *create* the structure defined via the UI as that
 * happens once the final review stage has been completed.
 *
 * @param $agent
 *   A stdClass ogject representing the agent to save the targeting structure for.
 * @param $targeting
 *   An array keyed by target audience with arrays of option IDs as values.
 * @throws \AcquiaLiftException
 */
function acquia_lift_save_targeting_structure($agent, $targeting) {
  if ($agent->plugin != 'acquia_lift_target') {
    throw new AcquiaLiftException('Invalid agent');
  }
  $option_set = acquia_lift_get_option_set_for_targeting($agent->machine_name);
  // Check that the targeting rules for the specified audiences exist in the
  // option set.
  foreach ($targeting as $audience => $vars) {
    if (!isset($option_set->targeting[$audience])) {
      throw new AcquiaLiftException('Invalid audience');
    }
  }
  $agent->data['lift_targeting'] = $targeting;
  personalize_agent_save($agent);
}

/**
 * Returns a mapping of audiences to option IDs based on the targeting set-up.
 *
 * Returns an array of option IDs for each audience, so if the audience is
 * assigned a single option, then it's a single element array, if it's assigned
 * a nested option set, we pull out the option IDs for the nested option set and
 * return them as an array.
 *
 * @param $option_set
 *   The option set to get the targeting structure for.
 * @return array
 *   An associative array whose keys are target audiences and whose values are
 *   arrays of option IDs.
 */
function acquia_lift_get_structure_from_targeting($option_set) {
  $targeting_structure = array();
  foreach ($option_set->targeting as $name => $targ) {
    $targeting_structure[$name] = array();
    if (isset($targ['osid'])) {
      $nested_option_set = personalize_option_set_load($targ['osid']);
      foreach ($nested_option_set->options as $option) {
        $targeting_structure[$name][] = $option['option_id'];
      }
    }
    elseif (isset($targ['option_id'])) {
      $targeting_structure[$name][] = $targ['option_id'];
    }
  }
  return $targeting_structure;
}

/**
 * Get the option set where targeting rules are defined.
 *
 * The assumption here is that if you have multiple option sets then targeting
 * is only defined on one of them as we currently do not support targeting for
 * MVTs.
 *
 * @param $agent
 *   A stdClass object representing the agent to get the option set for.
 * @return stdClass|NULL
 *   The option set to use for targeting rules or NULL if the agent has no option
 *   sets.
 */
function acquia_lift_get_option_set_for_targeting($agent_name) {
  $option_sets = personalize_option_set_load_by_agent($agent_name, TRUE);
  if (empty($option_sets)) {
    return NULL;
  }
  foreach ($option_sets as $option_set) {
    if (!empty($option_set->targeting)) {
      return $option_set;
    }
  }
  return reset($option_sets);
}
