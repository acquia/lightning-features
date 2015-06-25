<?php

use Drupal\DrupalExtension\Context\RawDrupalContext;
use Drupal\DrupalExtension\Event\EntityEvent;
use Behat\Behat\Context\SnippetAcceptingContext;
use Behat\Behat\Exception\PendingException;
use Behat\Gherkin\Node\PyStringNode;
use Behat\Gherkin\Node\TableNode;

/**
 * Defines application features from the specific context.
 */
class FeatureContext extends RawDrupalContext implements SnippetAcceptingContext {

  /**
   * Initializes context.
   *
   * Every scenario gets its own context instance.
   * You can also pass arbitrary arguments to the
   * context constructor through behat.yml.
   */
  public function __construct() {
  }

  /**
   * @Then /^The xpath "([^"]*)" should exist$/
   */
  public function theXpathShouldExist($arg1) {
    $page = $this->getMink()->getSession()->getPage();

    $element = $page->find('xpath', $arg1);

    if (!count($element)) {
      throw new \Exception("Path {$arg1} not found.");
    }
  }

}
