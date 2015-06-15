/**
 * Acquia Lift goals processing queue functions.
 */

(function ($, Drupal) {
  "use strict";

  Drupal.acquiaLiftUtility = Drupal.acquiaLiftUtility || {};
  Drupal.acquiaLiftUtility.GoalQueue = Drupal.acquiaLiftUtility.GoalQueue || (function($) {

    var acquiaLiftAPI;

    /**
     * Converts the data for a goal into the format for saving in the queue.
     *
     * @param goal
     *   An object with the following keys:
     *   - agentName: The name of the agent for this gaol
     *   - options: An object of goal options to be sent with the goal.
     * @return object
     *   A simple object of data to be saved.
     */
    function convertGoalToQueueData(goal) {
      return {'a': goal.agentName, 'o': goal.options};
    }

    /**
     * Converts the queue data into the data for a goal.
     *
     * @param item
     *   The queue item data.
     * @return object
     *   An object with the following keys:
     *   - agentName: The name of the agent for this gaol
     *   - options: An object of goal options to be sent with the goal.
     */
    function convertQueueDataToGoal(item) {
      if (!item.a || !item.o) {
        return {};
      }
      // Make a deep copy of the object data as the goal data will be
      // transformed and updated by the API call.
      return {
        'agentName': item.a,
        'options': $.extend(true, {}, item.o)
      };
    }

    /**
     * Processes a goal QueueItem through the Acquia Lift service.
     *
     * @param queueItem
     *   The item to process.
     * @param callback
     *   A callback function to be used for notification when processing is
     *   complete.  The callback will receive:
     *   - an instance of the QueueItem processed
     *   - a boolean indicating if the processing was successful.
     */
    function processGoalItem(queueItem, callback) {
      var api = Drupal.acquiaLiftAPI.getInstance();
      var goal = convertQueueDataToGoal(queueItem.getData());
      if (!goal.agentName || !goal.options) {
        throw new Error('Invalid goal data.');
      }
      api.goal(goal.agentName, goal.options, function(accepted, session, retryable) {
        if (callback && typeof callback === 'function') {
          callback(queueItem, accepted, session, retryable);
        }
      });
      if (api.isManualBatch()) {
        api.batchSend();
      }
    }

    return {
      /**
       * Adds goal data to the persistent queue.
       *
       * @param agentName
       *   The name of the agent for the goal.
       * @param options
       *   Goal options to send with the goal.
       *  @param process
       *    Boolean indicating if goals should be immediately processed,
       *    defaults to true.
       */
      'addGoal': function (agentName, options, process) {
        var data = convertGoalToQueueData({'agentName': agentName, 'options': options});
        var process = process == undefined ? true : process;
        // Add the data to the persistent queue.
        Drupal.acquiaLiftUtility.Queue.add(data);
        // Now attempt to process the queue.
        if (process) {
          this.processQueue();
        }
      },

      /**
       * Process the queue by sending goals to the Acquia Lift agent.
       *
       * @param reset
       *   (Optional) True if the queue should be reset such that all items are
       *   tried (such as in an initial processing for the page request).
       */
      'processQueue': function (reset) {
        reset = reset || false;
        // The processing status should be reset upon the initial page load.
        if (reset) {
          Drupal.acquiaLiftUtility.Queue.reset();
        }

        var failed = [];

        // Function to kick off the processing for the next goal.
        function processNext () {
          var item = Drupal.acquiaLiftUtility.Queue.getNext();
          if (item) {
            try {
              processGoalItem(item, processComplete);
            }
            catch (e) {
              // If there was an exception, then this goal data cannot be
              // processed so remove it and move on.
              Drupal.acquiaLiftUtility.Queue.remove(item);
              processNext();
            }
          } else {
            // We are all done with processing the queue.  Add back in any
            // failures to try again the next time the queue is processed.
            var i, num = failed.length;
            for (i = 0; i < num; i++) {
              Drupal.acquiaLiftUtility.Queue.add(failed[i]);
            }
          }
        }

        // Callback for when a single goal processing call is complete.
        function processComplete (item, accepted, session, retryable) {
          if (!accepted && retryable) {
            failed.push(item);
          } else {
            Drupal.acquiaLiftUtility.Queue.remove(item)
          }
          processNext();
        }
        // Kick off the queue.
        processNext();
      }
    }
  }($));


}(Drupal.jQuery, Drupal));
