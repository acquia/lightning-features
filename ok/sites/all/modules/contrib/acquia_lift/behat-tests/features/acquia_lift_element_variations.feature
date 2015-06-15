@api @javascript @insulated @campaign
Feature: Personalize elements variations can be edited for an existing campaign.
  In order to manage element variations in context
  As a site marketer
  I want the ability to add, edit, and delete existing personalize element variations in context.

  Scenario: Add personalize elements to a campaign
    # I have a campaign.
    # I login with the marketer role.
    # I am on an article page.
    Given "acquia_lift" agents:
      | machine_name                    | label                           |
      | testing-campaign-add-variations | Testing campaign add variations |
    And I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am viewing an "Article" content:
      | title | Test Article Title - Original |
    When I click "Acquia Lift" in the "menu" region
    And I wait for AJAX to finish
    Then I should see the link "Campaigns" in the "lift_tray" region

    # I open the variation set's menu.
    When I hover over "Campaigns" in the "lift_tray" region
    And I click "Testing campaign add variations" in the "lift_tray" region
    Then I should visibly see the link "Variation Sets" in the "lift_tray" region
    And I should see "0" for the "variation set" count

    # I bring up the "Add variation set" interface.
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should see the link "Add variation set" in the "lift_tray" region
    Then I should see the link "All variation sets" in the "lift_tray" region
    When I click "Add variation set"
    Then I should see the modal with title "Add a variation set"
    And I should see the link "Webpage elements" in the "modal_content" region
    And I should see the link "Drupal blocks" in the "modal_content" region
    When I click "Webpage elements" in the "modal_content" region
    Then I should not see the modal

    # I add a new variation set.
    When I click "#page-title" element in the "page_content" region
    Then I should see the text "<H1>" in the "dialog_variation_type" region
    When I click "Edit text" in the "dialog_variation_type" region
    Then I should not see the variation type dialog
    And I should see the text "Edit text: <H1>" in the "dialog_variation_type_form" region
    And the "personalize_elements_content" field should contain text that has "Test Article Title - Original"
    When I fill in "Test Article Title - Updated 1" for "personalize_elements_content"
    And I fill in "Test variation set" for "title"
    # There are two "Save" buttons on the page and we are clicking one by element id.
    And I click "#edit-variation-type-submit-form" element in the "dialog_variation_type_form" region

    # I verify my variation set is created.
    Then I should see the message "The variation set has been created." in the messagebox
    Then I should not see the variation type form dialog
    And I should see the text "Test Article Title" in the "page_content" region
    And I should see "1" for the "variation set" count

    # I bring up the "Add variation" interface.
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should see the text "Test variation set" in the "lift_tray" region
    And I should see the link "Control variation" in the "lift_tray" region
    And I should see the link "Variation #1" in the "lift_tray" region
    And I should see the link "Add variation" in the "lift_tray" region
    And I should see the link "Add variation set" in the "lift_tray" region

    # I add a new variation to to the existing variation set.
    When I click "Add variation" in the "lift_tray" region
    Then I should see "#page-title" element in the "page_content" region is "highlighted" for editing
    And I should see the text "Edit text: <H1>" in the "dialog_variation_type_form" region
    And the "personalize_elements_content" field should contain "Test Article Title - Updated 1"
    And I should not see the link "Edit selector" in the "dialog_variation_type_form" region
    When I fill in "Test Article Title - Updated 2" for "personalize_elements_content"
    # There are two "Save" buttons on the page and we are clicking one by element id.
    And I click "#edit-variation-type-submit-form" element in the "dialog_variation_type_form" region

    # I verify my new variation is add.
    Then I should see the message "The variation has been created." in the messagebox
    Then I should not see the variation type form dialog
    And I should see the text "Test Article Title - Updated 2" in the "page_content" region
    And I should see "1" for the "variation set" count
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should see the link "Variation #1" in the "lift_tray" region
    Then I should see the link "Variation #2" in the "lift_tray" region

  Scenario: Edit existing personalize elements for an acquia_lift campaign.
    # I have a campaign and a variation set.
    # I login with the marketer role.
    # I am on an article page.
    Given "acquia_lift" agents:
      | machine_name                     | label                            |
      | testing-campaign-edit-variations | Testing campaign edit variations |
    And personalized elements:
      | label              | agent                            | selector    | type     | content                |
      | Page title updated | testing-campaign-edit-variations | #page-title | editText | The Rainbow Connection |
    And I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am viewing an "Article" content:
      | title | Test Article Title - Original |
    When I click "Acquia Lift" in the "menu" region
    And I wait for AJAX to finish
    Then I should see the link "Campaigns" in the "lift_tray" region

    # I open the variation set's menu.
    When I hover over "Campaigns" in the "lift_tray" region
    And I click "Testing campaign edit variations" in the "lift_tray" region
    Then I should visibly see the link "Variation Sets" in the "lift_tray" region
    And I should see "1" for the "variation set" count

    # I bring up the "Edit" variation set interface.
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should see the text "Page title updated" in the "lift_tray" region
    And I should visibly see the link "Option A" in the "lift_tray" region
    And "Page title updated" set "Control variation" variation should not have the "Edit" link
    And "Page title updated" set "Control variation" variation should not have the "Delete" link
    And "Page title updated" set "Option A" variation should have the "Edit" link
    And "Page title updated" set "Option A" variation should have the "Delete" link

    # I edit the variation set.
    When I click "Edit" link for the "Page title updated" set "Option A" variation
    Then I should see the text "Edit text: <H1>" in the "dialog_variation_type_form" region
    And the "personalize_elements_content" field should contain "The Rainbow Connection"
    And the "option_label" field should contain "Option A"
    When I fill in "Variation 1" for "option_label"
    And I fill in "Moving Right Along" for "personalize_elements_content"
    # There are two "Save" buttons on the page and we are clicking one by element id.
    And I click "#edit-variation-type-submit-form" element in the "dialog_variation_type_form" region

    # I verify my variation is updated.
    Then I should see the message "The variation has been updated." in the messagebox
    Then I should not see the variation type form dialog
    And I should see the text "Moving Right Along" in the "page_content" region
    And I should see "1" for the "variation set" count
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should see the text "Page title updated" in the "lift_tray" region
    And I should visibly see the link "Variation 1" in the "lift_tray" region

  Scenario: Delete an existing personalize element variation for an acquia_lift campaign.
    # I have a campaign and a variation set.
    # I login with the marketer role.
    # I am on an article page.
    Given "acquia_lift" agents:
      | machine_name                       | label                              |
      | testing-campaign-delete-variations | Testing campaign delete variations |
    And personalized elements:
      | label              | agent                              | selector    | type     | content                                    |
      | Page title updated | testing-campaign-delete-variations | #page-title | editText | The Rainbow Connection, Moving Right Along |
    And I am logged in as a user with the "access administration pages,access toolbar,administer visitor actions,manage personalized content" permission
    And I am viewing an "Article" content:
      | title | Test Article Title - Original |
    When I click "Acquia Lift" in the "menu" region
    And I wait for AJAX to finish
    Then I should see the link "Campaigns" in the "lift_tray" region

    # I open the variation set's menu.
    When I hover over "Campaigns" in the "lift_tray" region
    And I click "Testing campaign delete variations" in the "lift_tray" region
    Then I should visibly see the link "Variation Sets" in the "lift_tray" region
    And I should see "1" for the "variation set" count

    # I bring up the "Delete" variation set interface.
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should see the text "Page title updated" in the "lift_tray" region
    And I should visibly see the link "Option A" in the "lift_tray" region
    And I should visibly see the link "Option B" in the "lift_tray" region
    And "Page title updated" set "Control variation" variation should not have the "Edit" link
    And "Page title updated" set "Control variation" variation should not have the "Delete" link
    And "Page title updated" set "Option A" variation should have the "Edit" link
    And "Page title updated" set "Option A" variation should have the "Delete" link
    And "Page title updated" set "Option B" variation should have the "Edit" link
    And "Page title updated" set "Option B" variation should have the "Delete" link

    # I delete a variation, "Option A".
    When I click "Delete" link for the "Page title updated" set "Option A" variation
    Then I should see the modal with title "Delete variation"
    When I press "Delete"

    # I verify the variation "Option A" is deleted.
    Then I should see the message "The variation has been deleted." in the messagebox
    Then I should see "1" for the "variation set" count
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should see the text "Page title updated" in the "lift_tray" region
    And I should not see the link "Option A" in the "lift_tray" region
    And I should visibly see the link "Option B" in the "lift_tray" region
    And "Page title updated" set "Control variation" variation should not have the "Edit" link
    And "Page title updated" set "Control variation" variation should not have the "Delete" link
    And "Page title updated" set "Option B" variation should have the "Edit" link
    And "Page title updated" set "Option B" variation should have the "Delete" link

    # I delete another variation, "Option B".
    When I click "Delete" link for the "Page title updated" set "Option B" variation
    Then I should see the modal with title "Delete variation"
    When I press "Delete"

    # I verify the variation "Option B" is deleted.
    Then I should see the message "The variation set has been deleted." in the messagebox
    Then I should see "0" for the "variation set" count
    When I hover over "Variation Sets" in the "lift_tray" region
    Then I should not see the text "Page title updated" in the "lift_tray" region
    And I should not see the link "Option B" in the "lift_tray" region

    # I verify both variations are deleted and there is none left.
    And I should not see the link "Control variation" in the "lift_tray" region
    And I should see the text "No variations" in the "lift_tray" region
