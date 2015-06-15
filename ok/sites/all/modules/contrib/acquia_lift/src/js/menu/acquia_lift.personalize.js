/**
 * The basic backbone application shared utilities for the unified navigation
 * bar tray.
 */
(function (Drupal, $, _, Backbone) {

  var startPath = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/acquia_lift/start/';

  Drupal.acquiaLiftUI = Drupal.acquiaLiftUI || {};
  Drupal.acquiaLiftUI.views = Drupal.acquiaLiftUI.views || [];
  Drupal.acquiaLiftUI.models = Drupal.acquiaLiftUI.models || {};
  Drupal.acquiaLiftUI.collections = Drupal.acquiaLiftUI.collections || {};
  Drupal.acquiaLiftUI.collections['option_sets'] = Drupal.acquiaLiftUI.collections['option_sets'] || {};

  var initialized = false;
  Drupal.acquiaLiftUI.utilities = Drupal.acquiaLiftUI.utilities || {};
  Drupal.acquiaLiftUI.utilities.updateNavbar = _.throttle(function() {
    if (initialized && Drupal.behaviors.acquiaLiftNavbarMenu) {
      Drupal.behaviors.acquiaLiftNavbarMenu.attach();
      Drupal.behaviors.ZZCToolsModal.attach($('.acquia-lift-controls'));
    }
  }, 500);

  Drupal.acquiaLiftUI.utilities.setInitialized = function(value) {
    initialized = value;
  };

  Drupal.acquiaLiftUI.utilities.shutDownGoalsUI = _.throttle(function () {
    $(document).trigger('visitorActionsUIShutdown');
  }, 500);

  /**
   * Apply a callback to values in an object.
   *
   * @param object obj
   *   The object to be looped over.
   * @param function cb
   *   The callback to be invoked on each object value.
   *
   * @return object
   *   The merged results of the callback.
   */
  Drupal.acquiaLiftUI.utilities.looper = function (obj, cb) {
    var composite = {};
    for (var key in obj) {
      if (obj.hasOwnProperty(key)) {
        var args = Array.prototype.slice.call(arguments, 2);
        // Pass the object item and its key as the first two arguments of the
        // callback, in addition to any other arguments passed to this function.
        args.unshift(obj[key], key);
        // Merge in the return objects from the callback.
        var ret = cb.apply(obj, args);
        if (ret && typeof ret === 'object') {
          $.extend(composite, ret);
        }
      }
    }
    return composite;
  }

  /**
   * Finds the model of the active campaign sets isActive to true.
   *
   * @param string activeCampaign
   *   The name of the active campaign.
   */
  Drupal.acquiaLiftUI.setActiveCampaign = function (activeCampaign) {
    // Refresh the model data for the campaigns.
    var newCampaign = Drupal.acquiaLiftUI.collections['campaigns'].findWhere({'name': activeCampaign});
    if (newCampaign) {
      newCampaign.set('isActive', true);
    } else {
      activeCampaign = '';
    }
    Drupal.settings.personalize.activeCampaign = activeCampaign;
  };

  /**
   * Updates the active campaign session variable on the server.
   *
   * @param string name
   *   The name of the selected campaign.
   */
  Drupal.acquiaLiftUI.setActiveCampaignAjax = function (name, view) {
    var url = Drupal.settings.personalize && Drupal.settings.personalize.links.campaigns.setActive;
    if (url) {
      // This could do with some error checking.
      $.ajax({
        url: url.replace('%personalize_agent', name),
        success: function (response) {
          if (response.personalize_campaign) {
            Drupal.acquiaLiftUI.setActiveCampaign(response.personalize_campaign);
          }
        },
        complete: function () {
          // @todo Finish styling a throbber for campaign links that fire an
          // ajax event.
          //view.$el.find('.acquia-lift-campaign').next('.ajax-progress').remove();
        }
      });
    }
  };

}(Drupal, Drupal.jQuery, _, Backbone));
