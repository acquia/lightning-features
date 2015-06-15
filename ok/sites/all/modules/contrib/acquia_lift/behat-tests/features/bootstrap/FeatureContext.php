<?php

use Drupal\DrupalExtension\Context\RawDrupalContext;
use Behat\Behat\Context\SnippetAcceptingContext;
use Behat\Gherkin\Node\TableNode;
use Behat\Behat\Hook\Scope\AfterStepScope;
use Behat\Mink\Driver\Selenium2Driver;
use Behat\Testwork\Hook\Scope\BeforeSuiteScope;
use Behat\Behat\Hook\Scope\BeforeScenarioScope;
use Behat\Behat\Hook\Scope\AfterScenarioScope;

/**
 * Defines application features from the specific context.
 */
class FeatureContext extends RawDrupalContext implements SnippetAcceptingContext {

  /**
   * Stores the context parameters that are passed in for the test suite.
   * Parameters include default values for:
   *   - temp_path: The path to temporary location where files, such as error
   *     screenshots, can be written. Default value: /tmp
   */
  protected $context_parameters = array();

  /**
   * Stores contexts.
   */
  protected $contexts = array();

  /**
   * Stores campaigns at start of scenario for comparison with those at the end.
   */
  protected $campaigns = array();

  /**
   * Stores visitor actions at start of scenario for comparison at end.
   */
  protected $actions = array();

  /**
   * Initializes context.
   *
   * Every scenario gets its own context instance.
   * You can also pass arbitrary arguments to the
   * context constructor through behat.yml.
   */
  public function __construct($context_parameters) {
    $this->setContextParameters($context_parameters);
  }

  /****************************************************
   *        H O O K S
   ***************************************************/
  /**
   * Perform before suite actions:
   * - Stage the environment.
   *
   * @BeforeSuite
   */
  static public function beforeSuite(BeforeSuiteScope $scope) {
    // Make sure unibar can update status.
    variable_set('acquia_lift_unibar_allow_status_change', TRUE);
  }

  /**
   * Perform before scenario actions:
   * - Gather all contexts so they can be reused in the current context.
   *
   * @BeforeScenario
   */
  public function beforeScenario(BeforeScenarioScope $scope) {
    // Gather all contexts.
    $contexts = $scope->getEnvironment()->getContexts();
    foreach ($contexts as $context) {
      $context_class_name = get_class($context);
      $this->contexts[$context_class_name] = $context;
    }
  }

  /**
   * Gets a reference to current campaigns, option sets, goals, etc. for
   * tracking purposes.
   *
   * @BeforeScenario @campaign
   */
  public function beforeScenarioCampaign(BeforeScenarioScope $scope) {
    $this->campaigns = personalize_agent_load_multiple();
    $this->actions = visitor_actions_custom_load_multiple();
  }

  /**
   * Delete any campaigns, option sets, goals, etc. created during the
   * scenario.
   *
   * @AfterScenario @campaign
   */
  public function afterScenarioCampaign(AfterScenarioScope $event) {
    $original_campaigns = $this->campaigns;
    $all_campaigns = personalize_agent_load_multiple(array(), array(), TRUE);
    foreach ($all_campaigns as $name => $campaign) {
      if (!isset($original_campaigns[$name])) {
        $option_sets = personalize_option_set_load_by_agent($name);
        foreach($option_sets as $option_set) {
          personalize_option_set_delete($option_set->osid);
        }
        personalize_agent_delete($name);
      }
    }

    $original_actions = $this->actions;
    $all_actions = visitor_actions_custom_load_multiple();
    foreach ($all_actions as $name => $action) {
      if (!isset($original_actions[$name])) {
        visitor_actions_delete_action($name);
      }
    }
  }

  /**
   * For javascript enabled scenarios, always wait for AJAX before clicking.
   *
   * @BeforeStep
   */
  public function beforeJavascriptStep($event) {
    $text = $event->getStep()->getText();
    if (preg_match('/(follow|press|click|submit)/i', $text)) {
      $this->spinUntilAjaxIsFinished();
    }
  }

  /**
   * For javascript enabled scenarios, always wait for AJAX after clicking.
   *
   * @AfterStep
   */
  public function afterJavascriptStep($event) {
    $text = $event->getStep()->getText();
    if (preg_match('/(follow|press|click|submit)/i', $text)) {
      $this->spinUntilAjaxIsFinished();
    }
  }

  /**
   * Take screenshot when step fails.
   * Works only with Selenium2Driver.
   *
   * @AfterStep
   */
  public function takeScreenshotAfterFailedStep(AfterStepScope $scope) {
    if (!$scope->getTestResult()->isPassed()) {
      $driver = $this->getSession()->getDriver();
      if (!($driver instanceof Selenium2Driver)) {
        //throw new UnsupportedDriverActionException('Taking screenshots is not supported by %s, use Selenium2Driver instead.', $driver);
        return;
      }
      $step = $scope->getStep();
      $step_line = $step->getLine();
      $temp_path = $this->getContextParameter('temp_path');
      $filename = $temp_path . '/stepAtLine' . $step_line . '.png';
      $screenshot = $driver->getWebDriverSession()->screenshot();
      file_put_contents($filename, base64_decode($screenshot));
      echo "Saved Screenshot To $filename \n";
      $filename = $temp_path . '/stepAtLine' . $step_line .'.html';
      $source = $driver->getWebDriverSession()->source();
      file_put_contents($filename, $source);
      echo "Saved Source To $filename\n";
    }
  }

  /****************************************************
   *        G I V E N S
   ***************************************************/

  /**
   * @Given /^"(?P<type>[^"]*)" agents:$/
   */
  public function createAgents($type, TableNode $agentsTable) {
    foreach ($agentsTable->getHash() as $agentHash) {
      $agent = (object) $agentHash;
      $agent->plugin = $type;
      $data = array();
      if (!empty($agentHash['url_contexts'])) {
        $data['visitor_context'] = array(
          'querystring_context' => array()
        );
        $contexts = explode(',', $agentHash['url_contexts']);
        foreach ($contexts as $context) {
          $data['visitor_context']['querystring_context'][$context] = $context;
        }
      }
      $agent->data = $data;
      $saved = personalize_agent_save($agent);
      personalize_agent_set_status($saved->machine_name, PERSONALIZE_STATUS_RUNNING);
    }
  }

  /**
   * @Given /^personalized elements:$/
   */
  public function createPersonalizedElements(TableNode $elementsTable) {
    foreach ($elementsTable->getHash() as $optionSetHash) {
      $option_set = (object) $optionSetHash;
      $option_set->plugin = 'elements';
      $option_set->data = array(
        'personalize_elements_selector' => $option_set->selector,
        'personalize_elements_type' => $option_set->type,
      );
      $option_set->executor = 'personalizeElements';
      $content_options = explode(',', $option_set->content);
      $options = array();
      $context_values = array();
      // Grab explicit targeting values if specified.
      if (!empty($option_set->targeting)) {
        $contexts = variable_get('personalize_url_querystring_contexts', array());
        if (isset($contexts[$option_set->targeting])) {
          foreach ($contexts[$option_set->targeting] as $value) {
            $context_values[] = $option_set->targeting . '::' . $value;
          }
        }
      }
      foreach ($content_options as $index => $content) {
        $content = trim($content);
        $option = array(
          'option_label' => personalize_generate_option_label($index),
          'personalize_elements_content' => $content,
        );
        // Set up fixed targeting if there's an available fixed targeting value.
        if (!empty($context_values)) {
          $option['fixed_targeting'] = array(array_shift($context_values));
        }
        $options[] = $option;
      }
      $options = personalize_ensure_unique_option_ids($options);
      $control_option = array('option_label' => PERSONALIZE_CONTROL_OPTION_LABEL, 'option_id' => PERSONALIZE_CONTROL_OPTION_ID, 'personalize_elements_content' => '');
      array_unshift($options, $control_option);
      $option_set->options = $options;
      personalize_option_set_save($option_set);
      personalize_agent_set_status($option_set->agent, PERSONALIZE_STATUS_RUNNING);
    }
  }


  /****************************************************
   *        A S S E R T I O N S
   ***************************************************/

  /**
   * @Then I should see :count for the :type count
   */
  public function assertMenuCount($count, $type) {
    switch ($type) {
      case 'variation':
      case 'variation set':
        $region_name = 'lift_tray_variation_count';
        break;
      case 'goal':
        $region_name = 'lift_tray_goal_count';
        break;
      default:
        throw new \Exception(sprintf('The count type %s is not supported.', $type));
    }
    $regions = $this->getRegions($region_name);
    foreach ($regions as $current) {
      if ($current->isVisible()) {
        $region = $current;
        break;
      }
    }
    if (empty($region)) {
      throw new \Exception(sprintf('There is no visible goal region'));
    }
    $actual_count = $region->getText();
    if ($actual_count !== $count) {
      throw new \Exception(sprintf('The count for type %s was %s rather than the expected %s.', $type, $actual_count, $count));
    }
  }

  /**
   * @When I hover over :link in the :region( region)
   *
   * @throws \Exception
   *   If region or link within it cannot be found.
   */
  public function assertRegionLinkHover($link, $region) {
    $linkObj = $this->findLinkInRegion($link, $region);
    if (empty($linkObj)) {
      throw new \Exception(sprintf('The link "%s" was not found in the region "%s" on the page %s', $link, $region, $this->getSession()->getCurrentUrl()));
    }
    $linkObj->mouseOver();
  }

  /**
   * @When I hover over :id id in the :region( region)
   *
   * @throws \Exception
   *   If region or element within it cannot be found.
   */
  public function assertRegionElementHover($id, $region) {
    $element = $this->findElementInRegion($id, $region);
    if (empty($element)) {
      throw new \Exception(sprintf('The element "%s" was not found in the region "%s" on the page %s', $id, $region, $this->getSession()->getCurrentUrl()));
    }
    $element->mouseOver();
  }

  /**
   * @When I click :selector element in the :region region
   *
   * @throws \Exception
   *   If region or element within it cannot be found.
   */
  public function assertRegionElementClick($selector, $region) {
    $element = $this->findElementInRegion($selector, $region);
    if (empty($element)) {
      throw new \Exception(sprintf('The element "%s" was not found in the region "%s" on the page %s', $selector, $region, $this->getSession()->getCurrentUrl()));
    }
    $element->click();
  }

  /**
   * @Then :variation_set set :variation variation should have the :link link
   *
   * @throws \Exception
   *   If the menu or link cannot be found.
   */
  public function assertRegionVariationHasLink($variation_set, $variation, $link) {
    $css = $this->getVariationLinkCss($variation_set, $variation, $link);
    // Now find the link and return it.
    $element = $this->findElementInRegion($css, 'lift_tray');
    if (empty($element)) {
      throw new \Exception(sprintf('Cannot load the link "%s" for set "%s" and variation "%s" on page %s using selector %s.', $link, $variation_set, $variation, $this->getSession()->getCurrentUrl(), $css));
    }
    return $element;
  }

  /**
   * @Then :variation_set set :variation variation should not have the :link link
   *
   * @throws \Exception
   *   If the menu cannot be found or the link can be found.
   */
  public function assertNotRegionVariationHasLink($variation_set, $variation, $link) {
    $css = $this->getVariationLinkCss($variation_set, $variation, $link);
    // Now find the link and return it.
    $element = $this->findElementInRegion($css, 'lift_tray');
    if (!empty($element)) {
      throw new \Exception(sprintf('Found the link "%s" for set "%s" and variation "%s" on page %s using selector %s.', $link, $variation_set, $variation, $this->getSession()->getCurrentUrl(), $css));
    }
  }

  /**
   * @When I click :link link for the :variation_set set :variation variation
   *
   * @throws \Exception
   *   If the menu or link cannot be found.
   */
  public function assertRegionVariationLinkClick($link, $variation_set, $variation) {
    $element = $this->assertRegionVariationHasLink($variation_set, $variation, $link);
    if (!empty($element)) {
      $element->click();
    }
  }

  /**
   * @Then the variation edit mode is :state
   */
  public function assertVariationEditMode($expected_state) {
    if (!in_array($expected_state, array('active','inactive','hidden','disabled'))) {
      throw new \Exception(sprintf('Invalid expected state for variation toggle: %s', $expected_state));
    }
    $element = $this->findElementInRegion('#acquia-lift-menu-page-variation-toggle', 'lift_tray');
    if (empty($element)) {
      throw new \Exception(sprintf('The variation toggle edit link cannot be found on the page %s', $this->getSession()->getCurrentUrl()));
    }
    $current_state = 'inactive';
    if ($element->hasClass('acquia-lift-page-variation-toggle-hidden')) {
      $current_state = 'hidden';
    }
    else if ($element->hasClass('acquia-lift-page-variation-toggle-active')) {
      $current_state = 'active';
    }
    else if ($element->hasClass('acquia-lift-page-variation-toggle-disabled')) {
      $current_state = 'disabled';
    }
    if ($current_state !== $expected_state) {
      throw new \Exception(sprintf('The variation toggle edit link is currently in the %s state and not the expected %s state.', $current_state, $expected_state));
    }
  }

  /**
   * @Then the :field field should contain the site title
   *
   * @throws \Exception
   *   If the region or element cannot be found or does not have the specified
   *   class.
   */
  public function assertFieldHasSiteTitle($field) {
    // Read the site name dynamically.
    $site_name = variable_get('site_name', "Default site name");
    $mink_context = $this->contexts['Drupal\DrupalExtension\Context\MinkContext'];
    $mink_context->assertFieldContains($field, $site_name);
  }

  /**
   * @Then the :field field should contain text that has :needle
   *
   * @throws \Exception
   *   If the the substring cannot be found in the given field.
   */
  public function assertFieldContains($field, $needle) {
    $node = $this->assertSession()->fieldExists($field);
    $haystack = $node->getValue();
    if (strpos($haystack, $needle) === false) {
      throw new \Exception(sprintf('The field "%s" value is "%s", but we are looking for "%s".', $field, $haystack, $needle));
    }
  }

  /**
   * @Then I should see :selector element in the :region region is :state for editing
   *
   * @throws \Exception
   *   If the region or element cannot be found or is not in a specified state.
   */
  public function assertRegionElementIsInState($selector, $region, $state) {
    $state_class = array(
      'highlighted' => 'acquia-lift-page-variation-item',
      'available' => 'visitor-actions-ui-enabled',
    );
    if (!isset($state_class[$state])) {
      $state_options_array = array_keys($state_class);
      $state_options_string = implode(', ', $state_options_array);
      throw new \Exception(sprintf('The element state "%s" is not defined. Available options are "%s".', $state, $state_options_string));
    }
    $element = $this->findElementInRegion($selector, $region);
    if (empty($element)) {
      throw new \Exception(sprintf('The element "%s" was not found in the region "%s" on the page %s.', $selector, $region, $this->getSession()->getCurrentUrl()));
    }
    $class = $state_class[$state];
    if (!$element->hasClass($class)) {
      throw new \Exception(sprintf('The element "%s" in region "%s" on the page %s is not in "%s" state.', $selector, $region, $this->getSession()->getCurrentUrl(), $state));
    }
  }

  /**
   * @When I wait for Lift to synchronize
   */
  public function waitForLiftSynchronize() {
    $this->spinUntilLiftCampaignsAreSynchronized();
  }

  /**
   * @Then I should visibly see the link :link in the :region( region)
   *
   * @throws \Exception
   *   If region or link within it cannot be found or is hidden.
   */
  public function assertLinkVisibleRegion($link, $region) {
    $result = $this->findLinkInRegion($link, $region);
    if (empty($result) || !$result->isVisible()) {
      throw new \Exception(sprintf('No link to "%s" in the "%s" region on the page %s', $link, $region, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * @Then I should not visibly see the link :link in the :region( region)
   *
   * @throws \Exception
   *   If link is found in region and is visible.
   */
  public function assertNotLinkVisibleRegion($link, $region) {
    $result = $this->findLinkInRegion($link, $region);
    if (!empty($result) && $result->isVisible()) {
      throw new \Exception(sprintf('Link to "%s" in the "%s" region on the page %s', $link, $region, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * @Then /^I should see the modal with title "([^"]*)"$/
   *
   * @throws \Exception
   *   If the modal with the specified title cannot be found on the page or is
   *   invisible.
   */
  public function assertModalWindowWithTitle($title) {
    $region = $this->getRegion('modal_title');
    if (!$region || !$region->isVisible()) {
      throw new \Exception(sprintf('The modal dialog titled %s is not visible on the page %s', $title, $this->getSession()->getCurrentUrl()));
    }
    $regionText = $region->getText();
    if (strpos($regionText, $title) === FALSE) {
      throw new \Exception(sprintf("The title '%s' was not found in the modal title region on the page %s", $title, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * @Then /^I should not see the modal$/
   */
  public function assertNoModalWindow() {
    $this->assertNoRegion('modal_content', 'modal dialog');
  }

  /**
   * @Then /^I should not see the variation type dialog$/
   */
  public function assertNoVariationTypeDialogWindow() {
    $this->assertNoRegion('dialog_variation_type', 'variation type dialog');
  }

  /**
   * @Then /^I should not see the variation type form dialog$/
   */
  public function assertNoVariationTypeFormDialogWindow() {
    $this->assertNoRegion('dialog_variation_type_form', 'variation type form dialog');
  }

  /**
   * @Given /^menu item "([^"]*)" should be "(active|inactive)"$/
   */
  public function assertMenuItemInactive($link, $status) {
    $class = 'acquia-lift-menu-disabled';
    $element = $this->findLinkInRegion($link, 'lift_tray');
    if (empty($element)) {
      throw new \Exception(sprintf('The link element %s was not found on the page %s', $link, $this->getSession()->getCurrentUrl()));
    }
    if ($element->hasClass($class)) {
      if ($status === 'active') {
        throw new \Exception(sprintf('The link element %s on page %s is inactive but should be active.', $link, $this->getSession()->getCurrentUrl()));
      }
    }
    else if ($status === 'inactive') {
      throw new \Exception(sprintf('The link element %s on page %s is active but should be inactive.', $link, $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * @Then I should see element with :id id in :region region with the :class class
   */
  public function assertElementWithIDHasClass($id, $region, $class) {
    $element = $this->findElementInRegion($id, $region);
    if (empty($element)) {
      throw new \Exception(sprintf('The element with %s was not found in region %s on the page %s', $id, $region, $this->getSession()->getCurrentUrl()));
    }
    if (!$element->hasClass($class)) {
      throw new \Exception(sprintf('The element with id %s in region %s on page %s does not have class %s', $id, $region, $this->getSession()->getCurrentUrl(), $class));
    }
  }

  /**
   * @Then I should see the message :text in the messagebox
   */
  public function assertTextInMessagebox($text) {
    $this->spinUntilMessageBoxIsPopulated();

    $script = "return jQuery('#acquia-lift-message-box').find('.message').text();";
    $message = $this->getSession()->evaluateScript($script);
    if (strpos($message, $text) === FALSE) {
      throw new \Exception(sprintf('The message "%s" was not found in the messagebox.', $text));
    }
  }

  /****************************************************
   *        H E L P E R  F U N C T I O N S
   ***************************************************/

  /**
   * Helper function to generate the css for a variation action link.
   *
   * @param string $variation_set
   *   The name of the variation set displayed
   * @param string $variation
   *   The name of the variation displayed
   * @param string $link
   *   The link text to find.  One of "rename", "edit", or "delete".
   *
   * @throws \Exception
   *   If the link type is invalid or the current campaign is not available.
   */
  public function getVariationLinkCss($variation_set, $variation, $link) {
    $link = drupal_strtolower($link);
    if (!in_array($link, array('edit', 'rename', 'delete'))) {
      throw new \Exception(sprintf('The variation action "%s" is invalid.', $link));
    }
    $campaign = $this->getCurrentCampaign();
    if (empty($campaign)) {
      throw new \Exception(sprintf('Cannot determine the current campaign for variation set %s.', $variation_set));
    }
    $agent_instance = personalize_agent_load_agent($campaign);
    if (empty($agent_instance)) {
      throw new \Exception(sprintf('Cannot load the current agent instance for campaign %s.', $campaign));
    }
    $option_sets = personalize_option_set_load_by_agent($campaign);
    if ($agent_instance instanceof AcquiaLiftSimpleAB) {
      // One decision with many variations.
      $option_set = reset($option_sets);
      foreach ($option_set->options as $index => $option) {
        if ($option['option_label'] == $variation) {
          break;
        }
      }
      $css = '.acquia-lift-menu-item[data-acquia-lift-personalize-agent="' . $campaign . '"]';
      switch ($link) {
        case "rename":
          $css .= ' a.acquia-lift-variation-rename';
          break;
        case "delete":
          $css .= ' a.acquia-lift-variation-delete';
          break;
        default:
          throw new \Exception(sprintf('Campaign %s does not support edit links for variations.', $campaign));
      }
      $css .= '[data-acquia-lift-personalize-page-variation="' . $index . '"]';
    }
    else {
      // Standard option set names displayed.
      foreach ($option_sets as $option_set) {
        if ($option_set->label == $variation_set) {
          $osid = $option_set->osid;
          foreach ($option_set->options as $option) {
            if ($option['option_label'] == $variation) {
              $option_id = $option['option_id'];
              break;
            }
          }
          break;
        }
      }
      $css = '.acquia-lift-menu-item[data-acquia-lift-personalize-option-set="' . personalize_stringify_osid($osid) . '"]';
      switch ($link) {
        case "edit":
          $css .= ' a.acquia-lift-variation-edit';
          break;
        case "rename":
          $css .= ' a.acquia-lift-variation-rename';
          break;
        case "delete":
          $css .= ' a.acquia-lift-variation-delete';
          break;
      }
      $css .= '[data-acquia-lift-personalize-option-set-option="' . $option_id . '"]';
    }
    return $css;
  }

  /**
   * Helper function to retrieve a context parameter.
   *
   * @param $param_name
   *   The name of the parameter to retrieve
   * @return
   *   The parameter value or NULL if undefined.
   */
  public function getContextParameter($param_name) {
    return !empty($this->context_parameters[$param_name]) ? $this->context_parameters[$param_name] : NULL;
  }

  /**
   * Helper function to set the context parameters.
   *
   * @param array $parameters
   *   The parameters to set.
   */
  public function setContextParameters($parameters) {
    $this->context_parameters = array_merge($this->context_parameters, $parameters);
  }

  /**
   * Helper function to return a link in a particular region.
   *
   * Note: the selector is translated to xpath in order to allow selection of
   * the link even if it needs to be scrolled in order to visible.
   *
   * @param string $link
   *   link id, title, text or image alt
   * @param $region
   *   region identifier from configuration.
   *
   * @return \Behat\Mink\Element\NodeElement|null
   *   The element node for the link or null if not found.
   */
  private function findLinkInRegion($link, $region) {
    $regionObj = $this->getRegion($region);
    $element = $regionObj->findLink($link);

    if (empty($element)) {
      throw new \Exception(sprintf('Could not find element in "%s" using link "%s"', $region, $link));
    }
    return $element;
  }

  /**
   * Helper function to return an element in a particular region.
   *
   * @param string $selector
   *   the css selector for an element
   * @param $region
   *   region identifier from configuration.
   *
   * @return \Behat\Mink\Element\NodeElement|null
   *   The element node for the link or null if not found.
   */
  private function findElementInRegion($selector, $region) {
    $regionObj = $this->getRegion($region);
    return $regionObj->find('css', $selector);
  }

  /**
   * Thin wrapper over Drupal MinkContext's getRegion function.
   *
   * @param $region
   *   The region identifier to load.
   *
   * @return \Behat\Mink\Element\NodeElement|null
   *   The region element node or null if not found.
   *
   * @throws \Exception
   *   If the region cannot be found on the current page.
   */
  private function getRegion($region) {
    return $this->contexts['Drupal\DrupalExtension\Context\MinkContext']->getRegion($region);
  }

  /**
   * Helper function to retrieve a region defined in the configuration file that
   * may consist of multiple elements matching the selector.
   *
   * @param $region
   *   The region identifier to load.
   *
   * @return \Behat\Mink\Element\NodeElement|null
   *   The region element node or null if not found.
   *
   * @throws \Exception
   *   If the region cannot be found on the current page.
   */
  private function getRegions($region) {
    $mink = $this->getMink();
    $regions = $mink->getSession()->getPage()->findAll('region', $region);
    if (empty($regions)) {
      throw new \Exception(sprintf('The region %s was not found on the page %s', $region, $this->getSession()->getCurrentUrl()));
    }
    return $regions;
  }

  /**
   * Helper function to assert that a particular region is not visible.
   *
   * @param $region_id
   *   The id for the region defined in the behat.yml configuration file.
   * @param $region_name
   *   A human readable name for the region
   * @throws Exception
   *   If the region is visible on the page.
   */
  public function assertNoRegion($region_id, $region_name) {
    try {
      $region = $this->getRegion($region_id);
    } catch (\Exception $e) {
      // If the region was not found that is good.
      return;
    }
    if ($region && $region->isVisible()) {
      throw new \Exception(sprintf('The %s was found on the page %s', strtolower($region_name), $this->getSession()->getCurrentUrl()));
    }
  }

  /**
   * Helper function to retrieve the currently active campaign from the client
   * Javascript.
   *
   * @return string
   *   The machine name for the currently active campaign or empty string.
   */
  private function getCurrentCampaign() {
    $script = 'return Drupal.settings.personalize.activeCampaign;';
    return $this->getMink()->getSession()->evaluateScript($script);
  }

  /****************************************************
   *        S P I N  F U N C T I O N S
   ***************************************************/
  /**
   * Keep retrying assertion for a defined number of iterations.
   *
   * @param closure $lambda           Callback.
   * @param integer $attemptThreshold Number of attempts to execute the command.
   *
   * @throws \Exception If attemptThreshold is met.
   *
   * @return mixed
   */
  private function spin($lambda, $attemptThreshold = 15) {
    for ($iteration = 0; $iteration <= $attemptThreshold; $iteration++) {
      try {
        if (call_user_func($lambda)) {
          return;
        }
      } catch (\Exception $exception) {
        // do nothing
      }

      sleep(1);
    }
  }

  /**
   * Spin JavaScript evaluation.
   *
   * @param string  $assertionScript  Assertion script
   * @param integer $attemptThreshold Number of attempts to execute the command.
   */
  private function spinJavaScriptEvaluation($assertionScript, $attemptThreshold = 15) {
    $this->spin(function () use ($assertionScript) {
      return $this->getMink()->getSession()->evaluateScript('return ' . $assertionScript);
    }, $attemptThreshold);
  }

  /**
   * Spin until the Ajax is finished.
   */
  private function spinUntilAjaxIsFinished() {
    $assertionScript = '(typeof(jQuery)=="undefined" || (0 === jQuery.active && 0 === jQuery(\':animated\').length));';
    $this->spinJavaScriptEvaluation($assertionScript);
  }

  /**
   * Spin until the message box is populated.
   */
  private function spinUntilMessageBoxIsPopulated() {
    $assertionScript = "(jQuery('#acquia-lift-message-box').length > 0 && jQuery('#acquia-lift-message-box').hasClass('acquia-lift-messagebox-shown'));";
    $this->spinJavaScriptEvaluation($assertionScript);
  }

  /**
   * Spin until the Lift Campaigns are synchronized.
   */
  private function spinUntilLiftCampaignsAreSynchronized() {
    $assertionScript = '(typeof(jQuery)=="undefined" || (0 === jQuery.active && 0 === Drupal.acquiaLift.queueCount));';
    $this->spinJavaScriptEvaluation($assertionScript);
  }
}
