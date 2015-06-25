@lightning @filter
Feature: Lightning Filter
  Ensures that Lighting Filter text filters are created appropriately.

  @api
  Scenario: Ensure text formats exist
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/config/content/formats"
    Then I should see "Filtered HTML"
    And I should see "Full HTML"
    And I should see "User"
    And I should see "Plain Text"

  @api
  Scenario: Ensure Proper Settings Applied to User Format
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/config/content/formats/userbase"
    Then the checkbox "edit-filters-filter-html-status" should be checked
    And the checkbox "edit-filters-filter-html-escape-status" should not be checked
    And the checkbox "edit-filters-filter-autop-status" should be checked
    And the checkbox "edit-filters-filter-url-status" should not be checked
    And the checkbox "edit-filters-filter-htmlcorrector-status" should be checked

  @api
  Scenario: Ensure Proper Settings Applied to Full HTML Format
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/config/content/formats/full_html"
    Then the checkbox "edit-filters-filter-html-status" should not be checked
    And the checkbox "edit-filters-filter-html-escape-status" should not be checked
    And the checkbox "edit-filters-filter-autop-status" should be checked
    And the checkbox "edit-filters-filter-url-status" should be checked
    And the checkbox "edit-filters-filter-htmlcorrector-status" should be checked

  @api
  Scenario: Ensure Proper Settings Applied to Filtered HTML Format
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/config/content/formats/filtered_html"
    Then the checkbox "edit-filters-filter-html-status" should be checked
    And the checkbox "edit-filters-filter-html-escape-status" should not be checked
    And the checkbox "edit-filters-filter-autop-status" should be checked
    And the checkbox "edit-filters-filter-url-status" should be checked
    And the checkbox "edit-filters-filter-htmlcorrector-status" should be checked

  @api
  Scenario: Ensure Proper Settings Applied to Plain Text Format
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/config/content/formats/plain_text"
    Then the checkbox "edit-filters-filter-html-status" should not be checked
    And the checkbox "edit-filters-filter-html-escape-status" should be checked
    And the checkbox "edit-filters-filter-autop-status" should be checked
    And the checkbox "edit-filters-filter-url-status" should be checked
    And the checkbox "edit-filters-filter-htmlcorrector-status" should not be checked
