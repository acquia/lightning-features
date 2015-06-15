(function ($) {

  Drupal.acquiaLift = Drupal.acquiaLift || {};
  Drupal.acquiaLift.queueCount = Drupal.acquiaLift.queueCount || 0;

  var queueIsProcessing = false;
  /**
   * Actually trigger the Acquia Lift queue processing and let
   * Drupal handle any command results.
   */
  var processQueue = function() {
    if (queueIsProcessing) {
      return;
    }
    queueIsProcessing = true;
    var queue_url = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'acquia_lift/queue';
    Drupal.acquiaLift.queueCount++;
    $.ajax({
      url: queue_url,
      type: "POST",
      success: function (response, status, jqXHR) {
        Drupal.settings.acquia_lift.sync_queue = 0;
        queueIsProcessing = false;
        $(document).trigger('acquiaLiftQueueSyncComplete');
      },
      complete: function (jqXHR, status) {
        Drupal.acquiaLift.queueCount--;
      }
    });
  }

  /**
   * Run the queue processing if the settings indicate to do so.
   */
  Drupal.behaviors.acquia_lift_queue = {
    attach: function(context, settings) {
      if (Drupal.settings.acquia_lift && Drupal.settings.acquia_lift.sync_queue) {
        processQueue();
      }
    }
  };

  /**
   * A Drupal AJAX command to trigger the queue processing.
   */
  Drupal.ajax.prototype.commands.acquia_lift_process_queue = function (ajax, response, status) {
    processQueue();
  };
}(Drupal.jQuery));
