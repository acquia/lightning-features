@lightning @wysiwyg
Feature: Lightning WYSIWYG
  Ensure WYSIWYG settings are set.

  @api
  Scenario: Lightning WYSIWYG Configuration sanity check
    Given I am logged in as a user with the "administrator" role
    When I visit "admin/config/content/formats/filtered_html"
    Then the "Text editor" field should contain "CKEditor"
    And the checkbox "edit-editor-settings-image-upload-status" should be checked
