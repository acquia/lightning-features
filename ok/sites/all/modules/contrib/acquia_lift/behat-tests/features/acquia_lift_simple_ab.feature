@api @javascript @insulated @campaign
Feature: Creation of Simple A/B campaigns from unified navigation.
  In order to quickly create A/B tests
  As a site marketer
  I want the ability to create a campaign in context.

  Scenario: Create the simplest A/B campaign from start to finish.
    # I login with the marketer role.
    Given I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am on the homepage
    When I click "Acquia Lift" in the "menu" region
    Then I should visibly see the link "Campaigns" in the "lift_tray" region
    And I should not visibly see the link "Variation sets" in the "lift_tray" region
    And I should not visibly see the link "Goals" in the "lift_tray" region
    And I should not visibly see the link "Reports" in the "lift_tray" region
    And I should not visibly see the link "Status" in the "lift_tray" region

    # I bring up the "Create a campaign" interface.
    When I hover over "Campaigns" in the "lift_tray" region
    Then I should see the link "Add campaign" in the "lift_tray" region
    Then I should see the link "All campaigns" in the "lift_tray" region
    When I click "Add campaign" in the "lift_tray" region
    Then I should see the modal with title "Create a campaign"
    And I should see the link "A/B test" in the "modal_content" region
    And I should see the link "Custom Lift campaign" in the "modal_content" region

    # I create a simple A/B test campaign.
    When I click "A/B test" in the "modal_content" region
    Then I should see the modal with title "Create a campaign"
    And I should see the link "Change type of test" in the "modal_content" region
    When I click "Change type of test" in the "modal_content" region
    Then I should see the modal with title "Create a campaign"
    And I should see the link "A/B test" in the "modal_content" region
    And I should see the link "Custom Lift campaign" in the "modal_content" region
    When I click "A/B test" in the "modal_content" region
    Then I should see the modal with title "Create a campaign"
    And I should see the link "Change type of test" in the "modal_content" region
    When I fill in "My test campaign" for "edit-agent-basic-info-title"
    And I press the "Create campaign" button

    # I verify my campaign is created.
    Then I should see the message "Click the element you want to change in Variation #1" in the messagebox
    When I should see the text "Campaign: My test campaign" in the "lift_tray_campaign_header" region
    And I should visibly see the link "Variations" in the "lift_tray" region
    And I should see "0" for the "variation" count
    And I should visibly see the link "Goals" in the "lift_tray" region
    And I should see "0" for the "goal" count
    And menu item "Reports" should be "inactive"
    And menu item "Start campaign" should be "inactive"
    And I should not see the modal
    And the variation edit mode is "active"

    # I create a new variation.
    When I click "#site-name a span" element in the "page_content" region
    Then I should see the text "<SPAN>" in the "dialog_variation_type" region
    When I click "Edit text" in the "dialog_variation_type" region
    And I wait for AJAX to finish
    Then I should not see a "#acquia-lift-modal-variation-type-select-dialog" element
    And I should see the text "Edit text: <SPAN>" in the "dialog_variation_type_form" region
    And the "personalize_elements_content" field should contain the site title
    When I fill in "Lift Testing" for "personalize_elements_content"
    And I press the "Save" button

    # I verify my variation is created.
    Then I should see the message "The variation has been created. Add one or more goals by clicking Goals > Add goal." in the messagebox
    Then I should not see the variation type form dialog
    And I should see the text "Lift Testing"
    And the variation edit mode is "inactive"

    # I bring up the "Add goal" interface.
    When I hover over "Goals" in the "lift_tray" region
    Then I should see the link "Add goal" in the "lift_tray" region
    Then I should see the link "All goals" in the "lift_tray" region
    When I click "Add goal" in the "lift_tray" region
    Then I should see the modal with title "Add a goal"
    And I should see the link "Predefined goal" in the "modal_content" region
    And I should see the link "New element goal" in the "modal_content" region
    And I should see the link "New page goal" in the "modal_content" region

    # I go to "Predefined goal" sub-menu then return.
    When I click "Predefined goal" in the "modal_content" region
    Then I should see the modal with title "Add a goal"
    And I should see the link "Change type of goal" in the "modal_content" region
    When I click "Change type of goal" in the "modal_content" region

    # I verify I am back to the "Add goal" interface.
    Then I should see the modal with title "Add a goal"
    And I should see the link "Predefined goal" in the "modal_content" region
    And I should see the link "New element goal" in the "modal_content" region
    And I should see the link "New page goal" in the "modal_content" region

    # I add a new page goal.
    When I click "New page goal" in the "modal_content" region
    Then I should see the modal with title "Add a goal"
    And I should see the link "Change type of goal" in the "modal_content" region
    When I fill in "My test goal" for "edit-title"
    And I press "Add goal"

    # I verify my page goal is added.
    Then I should see the message "My test goal goal added to campaign." in the messagebox
    Then I should see "1" for the "goal" count
    When I hover over "Goals" in the "lift_tray" region
    Then I should see the text "My test goal" in the "lift_tray" region

    # I verify my campaign's status.
    When I wait for Lift to synchronize
    Then menu item "Reports" should be "inactive"
    And menu item "Start campaign" should be "active"

  Scenario: Create several A/B test campaigns for later use.
    # I login with the marketer role.
    Given I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am on the homepage
    When I click "Acquia Lift" in the "menu" region
    And I wait for AJAX to finish
    Then I should see the link "Campaigns" in the "lift_tray" region

    # I bring up the "Create a campaign" interface.
    When I hover over "Campaigns" in the "lift_tray" region
    And I wait for AJAX to finish
    Then I should see the link "Add campaign" in the "lift_tray" region
    When I click "Add campaign" in the "lift_tray" region
    Then I should see the modal with title "Create a campaign"

    # I create my first simple A/B test campaign.
    When I click "A/B test" in the "modal_content" region
    Then I should see the modal with title "Create a campaign"
    When I fill in "Test campaign 2" for "agent_basic_info[title]"
    And I press the "Create campaign" button

    # I verify my first campaign is created.
    Then I should see the message "Click the element you want to change in Variation #1" in the messagebox
    When I wait for AJAX to finish
    Then I should see the text "Campaign: Test campaign 2" in the "lift_tray_campaign_header" region
    And the variation edit mode is "active"

    # I bring up the "Create a campaign" interface, again.
    When I hover over "Campaign: Test campaign 2" in the "lift_tray" region
    And I wait for AJAX to finish
    Then I should see the link "Add campaign" in the "lift_tray" region
    When I click "Add campaign" in the "lift_tray" region
    Then I should see the modal with title "Create a campaign"
    And the variation edit mode is "disabled"

    # I create my second simple A/B test campaign.
    When I click "A/B test" in the "modal_content" region
    Then I should see the modal with title "Create a campaign"
    When I fill in "Test campaign 3" for "agent_basic_info[title]"
    And I press the "Create campaign" button

    # I verify my second campaign is created.
    Then I should see the message "Click the element you want to change in Variation #1" in the messagebox
    Then I should see the text "Campaign: Test campaign 3" in the "lift_tray_campaign_header" region
    And the variation edit mode is "active"
