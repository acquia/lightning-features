<?php
/**
 * @file
 * Provides an test agent type for testing auto-personalization with fixed
 * targeting options provided by those selected for campaign context.
 */

class PersonalizeTestAutopersonalizeAgent extends PersonalizeAgentBase implements PersonalizeExplicitTargetingInterface, PersonalizeAutoTargetingInterface {

  public $goals_received = array();

  /**
   * Implements PersonalizeAgentInterface::create().
   */
  public static function create($agent_data) {
    $status = personalize_agent_get_status($agent_data->machine_name);
    return new self($agent_data->machine_name, $agent_data->label, $agent_data->data, $status, !empty($agent_data->started) ? $agent_data->started : NULL);
  }

  /**
   * Implements PersonalizeAgentInterface::getType().
   */
  public function getType()
  {
    return 'test_autopersonalize_agent';
  }

  /**
   * Implements PersonalizeExplicitTargetingInterface::explicitTargetingSupportMultiple().
   */
  public static function explicitTargetingSupportMultiple() {
    return PersonalizeExplicitTargetingInterface::EXPLICIT_TARGETING_MULTIPLE_BOTH;
  }

  /**
   * Implements PersonalizeAutoTargetingInterface::constrainExplicitTargetingContexts
   */
  public static function constrainExplicitTargetingContexts() {
    return TRUE;
  }
}
