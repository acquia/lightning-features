Feature: Lightning Roles

  @api
  Scenario: Ensure proper roles have been built
    Given I am logged in as a user with the "Administrator" role
    And I visit "admin/people/permissions/roles"
    Then I should see "curator"
    Then I should see "reviewer"
    Then I should see "marketer"
