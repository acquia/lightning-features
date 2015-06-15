@api @insulated
Feature: Access to the Acquia Lift tray controls.
  In order to protect site content
  As a Lift engineer
  I want to ensure that only authorized users have access to the lift tray.

  Scenario: I should not see the Lift controls when I am anonymous.
    Given I am an anonymous user
    And I am on the homepage
    Then the response should not contain "admin/acquia_lift"

  Scenario: I should see the Lift controls when authorized.
    Given I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am on the homepage
    Then I should see the link "Acquia Lift" in the "menu" region

  Scenario: I should not see the Lift controls when unauthorized.
    Given I am logged in as a user with the "access administration pages,access toolbar" permission
    Then I should not see the link "Acquia Lift" in the "menu" region
