<?php
/**
 * @file
 * Provides an agent type for Acquia Lift
 */

interface AcquiaLiftAgentInterface {
  /**
   * Synchronizes the decision structure for this agent to Acquia Lift.
   *
   * If decision names or decision points have been renamed or removed, they
   * need to be deleted in Acquia Lift and the new points and decisions
   * added.
   *
   * @param $old_decisions
   *   Array representing the old decision structure from when it was
   *   last sync'd to AcquiaLift.
   * @param $new_decisions
   *   Array representing the new decision structure.
   * @return mixed
   */
  public function syncDecisions($old_decisions, $new_decisions);

  /**
   * Synchronize the goals for this agent to Acquia Lift.
   *
   * @param $old_goals
   *   The old goals that Acquia Lift knows about, with goal names as
   *   keys and goal values as values.
   * @param $new_goals
   *   The new goals, with goal names as keys and goal values as values.
   */
  public function syncGoals($old_goals, $new_goals);

  /**
   * Syncs fixed targeting rules to Acquia Lift.
   *
   * @param $option_sets
   *   An array of option set objects whose options may have fixed targeting rules
   *   associated with them.
   */
  public function syncFixedTargeting($option_sets);

  /**
   * Syncs the agent's status to Acquia Lift.
   */
  public function syncAgentStatus();

  /**
   * Organizes an array of option sets into decision points.
   *
   * @param $option_sets
   *   An array of option sets.
   *
   * @return array
   *   An associative array keyed by decision points, whose values are associative
   *   arrays with decision names as keys and an array of choices as values.
   */
  public static function convertOptionSetsToDecisions($option_sets);

  /**
   * Retrieves a conversion report.
   *
   * @param $options
   *   An array of report options.
   *   - decision_name: (Optional) The decision to get conversion report for.
   *   - goal: (Optional) The goal for limiting the report, defaults to all goals.
   *   - start: (Optional) The start time for report data in Y-m-d format.
   *     Defaults to agent start.
   *   - end: (Optional) The end time for report data in Y-m-d format.  Defaults
   *     to the current date.
   * @return array
   *   A renderable array.
   */
  public function buildConversionReport($options);
}

interface AcquiaLiftSimplifiedAgentInterface {
  public static function simplifiedForm($agent_data);
}

/**
 * An interface to implement when an agent handles page-level variations such
 * as a simple A/B test.
 */
interface AcquiaLiftPageVariationInterface { };

interface AcquiaLiftReportInterface {
  /**
   * Get the confidence measure for determining the statistical significance
   * of tests.
   */
  public function getConfidenceMeasure();

  /**
   * Set the confidence measure for determining the statistical significance
   * of tests.
   *
   * @param $value
   *   The value as a number between 0 and 100.
   */
  public function setConfidenceMeasure($value);

  /**
   * Retrieves a conversion report.
   *
   * @param $options
   *   An array of report options.
   *   - decision_name: (Optional) The decision to get conversion report for.
   *   - goal: (Optional) The goal for limiting the report, defaults to all goals.
   *   - start: (Optional) The start time for report data in Y-m-d format.
   *     Defaults to agent start.
   *   - end: (Optional) The end time for report data in Y-m-d format.  Defaults
   *     to the current date.
   * @return array
   *   A renderable array.
   */
  public function buildConversionReport($options);
}

/**
 * Factory class to create a report object for Acquia Lift.
 */
class AcquiaLiftReportFactory {

  /**
   * Creates an instance of the Acquia Lift Report class.
   *
   * @param $agent_instance
   *   The Acquia Lift agent that the reporting is for.
   * @param $api_instance
   *   The Acquia Lift API classes in use.
   * @return
   *   The AcquiaLiftReport class to use.
   */
  public static function create(PersonalizeAgentInterface $agent_instance, $api_instance) {
    $agent_name = $agent_instance->getMachineName();
    // Check if this agent is set up to read reports from a file instead of
    // calling the API.
    $report_file = variable_get("acquia_lift_report_source_$agent_name", '');
    if (!empty($report_file)) {
      $report_source = new AcquiaLiftReportDataFromFile($report_file, new AcquiaLiftReportCache());
    }
    else {
      $report_source = $api_instance;
    }

    if ($agent_instance instanceof AcquiaLiftSimpleAB) {
      $report = new AcquiaLiftABReport($agent_instance, $report_source);
    }
    else {
      $report = new AcquiaLiftReport($agent_instance, $report_source);
    }
    $report->setConfidenceMeasure(variable_get('acquia_lift_confidence_measure', 95));
    return $report;
  }
}

class AcquiaLiftAgent extends PersonalizeAgentBase implements PersonalizeAgentGoalInterface, AcquiaLiftAgentInterface, PersonalizeExplicitTargetingInterface, PersonalizeAutoTargetingInterface, PersonalizeAgentReportInterface {

  /**
   * An object containing the agent data.
   *
   * @var stdClass
   */
  protected $agent;

  /**
   * An instance of AcquiaLiftAPI.
   *
   * @var AcquiaLiftAPI
   */
  protected $liftAPI;

  /**
   * An instance of PersonalizeAgentReportInterface.
   *
   * The agent acts as a facade to the reporting class for reporting requests.
   *
   * @var PersonalizeAgentReportInterface
   */
  protected $reporting;

  /**
   * An instance of DrupalQueueInterface
   *
   * @var DrupalQueueInterface
   */
  protected $queue;

  protected $globalConfig;

  /**
   * Implements PersonalizeAgentInterface::create().
   */
  public static function create($agent_data) {
    try {
      $acquia_lift_api = AcquiaLiftAPI::getInstance(variable_get('acquia_lift_account_info', array()));
      $status = personalize_agent_get_status($agent_data->machine_name);
      $config = array(
        'confidence_measure' => variable_get('acquia_lift_confidence_measure', 95),
        'minimum_runtime' => acquia_lift_config_min_runtime(),
        'minimum_decisions' => variable_get('acquia_lift_min_decisions', 1000)
      );
      return new static($agent_data->machine_name, $agent_data->label, $agent_data->data, $status, !empty($agent_data->started) ? $agent_data->started : NULL, $acquia_lift_api, $config);
    }
    catch (AcquiaLiftException $e) {
      watchdog('Acquia Lift', 'Unable to instantiate Acquia Lift Agent');
      return NULL;
    }
  }

  /**
   * Constructs an Acquia Lift agent.
   *
   * @param stdClass $agent_data
   *   An object containing the agent data.
   * @param $acquia_lift_api
   *   An instance of the AcquiaLiftAPI class.
   */
  public function __construct($machine_name, $title, $data, $status, $started, AcquiaLiftAPI $acquia_lift_api, $global_config) {
    parent::__construct($machine_name, $title, $data, $status, $started);
    $this->liftAPI = $acquia_lift_api;
    $this->globalConfig = $global_config;
  }

  /**
   * Implements PersonalizeAgentInterface::getType();
   */
  public function getType() {
    return 'acquia_lift';
  }

  /**
   * Implements PersonalizeAgentInterface::getAssets().
   */
  public function getAssets() {
    $path = drupal_get_path('module', 'acquia_lift');
    return array(
      'js' => array(
        array(
          'type' => 'setting',
          'data' => array(
            'acquia_lift' => array(
              'apiKey' => $this->liftAPI->getApiKey(),
              'owner' => $this->liftAPI->getOwnerCode(),
              'baseUrl' => $this->liftAPI->getApiUrl(),
              'featureStringReplacePattern' => AcquiaLiftAPI::FEATURE_STRING_REPLACE_PATTERN,
              'featureStringMaxLength' => AcquiaLiftAPI::FEATURE_STRING_MAX_LENGTH,
              // @todo Add support for mutually exclusive values.
              'featureStringSeparator' => AcquiaLiftAPI::FEATURE_STRING_SEPARATOR_NONMUTEX,
              'batchMode' => variable_get('acquia_lift_batch_decisions', FALSE)
            )
          )
        ),
        // Add the Acquia Lift API js wrapper and the Acquia Lift integration js.
        $path . '/js/acquia_lift.js' => array('type' => 'file', 'scope' => 'footer', 'defer' => TRUE),
      ),
      'library' => array(
        array('acquia_lift', 'acquia_lift.agent_api'),
      ),
    );
  }

  /**
   * Implements PersonalizeAgentGoalInterface::useClientSideGoalDelivery().
   */
  public function useClientSideGoalDelivery() {
    return variable_get('acquia_lift_client_side_goals', TRUE);
  }

  /**
   * Implements PersonalizeAgentGoalInterface::sendGoal().
   */
  public function sendGoal($goal_name, $value = NULL) {
    // @todo Implement server-side goal delivery.
  }

  /**
   * Implements PersonalizeAgentInterface::optionsForm().
   */
  public static function optionsForm($agent_data, $option_parents = array()) {
    return self::buildOptionsForm($agent_data, FALSE, $option_parents);
  }

  /**
   * Builds the options to display for a campaign form.
   *
   * @param $agent_data
   *   The existing campaign data.
   * @param bool $simplified
   *   Indicates if the form should be shown in simplified format or with all
   *   advanced options.
   *
   * @return array
   *   The form render array.
   */
  protected static function buildOptionsForm($agent_data, $simplified = FALSE, $option_parents = array()) {
    $account_info = variable_get('acquia_lift_account_info', array());
    if (empty($account_info)) {
      drupal_set_message(t('Your Acquia Lift account info has not been configured. Any Acquia Lift campaigns you create here will not work until you configure your account info !here', array('!here' => l('here', 'admin/config/content/personalize/acquia_lift'))), 'error');
    }
    $form = array();
    $form['#attached'] = array(
      'css' => array(
        drupal_get_path('module', 'acquia_lift') . '/css/personalize_acquia_lift_admin.css',
        drupal_get_path('module', 'acquia_lift') . '/css/acquia_lift.admin.css',
      ),
      'js' => array(
        drupal_get_path('module', 'acquia_lift') . '/js/acquia_lift.agent.admin.js',
      ),
    );
    if (empty($option_parents)) {
      $option_parents = array('agent_basic_info', 'options', 'acquia_lift');
    }

    $control_rate = isset($agent_data->data['control_rate']) ? $agent_data->data['control_rate'] : 10;
    if ($simplified) {
      $form['control_rate'] = array(
        '#type' => 'value',
        '#value' => $control_rate,
      );
    }
    else {
      $form['control'] = array(
        '#type' => 'fieldset',
        '#tree' => FALSE,
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => t('Control (@control_rate%)', array('@control_rate' => $control_rate)),
      );
      $control_rate_parents = $option_parents;
      $control_rate_parents[] = 'control_rate';
      $form['control']['control_rate'] = array(
        '#type' => 'acquia_lift_percentage',
        '#parents' => $control_rate_parents,
        '#title' => t('Control Group'),
        '#field_suffix' => '%',
        '#size' => 3,
        '#description' => t('A fixed baseline variation will be shown, by default the first variation in the set.'),
        '#default_value' => $control_rate,
        '#rest_title' => t('Test Group'),
        '#rest_description' => t('Personalized variations will be shown.'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
    }
    $decision_style = isset($agent_data->data['decision_style']) ? $agent_data->data['decision_style'] : 'adaptive';
    if ($simplified) {
      $form['decision_style'] = array(
        '#type' => 'value',
        '#value' => $decision_style,
      );
    }
    else {
      $form['decision_style'] = array(
        '#type' => 'radios',
        '#title' => t('Decision Style'),
        '#options' => array('adaptive' => t('Auto-personalize'), 'random' => t('Test only')),
        '#default_value' => $decision_style,
        '#title_display' => 'invisible',
      );
      $form['decision_style']['adaptive'] = array(
        '#description' => t('Adapts to users and chooses the best option over time.'),
      );
      $form['decision_style']['random'] = array(
        '#description' => t('Tests variations and reports results.'),
      );
    }
    $explore_rate = isset($agent_data->data['explore_rate']) ? $agent_data->data['explore_rate'] : 20;
    if ($simplified) {
      $form['distribution'] = array(
        '#type' => 'value',
        '#value' => $explore_rate,
      );
    }
    else {
      $decision_style_parents = $option_parents;
      $decision_style_parents[] = 'decision_style';
      $decision_style_form_element = '';
      foreach ($decision_style_parents as $i => $parent_name) {
        $decision_style_form_element .= ($i ? '[' . $parent_name . ']' : $parent_name);
      }
      $form['distribution'] = array(
        '#type' => 'fieldset',
        '#tree' => FALSE,
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
        '#title' => t('Distribution (@explore_rate/@rest)', array(
          '@explore_rate' => $explore_rate,
          '@rest' => (100 - $explore_rate),
        )),
        '#states' => array(
          'visible' => array(
            ':input[name="' . $decision_style_form_element . '"]' => array('value' => 'adaptive'),
          ),
        ),
      );
      $explore_rate_parents = $option_parents;
      $explore_rate_parents[] = 'explore_rate';
      $form['distribution']['explore_rate'] = array(
        '#type' => 'acquia_lift_percentage',
        '#parents' => $explore_rate_parents,
        '#title' => t('Random Group'),
        '#field_suffix' => '%',
        '#description' => t('Variations will be shown randomly and tracked to adjust for false positives.'),
        '#size' => 3,
        '#default_value' => isset($agent_data->data['explore_rate']) ? $agent_data->data['explore_rate'] : 20,
        '#rest_title' => t('Personalized Group'),
        '#rest_description' => t('The "best" variation will be shown for each visitor based on our algorithm.'),
        '#collapsible' => TRUE,
        '#collapsed' => TRUE,
      );
    }
    // This will get overridden by the 'campaign_end' dropdown.
    $form['auto_stop'] = array(
      '#type' => 'value',
      '#value' => 0
    );
    return $form;
  }

  /**
   * Implements PersonalizeAgentInterface::optionsFormValidate().
   */
  public static function optionsFormValidate($form, &$form_state, $option_parents = array()) {
    $values = &$form_state['values'];
    foreach($option_parents as $parent) {
      $values = &$values[$parent];
    }
    $error_parents = implode('][', $option_parents);
    if (isset($values['control_rate'])) {
      $rate = $values['control_rate'];
      if (!is_numeric($rate) || !($rate >= 0 && $rate <= 100)) {
        $error_element = empty($error_parents) ? 'control_rate' : $error_parents . '][contral_rate';
        form_set_error($error_element, t('Invalid percent to test specified'));
      }
    }
    if (isset($values['explore_rate'])) {
      $rate = $values['explore_rate'];
      if (!is_numeric($rate) || !($rate >= 0 && $rate <= 100)) {
        $error_element = empty($error_parents) ? 'explore_rate' : $error_parents . '][explore_rate';
        form_set_error($error_element, t('Invalid percent to test specified'));
      }
    }
    // If "Run until a winner is found" was selected, we need to add this to the agent's
    // data property.
    if (isset($form_state['values']['campaign_end']) && $form_state['values']['campaign_end'] == 'auto') {
      $values['auto_stop'] = 1;
    }
  }

  /**
   * Implements PersonalizeAgentInterface::postSave().
   */
  public function postSave($old_data) {
    $items = $this->getAgentSyncOperations(isset($old_data->data['visitor_context']['acquia_lift_context']));

    $this->queueItems($items);
  }

  /**
   * Returns the operations needed to sync an agent to Acquia Lift.
   *
   * @param $targeting_rule_exists
   *   Whether the existing agent in Lift has a targeting rule set up that may
   *   need to be deleted.
   * @return array
   *   An array of items representing API calls to be made to Acquia Lift.
   */
  public function getAgentSyncOperations($targeting_rule_exists = FALSE) {
    $items = array();
    $acquia_lift_control_rate = .1;
    $acquia_lift_explore_rate = .2;
    if (isset($this->data['control_rate'])) {
      // Acquia Lift takes the control rate as a number between 0 and 1.
      $acquia_lift_control_rate = $this->data['control_rate'] / 100;
    }
    if (isset($this->data['explore_rate']) && isset($this->data['decision_style'])) {
      if ($this->data['decision_style'] === 'adaptive') {
        // Acquia Lift takes the explore rate as a number between 0 and 1.
        $acquia_lift_explore_rate = $this->data['explore_rate'] / 100;
      }
      else {
        // If the decision style is to only test, then the explore rate is the
        // full group.
        $acquia_lift_explore_rate = 1;
      }
    }
    // Add an item for saving the agent to Acquia Lift.
    $items[] = array(
      'method' => 'saveAgent',
      'args' => array(
        $this->machineName,
        $this->title,
        $this->data['decision_style'],
        $this->status,
        $acquia_lift_control_rate,
        $acquia_lift_explore_rate,
        isset($this->data['cache_decisions']) && $this->data['cache_decisions']
      ),
    );
    $acquia_lift_context_needs_deleting = $targeting_rule_exists;
    if (isset($this->data['visitor_context']['acquia_lift_context'])) {
      // Whereas non-Acquia Lift visitor_context plugins operate by adding extra
      // info at the time of getting a decision, Acquia Lift targeting needs to
      // be set up on the Acquia Lift side before any decisions are made.
      $auto_targeting = array_filter($this->data['visitor_context']['acquia_lift_context']);
      if (!empty($auto_targeting)) {
        $acquia_lift_context_needs_deleting = FALSE;
        // Add an item for saving the targeting rule.
        $items[] = array(
          'method' => 'saveAutoTargetingRule',
          'args' => array(
            $this->machineName,
            array_keys($auto_targeting)
          ),
        );
      }
    }
    if ($acquia_lift_context_needs_deleting) {
      // Acquia Lift may have a targeting rule for this agent, so we need
      // to delete it.
      $items[] = array(
        'method' => 'deleteAutoTargetingRule',
        'args' => array(
          $this->machineName
        ),
      );
    }
    return $items;
  }

  /**
   * Implements PersonalizeExplicitTargetingInterface::explicitTargetingSupportMultiple().
   */
  public static function explicitTargetingSupportMultiple() {
    return PersonalizeExplicitTargetingInterface::EXPLICIT_TARGETING_MULTIPLE_BOTH;
  }

  /**
   * Implements PersonalizeAutoTargetingInterface::constrainExplicitTargetingContexts().
   */
  public static function constrainExplicitTargetingContexts() {
    return TRUE;
  }

  /**
   * Implements PersonalizeAgentInterface::convertContextToFeatureString().
   *
   * This is essentially a PHP version of the js in the convertContextToFeatureString
   * function in acquia_lift.js
   */
  public static function convertContextToFeatureString($name, $value, $is_mutex = FALSE) {
    $separator = $is_mutex ? AcquiaLiftAPI::FEATURE_STRING_SEPARATOR_MUTEX : AcquiaLiftAPI::FEATURE_STRING_SEPARATOR_NONMUTEX;
    $prefix_max_length = floor((AcquiaLiftAPI::FEATURE_STRING_MAX_LENGTH - strlen($separator)) / 2);

    $prefix = AcquiaLiftAPI::cleanFeatureString($name);
    $value = AcquiaLiftAPI::cleanFeatureString($value);
    // Make a string of the visitor context item in the format Acquia Lift can
    // consume.
    $feature_string = $prefix . $separator . $value;

    while (strlen($feature_string) > AcquiaLiftAPI::FEATURE_STRING_MAX_LENGTH) {
      // Acquia Lift has a hard character limit for feature strings.
      if (strlen($prefix) > $prefix_max_length) {
        // Start by truncating the prefix down to half the max length.
        $prefix = substr($prefix, 0, $prefix_max_length);
        $feature_string = $prefix . $separator . $value;
      }
      else {
        // Otherwise just truncate the whole thing down to the max length.
        $feature_string = substr($feature_string, 0, AcquiaLiftAPI::FEATURE_STRING_MAX_LENGTH);
      }
    }
    return $feature_string;
  }

  /**
   * Get a reference to this agent's reporting class.
   *
   * Allows for delayed instantiation of reporting class.
   */
  protected function getReporting() {
    if (empty($this->reporting)) {
      $this->reporting = AcquiaLiftReportFactory::create($this, $this->liftAPI);
    }
    return $this->reporting;
  }

  /**
   * Implements PersonalizeAgentReportInterface::renderStatsForOptionSet().
   *
   * A facade to the reporting class.
   */
  public function renderStatsForOptionSet($option_set, $date_from, $date_to = NULL) {
    return $this->getReporting()->renderStatsForOptionSet($option_set, $date_from, $date_to);
  }

  /**
   * Implements PersonalizeAgentReportInterface::buildCampaignReports().
   *
   * A facade to the reporting class.
   */
  public function buildCampaignReports($options) {
    return $this->getReporting()->buildCampaignReports($options);
  }

  /**
   * Implements AcquiaLiftReportInterface()::buildConversionReport().
   */
  public function buildConversionReport($options) {
    return $this->getReporting()->buildConversionReport($options);
  }

  /**
   * Implements AcquiaLiftAgent::convertOptionSetsToDecisions().
   */
  public static function convertOptionSetsToDecisions($option_sets) {
    $points = array();
    foreach ($option_sets as $option_set) {
      // If for some reason one of our option sets is missing a point name or
      // decision name, throw an exception as we cannot proceed.
      if (!isset($option_set->decision_point) || !isset($option_set->decision_name)) {
        throw new AcquiaLiftException('Cannot convert option sets to a structured decision hierarchy without decision points and decision names');
      }
      $points[$option_set->decision_point] = isset($points[$option_set->decision_point]) ?
        $points[$option_set->decision_point] : array();
      $points[$option_set->decision_point][$option_set->decision_name] = isset($points[$option_set->decision_point][$option_set->decision_name]) ?
        $points[$option_set->decision_point][$option_set->decision_name] : array();
      foreach ($option_set->options as $option) {
        $points[$option_set->decision_point][$option_set->decision_name][] = $option['option_id'];
      }
    }
    return $points;
  }

  /**
   * Implements PersonalizeAgentInterface::errors().
   */
  public function errors() {
    $errors = array();
    try {
      $acquia_lift_agent = $this->liftAPI->getAgent($this->machineName);
    }
    catch (AcquiaLiftException $e) {
      return $this->convertAgentExceptionToErrors($e, $errors);
    }

    if ($acquia_lift_agent['status'] === AcquiaLiftAPI::PROVISIONAL_STATUS) {
      $errors[] = t('The status of the Acquia Lift agent is @status', array('@status' => $acquia_lift_agent['status']));
    }
    // Make sure Acquia Lift knows about the agent's goals.
    $goals = personalize_goal_load_by_conditions(array('agent' => $this->machineName));
    $discrepancies = FALSE;
    if (empty($goals)) {
      // Acquia Lift agents need goals.
      $errors[] = t('No goals have been set up for this agent');
    }

    try {
      $acquia_lift_goals = $this->liftAPI->getGoalsForAgent($this->machineName);
    }
    catch (AcquiaLiftException $e) {
      return $this->convertAgentExceptionToErrors($e, $errors);
    }
    foreach($goals as $goal) {
      if (!in_array($goal->action, $acquia_lift_goals)) {
        $errors[] = t('Goal @goal has not been sync\'d to the Acquia Lift agent.', array('@goal' => $goal->action));
        $discrepancies = TRUE;
      }
    }

    // Make sure all decision points are known by Acquia Lift.
    $option_sets = personalize_option_set_load_by_agent($this->machineName);
    if (empty($option_sets)) {
      // Acquia Lift agents need option sets.
      $errors[] = t('No variation sets have been set up for this agent');
    }
    $decision_tree = self::convertOptionSetsToDecisions($option_sets);
    try {
      $acquia_lift_points = $this->liftAPI->getPointsForAgent($this->machineName);
    }
    catch (AcquiaLiftException $e) {
      return $this->convertAgentExceptionToErrors($e, $errors);
    }
    foreach ($decision_tree as $point => $decisions) {
      if (!in_array($point, $acquia_lift_points)) {
        $errors[] = t('Point @point has not been sync\'d to the Acquia Lift agent.', array('@point' => $point));
        $discrepancies = TRUE;
        continue;
      }
      try {
        $acquia_lift_decisions = $this->liftAPI->getDecisionsForPoint($this->machineName, $point);
      }
      catch (AcquiaLiftException $e) {
        return $this->convertAgentExceptionToErrors($e, $errors);
      }
      foreach ($decisions as $decision_name => $options) {
        if (!in_array($decision_name, $acquia_lift_decisions)) {
          $errors[] = t('Decision @decision has not been sync\'d to the Acquia Lift agent.', array('@decision' => $decision_name));
          $discrepancies = TRUE;
        }
        try {
          $acquia_lift_choices = $this->liftAPI->getChoicesForDecision($this->machineName, $point, $decision_name);
        }
        catch (AcquiaLiftException $e) {
          return $this->convertAgentExceptionToErrors($e, $errors);
        }

        foreach ($options as $option) {
          if (!in_array($option, $acquia_lift_choices)) {
            $errors[] = t('Option @choice has not been sync\'d to the Acquia Lift agent.', array('@choice' => $option));
            $discrepancies = TRUE;
          }
        }
      }
      // @todo Check the fixed targeting for each decision point.
    }
    if ($discrepancies) {
      // Add a general message about how to resolve discrepancies.
      $message = t('To resolve the discrepancies between your agent configuration here and what has been sync\'d to the Acquia Lift service, try saving your campaign and its variation sets and goals again.');
      if (user_access('administer site configuration')) {
        $message .= t(' If that still does not resolve it, try <a href="@cron">running cron</a>.', array('@cron' => url('admin/reports/status/run-cron')));
      }
      $errors[] = $message;
    }
    return $errors;
  }

  /**
   * Implements PersonalizeAgentInterface::stopNow().
   */
  public function stopNow() {
    // Check whether the agent is configured to stop once a winner's been found.
    if (!isset($this->data['auto_stop']) || !$this->data['auto_stop']) {
      return parent::stopNow();
    }

    // If neither minimum runtime nor minimum decisions has been configured,
    // then return FALSE.
    if(!$this->globalConfig['minimum_runtime'] && !$this->globalConfig['minimum_decisions']) {
      return FALSE;
    }

    // If there a minimum runtime, check that it has been reached.
    $time_now = time();
    if ($this->globalConfig['minimum_runtime']) {
      $runtime = $time_now - $this->startTime;
      if ($runtime < $this->globalConfig['minimum_runtime']) {
        // Campaign has not been running long enough.
        return FALSE;
      }
    }

    // Need to go through each option set for this agent and establish whether each
    // of them satisfies the minimum number of decisions made, if this has been set.
    $option_sets = $this->data['decisions'];
    if (empty($option_sets)) {
      return FALSE;
    }
    $decision_points = self::convertOptionSetsToDecisions($option_sets);
    $points = array_keys($decision_points);
    $winners = array();
    foreach ($points as $point) {
      $num_decisions = 0;
      $vMeans = array();
      // Get the confidence report directly from Lift.
      $report_options = array(
        'confidence-measure' => $this->globalConfig['confidence_measure'],
        'aggregated-over-dates' => TRUE,
        'features' => "(none)",
      );
      $confidence_report = $this->liftAPI->getConfidenceReport(
        $this->machineName,
        $this->startTime,
        $time_now,
        $point,
        $report_options
      );

      $items = $confidence_report['data']['items'];
      if (empty($items)) {
        return FALSE;
      }

      foreach ($items as $item) {
        $choice = $item['choice'];
        $vMeans[$choice] = $item['vMean'];
        $num_decisions += $item['count'];
      }
      if ($this->globalConfig['minimum_decisions']) {
        // If any of the decision points has fewer than the required minimum number
        // of decisions, don't stop the campaign.
        if ($num_decisions < $this->globalConfig['minimum_decisions']) {
          return FALSE;
        }
      }

      // Find the variation with the highest estimated value.
      arsort($vMeans);
      $winners[$point] = key($vMeans);
    }

    // If we reach here, we have a winner at each decision point and have satisfied all
    // criteria for choosing a winner and pausing the campaign. Get all option sets for the
    // agent and set the winner for each one.
    $option_sets = personalize_option_set_load_by_agent($this->machineName);

    foreach ($winners as $point => $choice) {
      // The "winner" at any decision point might be a single decision or a combination
      // of decisions, in which case they'll be separated by a comma.
      $decisions = explode(',', $choice);
      foreach ($decisions as $decision) {
        list($decision_name, $option_id) = explode(':', $decision);
        // Get all option sets for this decision name.
        foreach ($option_sets as $osid => $option_set) {
          if ($option_set->decision_point == $point && $option_set->decision_name == $decision_name) {
            $option_set->winner = $option_id;
            personalize_option_set_save($option_set);
          }
        }
      }
    }
    return TRUE;
  }

  /**
   * Converts an exception thrown by the API class into errors added to the passed in array.
   *
   * @param AcquiaLiftException $e
   *   The excetion that was thrown.
   * @param $errors
   *   An array of errors to add to.
   * @return array
   *   The new errors array.
   */
  protected function convertAgentExceptionToErrors(AcquiaLiftException $e, &$errors) {
    if ($e instanceof AcquiaLiftNotFoundException) {
      $errors[] = t('This agent has not yet been pushed to Acquia Lift');
    }
    else {
      $errors[] = t('There was a problem communicating with the Acquia Lift server.');
    }
    return $errors;
  }

  /**
   * Returns a queue to use.
   *
   * @return DrupalQueueInterface
   */
  protected function getQueue() {
    if ($this->queue !== NULL) {
      return $this->queue;
    }
    return DrupalQueue::get('acquia_lift_sync');
  }

  /**
   * Sets the queue to use.
   *
   * @param DrupalQueueInterface $queue
   */
  public function setQueue(DrupalQueueInterface $queue) {
    $this->queue = $queue;
  }


  /**
   * Implements AcquiaLiftAgentInterface::syncAgentStatus().
   */
  public function syncAgentStatus() {
    $items = array();
    $items[] = array(
      'method' => 'updateAgentStatus',
      'args' => array(
        $this->machineName,
        $this->status,
      ),
    );
    $this->queueItems($items);
  }

  /**
   * Implements AcquiaLiftAgentInterface::syncDecisions().
   */
  public function syncDecisions($old_decisions, $new_decisions) {
    $items = $this->getDecisionSyncOperations($old_decisions, $new_decisions);
    $this->queueItems($items);
  }

  /**
   * Returns the operations needed to sync decisions to Acquia Lift.
   *
   * @param $old_decisions
   *   An array representing the old decisions Acquia Lift knows about.
   * @param $new_decisions
   *   An array representing the new decisions.
   * @return array
   *   An array of items representing API calls to be made to Acquia Lift.
   */
  public function getDecisionSyncOperations($old_decisions, $new_decisions) {
    $items = array();
    // Save everything in $new_decisions to Acquia Lift.
    foreach ($new_decisions as $point => $decisions) {
      $items[] = array(
        'method' => 'savePoint',
        'args' => array(
          $this->machineName,
          $point
        )
      );
      foreach ($decisions as $decision_name => $choices) {
        $items[] = array(
          'method' => 'saveDecision',
          'args' => array(
            $this->machineName,
            $point,
            $decision_name
          )
        );
        foreach ($choices as $choice) {
          $items[] = array(
            'method' => 'saveChoice',
            'args' => array(
              $this->machineName,
              $point,
              $decision_name,
              $choice
            )
          );
        }
      }
    }
    // Now remove anything that was in $old_decisions but not in
    // $new_decisions.
    foreach ($old_decisions as $point => $decisions) {
      if (!isset($new_decisions[$point])) {
        $items[] = array(
          'method' => 'deletePoint',
          'args' => array(
            $this->machineName,
            $point
          )
        );
      }
      else {
        foreach ($decisions as $decision_name => $choices) {
          if (!isset($new_decisions[$point][$decision_name])) {
            // Delete this decision from the decision point.
            $items[] = array(
              'method' => 'deleteDecision',
              'args' => array(
                $this->machineName,
                $point,
                $decision_name
              )
            );
          }
          else {
            foreach ($choices as $choice) {
              if (!in_array($choice, $new_decisions[$point][$decision_name])) {
                // Delete this choice from the decision.
                $items[] = array(
                  'method' => 'deleteChoice',
                  'args' => array(
                    $this->machineName,
                    $point,
                    $decision_name,
                    $choice
                  )
                );
              }
            }
          }
        }
      }
    }
    return $items;
  }

  /**
   * Implements AcquiaLiftAgentInterface::syncGoals().
   */
  public function syncGoals($old_goals, $new_goals) {
    $items = $this->getGoalSyncOperations($old_goals, $new_goals);
    $this->queueItems($items);
  }

  /**
   * Returns the operations needed to sync goals to Acquia Lift.
   *
   * @param $old_goals
   *   An array representing the old goals Acquia Lift knows about.
   * @param $new_goals
   *   An array representing the new goals.
   * @return array
   *   An array of items representing API calls to be made to Acquia Lift.
   */
  public function getGoalSyncOperations($old_goals, $new_goals) {
    $items = array();
    // Save the new goals to Acquia Lift
    foreach ($new_goals as $goal_name => $goal_value) {
      $items[] = array(
        'method' => 'saveGoal',
        'args' => array(
          $this->machineName,
          $goal_name
        )
      );
    }
    // Now delete any old goals that are not in the new goals array.
    foreach ($old_goals as $goal_name => $goal_value) {
      if (!isset($new_goals[$goal_name])) {
        $items[] = array(
          'method' => 'deleteGoal',
          'args' => array(
            $this->machineName,
            $goal_name
          )
        );
      }
    }
    return $items;
  }

  /**
   * Implements AcquiaLiftAgentInterface::syncFixedTargeting().
   */
  public function syncFixedTargeting($option_sets) {
    $items = $this->getFixedTargetingSyncOperations($option_sets);
    $this->queueItems($items);
  }

  /**
   * Returns the operations needed to sync goals to Acquia Lift.
   *
   * @param $option_sets
   *   An array representing the option sets whose targeting rules need to be sync'd.
   * @return array
   *    An array of items representing API calls to be made to Acquia Lift.
   */
  public function getFixedTargetingSyncOperations($option_sets) {
    $items = array();
    // If any of this agent's option sets has explicit targeting mappings configured,
    // we need to send these mappings to Acquia Lift.
    $mappings = array();
    foreach ($option_sets as $option_set) {
      if (empty($option_set->targeting)) {
        continue;
      }
      $point_name = $option_set->decision_point;
      $decision_name = $option_set->decision_name;
      $mappings[$point_name] = isset($mappings[$point_name]) ? $mappings[$point_name] : array();
      foreach ($option_set->targeting as $targ) {
        if (!isset($targ['option_id'])) {
          continue;
        }
        if (isset($targ['targeting_features'])) {
          // Check if we're supposed to AND or OR mulitple features together.
          if (isset($targ['targeting_strategy']) && $targ['targeting_strategy'] == 'AND') {
            // Create a single mapping, with a comma-separated list of features.
            $mappings[$point_name][] = array(
              'feature' => implode(',', $targ['targeting_features']),
              'decision' => $decision_name . ':' . $targ['option_id']
            );
          }
          else {
            // Create a mapping for each feature and they will be OR'd together.
            foreach ($targ['targeting_features'] as $feature) {
              $mappings[$point_name][] = array(
                'feature' => $feature,
                'decision' => $decision_name . ':' . $targ['option_id']
              );
            }
          }
        }
      }
    }

    // Send mappings per decision point.
    foreach ($mappings as $point_name => $map) {
      $items[] = array(
        'method' => 'saveFixedTargetingMapping',
        'args' => array(
          $this->machineName,
          $point_name,
          $map
        )
      );
    }
    return $items;
  }

  /**
   * Adds items to the queue and sets a message to inform the user.
   *
   * @param $items
   *   An array of items to add to the queue.
   */
  protected function queueItems($items) {
    if (!empty($items)) {
      foreach ($items as $item) {
        // Create a hash of the item. This will prevent duplicate items from
        // being added to the queue.
        $hash = md5(serialize($item));
        $item['hash'] = $hash;
        // We add the agent machine name to the queue item so that if it fails
        // the agent in question can be paused.
        $item['agent'] = $this->machineName;
        $this->getQueue()->createItem($item);
      }
      // Make sure the queue gets triggered on the next request.
      $_SESSION['acquia_lift_queue_trigger'] = 1;
    }
  }
}

/**
 * Class AcquiaLiftSimpleAB
 *
 * This is the class for agents of type 'acquia_lift_simple_ab'. For now it only
 * differs from the regular Acquia Lift agent class in the options form it presents
 * to users.
 */
class AcquiaLiftSimpleAB extends AcquiaLiftAgent implements AcquiaLiftSimplifiedAgentInterface, AcquiaLiftPageVariationInterface, PersonalizeAutoTargetingInterface {

  /**
   * Implements PersonalizeAgentInterface::optionsForm().
   */
  public static function simplifiedForm($agent_data) {
    return self::buildOptionsForm($agent_data, TRUE);
  }

  /**
   * Implements PersonalizeAgentInterface::supportsMVTs().
   */
  public function supportsMVTs() {
    return FALSE;
  }

  /**
   * Implements PersonalizeAgentInterface::supportsMultipleDecisionPoints().
   */
  public function supportsMultipleDecisionPoints() {
    return FALSE;
  }

}

/**
 * Base class providing report data loading functionality common to all
 * Acquia Lift Reports.
 */
abstract class AcquiaLiftReportBase implements PersonalizeAgentReportInterface, AcquiaLiftReportInterface {

  /**
   * The value to show when report data is not applicable.
   */
  const DATA_NA = '&mdash;';

  /**
   * The value representing no features applied to an experiment.
   */
  const NO_FEATURES = '(none)';

  /**
   * The Acquia Lift agent instance for reporting on.
   *
   * @var AcquiaLiftAgent
   */
  protected $agent;

  /**
   * An instance of AcquiaLiftAPI.
   *
   * @var AcquiaLiftAPI
   */
  protected $liftAPI;

  /**
   * The confidence measure for determining statistical significance.
   */
  protected $confidence_measure = 95;

  /**
   * The extracted report data for each of the Acquia Lift API calls keyed
   * by date/feature set requested.
   *
   * @var array;
   */
  protected $report_data;

  /**
   * Constructs an AcquiaLiftReport object
   *
   * @param PersonalizeAgentInterface $agent
   *   The agent the report is for.
   *
   * @param AcquiaLiftReportDataSourceInterface $report_data_src
   *   The source for the report data.
   */
  function __construct(PersonalizeAgentInterface $agent, AcquiaLiftReportDataSourceInterface $report_data_src) {
    $this->agent = $agent;
    $this->reportDataSrc = $report_data_src;
  }

  /**
   * Implements AcquiaLiftReportInterface()::getConfidenceMeasure().
   */
  public function getConfidenceMeasure() {
    return $this->confidence_measure;
  }

  /**
   * Implements AcquiaLiftReportInterface()::setConfidenceMeasure().
   */
  public function setConfidenceMeasure($value) {
    if ($value < 0) {
      $value = 0;
    }
    if ($value > 100) {
      $value = 100;
    }
    $this->confidence_measure = $value;
  }

  /**
   * Implements AcquiaLiftReportInterface()::buildConversionReport().
   */
  public function buildConversionReport($options) {
    $report_data = $this->generateReportConfiguration($options);
    $report_name = t('All goals');

    $report_options = array();
    if (!empty($options['goal'])) {
      // Get the summary data for the whole campaign in order to have overall
      // statistics.
      $this->loadConversionReportHelper($report_data, FALSE, $options);
      $report_options['goal'] = $options['goal'];
      $actions = visitor_actions_get_actions();
      // If the action still exists, use the label, otherwise just use the goal
      // machine name.
      if (isset($actions[$options['goal']])) {
        $report_name = $actions[$options['goal']]['label'];
      }
      else {
        $report_name = $options['goal'];
      }

    }
    $this->loadConversionReportHelper($report_data, TRUE, $report_options);
    $this->loadConversionReportHelper($report_data, FALSE, $report_options);

    $reports = $this->buildConversionReports(array(
      'name' => $report_name,
      'detail' => empty($options['goal']) ? $report_data['conversion_all']['detail'] : $report_data['conversion_goals'][$options['goal']]['detail'],
      'summary' => empty($options['goal']) ? $report_data['conversion_all']['summary'] : $report_data['conversion_goals'][$options['goal']]['summary'],
    ), $report_data);
    return $reports;
  }

  /**
   * Generates a message to show when there is insufficient confidence in the
   * test results.
   *
   * @return string
   */
  protected function getLowConfidenceMessage() {
    return t('There is not enough data to declare a winner with @confidence% confidence. Consider letting the test run longer before using the results.', array(
      '@confidence' => $this->getConfidenceMeasure(),
    ));
  }

  /**
   * Generates the variation abbreviated label.
   *
   * @param $counter
   *   Indicates the number for the variation.
   * @param $is_control
   *   True if this is the control option.
   */
  protected function getVariationLabel($counter, $is_control) {
    if ($is_control) {
      return t('Control');
    }
    else {
      return t('V@num', array('@num' => $counter));
    }
  }

  /**
   * Generates an internal raw report name for a confidence report based on
   * the options.
   *
   * @param $options
   *   An array of options passed to the confidence report loader.
   * @returns string
   *   The name of a report to reference within the raw reporting data.
   */
  protected function getConfidenceReportRawName($options = array()) {
    $report_name = 'confidence';
    if (!empty($options['goal'])) {
      $report_name .= '_' . $options['goal'];
    }
    if (isset($options['aggregated-over-dates']) && $options['aggregated-over-dates'] == FALSE) {
      $report_name .= '_detail';
    }
    return $report_name;
  }

  /**
   * Helper function to generate the report options necessary to get a detailed
   * confidence report rather than a summary report.
   *
   * @returns array
   *   An array of confidence report options.
   */
  protected function getConfidenceDetailReportOptions() {
    $options['aggregated-over-dates'] = FALSE;
    return $options;
  }

  /**
   * Generates the general report configuration that is used to load any report.
   *
   * @param $options
   *   An array of options for the report.
   *   - decision: (Optional) decision point name to limit results.
   *   - start: (Optional) start date for report, defaults to agent start.
   *   - end: (Optional) end date for report, defaults to current date.
   *   - goal: (Optional) goal to show in report, defaults to all.
   *   - conversion_metric: (Optional) metric to show in report, defaults to
   *     'rate'.
   * @return array
   *   The basic reporting configuration.
   */
  protected function generateReportConfiguration($options) {
    $decision_name = empty($options['decision']) ? NULL : $options['decision'];
    $date_from = empty($options['start']) ? NULL : $options['start'];
    $date_to = empty($options['end']) ? NULL : $options['end'];

    $machine_name = $this->agent->getMachineName();
    $today_only = $date_from === date('Y-m-d') && empty($date_to);
    $date_from = empty($date_from) ? date('Y-m-d', $this->agent->getStartTime()) : $date_from;
    $date_to = empty($date_to) ? date('Y-m-d') : $date_to;
    $key = 'S' . $date_from . 'E' . $date_to;

    if (!isset($this->report_data[$key])) {
      $confidence_measure = $this->getConfidenceMeasure();
      // Convert the confidence measure from a percentage to a value between
      // 0 and 1 as expected by the Lift API.
      $confidence_measure /= 100;

      // Save basic report generation information with the data.
      $this->report_data[$key]['today_only'] = $today_only;
      $this->report_data[$key]['date_from'] = $date_from;
      $this->report_data[$key]['date_to'] = $date_to;
      $this->report_data[$key]['decision_name'] = $decision_name;
      $this->report_data[$key]['machine_name'] = $machine_name;
      $this->report_data[$key]['confidence_measure'] = $confidence_measure;
      $this->report_data[$key]['features'] = array(AcquiaLiftReportBase::NO_FEATURES);
      $this->report_data[$key]['goal'] = empty($options['goal']) ? NULL : $options['goal'];
      $this->report_data[$key]['conversion_metric'] = empty($options['conversion_metric']) ? 'rate' : $options['conversion_metric'];
    }
    return $this->report_data[$key];
  }


  /**
   * Loads the context filter raw values into the report data.
   *
   * @param $report_data
   *   The current reporting array by reference.
   * @return array
   *   The updated reporting data for chaining purposes.  Note that the
   *   reporting data is updated by reference as well.
   */
  protected function loadContextFilterData(&$report_data) {
    // Check and see if it is already loaded.
    if (isset($report_data['raw']['potential_context'])) {
      return $report_data;
    }
    // Load it from the report source.
    try {
      $report_data['raw']['potential_context'] = $this->reportDataSrc->getContextFilters($report_data['machine_name']);
    }
    catch (Exception $e) {
      $report_data['raw']['potential_context']['error'] = $e->getMessage();
    }
    return $report_data;
  }

  /**
   * Loads the agent status raw reporting data.
   *
   * @param $report_data
   *   The current reporting array by reference.
   * @return array
   *   The updated reporting data for chaining purposes.  Note that the
   *   reporting data is updated by reference as well.
   */
  protected function loadAgentStatusData(&$report_data) {
    // Check and see if it is already loaded.
    if (isset($report_data['raw']['status'])) {
      return $report_data;
    }
    try {
      if ($report_data['today_only']) {
        $num_days = 1;
      }
      else {
        $interval = date_diff(date_create($report_data['date_from']), date_create($report_data['date_to']));
        $num_days = $interval->days;
      }
      $report_data['raw']['status'] = $this->reportDataSrc->getAgentStatusReport(array($report_data['machine_name']), $num_days);
    }
    catch (Exception $e) {
      $report_data['raw']['status']['error'] = $e->getMessage();
    }
    return $report_data;
  }

  /**
   * Loads the agent confidence raw reporting data.
   *
   * @param $report_data
   *   The current reporting array by reference.
   * @param $options
   *   (Optional) An array of options to pass to the report source.
   * @return array
   *   The updated reporting data for chaining purposes.  Note that the
   *   reporting data is updated by reference as well.
   */
  protected function loadConfidenceData(&$report_data, $options = array()) {
    $report_name = $this->getConfidenceReportRawName($options);
    // Check and see if it is already loaded.
    if (isset($report_data['raw'][$report_name])) {
      return $report_data;
    }
    try {
      $defaults = array(
        'features' => 'all',
        'confidence-measure' => $report_data['confidence_measure'],
      );
      $options = array_merge($defaults, $options);
      $report_data['raw'][$report_name] = $this->reportDataSrc->getConfidenceReport(
        $report_data['machine_name'],
        $report_data['date_from'],
        $report_data['date_to'],
        $report_data['decision_name'],
        $options
      );
    }
    catch (Exception $e) {
      $report_data['raw'][$report_name]['error'] = $e->getMessage();
    }
    return $report_data;
  }

  /**
   * Loads the agent targeting raw reporting data.
   *
   * @param $report_data
   *   The current reporting array by reference.
   * @return array
   *   The updated reporting data for chaining purposes.  Note that the
   *   reporting data is updated by reference as well.
   */
  protected function loadTargetingData(&$report_data) {
    // Check and see if it is already loaded.
    if (isset($report_data['raw']['targeting'])) {
      return $report_data;
    }
    try {
      $report_data['raw']['targeting'] = $this->reportDataSrc->getTargetingImpactReport(
        $report_data['machine_name'],
        $report_data['date_from'],
        $report_data['date_to'],
        $report_data['decision_name']
      );
    }
    catch (Exception $e) {
      $report_data['raw']['confidence']['error'] = $e->getMessage();
    }
    return $report_data;
  }

  /**
   * Formats a percentage value for use in reports.
   *
   * @param $value
   *   The number to show as a percentage.
   * @param bool $include_sign
   *   True to include positive/negative sign indicators.
   * @param $trim
   *   Boolean indicating whether the number should be trimmed of trailing 0s.
   * @param $decimals
   *   The number of decimal places to display.
   * @param $padding
   *   The total number of characters (including decimal) for padding of the
   *   final number.  This allows numbers to align properly  in column views.
   *   This will have no effect if trim is set to true.
   * @return string
   *   The formatted number to display.
   */
  protected function formatReportPercentage($value, $include_sign = FALSE, $trim = TRUE, $decimals = 2, $padding = 1) {
    $percent = (float) $value * 100;
    if ($percent > 0 && $include_sign) {
      return '+' . $this->formatReportNumber($percent, $trim, $decimals, $padding) . '%';
    }
    return $this->formatReportNumber($percent, $trim, $decimals, $padding) . '%';
  }

  /**
   * Formats a number value for use in reports.
   *
   * @param $value
   *   The number of format (or an empty value).
   * @param $trim
   *   Boolean indicating whether the number should be trimmed of trailing 0s.
   * @param $decimals
   *   The number of decimal places to display.
   * @param $padding
   *   The total number of characters to pad to the left of the decimal point.
   * @return string
   *   The formatted number to display.
   */
  protected function formatReportNumber($value, $trim = TRUE, $decimals = 2, $padding = 1) {
    if (is_numeric($value)) {
      $value = number_format($value, $decimals);
      if ($trim) {
        $value = rtrim(rtrim($value, '0'), '.');
      }
      if ($padding > 0) {
        $value = str_pad($value, $padding, '0', STR_PAD_LEFT);
      }
    }
    if (empty($value)) {
      $value = 0;
    }
    return $value;
  }

  /**
   * Builds the conversion reports to show basic conversion metrics for report
   * requested in the report_data.
   *
   * @param array $report_data
   *   The loaded report data for the selection decision and dates.
   * @return array
   *   The render array for the report.
   */
  protected function buildAllConversionReports($report_data) {
    if (empty($report_data['goal'])) {
      // Generate the conversion reports for all goals.
      $reports = $this->buildConversionReports(array(
        'name' => t('All goals'),
        'detail' => $report_data['conversion_all']['detail'],
        'summary' => $report_data['conversion_all']['summary'],
      ), $report_data);
    }
    else {
      // Generate the conversion reports for the specified goal.
      $reports = $this->buildConversionReports($report_data['conversion_goals'][$report_data['goal']], $report_data);
    }
    if ($reports == FALSE) {
      drupal_set_message(t('There was a problem retrieving the report data.  Please try again later.'), 'error');
    }
    $build['reports'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('lift-statistics'),
      ),
      'conversion' => $reports,
    );
    return $build;
  }

  /**
   * Handles all of the logic to load and extract a conversion report.
   *
   * @param $report_data
   *   The array of existing report data.
   * @param $detail
   *   True if the report should be a detailed report and false for summary.
   * @param array $options
   *   An array of report options such as a goal for limiting.
   */
  protected function loadConversionReportHelper(&$report_data, $detail, $options = array()) {
    // Limit report results to the experimental group.
    $options['policies'] = 'explore';
    if ($detail) {
      $options = array_merge($options, $this->getConfidenceDetailReportOptions());
    }
    $new_report = array();
    $report_name = $this->getConfidenceReportRawName($options);
    if (isset($options['goal'])) {
      // Add the report name for this goal to track which goals are included.
      $report_data['goal_reports'][] = $report_name;
      // Determine the parent for reports added by this helper.
      if (!isset($report_data['conversion_goals'])) {
        $report_data['conversion_goals'][$options['goal']] = FALSE;
      }
      $detail_report = &$report_data['conversion_goals'][$options['goal']];
    }
    else {
      // Determine the parent for reports added by this helper.
      if (!isset($report_data['conversion_all'])) {
        $report_data['conversion_all'] = FALSE;
      }
      $detail_report = &$report_data['conversion_all'];
    }
    // Check if the report is already loaded.
    $subreport_name = $detail ? 'detail' : 'summary';
    if (isset($detail_report[$subreport_name])) {
      return;
    }
    // Load the raw report data.
    if (!isset($report_data['raw'][$report_name])) {
      $this->loadConfidenceData($report_data, $options);
      if (isset($report_data['raw'][$report_name]['error'])) {
        return;
      }
    }
    if (!isset($report_data['raw'][$report_name]['data'])) {
      return;
    }
    // Extract the data into full report data from the raw data.
    $detail_report[$subreport_name] = $detail ? $this->extractConversionReportData($report_data['raw'][$report_name]['data']['items']) : $this->extractConversionSummaryData($report_data['raw'][$report_name]['data']['items']);
  }

  /**
   * Extracts the required overview data from the report data returned by
   * Acquia Lift.
   *
   * @param $items
   *   An array of items as returned from Acquia Lift.
   * @return array
   *   An associative array with information for today's and the total overview.
   */
  protected function extractOverviewReportData($items) {
    $agent_data = $this->agent->getData();
    $decision_style = $agent_data['decision_style'] === 'adaptive' ? t('Auto-personalize') : t('A/B');
    $total_variations = isset($agent_data['decisions']) ? count($agent_data['decisions']) : 0;

    // Create an array of data for each time option.
    $report['today'] = array(
      'unformatted' => array(
        'total_lift' => $items['today']['liftOverDefaultUsingGoals'],
        'total_shown' => $items['today']['sessionCount'],
        'total_goals' => $items['today']['goalCount'],
      ),
      'test_type' => $decision_style,
      'total_shown' => $this->formatReportNumber($items['today']['sessionCount']),
      'total_goals' => $this->formatReportNumber($items['today']['goalCount']),
      'total_goals_positive' => $items['today']['goalCount'] > 0,
      'total_lift' => $this->formatReportPercentage($items['today']['liftOverDefaultUsingGoals']),
      'total_variations' => $total_variations,
    );
    $report['all'] = array(
      'unformatted' => array(
        'total_shown' => $items['totals']['sessions']['count'],
        'total_goals' => $items['totals']['goals']['count'],
        'total_lift' => $items['today']['liftOverDefaultUsingGoalsToDate'],
      ),
      'test_type' => $decision_style,
      'total_shown' => $this->formatReportNumber($items['totals']['sessions']['count']),
      'total_goals' => $this->formatReportNumber($items['totals']['goals']['count']),
      'total_goals_positive' => $items['totals']['goals']['count'] > 0,
      'total_lift' => $this->formatReportPercentage($items['today']['liftOverDefaultUsingGoalsToDate']),
      'total_variations' => $total_variations,
    );
    return $report;
  }

  /**
   * Extracts data from the raw confidence detail report that is prepared for use
   * within the conversion report rendering process.
   *
   * @param $items
   *   An array of items as return from Acquia Lift.
   * @return array
   *   An associative array with information about the performance of each choice.
   */
  protected function extractConversionReportData($items) {
    if (empty($items)) {
      return array();
    }
    $shown = array();
    $counter = NAN;
    $data = array();

    $total_count = $total_goals = $total_val = array();
    foreach ($items as $item) {
      // Check to see if we are in a new grouping of choices.
      $check = $item['choice'];
      if (is_nan($counter) || isset($shown[$check])) {
        $shown = array();
        $counter = 0;
      }
      else {
        $counter++;
      }
      $shown[$check] = $check;

      $choice = $option_id = $item['choice'];
      $choice_id = $choice;
      if (strpos($choice, ':') !== FALSE) {
        list($decision_name, $option_id) = explode(':', $choice);
      }
      if ($option_label = personalize_get_option_label_for_decision_and_choice($decision_name, $option_id)) {
        $choice_id = $option_label;
      }
      $goals = $item['totals']['goals'];
      $count = $item['totals']['count'];
      $val = $item['totals']['val'];
      $total_count[$counter] = isset($total_count[$counter]) ? $total_count[$counter] + $count : $count;
      $total_goals[$counter] = isset($total_goals[$counter]) ? $total_goals[$counter] + $goals : $goals;
      $total_val[$counter] = isset($total_val[$counter]) ? $total_val[$counter] + $val : $val;

      $rate = $total_count[$counter] > 0 ? ($total_goals[$counter]/$total_count[$counter]) * 100 : 0;
      $val_rate = $total_count[$counter] > 0 ? ($total_val[$counter]/$total_count[$counter]) : 0;
      $margin = (($item['bHi'] - $item['bLo'])/2) * 100;

      $data[$item['feature']][] = array(
        'choice_id' => $choice_id,
        'raw_label' => $option_id,
        'goals' => $total_goals[$counter],
        'count' => $total_count[$counter],
        'date' => $item['date'],
        'timestamp' => strtotime($item['date']),
        'conversion' => $this->formatReportNumber($rate, TRUE, 4),
        'conversion_value' => $this->formatReportNumber($val_rate, TRUE, 4),
        'estimated_value' => $this->formatReportNumber($item['vMean'], TRUE, 4),
        'margin_error' => $this->formatReportNumber($margin, TRUE, 4),
        'counter' => $counter,
        'control' => $counter === 0,
      );
    }
    return $data;
  }

  /**
   * Loads and formats the necessary reporting data in order to generate a
   * conversion metrics graph/report.
   *
   * @param array
   *   The report data object to load conversion report data into.
   */
  protected function loadConversionReportData(&$report_data) {
    // All goals - summary report is always loaded in order to get the overview
    // data for the campaign.
    if (!isset($report_data['conversion'])) {
      $this->loadConversionReportHelper($report_data, FALSE);
    }
    // All goals conversion report.
    if (empty($report_data['goal'])) {
      // All goals - detail report.
      if (!isset($report_data['conversion_detail'])) {
        $this->loadConversionReportHelper($report_data, TRUE);
      }
      return;
    }
    // Load the detail and summary reports for the specified goal.
    $actions = visitor_actions_get_actions();
    $goal_id = $report_data['goal'];
    if (!isset($report_data['conversion_goals'][$goal_id])) {
      $report_data['conversion_goals'][$goal_id]['name'] = isset($actions[$goal_id]) ? $actions[$goal_id]['label'] : $goal_id;
      $options['goal'] = $goal_id;
      // Summary report.
      $this->loadConversionReportHelper($report_data, FALSE, $options);
      // Detail report.
      $this->loadConversionReportHelper($report_data, TRUE, $options);
    }
  }

  /**
   * Extracts data from the raw aggregate confidence report that is prepared for
   * use within the report rendering process.
   *
   * @param $items
   *   An array of items as return from Acquia Lift.
   * @return array
   *   An associative array with information about the performance of each choice.
   */
  protected function extractConversionSummaryData($items) {
    if (empty($items)) {
      return array();
    }
    $shown = array();
    $counter = NAN;
    $data = array();
    $confidence = FALSE;
    $winner = '';
    $winning_value = NAN;
    foreach ($items as $item) {
      // Check to see if we are in a new grouping of choices.
      $check = $item['choice'];
      // First item in the loop so this is the control.
      if (is_nan($counter) || isset($shown[$check])) {
        $shown = array();
        $counter = 0;
      }
      // One of the non-control variations.
      else {
        $counter++;
      }
      $shown[$check] = $check;

      $choice = $option_id = $item['choice'];
      $choice_id = $choice;
      if (strpos($choice, ':') !== FALSE) {
        list($decision_name, $option_id) = explode(':', $choice);
      }
      if ($option_label = personalize_get_option_label_for_decision_and_choice($decision_name, $option_id)) {
        $choice_id = $option_label;
      }
      $goals = $item['totals']['goals'];
      $count = $item['totals']['count'];
      // The format for percentages will already multiply by 100.
      $rate = $count > 0 ? ($goals/$count) : 0;
      $margin = (($item['bHi'] - $item['bLo'])/2);

      if ($item['signif']) {
        if (is_nan($winning_value) || $item['confidence'] > $winning_value) {
          $winning_value = $item['lift']['default'];
          $winner = $counter;
          $confidence = TRUE;
        }
      }

      $data[$item['feature']][] = array(
        'counter' => $counter,
        'choice_id' => $choice_id,
        'raw_label' => $option_id,
        'goals' => $goals,
        'count' => $count,
        'date' => $item['date'],
        'timestamp' => strtotime($item['date']),
        'conversion' => $this->formatReportPercentage($rate),
        'estimated_value' => $this->formatReportNumber($item['vMean'], TRUE, 4),
        'estimated_higher' => $this->formatReportNumber($item['bHi'], TRUE, 4),
        'estimated_lower' => $this->formatReportNumber($item['bLo'], TRUE, 4),
        'margin_error' => $this->formatReportNumber($margin, TRUE, 4),
        'significant' => $item['signif'],
        'control' => $counter === 0,
        'confidence' => $counter === 0 ? self::DATA_NA : $this->formatReportPercentage($item['confidence']/100),
        'lift_default' => $counter === 0 ? self::DATA_NA : $this->formatReportPercentage($item['lift']['default']/100, TRUE),
        'lift_random' => $this->formatReportPercentage($item['lift']['random']/100, TRUE),
      );
    }
    $report = array(
      'data' => $data,
      'overview' => array(
        'confidence' => $confidence,
        'winner' => $winner,
      ),
    );
    return $report;
  }

  /**
   * Builds the render array for the metrics portion of the report.
   *
   * @param array $report_data
   *   All of the reporting data for this AB report.
   * @param array $all_report_data
   *   The report data for all reports to be build including overview data.
   * @param array|bool
   *   A render array for the report or FALSE if it cannot be generated.
   */
  protected function buildConversionDetailReport($report_data, $all_report_data) {
    if ($report_data === FALSE) {
      return FALSE;
    }
    $headers = array(
      t('Date'),
      t('Content variation'),
      array(
        'data' => t('Conversion rate (%)'),
        'data-conversion-metric' => 'rate',
      ),
      array(
        'data' => t('Conversion value'),
        'data-conversion-metric' => 'value',
      ),
      t('Margin of error (%)'),
    );
    $rows = array();
    foreach ($report_data as $feature => $feature_data) {
      if (!in_array($feature, $all_report_data['features'])) {
        continue;
      }
      foreach($feature_data as $data) {
        $rows[] = array(
          'data' => array(
            array(
              'data' => $data['timestamp'],
            ),
            array(
              'data' => $data['choice_id'],
              'data-acquia-lift-variation-label' => $this->getVariationLabel($data['counter'], $data['control']),
            ),
            array(
              'data' => $data['conversion'],
            ),
            array(
              'data' => $data['conversion_value'],
            ),
            array(
              'data' => $data['margin_error'],
            ),
          ),
          'no_striping' => TRUE,
        );
      }
    }
    if (!empty($rows)) {
      $build['metric_table'] = array(
        '#theme' => 'table',
        '#header' => $headers,
        '#rows' => $rows,
        '#sticky' => FALSE,
        '#attributes' => array(
          'data-lift-statistics' => '',
          'data-liftGraph-columnName' => '2',
          'data-liftGraph-columnX' => '1',
          'data-liftGraph-renderer' => 'line',
          'data-liftgraph-excluded' => '5',
          'data-acquia-lift-campaign' => $all_report_data['machine_name'],
          'data-acquia-lift-decision-name' => $all_report_data['decision_name'],
        )
      );
    }
    return $build;
  }

  /**
   * Builds the render array for the summary portion of the report.
   *
   * @param array $report_data
   *   Reporting data for this summary report.
   * @param array $all_report_data
   *   The report data for all reports to be build including overview data.
   * @param array|bool
   *   A render array for the report or FALSE if it cannot be generated.
   */
  protected function buildConversionSummaryReport($report_data, $all_report_data) {
    if ($report_data === FALSE) {
      return FALSE;
    }
    $confidence = $all_report_data['conversion_all']['summary']['overview']['confidence'];
    $winner = $all_report_data['conversion_all']['summary']['overview']['winner'];
    $headers = array(
      t('Variation'),
      array(
        'data' => t('Total goals met'),
        'data-help-tooltip' => t('Number of times visitors completed a goal after viewing the variation.'),
      ),
      array(
        'data' => t('Total conversion rate'),
        'data-help-tooltip' => t('Percentage of goals met for each display of the variation.'),
      ),
      array(
        'data' => t('Chance to beat control'),
        'data-help-tooltip' => t('Likelihood visitors will complete goals for a variation compared to the control.'),
      ),
      array(
        'data' => t('Lift'),
        'data-help-tooltip' => t('Likelihood visitors will complete goals for a variation compared to the control.'),
      ),
      array(
        'data' => t('Winner'),
        'data-help-tooltip' => t('Most effective variation for visitors based on a @confidence% confidence level.', array(
          '@confidence' => $this->getConfidenceMeasure(),
        )),
      ),
    );
    $confidence_message_shown = FALSE;
    $rows = array();
    foreach ($report_data['data'] as $feature => $feature_data) {

      if (!in_array($feature, $all_report_data['features'])) {
        continue;
      }
      foreach ($feature_data as $data) {
        $row_data = array(
          array(
            'data' => $data['choice_id'],
            'data-acquia-lift-variation-label' => $this->getVariationLabel($data['counter'], $data['control']),
          ),
          array(
            'data' => $data['goals'],
          ),
          array(
            'data' => $data['conversion'],
          ),
          array(
            'data' => $data['confidence'],
          ),
          array(
            'data' => $data['lift_default'],
          ),
        );
        // Add the winner column data.
        if (empty($rows) && $report_data['overview']['confidence'] === FALSE) {
          // If there is low confidence then show the message throughout the
          // winner column.
          $row_data[] = array(
            'data' => $this->getLowConfidenceMessage(),
            'rowspan' => count($feature_data),
            'class' => array('acquia-lift-ab-winner'),
          );
          $confidence_message_shown = TRUE;
        }
        else if (!$confidence_message_shown) {
          // Show the winner indicator if this is the winning variation.
          $row_data[] = $confidence && $winner == $data['counter'] ? '<span class="lift-winner">' . t('Winner') . '</span>' : '';
        }
        $rows[] = array(
          'data' => $row_data,
          'no_striping' => TRUE,
        );
      }
    }
    if (empty($rows)) {
      return array();
    }
    $build['summary_holder'] = array(
      '#type' => 'container',
      '#attributes' => array(
        'class' => array('lift-graph-result'),
      ),
    );
    $build['summary_holder']['summary_table'] = array(
      '#theme' => 'table',
      '#header' => $headers,
      '#rows' => $rows,
      '#sticky' => FALSE,
      '#attributes' => array(
        'class' => array('lift-graph-result-data'),
        'data-acquia-lift-campaign' => $all_report_data['machine_name'],
        'data-acquia-lift-decision-name' => $all_report_data['decision_name'],
      ),
      '#attached' => array(
        'library' => array(
          array('acquia_lift', 'acquia_lift.help'),
        ),
      ),
    );
    return $build;
  }

  /**
   * Build a set of confidence reports from the report data.
   *
   * @param array $report_data
   *   The extracted report data with the following keys:
   *   - name: The name of the section
   *   - detail:  The detail report data
   *   - summary: The summary report data
   * @param array $all_report_data
   *   The full report data for all reports including overview information.
   * @return array|bool
   *   The render array for the reports or false if invalid data.
   */
  protected function buildConversionReports($report_data, $all_report_data) {
    if ($report_data['detail'] == FALSE || $report_data['summary'] == FALSE) {
      return FALSE;
    }
    $build = array();
    $build['reports']['title'] = array(
      '#theme' => 'html_tag',
      '#tag' => 'h3',
      '#value' => $report_data['name'],
      '#attributes' => array('class' => array('lift-statistic-category-name', 'element-invisible')),
    );
    $build['reports']['detail'] = $this->buildConversionDetailReport($report_data['detail'], $all_report_data);
    $build['reports']['summary'] = $this->buildConversionSummaryReport($report_data['summary'], $all_report_data);
    $build['reports']['#theme_wrappers'] = array('container');
    $build['reports']['#attributes'] = array('class' => array('lift-statistic-category'));
    return $build;
  }
}

/**
 * Responsible for retrieving and generating Acquia Lift AB Reports.
 */
class AcquiaLiftABReport extends AcquiaLiftReportBase {

  /**
   * Implements PersonalizeAgentReportInterface::renderStatusForOptionSet().
   */
  public function renderStatsForOptionSet($option_set, $date_from, $date_to = NULL) {
    // Not applicable to A/B tests made up of page variations.
    return array();
  }

  /**
   * Generates report overview data.
   */
  public function generateOverviewData(&$report_data) {
    $this->loadAgentStatusData($report_data);
    $report_data['status'] = $report = $this->extractOverviewReportData($report_data['raw']['status']['data'][$report_data['machine_name']]);
    if ($report === FALSE) {
      return array();
    }

    if ($report_data['today_only']) {
      $overview_report = $report_data['status']['today'];
    }
    else {
      $overview_report = $report_data['status']['all'];
    }
    $report_data['has_data'] = $overview_report['total_shown'] > 0;
    return $report_data;
  }

  /**
   * Implements PersonalizeAgentReportInterface::buildCampaignReports().
   */
  public function buildCampaignReports($options) {
    $report_data = $this->generateReportConfiguration($options);
    $this->loadConversionReportData($report_data);
    $this->generateOverviewData($report_data);
    $reports = $this->buildAllConversionReports($report_data);
    $reports['#has_data'] = $report_data['has_data'];
    return $reports;
  }
}

/**
 * Responsible retrieving and generating reports on the Acquia Lift agent.
 */
class AcquiaLiftReport extends AcquiaLiftReportBase {

  /**
   * The threshold to use above which lift percentage will be positively noted.
   */
  const LIFT_THRESHHOLD = 0;

  /**
   * The threshold to use above which stability will be positively noted.
   */
  const STABILITY_THRESHOLD = 25;

  /**
   * Implements PersonalizeAgentReportInterface::renderStatsForOptionSet().
   */
  public function renderStatsForOptionSet($option_set, $date_from, $date_to = NULL) {
    $date_start = date('Y-m-d', $date_from);
    if (empty($date_to)) {
    $date_to = time();
    }
    $date_end = date('Y-m-d', $date_to);
    $report_data = $this->generateReportConfiguration(array(
      'start' => $date_start,
      'end' => $date_end,
    ));

    if (!isset($report_data['confidence'])) {
      $this->loadConfidenceData($report_data);
      if (isset($report_data['raw']['confidence']['error']) || !isset($report_data['raw']['confidence']['data'])) {
        return array();
      }
      else {
        $data = $this->extractConfidenceReportData($report_data['raw']['confidence']['data']['items']);
      }
    }

    $decisions = $goals = 0;
    // Get a total count of all decisions made for this Option Set and all
    // goals received.
    foreach ($data['features'][self::NO_FEATURES] as $choice => $info) {
      list($decision_name, $option_id) = explode(':', $choice);
      if ($decision_name != $option_set->decision_name) {
        continue;
      }
      $decisions += $info['decisions'];
      $goals += $info['goals'];
    }
    $report[] = format_plural($decisions, '1 view', '@count views');
    $report[] = format_plural($goals, '1 goal', '@count goals');
    return $report;
  }

  /**
   * Implements PersonalizeAgentReportInterface::buildCampaignReports().
   */
  public function buildCampaignReports($options) {
    $report_data = $this->loadReportData($options);
    $reports = array(
      'overview' => $this->buildOverviewReport($report_data),
      'experiment' => $this->buildAllConversionReports($report_data),
      'context' => $this->buildContextReport($report_data),
      'stability' => $this->buildStabilityReport($report_data),
      'targeting' => $this->buildReportContextSelection($report_data),
    );
    $reports['#has_data'] = isset($reports['overview']['shown']['#title']) ? $reports['overview']['shown']['#title'] > 0 : FALSE;

    if (!is_array($report_data['status']) || !is_array($report_data['confidence']) || !is_array($report_data['targeting']) || !is_array($report_data['potential_context'])) {
      drupal_set_message(t('There was a problem retrieving the report data.  Please try again later.'), 'error');
    }
    else if ($reports['#has_data'] && $report_data['status']['all']['total_confident'] == 0) {
      drupal_set_message($this->getLowConfidenceMessage(), 'warning');
    }
    return $reports;
  }

  /**
   * Loads all of the data necessary to generate the reports for the agent.
   *
   * @param $options
   * An array of report filter options.
   * - decision: Preselected decision to display.
   * - start: The start date of the report.
   * - from: The end date of the report.
   * - goal: (optional) A selected goal for reporting; defaults to all.
   * - conversion_metric: (optional) Metric to display within the conversion report.  One
   *   of 'rate' (conversion rate) or 'value' (conversion value).
   * @return array
   *   The reporting data for the date range as an array with keys for
   *   - status: The general agent status report data
   *   - confidence: The confidence report data
   *   - targeting: The targeting report data
   */
  protected function loadReportData($options) {
    $report_data = $this->generateReportConfiguration($options);
    $this->loadConversionReportData($report_data);

    // Context filters.
    if (!isset($report_data['potential_context'])) {
      $this->loadContextFilterData($report_data);
      if (isset($report_data['raw']['potential_context']['error'])) {
        $report_data['potential_context'] = FALSE;
      }
      else {
        $report_data['potential_context'] = $this->extractPotentialTargetingValues($report_data['raw']['potential_context']['data']);
      }
    }
    // Agent status
    if (!isset($report_data['status'])) {
      $this->loadAgentStatusData($report_data);
      if (isset($report_data['raw']['status']['error'])) {
        $report_data['status'] = FALSE;
      }
      else {
        $report_data['status'] = $this->extractOverviewReportData($report_data['raw']['status']['data'][$report_data['machine_name']]);
      }
    }
    // Confidence report data.
    if (!isset($report_data['confidence'])) {
      $this->loadConfidenceData($report_data);
      if (isset($report_data['raw']['confidence']['error'])) {
        $report_data['confidence'] = FALSE;
      }
      else {
        $report_data['confidence'] = $this->extractConfidenceReportData($report_data['raw']['confidence']['data']['items']);
      }
    }
    // Targeting report data.
    if (!isset($report_data['targeting'])) {
      $this->loadTargetingData($report_data);
      if (isset($report_data['raw']['targeting']['error']) || !isset($report_data['raw']['targeting']['data']['items'])) {
        $report_data['targeting'] = FALSE;
      }
      else {
        $report_data['targeting'] = $this->extractTargetingReportData($report_data['raw']['targeting']['data']['items']);
      }
    }
    if (!isset($report_data['extracted_total'])) {
      // Combine data to form all campaign report data.
      $this->extractCampaignReportData($report_data);
    }
    return $report_data;
  }

  /**
   * Alter the report data returned from API calls to combine into data that is
   * ready for presentation within individual campaign reports.
   *
   * @param array $report_data
   *   The report data loaded and formatted from Acquia Lift.
   *
   * @see loadReportData()
   * @see extractOverviewReportData()
   * @see extractConfidenceReportData()
   * @see extractPotentialTargetingValues()
   * @see extractTargetingReportData()
   */
  protected function extractCampaignReportData(&$report_data) {
    $report_data['extracted_total'] = TRUE;
    if (!is_array($report_data['status']) || !is_array($report_data['confidence']) || !is_array($report_data['targeting']) || !is_array($report_data['potential_context'])) {
      return;
    }
    $agent_data = $this->agent->getData();
    $isAdaptive = $agent_data['decision_style'] === 'adaptive';

    // Determine overall confidence based on confidence in choices.
    $total_confident = 0;
    if (isset($report_data['confidence']['features'][self::NO_FEATURES])) {
      foreach ($report_data['confidence']['features'][self::NO_FEATURES] as $choice) {
        if ($choice['significant']) {
          $total_confident++;
        }
      }
    }

    // Determine the overall time running for this agent.
    $interval_start = new DateTime();
    $interval_start->setTimestamp($this->agent->getStartTime());
    $interval = date_diff($interval_start, date_create());

    // Update the status report data (used for overview report).
    foreach($report_data['status'] as &$report) {
      $report['total_lift_positive'] = $report['unformatted']['total_lift'] > self::LIFT_THRESHHOLD && $total_confident > 0;
      $report['total_confident'] = $total_confident;
      $report['confidence_level'] = $total_confident > 0 ? 'high' : 'low';
      $report['time_running'] = isset($interval) ? $interval->format('%mm, %dd') : '1d';
    }

    // Update context report data.
    $option_numbers = array();
    if (isset($report_data['confidence']['features'])) {
      foreach ($report_data['confidence']['features'] as $feature_string => $feature) {
        // Get the user-friendly feature label from the possible contextual values.
        $feature_label = $feature_string;
        if (isset($report_data['potential_context'][$feature_string])) {
          $feature_label = $report_data['potential_context'][$feature_string]['label'];
        }

        // This report only shows features that can be targeted.
        if (!isset($report_data['targeting'][$feature_string])) {
          continue;
        }

        // Get the data from the targeting report for this feature.
        $targeting_data = $report_data['targeting'][$feature_string];

        // Create a hash of choice numbers for stability report.
        // Don't show system-defined features.
        if ($targeting_data['system'] === TRUE) {
          continue;
        }
        foreach ($feature as $choice_id => $choice) {
          $report_data['context']['features'][$feature_string][$choice_id] = array(
            'counter' => $choice['counter'],
            'choice_id' => $choice['choice_id'],
            'best' => ($isAdaptive && $targeting_data['favored_selection'] === $choice['raw_label']),
            'decisions' => $choice['decisions'],
            'lift_default' => $choice['control'] ? self::DATA_NA : $choice['lift_default'],
            'lift_default_positive' => $choice['unformatted']['lift_default'] > self::LIFT_THRESHHOLD,
            'lift_random' => $choice['lift_random'],
            'lift_random_positive' => $choice['unformatted']['lift_random'] > self::LIFT_THRESHHOLD,
            'control' => $choice['control'],
            'feature_label' => $feature_label,
            'goals' => $choice['goals'],
            'conversion' => $choice['conversion'],
          );
          $option_numbers[$feature_string . '|' . $choice['raw_label']] = $choice['counter'];
        }
      }
    }

    // Build the experiment report data
    $report_data['experiment']['choices'] = isset($report_data['confidence']['features'][self::NO_FEATURES]) ? $report_data['confidence']['features'][self::NO_FEATURES] : array();

    // Build the stability report data
    foreach ($report_data['targeting'] as $feature_string => &$feature) {
      if ($feature['system']) {
        unset($report_data['targeting'][$feature_string]);
        continue;
      }
      // Get the user-friendly feature label from the possible contextual values.
      $feature['feature_label'] = $feature['label'];
      if (isset($report_data['potential_context'][$feature_string])) {
        $feature['feature_label'] = $report_data['potential_context'][$feature_string]['label'];
      }
      if (isset($option_numbers[$feature_string . '|' . $feature['favored_selection']])) {
        $feature['favored_selection_number'] = $option_numbers[$feature_string . '|' . $feature['favored_selection']];
      }
      if (!$isAdaptive) {
        unset($feature['favored_selection_number']);
        unset($feature['favored_selection']);
      }
    }
  }

  /**
   * Returns a render array representing the overview report for the given dates.
   *
   * @param array $report_data
   *   All of the reporting data for the campaign.
   * @return array
   *   A render array representing the overview report.
   */
  protected function buildOverviewReport($report_data) {
    $report = $report_data['status'];
    if ($report === FALSE) {
      return array();
    }

    if ($report_data['today_only']) {
      $overview_report = $report_data['status']['today'];
    }
    else {
      $overview_report = $report_data['status']['all'];
    }

    // Create report renderable.
    $build = array();
    $build['test_type'] = array(
      '#type' => 'container',
      '#theme' => 'acquia_lift_report_overview',
      '#title' => $overview_report['test_type'],
      '#description' => t('test type'),
      '#attributes' => array(
        'id' => 'acquia-lift-overview-type',
      )
    );
    if (isset($overview_report['time_running'])) {
      $build['total_running'] = array(
        '#type' => 'container',
        '#theme' => 'acquia_lift_report_overview',
        '#attributes' => array(
          'id' => 'acquia-lift-overview-running',
        ),
        '#title' => $overview_report['time_running'],
        '#description' => t('total time running'),
      );
    }
    $build['shown'] = array(
      '#type' => 'container',
      '#theme' => 'acquia_lift_report_overview',
      '#attributes' => array(
        'id' => 'acquia-lift-overview-shown',
      ),
      '#title' => $overview_report['total_shown'],
      '#description' => format_plural($overview_report['total_shown'], 'time shown', 'times shown'),
    );
    $build['goals'] = array(
      '#type' => 'container',
      '#theme' => 'acquia_lift_report_overview',
      '#attributes' => array(
        'id' => 'acquia-lift-overview-goals',
      ),
      '#title' => $overview_report['total_goals'],
      '#description' => t('goals met'),
    );
    if ($overview_report['total_goals_positive']) {
      $build['goals']['#attributes']['class'] = array('acquia-lift-report-positive');
    }
    /*
     * @todo: Figure out a way to present and explain this information so
     * that we can include these figures.
    $build['lift'] = array(
      '#type' => 'container',
      '#theme' => 'acquia_lift_report_overview',
      '#attributes' => array(
        'id' => 'acquia-lift-overview-lift',
      ),
      '#title' => $overview_report['total_lift'],
      '#description' => t('Predicted lift/control'),
      '#attributes' => array(
        'class' => array(
          $overview_report['total_lift_positive'] ? 'acquia-lift-report-positive' : 'acquia-lift-report-negative',
        ),
      ),
    );
    $build['confidence'] = array(
      '#type' => 'container',
      '#theme' => 'acquia_lift_report_overview',
      '#attributes' => array(
        'id' => 'acquia-lift-overview-confidence',
      ),
      '#title' => t('Confidence'),
      '#description' =>  $overview_report['confidence_level'] == 'high' ? format_plural($overview_report['total_confident'], 'High confidence, 1 var.', 'High confidence, @count vars.') : t('Low confidence'),
      '#attributes' => array(
        'class' => array(
          $overview_report['confidence_level'] == 'high' ? 'acquia-lift-report-positive' : 'acquia-lift-report-negative',
        ),
      ),
      '#total_confident' => $overview_report['total_confident'],
    );
    */
    return $build;
  }

  /**
   * Returns a render array representing the context report.
   *
   * @param array $report_data
   *   Reporting data for the selected dates and decision.
   * @return array
   *   A render array representing the variation set report.
   */
  protected function buildContextReport($report_data) {
    $build = array();

    if ($report_data['confidence'] === FALSE || !isset($report_data['context']['features'])) {
      return array();
    }
    $header = array(
      t('Var.'),
      t('Name'),
      t('Context'),
      t('Shown'),
      t('Goals'),
      t('Conversion rate'),
      t('Lift over control'),
      t('Lift over random'),
    );
    $rows = array();
    foreach($report_data['context']['features'] as $feature_string => $feature) {
      foreach($feature as $choice) {
        $lift_default_classes = array();
        if (!$choice['control']) {
          $lift_default_classes = $choice['lift_default_positive'] ? 'acquia-lift-report-positive' : 'acquia-lift-report-negative';
        }
        $row = array();
        $row[] = $this->getVariationLabel(($choice['counter']-1), $choice['control']);
        if ($choice['control']) {
          $row[] = t('Control: ') . $choice['choice_id'];
        }
        else {
          $row[] = $choice['choice_id'];
        }
        $row[] = array(
          'data' => $choice['best'] ?  $choice['feature_label'] . ' <span class="acquia-lift-best">' . t('best') . '</span>' : $choice['feature_label'],
          'class' => $choice['best'] ? array('acquia-lift-context-best') : array(),
        );
        $row[] = $choice['decisions'];
        $row[] = $choice['goals'];
        $row[] = $choice['conversion'];
        $row[] = array(
          'data' => $choice['lift_default'],
          'class' => $lift_default_classes,
        );
        $row[] = array(
          'data' => $choice['lift_random'],
          'class' => $choice['lift_random_positive'] ? array('acquia-lift-report-positive') : array('acquia-lift-report-negative'),
        );
        $rows[] = array(
          'data' => $row,
          'class' => $choice['control'] ? array('acquia-lift-report-control') : array(),
          'no_striping' => $choice['control'],
          'data-acquia-lift-feature' => $feature_string,
        );
      }
    }

    $build['content'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#sticky' => FALSE,
    );
    return $build;
  }

  /**
   * Returns a render array representing the stability report for targeting
   * features for the given dates.
   *
   * @param array $report_data
   *   Reporting data for the selected dates and decision.
   * @return array
   *   A render array representing the targeting report
   */
  protected function buildStabilityReport($report_data) {
    $build = array();
    if ($report_data['targeting'] === FALSE) {
      return array();
    }

    $data = $report_data['targeting'];

    $rows = array();
    $show_favored_selection = FALSE;
    foreach ($data as $feature => $f) {
      $row_data = array($f['feature_label']);
      if (isset($f['favored_selection_number'])) {
        $show_favored_selection = TRUE;
        $row_data[] = $this->getVariationLabel(($f['favored_selection_number']-1), ($f['favored_selection_number'] == 1));
      }
      $row_data[] = array(
        'data' => $f['percent_traffic'],
        'class' => array($f['percent_traffic_graph']),
      );
      $row_data[] = array(
        'data' => $f['stability'],
        'class' => $f['stability_positive'] ? array('acquia-lift-report-positive') : array('acquia-lift-report-negative'),
      );
      $rows[] = array(
        'data' => $row_data,
        'data-acquia-lift-feature' => $feature,
      );
    }
    $header[] = t('Context');
    if ($show_favored_selection) {
      $header[] = t('Best Variation');
    }
    $header[] = t('Percent of Traffic');
    $header[] = t('Stability');
    $build['content'] = array(
      '#theme' => 'table',
      '#header' => $header,
      '#rows' => $rows,
      '#sticky' => FALSE,
    );
    return $build;
  }

  /**
   * Returns a form input array for the context selector.
   *
   * @param array $report_data
   *   Reporting data for the selected dates and decision.
   * @return array
   *   A render array representing the variation set report.
   */
  protected function buildReportContextSelection($report_data) {
    if ($report_data['potential_context'] === FALSE || $report_data['targeting'] === FALSE) {
      return array();
    }
    $context_values = array();
    foreach($report_data['targeting'] as $code => $feature) {
      if ($feature['system'] === TRUE) {
        continue;
      }
      if (isset($report_data['potential_context'][$code])) {
        $type = empty($report_data['potential_context'][$code]['type']) ? t('Other') : $report_data['potential_context'][$code]['type'];
        $context_values[$type][$code] = $report_data['potential_context'][$code]['name'];
      }
      else {
        $type = t('Other');
        $context_values[$type][$code] = $code;
      }
    }
    if (count($context_values) <= 1) {
      return array();
    }
    return array(
      '#title' => t('Context: '),
      '#type' => 'select',
      '#options' => $context_values,
      '#multiple' => TRUE,
    );
  }

  /**
   * Extracts the required confidence report data from the items returned by Acquia Lift.
   *
   * @param $items
   *   An array of items as return from Acquia Lift.
   * @return array
   *   An associative array with information about the performance of each choice.
   */
  protected function extractConfidenceReportData($items) {
    if (empty($items)) {
      return array();
    }
    $data = array(
      'point' => $items[0]['point'],
      'features' => array(),
      'goal_value_differential' => FALSE,
    );
    $last_group = '';
    $counter = 1;
    foreach ($items as $i => $item) {
      // Check to see if we are in a new grouping of choices.
      $check = $item['feature'] . '|' . $item['point'];
      if ($last_group !== $check) {
        $last_group = $check;
        $counter = 1;
      }
      else {
        $counter++;
      }
      $choice = $option_id = $item['choice'];
      $choice_id = $choice;
      if (strpos($choice, ':') !== FALSE) {
        list($decision_name, $option_id) = explode(':', $choice);
        // @todo: Would like to get rid of this function call in order to make
        // the class unit-testable.
        if ($option_label = personalize_get_option_label_for_decision_and_choice($decision_name, $option_id)) {
          $choice_id = $option_label;
        }
      }
      $data['features'][$item['feature']][$choice] = array(
        'unformatted' => array(
          'lift_default' => $item['lift']['default'],
          'lift_random' => $item['lift']['random'],
        ),
        'counter' => $counter,
        'choice_id' => $choice_id,
        'raw_label' => $option_id,
        'decisions' => format_plural($item['totals']['count'], '1 time', '@count times'),
        'goals' => $item['totals']['goals'],
        'value' => $item['totals']['val'],
        'estimated_value' => $this->formatReportNumber($item['vMean'], TRUE, 4),
        'estimated_lower' => $this->formatReportNumber($item['bLo'], TRUE, 4),
        'estimated_higher' => $this->formatReportNumber($item['bHi'], TRUE, 4),
        'goals_per_decision' => $item['totals']['goals'] == 0 ? self::DATA_NA : $this->formatReportNumber($item['totals']['goalsPerDecision'], FALSE),
        'value_per_decision' => $item['totals']['goals'] == 0 ? self::DATA_NA : $this->formatReportNumber($item['totals']['valPerDecision'], FALSE),
        'selections' => $item['count'],
        'conversion' => $item['totals']['goals'] > 0 ? $this->formatReportPercentage($item['totals']['goals'] / $item['totals']['count']) : self::DATA_NA,
        'confidence' => $counter === 1 ? self::DATA_NA : $this->formatReportPercentage($item['confidence']/100),
        'lift_default' => $counter === 1 ? self::DATA_NA : $this->formatReportPercentage($item['lift']['default']/100, TRUE),
        'lift_random' => $this->formatReportPercentage($item['lift']['random']/100, TRUE),
        'significant' => $item['signif'],
        'control' => $counter === 1,
      );
      if (!$data['goal_value_differential'] && ($item['totals']['goals'] != $item['totals']['val'])) {
        $data['goal_value_differential'] = TRUE;
      }
    }
    return $data;
  }

  /**
   * Extracts the required targeting report data from the items returned by Acquia Lift.
   *
   * @param $items
   *   An array of items as return from Acquia Lift.
   * @return array
   *   An associative array with feature codes as keys and associative arrays of info as
   *   values.
   */
  protected function extractTargetingReportData($items) {
    if (empty($items)) {
      return array();
    }
    $data = array();
    foreach ($items as $item) {
      $feature = $item['feature'];
      $favored_selection = 0;
      foreach ($item['choices'] as $i => $choice) {
        // Note: Choices are not in order of option.
        if ($choice['score'] > $item['choices'][$favored_selection]['score']) {
          $favored_selection = $i;
        }
      }
      $data[$feature] = array(
        'raw_label' => $item['label'],
        'label' => $item['labelText'],
        'favored_selection' =>  $item['choices'][$favored_selection]['label'],
        'percent_traffic' => $this->formatReportPercentage($item['percentTraffic']),
        'percent_traffic_graph' => $this->getGraphLevelClass($item['percentTraffic']),
        'predicted_value' => $item['averageResponseValue'],
        'stability' => $this->formatReportNumber($item['stability']),
        'stability_positive' => $item['stability'] > self::STABILITY_THRESHOLD,
        'stability_level' => $item['stabilityLevel'],
        'system' => strpos($item['label'], '[share-alt]') !== FALSE,
      );
    }
    return $data;
  }

  /**
   * Extracts potential targeting values from the raw data returned by Acquia
   * Lift.
   *
   * @param $items
   *   An array of raw potential values.
   * $return array
   *   An associative array of potential targeting features keyed by code.
   */
  protected function extractPotentialTargetingValues($items) {
    $data = array();
    if (isset($items['potential']['features']) && !empty($items['potential']['features'])) {
      foreach ($items['potential']['features'] as $feature) {
        $data[$feature['code']] = array(
          'type' => isset($feature['typeName']) ? $feature['typeName'] : '',
          'name' => ($feature['name'] === '-' || $feature['name'] === '0') ? $feature['code'] : $feature['name'],
        );
        $data[$feature['code']]['label'] = empty($data[$feature['code']]['type']) ? $data[$feature['code']]['name'] : $data[$feature['code']]['type'] . ': ' . $data[$feature['code']]['name'];
      }
    }
    return $data;
  }

  /**
   * Gets the appropriate class name for a graph indicating percentage.
   *
   * @param $value
   *   The percentage value expressed as a number between 0 and 1.
   */
  protected function getGraphLevelClass($value) {
    if ($value >= 1) {
      return 'acquia-lift-graph-level-5';
    }
    else if ($value >= .8) {
      return 'acquia-lift-graph-level-4';
    }
    else if ($value >= .6 && $value < .8) {
      return 'acquia-lift-graph-level-3';
    }
    else if ($value >= .4 && $value < .6) {
      return 'acquia-lift-graph-level-2';
    }
    else if ($value >= .2 && $value < .4) {
      return 'acquia-lift-graph-level-1';
    }
    else {
      return 'acquia-lift-graph-level-0';
    }
  }
}
