<?php

use Drupal\DrupalExtension\Context\DrupalContext,
  Drupal\DrupalExtension\Event\EntityEvent;

use Behat\Behat\Exception\PendingException;

use Behat\Gherkin\Node\PyStringNode,
  Behat\Gherkin\Node\TableNode;

class FeatureContext extends DrupalContext {
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
