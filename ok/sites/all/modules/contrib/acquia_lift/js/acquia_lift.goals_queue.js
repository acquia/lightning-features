/**
 * Utility functions for stateless queue cookie processing.
 *
 * Functionality includes basic read/write functionality.
 */
(function ($, Drupal) {
  "use strict";

  Drupal.acquiaLiftUtility = Drupal.acquiaLiftUtility || {};

  Drupal.acquiaLiftUtility.QueueItem = function (params) {
    var queueItemUid,
      queueItemData,
      queueItemProcessing = false,
      numberTried = 0;

    /**
     * Returns the unique ID assigned to this queue item.
     */
    this.getId = function () {
      return queueItemUid;
    };

    /**
     * Sets the unique ID assigned to this queue item.
     */
    this.setId = function (value) {
      queueItemUid = value;
    }

    /**
     * Returns the data that is held by this queue item.
     */
    this.getData = function () {
      return queueItemData;
    };

    /**
     * Setter for the data held by this queue item.
     */
    this.setData = function (value) {
      queueItemData = value;
    }
;
    /**
     * Determines if this queue item is currently processing.
     */
    this.isProcessing = function () {
      return queueItemProcessing;
    };

    /**
     * Setter for the processing flag for this queue item.
     */
    this.setProcessing = function (isProcessing) {
      queueItemProcessing = isProcessing;
    };

    /**
     * Gets the number of times this item has been tried in the queue.
     */
    this.getNumberTried = function () {
      return numberTried;
    };

    /**
     * Sets the number of times this item has been tried in the queue.
     */
    this.setNumberTried = function (value) {
      numberTried = value;
    };

    /**
     * Increments the number of times this item has been tried.
     */
    this.incrementTries = function () {
      numberTried++;
    };

    // Constructor handling.
    if (params.hasOwnProperty('id')
      && params.hasOwnProperty('data')
      && params.hasOwnProperty('pflag')
      && params.hasOwnProperty('numberTried')) {
      this.setId(params.id);
      this.setData(params.data);
      this.setProcessing(params.pflag);
      this.setNumberTried(params.numberTried);
    } else {
      var uid = 'acquia-lift-ts-' + new Date().getTime() + Math.random();
      this.setId(uid);
      this.setData(params);
      this.setProcessing(false);
      this.setNumberTried(0);
    }
  };

  Drupal.acquiaLiftUtility.QueueItem.prototype = {
    constructor: Drupal.acquiaLiftUtility.QueueItem,

    /**
     * Determines if QueueItem is equal to the current QueueItem.
     *
     * @param queueItem
     *   The item to check against.
     * @returns
     *   True if equal, false if unequal.
     */
    'equals': function (queueItem) {
      return (queueItem instanceof Drupal.acquiaLiftUtility.QueueItem && queueItem.getId() == this.getId());
    },

    /**
     * Resets the processing flag on this queue item.
     */
    'reset': function() {
      this.setProcessing(false);
    },

    /**
     * Parses the QueueItem into a simple object.
     */
    'toObject': function () {
      return {
        'id': this.getId(),
        'data': this.getData(),
        'pflag': this.isProcessing(),
        'numberTried': this.getNumberTried()
      };
    }
  }

  Drupal.acquiaLiftUtility.Queue = Drupal.acquiaLiftUtility.Queue || (function($) {
    // @todo: Would be cool if we could swap out back-ends to local storage or
    // other mechanism.

    var cookieName = 'acquiaLiftQueue', maxRetries = 5;

    /**
     * Indicates if the cookie handling script handles object serialization.
     * This is not available in the jquery.cookie.js version that ships with
     * Drupal 7 but some installations use a later version.
     *
     * @return boolean
     *   True if cookie handles serialization, false if data must be manually
     *   serialized before writing.
     */
    function cookieHandlesSerialization() {
      return ($.cookie.json && $.cookie.json == true);
    }

    /**
     * Reads the queue from storage.
     *
     * @returns array
     *   An array of QueueItems.
     */
    function readQueue() {
      var queue = $.cookie(cookieName);
      var unserialized = cookieHandlesSerialization() ? queue : $.parseJSON(queue);
      return $.isArray(unserialized) ? unserialized : [];
    }

    /**
     * Returns a fully-parsed queue.
     */
    function getAll() {
      var unserialized = readQueue(), i, num = unserialized.length, queue = [];
      for (i = 0; i < num; i++) {
        queue.push(new Drupal.acquiaLiftUtility.QueueItem(unserialized[i]));
      }
      return queue;
    }

    /**
     * Returns the first unprocessed QueueItem.
     */
    function getFirstUnprocessed() {
      var unserialized = readQueue(), i, num = unserialized.length, item;
      for (i = 0; i < num; i++) {
        item = new Drupal.acquiaLiftUtility.QueueItem(unserialized[i]);
        if (!item.isProcessing()) {
          return item;
        }
      }
      return null;
    }

    /**
     * Find index of a QueueItem within the Queue.
     *
     * @param queue
     *   An instance of the queue to search.
     * @param item
     *   The QueueItem to find within the queue.
     * @return int
     *   The index of the item in the queue or -1 if not found.
     */
    function indexOf(queue, item) {
      var i,
        num = queue.length,
        test;
      // Only initialize as many as we have to in order to find a match.
      for (i = 0; i < num; i++) {
        test = new Drupal.acquiaLiftUtility.QueueItem(queue[i]);
        if (test.equals(item)) {
          return i;
        }
      }
      return -1;
    }


    /**
     * Writes the queue to storage.
     *
     * @param array
     *   The queue as an array.
     */
    function writeQueue(queue) {
      var queueData = [], i, num = queue.length;

      // Prepare the queue by making sure all items to save are simple objects.
      for (i = 0; i < num; i++) {
        if (queue[i] instanceof Drupal.acquiaLiftUtility.QueueItem) {
          queueData.push(queue[i].toObject())
        } else {
          queueData.push(queue[i]);
        }
      }
      // Serialize if necessary.
      if (!cookieHandlesSerialization()) {
        queueData = JSON.stringify(queueData);
      }
      // Write to the cookie.
      $.cookie(cookieName, queueData);
    }

    /**
     * Adds an existing QueueItem back to the queue for re-processing.
     *
     * @param queueItem
     *   The item to add back into the queue.
     * @param reset
     *   Boolean indicates if the processing flag should be reset, defaults
     *   false.
     */
    function addBack(queueItem, reset) {
      var queue = readQueue();
      var index = indexOf(queue, queueItem);

      if (reset && reset == true) {
        queueItem.reset();
        queueItem.incrementTries();
      }
      if (queueItem.getNumberTried() >= maxRetries) {
        // This item is beyond the maximum number of tries and should be
        // removed from the queue so don't add it back.
        Drupal.acquiaLiftUtility.Queue.remove(queueItem);
        return;
      }
      if (index >= 0) {
        queue.splice(index, 1, queueItem);
      } else {
        queue.push(queueItem);
      }
      writeQueue(queue);
    };

    /**
     * Publicly accessible queue methods.
     */
    return {
      /**
       * Adds a QueueItem to the queue.
       *
       * The item can be new data to add, a new QueueItem to add, or an
       * existing QueueItem to return to the queue for re-processing.
       *
       * @param data
       *   Data or a QueueItem to add to the queue.
       * @param reset
       *   Indicates if the processing should be reset (defaults to true).
       */
      'add': function (data, reset) {
        reset = reset == undefined ? true : reset;

        if (data instanceof Drupal.acquiaLiftUtility.QueueItem) {
          addBack(data, reset);
          return;
        }
        var queue = readQueue();
        queue.push(new Drupal.acquiaLiftUtility.QueueItem(data));
        writeQueue(queue);
      },

      /**
       * Gets the next unprocessed item in the queue for processing.
       * @returns a queueItem or null;
       *   The queueItem.
       */
      'getNext': function () {
        var item = getFirstUnprocessed();
        if (item) {
          item.setProcessing(true);
          // Save the updated item back into the queue (will overwrite).
          this.add(item, false);
        }
        return item;
      },

      /**
       * Removes a queueItem from the processing queue.
       *
       * @param queueItem
       *   The item to remove.
       * @returns
       *   True if the item was found to remove, false if not found.
       */
      'remove': function (queueItem) {
        var queue = readQueue();
        var index = indexOf(queue, queueItem);
        if (index >= 0) {
          queue.splice(index, 1);
          writeQueue(queue);
          return true;
        }
        return false;
      },

      /**
       * Resets the processing status on all items in the queue.
       */
      'reset': function () {
        var i,
          queue = getAll(),
          num = queue.length;
        for (i = 0; i < num; i++) {
          queue[i].reset();
        }
        writeQueue(queue);
      },

      /**
       * Empties the queue of all options (typcially used for testing purposes).
       */
      'empty': function () {
        writeQueue([]);
      }
    }
  }($));

}(Drupal.jQuery, Drupal));

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

//# sourceMappingURL=acquia_lift.goals_queue.js.map