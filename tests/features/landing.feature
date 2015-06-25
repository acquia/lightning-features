@lightning @landing
Feature: Lightning Landing
  Makes sure that the landing feature was created correctly.

  @api
  Scenario: Check for Identifiers taxonomy
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/types"
    Then I should see "Landing Page"

  @api
  Scenario: Make sure I can make a Landing page
    Given I am logged in as a user with the "administrator" role
    And "landing_page" content:
      | title                   | body          |
      | This is a landing page  | A placeholder |
    When I go to "/admin/content"
    Then I should see "This is a landing page"

  @api
  Scenario: Ensure defined fields exist
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/structure/types/manage/landing/fields"
    Then I should see "Title"
    And I should see "group_landing_campaign"
    And I should see "field_landing_identifiers"
    And I should see "field_landing_tags"
    And I should see "metatags"
