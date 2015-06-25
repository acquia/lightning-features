@lightning @page
Feature: Lightning Page
  Makes sure that the Page feature is present and configured.

  @api
  Scenario: Check Basic Page Content Type Present
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/types"
    Then I should see "Basic Page"

  @api
  Scenario: Check for Node Page body field instance present.
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/types/manage/page/fields"
    Then I should see "Title"
    And I should see "Body"

  @api
  Scenario: Make sure I can make a Page
    Given I am logged in as a user with the "administrator" role
    And "page" content:
      | title           | body                       |
      | Test Basic Page | A place for the body to go |
    When I go to "/admin/content"
    And I follow "Test Basic Page"
    Then I should see "A place for the body to go"
