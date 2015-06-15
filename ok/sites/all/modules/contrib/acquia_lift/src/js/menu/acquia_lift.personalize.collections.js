/**
 * The basic backbone application collections for the unified navigation bar
 * tray.
 */
(function (Drupal, $, _, Backbone) {

  /**
   * A base collection class for common functionality.
   */
  var CollectionBase = Backbone.Collection.extend({
    initialize: function() {
      // Enable models to send a remove event when they are removed from
      // a collection via the reset() function.
      this.on('reset', function (col, opts) {
        _.each(opts.previousModels, function(model) {
          model.trigger('remove');
        });
      });
    }
  });

  /**
   * Provides campaign model management.
   */
  Drupal.acquiaLiftUI.MenuCampaignCollection = CollectionBase.extend({
    model: Drupal.acquiaLiftUI.MenuCampaignModel,
    initialize: function () {
      this.on('change:isActive', this.changeVisibility, this);
      this.parent('inherit', this.options);
    },
    changeVisibility: function (changedModel, value, options) {
      // When a campaign is deactivated, we don't need to enforce anything.
      if (changedModel.get('isActive') === false) {
        return;
      }
      // This campaign was activated; deactivate all other campaigns.
      this.each(function (model) {
        if (model.get('isActive') === true && model !== changedModel) {
          model.set('isActive', false);
        }
      });
    }
  });

  /**
   * A collection of option sets within a model.
   */
  Drupal.acquiaLiftUI.MenuOptionSetCollection = CollectionBase.extend({
    model: Drupal.acquiaLiftUI.MenuOptionSetModel,

    /**
     * {@inheritDoc}
     */
    initialize: function() {
      // Allow certain model changes to trigger a general change event for
      // the entire collection.
      this.variations = null;
      this.on('change:options', this.triggerChange, this);
      this.on('reset', this.triggerChange, this);
      this.parent('inherit', this.options);
    },

    /**
     * Causes the cached variation list to be reset.
     */
    triggerChange: function() {
      this.resetVariations();
      this.trigger('change:variations');
    },

    /**
     * Causes the cached variation list to be reset.
     */
    resetVariations: function() {
      this.variations = null;
    },

    /**
     * Generates the variations listing for page variations made up of the
     * option sets within this collection.
     *
     * The results are cached within a local variable that is invalidated
     * when the variations/options change.
     */
    getVariations: function() {
      if (this.variations !== null) {
        return this.variations;
      }
      if (this.length == 0) {
        return [];
      }
      var i,
        sample = this.at(0),
        sampleOptions = sample ? sample.get('options') : null,
        num = sampleOptions ? sampleOptions.length : 0,
        variations = [],
        variation,
        options,
        option,
        valid,
        variationNum;
      for (i=0; i < num; i++) {
        valid = true;
        variationNum = i+1;
        variation = {
          index: i,
          original_index: i,
          options: [],
          agent: sample.get('agent')
        };
        this.each(function (model) {
          options = model.get('options');
          if (options instanceof Backbone.Collection) {
            options = options.toJSON();
          }
          if (options.length <= i) {
            // This variation is invalid because it does not have an option
            // in each option set.
            valid = false;
          } else {
            option = {
              decision_name: model.get('decision_name'),
              executor: model.get('executor'),
              osid: model.get('osid'),
              plugin: model.get('plugin'),
              selector: model.get('selector'),
              stateful: model.get('stateful'),
              winner: model.get('winner'),
              option: options[i]
            };
            variation.label = options[i].option_label;
            variation.options.push(option);
            variation.original_index = options[i].original_index;
          }
        });
        if (valid) {
          variations.push(variation);
        }
      }
      this.variations = variations;
      return this.variations;
    }
  });

  /**
   *  A collection of options.
   */
  Drupal.acquiaLiftUI.MenuOptionCollection = CollectionBase.extend({
    model: Drupal.acquiaLiftUI.MenuOptionModel,

    initialize: function(options) {
      this.parent('inherit', this.options);
    },

    /**
     * Remember the original index for an option within the option set.
     */
    add: function(models, options) {
      if (_.isArray(models)) {
        _.each(models, function (model, index) {
          model.original_index = index;
        });
      }
      Backbone.Collection.prototype.add.call(this, models, options);
    }
  });

  /**
   * A collection of goals (used for a campaign).
   */
  Drupal.acquiaLiftUI.MenuGoalCollection = Backbone.Collection.extend({
    model: Drupal.acquiaLiftUI.MenuGoalModel
  });

}(Drupal, Drupal.jQuery, _, Backbone));
