Feature: Lightning Cache
  Makes sure that the Cache feature is properly configured .

  @api
  Scenario: Check for Views Content Cache
    Given I am logged in as a user with the "administrator" role
    When I visit "/admin/structure/views/settings/advanced"
    And I check "edit-views-no-javascript"
    And I press "Save configuration"
    Then the checkbox "edit-views-no-javascript" should be checked
    When I visit "/admin/structure/views"
    Then I should see "Archive"
    And I follow "Enable"
    Then I visit "/admin/structure/views/view/archive/edit"
    And I follow "views-default-cache"
    And I select "views_content_cache" from "edit-cache-type-views-content-cache"
    And I press "Apply"
    Then I should see "Cache segments"
    And I press "Apply"
    Then I should see "Click Cancel to discard your changes."
    And I press "Save"
    Then I should see "The view Archive has been saved." 
