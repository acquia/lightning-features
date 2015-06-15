/**
 * @file administration.tests.js
 *
 * QUnit tests related to the administration of campaigns.
 * These tests do not require Drupal integration.
 */
var $ = Drupal.jQuery;

QUnit.module("Acquia Lift Synchronization Queue");

QUnit.test("QueueItem unit tests", function(assert) {
  // Create a fake request for the queue.
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];

  expect(3);

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  // Mock to create a new Drupal AJAX handler.
  Drupal.ajax = function (base, element, element_settings) {};

  // Event listener for a queue complete event.
  $(document).bind('acquiaLiftQueueSyncComplete', function () {
    // The expect assertion will fail if this is not called the correct number
    // of times.
    assert.ok(true, 'Acquia Lift synchronization complete called.');
  });

  // Create the settings listener for the queue completion event.
  Drupal.behaviors.acquiaLiftUnibarListeners.attach();

  // Event listener for settings retrieval.
  // Note that the id will be created with a timestamp of 0 due to Sinon use.
  $('#acquia-lift-settings-0').bind('acquiaLiftSettingsUpdate', function () {
    // The expect assertion will fail if this is not called the correct number
    // of times.
    assert.ok(true, 'Acquia lift settings update triggered.');
  });

  // Trigger a queue synchronization event.
  Drupal.settings.acquia_lift.sync_queue = true;
  Drupal.behaviors.acquia_lift_queue.attach();

  // Trigger a response
  assert.equal(requests.length, 1, 'A queue request was made.');
  requests[0].respond(200);

});

