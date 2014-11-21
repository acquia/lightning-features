Feature: Lightning Article
  Makes sure that the article feature is present.

  @api
  Scenario: Check for Article Type Present
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/types"
    Then I should see "Article"

  @api
  Scenario: Make sure I can make an Article
    Given I am logged in as a user with the "administrator" role
    And I am viewing an "Article" node:
      | title | My article with fields! |
      | body  | A placeholder           |
    When I go to "/admin/content"
    Then I should see "My article with fields!"

  @api
  Scenario: Ensure defined fields exist
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/structure/types/manage/article/fields"
    Then I should see "Title"
    And I should see "Body"
    And I should see "Tags"
    And I should see "Image"