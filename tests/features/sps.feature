@lightning @sps
Feature: Lightning SPS
  Makes sure that the SPS feature is properly configured .

  @api
  Scenario: Check for Collection field on Article
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/types/manage/article/fields"
    Then I should see "field_collection"
