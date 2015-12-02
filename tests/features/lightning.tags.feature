@lightning @tags
Feature: Lightning Tags

  @api
  Scenario: Ensure proper vocabularies have been built
    Given I am logged in as a user with the "Administrator" role
    And I visit "admin/structure/taxonomy"
    Then I should see "Tags"
