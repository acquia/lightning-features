/**
 * The basic backbone application factories for the unified navigation bar
 * tray.
 */
(function (Drupal, $, _, Backbone) {

  Drupal.acquiaLiftUI.views.pageVariations = Drupal.acquiaLiftUI.views.pageVariations || {};
  Drupal.acquiaLiftUI.views.optionSets = Drupal.acquiaLiftUI.views.optionSets || {};
  Drupal.acquiaLiftUI.factories = Drupal.acquiaLiftUI.factories || {};
  Drupal.acquiaLiftUI.factories.MenuFactory = Drupal.acquiaLiftUI.factories.MenuFactory || {
    /**
     * Factory method to create the correct type of content variation set view
     * based on the type of data displayed.
     *
     * @param Drupal.acquiaLiftUI.MenuOptionSetModel model
     *   The model that will be the base for this view.
     * @param Drupal.acquiaLiftUI.MenuCampaignModel model
     *   The campaign model that owns the option set.
     * @param element
     *   The DOM element for the Backbone view.
     */
    createContentVariationView: function (model, campaignModel, element) {
      if (campaignModel instanceof Drupal.acquiaLiftUI.MenuCampaignABModel) {
        // There is only one page variation view per page per campaign, but
        // this may be called multiple times due to multiple option sets.
        var view, campaignName = campaignModel.get('name');
        if (!Drupal.acquiaLiftUI.views.pageVariations.hasOwnProperty(campaignName)) {
          view = Drupal.acquiaLiftUI.views.pageVariations[campaignName] = new Drupal.acquiaLiftUI.MenuPageVariationsView({
            model: campaignModel,
            el: element
          });
        }
        view = Drupal.acquiaLiftUI.views.pageVariations[campaignName];
      } else {
        view = Drupal.acquiaLiftUI.views.optionSets[model.get('osid')] = new Drupal.acquiaLiftUI.MenuOptionSetView({
          campaignModel: campaignModel,
          model: model,
          el: element
        });
      }
      return view;
    },

    /**
     * Factory method to create the correct type of campaign model based
     * on the type of data.
     *
     * @param object data
     *   The data to create the model from.
     */
    createCampaignModel: function (data) {
      if (data.type == 'acquia_lift_simple_ab') {
        return new Drupal.acquiaLiftUI.MenuCampaignABModel(data);
      } else {
        return new Drupal.acquiaLiftUI.MenuCampaignModel(data);
      }
    }
  };

}(Drupal, Drupal.jQuery, _, Backbone));
