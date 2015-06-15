/**
 * Contains the definition of the Acquia Lift API.
 *
 * The abstraction of the API wrapper into its own singleton allows different
 * parts of the system to initialize and use the API.
 */

(function ($, Drupal) {

  var AcquiaLiftAPI = (function () {

    var instance, api = null, sessionID;

    /**
     * The Singleton API instance.  All callers will be accessing the functions
     * defined here.
     *
     */
    function SingletonAPI()  {
      this.initializingSession = false;

      var settings = Drupal.settings.acquia_lift;

      var options = {
        'cookies': null, // we provide our own cookie support
        'scodestore': false,
        'server': settings.baseUrl
      };
      if (settings.batchMode) {
        options.batching = 'manual';
      }

      // At this stage we still may not have a session ID.
      if (readSessionID()) {
        options.session = sessionID;
      }

      api = new AcquiaLiftJS(
        settings.owner,
        settings.apiKey,
        options
      );
    }

    function readSessionID() {
      if (!sessionID) {
        sessionID = Drupal.personalize.initializeSessionID();
      }
      return sessionID;
    }

    /**
     * Instance of the API that is handed out.
     */
    SingletonAPI.prototype = {
      constructor: SingletonAPI,

      // Wrapper functions for core API functionality.
      batchSend: function () {
        api.batchSend();
      },
      decision: function (agent_name, options, callback) {
        if (sessionID) {
          options.session = sessionID;
        }
        api.decision(agent_name, options, callback);
      },
      goal: function(agent_name, options, callback) {
        if (readSessionID()) {
          options.session = sessionID;
        }
        api.goal(agent_name, options, callback);
      },
      isManualBatch: function () {
        return (api.opts.batching && api.opts.batching === 'manual');
      },
      getSessionID: function () {
        return sessionID;
      },
      initializeSessionID: function () {
        // This variable ensures subsequent requests for decisions will get
        // queued up until the first decision comes back from Acquia Lift
        // and the session ID gets set.
        this.initializingSession = !Boolean(readSessionID());
      },
      setSessionID: function (id) {
        sessionID = id;
      },
      reset: function () {
        this.initializingSession = false;
        sessionID = null;
        api = instance = undefined;
      }
    }

    // Return the static instance initializer.
    return  {
      name:  "AcquiaLiftAPI",
      getInstance:  function() {
        if (instance  ===  undefined) {
          instance = new SingletonAPI();
        }
        return instance;
      }
    };

  })();
  Drupal.acquiaLiftAPI = AcquiaLiftAPI;
})(Drupal.jQuery, Drupal);
