@lightning @wysiwyg
Feature: Lightning WYSIWYG
  Ensure WYSIWYG settings are set.

  @api
  Scenario: Check for Lightning Node Linkit Type Present
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/config/content/linkit"
    Then I should see "lightning Nodes"

  @api
  Scenario: Lightning Node Linkit Configuration sanity check
    Given I am logged in as a user with the "administrator" role
    When I visit "admin/config/content/linkit/list/lightning_wysiwyg_nodes/edit"
    Then the checkbox "edit-data-text-formats-filtered-html" should be checked
    And the checkbox "edit-data-search-plugins-entitynode-enabled" should be checked
    And the checkbox "edit-data-text-formats-full-html" should not be checked
