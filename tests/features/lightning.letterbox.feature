@lightning @letterbox
Feature: Lightning Letterbox
  Ensures that Lighting Letterbox feature is configured properly.

  @api
  Scenario: Ensure letterbox breakpoints exist
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/config/media/breakpoints/groups/letterbox"
    Then I should see "Export Letterbox"
