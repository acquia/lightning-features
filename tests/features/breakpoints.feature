@lightning @breakpoints
Feature: Lightning Breakpoints
  Ensures that Lighting Breakpoints are created properly.

  @api
  Scenario: Ensure breakpoints exist
    Given I am logged in as a user with the "administrator" role
    When I go to "/admin/config/media/breakpoints"
    Then The xpath "//input[@value='tablet']" should exist
    And The xpath "//input[@value='mobile']" should exist
    And The xpath "//input[@value='narrow']" should exist
    And The xpath "//input[@value='wide']" should exist
