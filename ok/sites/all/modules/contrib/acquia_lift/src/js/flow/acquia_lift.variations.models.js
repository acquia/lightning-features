/**
 * @file acquia_lift.variations.models.js
 * 
 * Backbone models for variation application.
 */
(function($, Drupal, Dialog, Backbone, _) {

  Drupal.acquiaLiftVariations.models = Drupal.acquiaLiftVariations.models || {};

  /**
   * Base model for a variation that can be shown or edited.
   *
   * Models that extend this class are responsible for setting the "option"
   * property which holds a reference to the option within an option set
   * settings that represents this variation.
   */
  Drupal.acquiaLiftVariations.models.BaseVariationModel = Backbone.Model.extend({
    // Each type of variation overrides this to return the index for the
    // variation.  -1 indicates a new variation.
    getVariationNumber: function () {
      return -1;
    },

    getVariationLabel: function () {
      var option = this.get('option');
      return option ? option.option_label : Drupal.t('Variation');
    },

    getContent: function () {
      var option = this.get('option');
      return option ? option.personalize_elements_content : '';
    }
  });

  $.extend(Drupal.acquiaLiftVariations.models, {
  /**
     * Backbone model for the variations process.
     */
    AppModel: Backbone.Model.extend({
      MODEL_MODE_PAGE: 'page',
      MODEL_MODE_ELEMENT: 'element',

      defaults: {
        // If this app is being loaded, it is because it is being launched into
        // an edit mode.
        editMode: true,
        modelMode: this.MODEL_MODE_PAGE,
        // The current variation being edited.
        // This will be a model that extends the BaseVariationModel class.
        variation: null
      },

      /**
       * Set the model mode.
       *
       * The mode can be either page-level variations (used for simple a/b
       * tests) or individual variations (used for all other campaigns).
       */
      setModelMode: function (pageLevel) {
        this.set('modelMode', pageLevel ? this.MODEL_MODE_PAGE : this.MODEL_MODE_ELEMENT);
      },

      /**
       * Determine if the model is in page variation mode or element mode.
       *
       * @returns boolean
       * True if page variation mode, false otherwise.
       */
      isPageModelMode: function () {
        return this.get('modelMode') === this.MODEL_MODE_PAGE;
      },

      /**
       * {@inheritdoc}
       */
      destroy: function (options) {
        this.trigger('destroy', this, this.collection, options);
      }
    }),

    /**
     * Backbone model representing a single element variation type
     * that can be presented within a contextual menu.
     *
     * Examples:  edit HTML, edit text, add class, etc.
     */
    VariationTypeModel: Backbone.Model.extend({
      defaults: {
        limitByChildrenType: ''
      }
    }),

    /**
     * Backbone model for a variation type form.
     */
    VariationTypeFormModel: Dialog.models.DialogModel.extend({
      defaults: _.extend({}, Dialog.models.DialogModel.prototype.defaults,
        {
          // A type of variation, e.g. 'editHTML', 'prependHTML'
          type: null,
          // The label for the variation type.
          typeLabel: null,
          selector: null,
          variation: null
        }
      )
    }),

    /**
     * The model for a variation within a personalize elements option set.
     */
    ElementVariationModel: Drupal.acquiaLiftVariations.models.BaseVariationModel.extend({
      defaults: {
        osid: null,
        optionId: null,
        option: null
      },

      initialize: function () {
        var osid = this.get('osid'),
          optionId = this.get('optionId'),
          that = this;
        if (Drupal.settings.personalize.option_sets.hasOwnProperty(osid)) {
          var options = Drupal.settings.personalize.option_sets[osid].options;
          _.each(options, function (option) {
            if (option['option_id'] === optionId) {
              that.set('option', option);
            }
          });
        }
      },

      getVariationNumber: function () {
        return this.get('optionId');
      }
    }),

    /**
     * The model for a variation within a page variation.
     */
    PageVariationModel: Drupal.acquiaLiftVariations.models.BaseVariationModel.extend({
      defaults: {
        agentName: null,
        variationIndex: -1,
        selector: null,
        option: null
      },

      initialize: function () {
        var variationIndex = this.get('variationIndex'),
          agentName = this.get('agentName'),
          selector = this.get('selector'),
          that = this;

        // Find the right option set for this agent and selector.
        _.each(Drupal.settings.personalize.option_sets, function(option_set) {
          if (option_set.agent === agentName && option_set.selector === selector) {
            if (option_set.options.hasOwnProperty(variationIndex)) {
              that.set('option', option_set.options[variationIndex]);
            }
          }
        });
      },

      getVariationNumber: function () {
        return this.get('variationIndex');
      }
    })
  });

}(Drupal.jQuery, Drupal, Drupal.visitorActions.ui.dialog, Backbone, _));
