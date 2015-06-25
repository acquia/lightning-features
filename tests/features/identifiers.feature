@lightning @identifiers
Feature: Lightning Identifiers
  Makes sure that the identifiers taxonomy is present.

  @api
  Scenario: Check for Identifiers taxonomy
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/taxonomy"
    Then I should see "Identifiers"
