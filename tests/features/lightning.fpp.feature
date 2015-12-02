@lightning @fpp
Feature: Lightning Fieldable Panel Panes
  Makes sure that the FPP feature is present and configured correctly.

  @api
  Scenario: Check for FPP Types
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/fieldable-panels-panes"
    Then I should see "fieldable_panels_pane"
    And I should see "content"
    And I should see "raw_html"
    And I should see "quick_links"
    And I should see "media_pane"
    And I should see "map"
    And I should see "text"

  @api
  Scenario: Check for HTML Fields
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/fieldable-panels-panes/manage/raw-html/fields"
    Then I should see "field_raw_html"

  @api
  Scenario: Check for Links Fields
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/fieldable-panels-panes/manage/quick-links/fields"
    Then I should see "field_quick_links_links"

  @api
  Scenario: Check for Media Fields
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/fieldable-panels-panes/manage/media-pane/fields"
    Then I should see "field_add_media"

  @api
  Scenario: Check for Map Fields
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/fieldable-panels-panes/manage/map/fields"
    Then I should see "field_map_address"
    And I should see "field_map_information"

  @api
  Scenario: Check for Text Fields
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/fieldable-panels-panes/manage/map/fields"
    Then I should see "field_basic_text_text"
