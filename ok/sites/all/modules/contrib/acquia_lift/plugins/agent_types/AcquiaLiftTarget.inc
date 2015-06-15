<?php
/**
 * @file
 * Provides an agent for doing fixed targeting.
 */

class AcquiaLiftTarget extends PersonalizeAgentBase implements PersonalizeExplicitTargetingInterface {

  /**
   * Implements PersonalizeAgentInterface::create().
   */
  public static function create($agent_data) {
    $status = personalize_agent_get_status($agent_data->machine_name);
    return new self($agent_data->machine_name, $agent_data->label, $agent_data->data, $status, !empty($agent_data->started) ? $agent_data->started : NULL);
  }

  /**
   * Implements PersonalizeExplicitTargetingInterface::explicitTargetingSupportMultiple().
   */
  public static function explicitTargetingSupportMultiple() {
    return PersonalizeExplicitTargetingInterface::EXPLICIT_TARGETING_MULTIPLE_BOTH;
  }

  /**
   * Implements PersonalizeAgentInterface::getType().
   */
  public function getType()
  {
    return 'acquia_lift_target';
  }

  /**
   * Implements PersonalizeAgentInterface::getAssets().
   */
  public function getAssets() {
    return array(
      'js' => array(
        drupal_get_path('module', 'acquia_lift') . '/js/acquia_lift_target.js' => array('type' => 'file', 'scope' => 'footer', 'defer' => TRUE),
      )
    );
  }

  /**
   * Implements PersonalizeAgentInterface::convertContextToFeatureString().
   */
  public static function convertContextToFeatureString($name, $value, $is_mutex = FALSE) {
    $value = self::cleanFeatureValue($value);
    return $name . '::' . $value;
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
