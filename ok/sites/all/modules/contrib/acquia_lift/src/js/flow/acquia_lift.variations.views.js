/**
 * @file acquia_lift.variations.views.js
 * 
 * Backbone views for the variations application.
 */
(function($, Drupal, Dialog, Backbone, _) {

  Drupal.acquiaLiftVariations.views = Drupal.acquiaLiftVariations.views || {

    /**
     * Backbone View for the full variation flow.
     */
    AppView: Backbone.View.extend({
      contextualMenuModel: null,
      variationTypeFormModel: null,
      anchor: null,
      // An array of jQuery instances that are available to the DOM selector.
      $watchElements: null,

      initialize: function (options) {
        _.bindAll(this, 'createContextualMenu', 'onElementSelected');

        var that = this;
        this.$el.DOMSelector({
          onElementSelect: function (element, selector) {
            that.onElementSelected(element, selector);
          }
        });
        Backbone.on('acquiaLiftVariationType', this.createVariationTypeDialog, this);
        this.listenTo(this.model, 'change:editMode', this.render);
        this.listenTo(this.model, 'change:editMode', this.updateEditMode);
        this.render(this.model, this.model.get('editMode'));
      },

      /**
       * {@inheritDoc}
       */
      render: function (model, editMode) {
        this.setSelectionMode(editMode);
      },

      /**
       * Handles showing/hiding a highlight around the anchoring element.
       * @param bool show
       *   True if showing the highlight, false if no highlight should be shown.
       */
      highlightAnchor: function(show) {
        var highlightClass = 'acquia-lift-page-variation-item';
        if (!this.anchor) {
          return;
        }
        if (show) {
          $(this.anchor).addClass(highlightClass);
        } else {
          // Remove the highlight from anywhere (the anchor may have been
          // changed).
          $('.' + highlightClass).removeClass(highlightClass);
        }
      },

      /**
       * Updates the application based on changes in edit mode in model.
       */
      updateEditMode: function(model, editMode) {
        if (editMode) {
          if (this.contextualMenuModel) {
            this.contextualMenuModel.set('active', true);
          }
          if (this.variationTypeFormModel) {
            this.variationTypeFormModel.set('active', true);
          }
        } else {
          this.highlightAnchor(false);
          if (this.contextualMenuModel) {
            this.contextualMenuModel.destroy();
            this.contextualMenuModel = null;
          }
          if (this.variationTypeFormModel) {
            this.variationTypeFormModel.destroy();
            this.variationTypeFormModel = null;
          }
        }
      },

      /**
       * Deactivates the view and the page variation process.
       */
      deactivate: function () {
        this.$watchElements.DOMSelector("stopWatching");
      },

      /**
       * Sets whether the DOM selector should be active to allow the end user
       * to select a DOM element.
       */
      setSelectionMode: function(inSelectionMode) {
        if (inSelectionMode) {
          // Must update the watched elements as the page DOM structure can
          // be changed in between each call.
          this.$watchElements = Drupal.acquiaLiftVariations.getAvailableElements();
          this.$el.DOMSelector("updateElements", this.$watchElements);
          this.$el.DOMSelector("startWatching");
        } else {
          this.$el.DOMSelector("stopWatching");
        }
      },

      /**
       * Event callback for when an element is selected in the DOM selector.
       */
      onElementSelected: function (element, selector) {
        this.setSelectionMode(false);
        this.createContextualMenu(element, selector);
      },

      /**
       * Creates a contextual page variation selection menu at the specified
       * element.
       */
      createContextualMenu: function (element, selector) {
        this.anchor = element;
        this.highlightAnchor(true);
        this.contextualMenuModel = new Drupal.visitorActions.ui.dialog.models.DialogModel({
          selector: selector,
          id: 'acquia-lift-modal-variation-type-select'
        });
        var dialogView = new Drupal.acquiaLiftVariations.views.PageVariationMenuView({
          el: element,
          model: this.contextualMenuModel
        });
        this.contextualMenuModel.set('active', this.model.get('editMode'));
      },

      /**
       * Creates a variation type dialog for a specific variation type based on
       * the type selected.
       *
       * @param event
       *   The triggering event that includes the model data/JSON for the selected
       *   VariationTypeModel.
       */
      createVariationTypeDialog: function(event) {
        var variation = this.model.get('variation');
        var formPath = Drupal.settings.basePath + Drupal.settings.pathPrefix +
          'admin/structure/acquia_lift/variation/' +
          Drupal.encodePath(event.data.id);
        if (event.data.osid) {
          formPath += '/' + Drupal.encodePath(event.data.osid);
          if (variation) {
            formPath += '/' + variation.getVariationNumber();
          }
        }
        this.variationTypeFormModel = new Drupal.acquiaLiftVariations.models.VariationTypeFormModel({
          selector: event.data.selector,
          id: 'acquia-lift-modal-variation-type-' + event.data.id,
          formPath: formPath,
          type: event.data.id,
          typeLabel: event.data.name,
          variation: variation
        });
        this.variationTypeView = new Drupal.acquiaLiftVariations.views.VariationTypeFormView({
          el: event.data.anchor,
          model: this.variationTypeFormModel,
          appModel: this.model
        });
        this.variationTypeFormModel.set('active', this.model.get('editMode'));
      },

      /**
       * Open a variation type dialog based on an existing variation set.
       *
       *  @param event
       *    The triggering event that includes the model data/JSON for the selected
       *    VariationTypeModel.
       */
      openExistingTypeDialog: function(event) {
        // Made sure the DOM selector is no longer active.
        this.setSelectionMode(false);
        // Highlight the affected element.
        this.anchor = event.data.anchor;
        this.highlightAnchor(true);
        if (this.variationTypeView) {
          this.variationTypeView.remove();
        }
        // Create the dialog.
        this.createVariationTypeDialog(event);
      },

      /**
       * Generates a page-level temporary unique identifier.
       */
      getTemporaryID: function() {
        return 'acquiaLiftVariations-' + new Date().getTime();
      }
    }),

    /**
     * Backbone view that displays the form to enter the value for a new
     * variation of a specific variation type.
     */
    VariationTypeFormView: Dialog.views.ElementDialogView.extend({
      className: 'acquia-lift-variation-type-form',
      /**
       * {@inheritDoc}
       */
      initialize: function (options) {
        options.myVerticalEdge = 'top';
        options.anchorVerticalEdge = 'bottom';
        this.appModel = options.appModel;
        this.parent('inherit', options);
      },

      /**
       * {@inheritDoc}
       */
      render: function(model, active) {
        var that = this;
        this.parent('render', model, active);
        // Add a title to this dialog.
        var title = Drupal.theme('acquiaLiftVariationsTypeFormTitle', {
          variationType: this.model.get('typeLabel'),
          elementType: this.anchor.nodeName
        });
        this.$el.find('.visitor-actions-ui-dialog-content').prepend($(title));
      },

      /**
       * {@inheritDoc}
       */
      formSuccessHandler: function (ajax, response, status) {
        this.parent('formSuccessHandler', ajax, response, status);

        var selector = this.model.get('selector');
        var type = this.model.get('type');
        var $input = this.$el.find('[name=personalize_elements_content]');
        var variation = this.model.get('variation');

        // Don't show the title field for page variations.
        if (this.appModel.isPageModelMode()) {
          this.$el.find('[name="title"]').val(this.model.get('typeLabel')).closest('.form-item').hide();
        }

        this.$el.find('[name="selector"]').val(selector);
        this.$el.find('[name="pages"]').val(Drupal.settings.visitor_actions.currentPath);
        this.$el.find('[name="agent"]').val(Drupal.settings.personalize.activeCampaign);
        // Call any variation type specific callbacks.
        $(document).trigger('acquiaLiftVariationTypeForm', [type, selector, $input]);

        // Override the form submission handler to verify the selector only
        // matches a single DOM element.
        Drupal.ajax['edit-variation-type-submit-form'].options.beforeSubmit = function (form_values, $element, options) {
          var $selectorInput = $('[name="selector"]', $element),
            selector = $selectorInput.val(),
            matches = 0,
            message = '';
          // If the selector wasn't shown then it doesn't need to be validated.
          if ($selectorInput.length == 0) {
            return true;
          }

          function displaySelectorError(message) {
            $selectorInput.addClass('error');
            if ($('.acquia-lift-js-message', $element).length == 0) {
              var errorHtml = '<div class="acquia-lift-js-message"><div class="messages error">';
              errorHtml += '<h2 class="element-invisible">' + Drupal.t('Error message') + '</h2>';
              errorHtml += '<span class="messages text"></span></div></div>';
              $element.prepend(errorHtml);
            }
            $('.acquia-lift-js-message .messages.error .messages.text').text(message);
            // Make sure the selector is visible for user to edit.
            if (!$selectorInput.is(':visible')) {
              $selectorInput.closest('div').slideToggle();
              $element.parent().find('.acquia-lift-selector-edit').text(Drupal.t('Hide selector'));
            }
            $selectorInput.focus();
          }

          // Check for a valid jQuery selector.
          try {
            matches = $(selector).length;
          } catch (error) {
            displaySelectorError(Drupal.t('Selector field must contain a valid jQuery selector.'));
            return false;
          }
          // Check to be sure the selector matches only one DOM element.
          var matches = $(selector).length;
          if (matches == 1) {
            return true;
          }
          if (matches > 1) {
            message = Drupal.t('The selector matches multiple DOM elements.');
          } else if (matches == 0) {
            message = Drupal.t('The selector does not match any DOM elements.');
          }
          message += ' ' + Drupal.t('Enter a selector that matches a single element, and then click "Save".');
          displaySelectorError(message);
          return false;
        };

      },

      /**
       * Completely remove children and unbind events.
       */
      remove: function() {
        this.unbind();
        Backbone.View.prototype.remove.call(this);
      }
    }),

    /**
     * Contextual menu view to allow selection of the type of variation to
     * create.
     */
    PageVariationMenuView: Dialog.views.ElementDialogView.extend({
      className: 'acquia-lift-context-menu',

      /**
       * {@inheritDoc}
       */
      initialize: function (options) {
        this.parent('inherit', options);
        Backbone.on('acquiaLiftVariationTypeSelected', this.onVariationTypeSelected, this);
        this.list = null;
      },

      /**
       * {@inheritDoc}
       */
      render: function (model, active) {
        var that = this;
        this.parent('render', model, active);
        // Generate the contextual menu HTML.
        var titleHtml = Drupal.theme('acquiaLiftVariationsMenuTitle', {
          elementType: this.anchor.nodeName
        });

        // Generate the collection of options.
        var collection = new Drupal.acquiaLiftVariations.collections.ElementVariationCollection();
        var modelAttributes = _.map(Drupal.settings.personalize_elements.contextualVariationTypes, function(data, type) {
          return {
            id: type,
            name: data.name,
            limitByChildrenType: data.limitByChildrenType
          };
        });
        collection.add(modelAttributes);
        this.list = new Drupal.acquiaLiftVariations.views.VariationTypeMenuListView({collection: collection.applicableToElement($(this.anchor))});
        this.list.render();
        this.$el.find('.visitor-actions-ui-dialog-content').html(titleHtml).append(this.list.el);
        this.position(function () {
          that.show();
        });
      },

      /**
       * Called when the user selects a variation type.
       *
       * Closes the contextual menu and removes it as it is no longer needed.
       * It also triggers a new event that includes the variation type data
       * from the original event plus the anchor information from this menu.
       */
      onVariationTypeSelected: function(event) {
        event.data.anchor = this.anchor;
        event.data.selector = this.model.get('selector');
        this.remove();
        Backbone.trigger('acquiaLiftVariationType', {data: event.data});
      },

      /**
       * Completely remove children and unbind events.
       */
      remove: function() {
        if (this.list) {
          this.list.remove();
        }
        Backbone.off('acquiaLiftVariationTypeSelected');
        this.unbind();
        Backbone.View.prototype.remove.call(this);
      }
    }),

    /**
     * A view for the list of variation options presented within the contextual
     * menu.
     */
    VariationTypeMenuListView: Backbone.View.extend({
      tagName: 'ul',
      className: 'acquia-lift-page-variation-list',

      /**
       * {@inheritDoc}
       */
      initialize: function (options) {
        _.bindAll(this, 'renderItem');
        this.subviews = [];
      },

      /**
       * Renders a single page variation menu item.
       */
      renderItem: function (model) {
        var itemView = new Drupal.acquiaLiftVariations.views.VariationTypeMenuListItemView({model: model});
        itemView.render();
        this.$el.append(itemView.el);
        this.subviews.push(itemView);
      },

      /**
       * {@inheritDoc}
       */
      render: function () {
        this.collection.each(this.renderItem);
      },

      /**
       * Completely remove children and unbind events.
       */
      remove: function() {
        _.invoke(this.subviews, 'remove');
        this.unbind();
        Backbone.View.prototype.remove.call(this);
      }
    }),

    /**
     * Backbone view for a single variation option presented within the
     * contextual menu.
     */
    VariationTypeMenuListItemView: Backbone.View.extend({
      tagName: 'li',

      /**
       * {@inheritDoc}
       */
      initialize: function(options) {
        _.bindAll(this, 'clicked');
      },

      /**
       * Event definitions: defines click handler when a variation type link
       * is clicked.
       */
      events: {
        "click a": "clicked"
      },

      /**
       * Event handler when a menu list item is clicked.
       * @param e
       */
      clicked: function (e){
        e.preventDefault();
        Backbone.trigger('acquiaLiftVariationTypeSelected', {data: this.model.toJSON()});
      },

      /**
       * {@inheritDoc}
       */
      render: function(){
        var html = Drupal.theme('acquiaLiftVariationsMenuItem', this.model.toJSON());
        this.$el.append(html);
      },

      /**
       * Completely remove children and unbind events.
       */
      remove: function() {
        this.unbind();
        Backbone.View.prototype.remove.call(this);
      }
    })
  }

}(Drupal.jQuery, Drupal, Drupal.visitorActions.ui.dialog, Backbone, _));
