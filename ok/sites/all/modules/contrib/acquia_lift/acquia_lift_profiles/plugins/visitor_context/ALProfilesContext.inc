<?php
/**
 * @file
 * Provides a context plugin for Acquia Lift Profiles
 */

class ALProfilesContext extends PersonalizeContextBase {

  /**
   * Implements PersonalizeContextInterface::create().
   */
  public static function create(PersonalizeAgentInterface $agent = NULL, $selected_context = array()) {
    try {
      $acquia_lift_profiles_api = ALProfilesAPI::getInstance();
      return new self($agent, $selected_context, $acquia_lift_profiles_api);
    }
    catch (Exception $e) {
      watchdog('Acquia Lift Profiles', $e->getMessage());
      return NULL;
    }
  }

  /**
   * Implements PersonalizeContextInterface::getOptions().
   */
  public static function getOptions() {
    $options = array();
    try {
      $acquia_lift_profiles_api = ALProfilesAPI::getInstance();
      $segments = $acquia_lift_profiles_api->getSegments();
      if (!empty($segments)) {
        foreach ($segments as $segment) {
          $options[$segment] = array(
            'name' => $segment,
            'group' => 'Acquia Lift Profiles segments',
            'cache_type' => 'local',
            'cache_expiration' => 'none',
          );
        }
      }
    }
    catch (Exception $e) {
      drupal_set_message($e->getMessage(), 'error');
    }
    return $options;
  }

  /**
   * Constructs an ALProfilesContext object.
   *
   * @param $selected_context
   * @param ALProfilesAPI $acquia_lift_profiles_api
   */
  public function __construct(PersonalizeAgentInterface $agent, $selected_context, ALProfilesAPI $acquia_lift_profiles_api) {
    parent::__construct($agent, $selected_context);
    $this->acquia_lift_profilesAPI = $acquia_lift_profiles_api;
  }

  /**
   * Implements PersonalizeContextInterface::getPossibleValues().
   */
  public function getPossibleValues($limit = FALSE) {
    $possible_values = array();
    $options = $this->getOptions();
    foreach ($options as $name => $info) {
      $possible_values[$name] = array(
        'value type' => 'boolean',
        'friendly name' =>  t('Acquia Lift Profiles segment: @segment', array('@segment' => $name)),
        'on_label' => t('Yes'),
        'off_label' => t('No')
      );
    }
    if ($limit) {
      $possible_values = array_intersect_key($possible_values, array_flip($this->selectedContext));
    }
    return $possible_values;
  }
}
