Feature: Lightning Forms
  Makes sure that the form feature is present.

  @api
  Scenario: Check for Form Type Present
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/types"
    Then I should see "Webform"

  @api
  Scenario: Make sure I can make an Webform
    Given I am logged in as a user with the "administrator" role
    And I am viewing an "Webform" node:
      | title | Test Webform  |
      | body  | A placeholder |
    When I go to "/admin/content"
    Then I should see "Test Webform"

  @api
  Scenario: Ensure defined fields exist for Webform
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/structure/types/manage/article/webform"
    Then I should see "Title"