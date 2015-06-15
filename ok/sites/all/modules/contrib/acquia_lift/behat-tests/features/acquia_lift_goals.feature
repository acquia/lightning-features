@api @javascript @insulated @campaign
Feature: Goals can be edited and managed for an Acquia Lift campaign from toolbar.
  In order to manage goals in context
  As a site marketer
  I want the ability to add, edit, and delete existing goals from the Lift toolbar.

  Scenario: Add page level goals to a campaign
    # I have a campaign.
    # I login with the marketer role.
    Given "acquia_lift" agents:
      | machine_name                    | label                          |
      | testing-campaign-add-goals-page | Testing campaign add page goal |
    And I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am on the homepage
    When I click "Acquia Lift" in the "menu" region
    And I wait for AJAX to finish
    Then I should see the link "Campaigns" in the "lift_tray" region

    # I open the goal's menu.
    When I hover over "Campaigns" in the "lift_tray" region
    And I click "Testing campaign add page goal" in the "lift_tray" region
    Then I should visibly see the link "Goals" in the "lift_tray" region
    And I should see "0" for the "goal" count

    # I bring up the "Add goal" interface.
    When I hover over "Goals" in the "lift_tray" region
    And I should visibly see the link "Add goal" in the "lift_tray" region
    When I click "Add goal" in the "lift_tray" region
    Then I should see the modal with title "Add a goal"
    And I should see the link "New page goal" in the "modal_content" region

    # I add a new page goal.
    When I click "New page goal" in the "modal_content" region
    And I wait for AJAX to finish
    When I select "scrolls to the bottom of" from "Event"
    Then I should see the text "Offset from bottom" in the "modal_content" region
    And I should not see the text "Time in seconds" in the "modal_content" region
    When I select "stays for longer than the specified time on" from "Event"
    Then I should see the text "Time in seconds" in the "modal_content" region
    And I should not see the text "Offset from bottom" in the "modal_content" region
    When I select "views" from "Event"
    Then I should not see the text "Time in seconds" in the "modal_content" region
    And I should not see the text "Offset from bottom" in the "modal_content" region
    When I fill in "Test goal #1" for "Title"
    And I press "Add goal"

    # I verify my page goal is added.
    Then I should see the message "Test goal #1 goal added to campaign" in the messagebox
    And I should not see the modal
    Then I should see "1" for the "goal" count
    When I hover over "Goals" in the "lift_tray" region
    Then I should see the text "Test goal #1" in the "lift_tray" region

  Scenario: Add pre-existing goals to a campaign
    # I have a campaign.
    # I login with the marketer role.
    Given "acquia_lift" agents:
      | machine_name                       | label                              |
      | testing-campaign-add-existing-goal | Testing campaign add existing goal |
    And I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am on the homepage
    When I click "Acquia Lift" in the "menu" region
    And I wait for AJAX to finish
    Then I should see the link "Campaigns" in the "lift_tray" region

    # I open the goal's menu.
    When I hover over "Campaigns" in the "lift_tray" region
    And I click "Testing campaign add existing goal" in the "lift_tray" region
    Then I should visibly see the link "Goals" in the "lift_tray" region
    And I should see "0" for the "goal" count

    # I bring up the "Add goal" interface, again.
    When I hover over "Goals" in the "lift_tray" region
    And I should visibly see the link "Add goal" in the "lift_tray" region
    When I click "Add goal" in the "lift_tray" region
    Then I should see the modal with title "Add a goal"
    And I should see the link "Predefined goal" in the "modal_content" region

    # I register a predefined goal to this campaign.
    When I click "Predefined goal" in the "modal_content" region
    And I wait for AJAX to finish
    When I select "Registers" from "Goal"
    And I press "Add goal"

    # I verify my predefined goal is registered with this campaign.
    Then I should see the message "Registers goal added to campaign" in the messagebox
    And I should not see the modal
    Then I should see "1" for the "goal" count
    When I hover over "Goals" in the "lift_tray" region
    Then I should see the text "Registers" in the "lift_tray" region

  Scenario: Add an element goals to a campaign
    # I have a campaign.
    # I login with the marketer role.
    Given "acquia_lift" agents:
      | machine_name                      | label                             |
      | testing-campaign-add-element-goal | Testing campaign add element goal |
    And I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am on the homepage
    When I click "Acquia Lift" in the "menu" region
    And I wait for AJAX to finish
    Then I should see the link "Campaigns" in the "lift_tray" region

    # I open and see the goal's menu.
    When I hover over "Campaigns" in the "lift_tray" region
    And I click "Testing campaign add element goal" in the "lift_tray" region
    Then I should visibly see the link "Goals" in the "lift_tray" region
    And I should see "0" for the "goal" count

    # I bring up the "Add goal" interface.
    When I hover over "Goals" in the "lift_tray" region
    And I should visibly see the link "Add goal" in the "lift_tray" region
    When I click "Add goal" in the "lift_tray" region
    Then I should see the modal with title "Add a goal"
    And I should see the link "New element goal" in the "modal_content" region

    # I appoint the "logo" element as the goal element,
    # and specify "hovers over" option to be the goal.
    When I click "New element goal" in the "modal_content" region
    Then I should not see the modal
    Then I should see "#logo" element in the "page_content" region is "available" for editing
    When I click "logo" in the "page_content" region
    And I wait for AJAX to finish
    Then I should see "Title" in the "dialog_goal_form" region
    And I should see "Event" in the "dialog_goal_form" region
    And I should see the link "Advanced Options" in the "dialog_goal_form" region
    When I fill in "New goal #2" for "Title"
    And I select "hovers over" from "Event"
    And I press "Save" in the "dialog_goal_form" region
    And I wait for AJAX to finish

    # I verify my new element goal is added.
    Then I should see the message "The action New goal #2 was saved."
    Then I should see "1" for the "goal" count
    When I hover over "Goals" in the "lift_tray" region
    Then I should see the text "New goal #2" in the "lift_tray" region
