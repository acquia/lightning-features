var _tcaq = _tcaq || [];
var _tcwq = _tcwq || [];

(function ($) {
  function generateTrackingId(){
    var d = new Date().getTime();
    var uuid = 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g,
      function(c) {
        var r = (d + Math.random()*16)%16 | 0;
        d = Math.floor(d/16);
        return (c=='x' ? r : (r&0x7|0x8)).toString(16);
      });
    return uuid;
  }
  var trackingId = generateTrackingId(), plugin = 'acquia_lift_profiles_context', callbackRegistered = false;


  Drupal.behaviors.acquia_lift_profiles = {
    'attach': function (context, settings) {
      Drupal.acquia_lift_profiles.init(settings);
      Drupal.acquia_lift_profiles.addActionListener(settings);
      Drupal.acquia_lift_profiles.processServerSideActions(settings);
      Drupal.acquia_lift_profiles.registerSegmentsCallback();
    }
  };

  /**
   * Handles storage and retrieval of segments in the cache.
   */
  var segmentCache = (function() {
    var visitorSegments = null;
    return {
      'store': function(segments) {
        visitorSegments = {};
        for (var i = 0; i < segments.length; i++){
          visitorSegments[segments[i]] = 1;
          // Store this in localStorage so it can be retrieved for use as Lift
          // visitor context.
          Drupal.personalize.visitor_context_write(segments[i], plugin, 1);
        }
        // Go through all available segments and add an entry in localStorage for
        // each one that the visitor does *not* have.
        if (Drupal.settings.acquia_lift_profiles.available_segments) {
          for (var j in Drupal.settings.acquia_lift_profiles.available_segments) {
            if (Drupal.settings.acquia_lift_profiles.available_segments.hasOwnProperty(j)) {
              var segmentName = Drupal.settings.acquia_lift_profiles.available_segments[j];
              if (!visitorSegments.hasOwnProperty(segmentName)) {
                visitorSegments[segmentName] = 0;
                Drupal.personalize.visitor_context_write(segmentName, plugin, 0);
              }
            }
          }
        }
        return visitorSegments;
      },
      'retrieve': function(settings) {
        var i, val, segmentName, segments = settings.available_segments;
        for (i in segments) {
          if (segments.hasOwnProperty(i)) {
            segmentName = segments[i];
            if (visitorSegments === null || !visitorSegments.hasOwnProperty(segmentName)) {
              val = Drupal.personalize.visitor_context_read(segmentName, plugin);
              if (val !== null) {
                visitorSegments = visitorSegments || {};
                visitorSegments[segmentName] = val;
              }
            }
          }
        }
        return visitorSegments;
      },
      'reset': function() {
        visitorSegments = null;
      }
    }
  })();

  Drupal.personalize = Drupal.personalize || {};
  Drupal.personalize.visitor_context = Drupal.personalize.visitor_context || {};
  Drupal.personalize.visitor_context.acquia_lift_profiles_context = {
    'getContext': function(enabled) {
      if ($.cookie('tc_dnt') === "true") {
        return {};
      }

      var i, j, context_values = {};
      // First check to see if we have the acquia_lift_profiles segments already stored
      // locally.
      var cached = segmentCache.retrieve(Drupal.settings.acquia_lift_profiles);
      if (cached) {
        for (i in enabled) {
          if (enabled.hasOwnProperty(i) && cached.hasOwnProperty(i)) {
            context_values[i] = cached[i];
          }
        }
        return context_values;
      }

      return new Promise(function(resolve, reject){
        // Define a callback function to receive information about the segments
        // for the current visitor.
        var segmentsCallback = function (segmentIds, captureInfo) {
          if (captureInfo.x['trackingId'] == trackingId) {
            var allSegments = segmentCache.store(segmentIds);
            for (j in enabled) {
              if (enabled.hasOwnProperty(j) && allSegments.hasOwnProperty(j)) {
                context_values[j] = allSegments[j];
              }
            }
            resolve(context_values);
          }
        };

        // Register our callback for receiving segments.
        _tcwq.push(["onLoad", segmentsCallback]);
        callbackRegistered = true;
      });
    }
  };

  // Keeps track of whether we've captured identity or not.
  var identityCaptured = false;

  /**
   * Send a captureIdentity event to ContextDB
   *
   * @param identifier
   *   The identifier to be sent.
   * @param identityType
   *   The type of identity to pass.
   */
  var pushCaptureIdentity = function(identifier, identityType) {
    if (identityCaptured) {
      return;
    }
    _tcaq.push( [ 'captureIdentity', identifier, identityType ] );
    identityCaptured = true;
  };

  /**
   * Sends a captureIdentity event to TC using the email address from the
   * passed in context.
   *
   * @param DrupalSettings
   *   An object containing the Acquia Lift Profiles settings from the server side.
   * @param context
   *   An object that must at least have a 'mail' property.
   */
  var pushCaptureEmail = function(DrupalSettings, context) {
    // Do nothing if identity has already been captured or should not be captured or
    // if we don't have an email address in the context.
    if (!(DrupalSettings.captureIdentity && context['mail'])) {
      return;
    }
    pushCaptureIdentity(context['mail'], 'email');
  };

  /**
   * Centralized functionality for acquia_lift_profiles behavior.
   */
  Drupal.acquia_lift_profiles = (function(){

    var processedListeners = {}, initialized = false, initializing = false, pageFieldValues = {};
    var agentNameToLabel = {};

    /**
     * Returns the agent label for the given agent name.
     * If there is no label then the agent name is returned.
     *
     * @param agent_name
     */
    function getAgentLabel( agent_name ) {
      var agent_label = agent_name;
      if ( agentNameToLabel[agent_name] ) {
        agent_label = agentNameToLabel[agent_name];
      }
      return agent_label;
    }

    return {
      'init': function(settings) {
        if (initialized || initializing) {
          return;
        }
        initializing = true;
        if ( settings.personalize && settings.personalize.agent_map ) {
          var agent_map = settings.personalize.agent_map;
          for (var agent_name in agent_map) {
            if (agent_map.hasOwnProperty(agent_name)) {
              if (agent_map[agent_name].label) {
                agentNameToLabel[agent_name] = agent_map[agent_name].label;
              }
            }
          }
        }
        var mappings = settings.acquia_lift_profiles.mappings, context_separator = settings.acquia_lift_profiles.mappingContextSeparator, plugins = {}, reverseMapping = {};
        for(var type in mappings) {
          if (mappings.hasOwnProperty(type)) {
            for (var udf in mappings[type]) {
              if (mappings[type].hasOwnProperty(udf)) {
                // We maintain a reverse mapping of all the UDFs that use each
                // context, so we can easily assign values once the contexts have
                // been retrieved.
                if (!reverseMapping.hasOwnProperty(mappings[type][udf])) {
                  reverseMapping[mappings[type][udf]] = [];
                }
                reverseMapping[mappings[type][udf]].push(udf);
                var context = mappings[type][udf].split(context_separator);
                var pluginName = context[0];
                var context_name = context[1];
                if (!plugins.hasOwnProperty(pluginName)) {
                  plugins[pluginName] = {};
                }
                plugins[pluginName][context_name] = context_name;
              }
            }
          }
        }

        var callback = function(contextValues) {
          for (var pluginName in contextValues) {
            if (contextValues.hasOwnProperty(pluginName)) {
              for (var contextName in contextValues[pluginName]) {
                if (contextValues[pluginName].hasOwnProperty(contextName)) {
                  var fullContextName = pluginName + context_separator + contextName;
                  if (reverseMapping.hasOwnProperty(fullContextName)) {
                    // Set this is as the value for all UDFs that use this context.
                    for (var i in reverseMapping[fullContextName]) {
                      if (reverseMapping[fullContextName].hasOwnProperty(i)) {
                        pageFieldValues[reverseMapping[fullContextName][i]] = contextValues[pluginName][contextName];
                      }
                    }
                  }
                }
              }
            }
          }

          // Ensure sensible defaults for our capture data.
          var pageInfo = $.extend({
            'content_title': 'Untitled',
            'content_type': 'page',
            'page_type': 'content page',
            'content_section': '',
            'content_keywords': '',
            'post_id': '',
            'published_date': '',
            'thumbnail_url': '',
            'persona': '',
            'engagement_score':'1',
            'author':'',
            'evalSegments': true,
            'trackingId': trackingId
          }, settings.acquia_lift_profiles.pageContext, pageFieldValues);
          _tcaq.push( [ 'captureView', 'Content View', pageInfo ] );

          if(settings.acquia_lift_profiles.hasOwnProperty('identity')) {
            pushCaptureIdentity(settings.acquia_lift_profiles.identity, settings.acquia_lift_profiles.identityType);
          }

          initialized = true;
        };
        $(document).bind('personalizeDecision', this["processPersonalizeDecision"]);
        $(document).bind('sentGoalToAgent', this["processSentGoalToAgent"]);
        Drupal.personalize.getVisitorContexts(plugins, callback);
      },
      'getTrackingID': function () {
        return trackingId;
      },
      'registerSegmentsCallback': function() {
        if (!callbackRegistered) {
          // Define a callback function to receive information about the segments
          // for the current visitor and add them to the visitorSegments object.
          var segmentsCallback = function (segmentIds, captureInfo) {
            if (captureInfo.x['trackingId'] == trackingId) {
              segmentCache.store(segmentIds);
            }
          };

          // Register our callback for receiving segments.
          _tcwq.push(["onLoad", segmentsCallback]);
          callbackRegistered = true;
        }
      },
      'clearSegmentMemoryCache': function() {
        segmentCache.reset();
      },
      /**
       * Sends an event to TC.
       *
       * @param eventName
       */
      'processEvent': function(eventName, settings, context) {
        var extra = {
          evalSegments: true
        };
        // Add the field and UDF values to the extra info we're sending about the event. The assumption
        // here is that this event is being processed *after* the initial page view capture has
        // already retrieved all the visitor context valuess. Since this happens asynchronously
        // it is not guaranteed that this is the case.
        $.extend(extra, pageFieldValues);
        // Send to acquia_lift_profiles.
        _tcaq.push(['capture', eventName, extra]);
        // If it's a special event with some other callback associated with it, call that
        // callback as well.
        if (typeof this.specialEvents[eventName] == 'function') {
          this.specialEvents[eventName].call(this, settings.acquia_lift_profiles, context);
        }
      },

      'processPersonalizeDecision':function(e, $option_set, decision, osid, agent_name) {
        if (decision == 'control-variation') {
          decision = 'Control';
        }

        _tcaq.push(['capture', 'Campaign Action', {'targetcampaignid':agent_name, 'targetcampaignname':getAgentLabel(agent_name), 'targetofferid': decision, 'targetactionname':decision } ]);

      },

      'processSentGoalToAgent':function(e, agent_name, goal_name, goal_value) {
        _tcaq.push(['capture', goal_name, {'targetcampaignid':agent_name, 'targetcampaignname':getAgentLabel(agent_name), 'targetgoalvalue':goal_value}]);
      },
      /**
       * Add an action listener for client-side goal events.
       */
      'addActionListener': function (settings) {
        if (Drupal.hasOwnProperty('visitorActions')) {
          var events = {}, new_events = 0;
          for (var i in settings.acquia_lift_profiles.tracked_actions) {
            if (settings.acquia_lift_profiles.tracked_actions.hasOwnProperty(i) && !processedListeners.hasOwnProperty(settings.acquia_lift_profiles.tracked_actions[i])) {
              var eventName = settings.acquia_lift_profiles.tracked_actions[i];
              processedListeners[eventName] = 1;
              events[eventName] = 1;
              new_events++;
            }
          }
          if (new_events > 0) {
            var self = this;
            var callback = function(eventName, jsEvent, context) {
              if (events.hasOwnProperty(eventName)) {
                self.processEvent(eventName, settings, {});
              }
            };
            Drupal.visitorActions.publisher.subscribe(callback);
          }
        }
      },

      /**
       * Goes through the server-side actions and calls the appropriate function for
       * each one, passing in the event context.
       *
       * @param settings
       */
      'processServerSideActions': function (settings) {
        if (settings.acquia_lift_profiles.serverSideActions) {
          for (var actionName in settings.acquia_lift_profiles.serverSideActions) {
            if (settings.acquia_lift_profiles.serverSideActions.hasOwnProperty(actionName)) {
              for (var i in settings.acquia_lift_profiles.serverSideActions[actionName]) {
                if (settings.acquia_lift_profiles.serverSideActions[actionName].hasOwnProperty(i) && !settings.acquia_lift_profiles.serverSideActions[actionName][i].processed) {
                  // Process the event.
                  this.processEvent(actionName, settings, settings.acquia_lift_profiles.serverSideActions[actionName][i]);
                  // Mark this event has having been processed so that it doesn't get sent again.
                  settings.acquia_lift_profiles.serverSideActions[actionName][i].processed = 1;
                }
              }
            }
          }
        }
      },

      // Holds the functions that should be called for particular events.
      'specialEvents': {
        'user_login': pushCaptureEmail,
        'user_register': pushCaptureEmail
      },

      /**
       * Helper function to reset variables during tests.
       */
      'resetAll' : function() {
        processedListeners = {};
        initialized = false;
        initializing = false;
        agentNameToLabel = {};
        identityCaptured = false;
        pageFieldValues = {};
        $(document).unbind('personalizeDecision', this["processPersonalizeDecision"]);
        $(document).unbind('sentGoalToAgent', this["processSentGoalToAgent"]);
      }
    }
  })();

})(jQuery);
