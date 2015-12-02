@lightning @moderation
Feature: Lightning Moderation
  Makes sure that the moderation feature is configured.

  @api
  Scenario: Check for Moderation States
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/config/workbench/moderation"
    Then I should see "draft"
    And I should see "needs_review"
    And I should see "published"
