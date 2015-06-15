/**
 * Functionality related specifically to the modal campaign management
 * procedures.
 */
(function ($, Drupal) {
  "use strict";

  Drupal.behaviors.acquiaLiftTypeModal = {
    attach: function (context, settings) {
      // Make the whole campaign type div clickable.
      $('div.ctools-modal-content .modal-content .acquia-lift-type', context).once(function() {
        $(this).on('click', function(e) {
          var $link = $(this).find('a.acquia-lift-type-select');
          // Special handling based on href values.
          if ($link.attr('href') == settings.basePath + settings.pathPrefix + 'admin/structure/visitor_actions') {
            // Trigger goals in context.
            $('#acquiaLiftVisitorActionsConnector').find('a').trigger('click');
            Drupal.CTools.Modal.dismiss();
            e.preventDefault();
            e.stopImmediatePropagation();
          } else if ($link.attr('href') == settings.basePath + settings.pathPrefix + 'admin/structure/personalize/variations/personalize-elements/add') {
            // Trigger variations in context.
            $(document).trigger('acquiaLiftElementVariationModeTrigger', [{start: true}]);
            Drupal.CTools.Modal.dismiss();
            e.preventDefault();
            e.stopImmediatePropagation();
          } else if ($link.hasClass('ctools-use-modal')) {
            // It needs to be the link that is triggered if we want CTools to
            // take over.
            if (!$(e.currentTarget).is('a')) {
              $link.trigger('click');
            }
            // Let the event bubble on to the next handler.
            return;
          } else {
            // If it's a regular link, then we also need to set the new location.
            window.location = $link.attr('href');
          }
        })
      });
      hidePageVisitorActionsButton();
      // When visitor actions is activated, remove the page actions button
      // because the user is selecting this through the modal process.
      $('body').once('acquiaLiftVisitorActionsHidePage', function() {
        $(document).bind('visitorActionsUIEditMode', function (event, isActive) {
          if (isActive) {
            hidePageVisitorActionsButton();
          }
        });
      });

      // The visitor actions ui application expects there to always be a
      // trigger link on the page, but with the modal process the trigger would
      // disappear when the modal closes.  We create a hidden trigger link
      // to handle the edit mode toggle.
      var $connector = $('#acquiaLiftVisitorActionsConnector');
      if ($connector.length == 0) {
        $('body').append('<div id="acquiaLiftVisitorActionsConnector"><a href="' + Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/visitor_actions/add" class="element-hidden">' + Drupal.t('Add goals') + '</a></div>');
        // Allow visitor actions UI to process the link.
        Drupal.attachBehaviors($('#acquiaLiftVisitorActionsConnector'));
        $(document).on('acquiaLiftVisitorActionsConnectorToggle', function(e) {
          $('#acquiaLiftVisitorActionsConnector').find('a').trigger('click');
        });
      }

      // Provide method to hide full selector in variation type details form
      // until the user selects to edit.
      // Note that the form is sent as the new context so we can't just check
      // within the context.
      // Note that the selector input may not be available if the user isn't
      // able to edit its contents.
      var $variationTypeForm = $('#acquia-lift-element-variation-details-form').not('.acquia-lift-processed');
      var $selectorInput = $variationTypeForm.find('input[name="selector"]');

      if ($variationTypeForm.length > 0 && $selectorInput.length > 0) {
        var editLink = '<a class="acquia-lift-selector-edit">' + Drupal.t('Edit selector') + '</a>';
        var $selector =  $selectorInput.closest('div');
        $variationTypeForm.parent().find('h2').append(editLink);
        $variationTypeForm.parent().find('.acquia-lift-selector-edit').on('click', function(e) {
          var newText = $(this).text() == Drupal.t('Edit selector') ? Drupal.t('Hide selector') : Drupal.t('Edit selector');
          $selector.slideToggle();
          $(this).text(newText);
        });
        $selector.hide();
        $variationTypeForm.addClass('acquia-lift-processed');
      }

      // Populate the pages input with the current page.
      // The form is sent as the context so we can't check within it.
      var $pageGoalForm = $('#acquia-lift-create-goal-type-form').not('acquia-lift-processed');
      if ($pageGoalForm.length > 0) {
        $pageGoalForm.find('input[name="pages"]').val(Drupal.settings.visitor_actions.currentPath);
        $pageGoalForm.addClass('acquia-lift-processed');
      }
    }
  };

  Drupal.behaviors.acquiaLiftOptionSetTypeList = {
    attach: function (context, settings) {
      $('#acquia-lift-option-set-type-list', context).once('acquia-lift-option-set-type-list').each(function () {
        var blockAnchor = $(this).find('a[href="' + settings.basePath + settings.pathPrefix + 'admin/structure/personalize/variations/personalize-blocks/add"]');
        // Add the current destination address to the personalize blocks anchor.
        blockAnchor.attr('href', blockAnchor.attr('href') + '?destination=' + settings.visitor_actions.currentPath);
      });
    }
  };

  function hidePageVisitorActionsButton() {
    $('#visitor-actions-ui-actionable-elements-without-identifiers').hide();
  }

  /**
   * Provide the HTML to create the modal dialog.
   */
  Drupal.theme.prototype.AcquiaLiftModalDialog = function () {
    var html = ''
    html += '  <div id="ctools-modal">'
    html += '    <div class="ctools-modal-content acquia-lift-modal">' // panels-modal-content
    html += '      <div class="modal-header">';
    html += '        <a class="close" href="#">';
    html +=            Drupal.CTools.Modal.currentSettings.closeText + Drupal.CTools.Modal.currentSettings.closeImage;
    html += '        </a>';
    html += '        <span id="modal-title" class="modal-title">&nbsp;</span>';
    html += '      </div>';
    html += '      <div id="modal-content" class="modal-content">';
    html += '      </div>';
    html += '    </div>';
    html += '  </div>';

    return html;
  }

}(Drupal.jQuery, Drupal));

/**
 * @file acquia_lift.variations.js
 *
 * General functionality required for all acquia_lift.variations application
 * components.
 */

/**
 * @file acquia_lift.elements.js
 */
(function($, Drupal) {

  Drupal.acquiaLiftVariations = Drupal.acquiaLiftVariations || {};
  Drupal.acquiaLiftVariations.app = Drupal.acquiaLiftVariations.app || {};

  /**
   * Gets a jQuery element array of all elements available for the DOM
   * selector.
   */
  Drupal.acquiaLiftVariations.getAvailableElements = function () {
    var ignoreRegions = Drupal.settings.acquia_lift.dom_selector_ignore;

    // Reduce the ignore region class list to a selector that includes
    // each region and all of its children, for example:
    // .page-top, .page-top *, .page-bottom, .page-bottom *
    var ignoreSelector = _.reduce(ignoreRegions, function (memo, current) {
      if (memo.length > 0) {
        memo += ', ';
      }
      return memo + '.' + current + ', .' + current + ' *';
    }, '');
    var $available = $('body').find('*').not(ignoreSelector).not('script, br, wbr, noscript').filter(function () {
      var el = this;
      var id = el.id || '';
      var className = typeof this.className === 'string' && this.className || '';
      var href = this.attributes['href'] && this.attributes['href'].value || '';
      // Eliminate any visitor actions components.
      var rVA = /^visitor-actions/;
      // Eliminate local tasks and contextual links.
      var rTask = /local-task|contextual/;
      // Eliminate admin links.
      var rAdmin = /^\/?admin/;
      // Eliminate node action links.
      var rNode = /^\/?node(\/)?(\d)*(?=\/add|\/edit|\/delete)/;
      // Reject the element if any tests match.
      if (rVA.test(id) || rTask.test(className) || rAdmin.test(href) || rNode.test(href)) {
        return false;
      }
      // Keep the element as the default.
      return true;
    })
    return $available;
  };

  /**
   * A command to trigger the page element selection process.
   *
   * The response should include a data object with the following keys:
   * - start: Boolean indicating if page variation mode should be on (true)
   *   or off (false).
   * - type: Indicates the type of variation mode: one of 'page' or 'element'.
   */
  Drupal.ajax.prototype.commands.acquia_lift_variation_toggle = function (ajax, response, status) {
    if (response.data.start) {
      initializeApplication();
      // Set the model to page or element variation mode.
      Drupal.acquiaLiftVariations.app.appModel.setModelMode(response.data.type === 'page');
      Drupal.acquiaLiftVariations.app.appModel.set('editMode', true);
      Drupal.acquiaLiftVariations.app.appModel.set('variation', null);
    } else {
      // End editing for the application.
      if (Drupal.acquiaLiftVariations.app.appModel) {
        Drupal.acquiaLiftVariations.app.appModel.set('editMode', false);
      }
    }
    // Let the other menu stuff clear out before we set a new variation mode.
    response.data.campaign = Drupal.settings.personalize.activeCampaign;
    _.defer(function () {
      $(document).trigger('acquiaLiftVariationMode', [response.data]);
    });
  };

  /**
   * A command to open a particular selector details form either to edit
   * an existing option or to add a new option to an existing option set on the
   * same selector/variation type.
   *
   * The response should include a data object with the following keys:
   * - type: Indicates the type of variation mode: one of 'page' or 'element'.
   * - variationType: The type of variation, e.g., editText, addClass, etc.
   * - selector: The selector for the affected DOM element.
   * - agentName: The machine name of the current campaign.
   * If type == page:
   * - variationIndex:  The variation index to edit.  A variationIndex of -1
   *   indicates creating a new variation.
   * If type == element
   * - osid: (optional) the option set id of an existing option set that is
   *   being modified either by adding a variation or by editing a variation
   *   within.
   */
  Drupal.ajax.prototype.commands.acquia_lift_variation_edit = function (ajax, response, status) {
    var data = response.data || {}, $selector = null;
    // Validate selector.
    try {
      if (data.selector) {
        var $selector = $(data.selector);
        // If the selector is not a unique match, then this can't proceed.
        // @todo: Log this using debugger tool.
        if ($selector.length !== 1) {
          return;
        }
      }
    } catch (err) {
      // @todo: Log this using debugger tool.
      // Selector is not correctly formatted.
      return;
    }
    // Validate variation type.
    if (!Drupal.settings.personalize_elements.contextualVariationTypes.hasOwnProperty(data.variationType)) {
      return;
    }
    var variationTypeData = Drupal.settings.personalize_elements.contextualVariationTypes[data.variationType];

    // Set up application.
    initializeApplication();
    // Set up the variation model for editing.
    var variation = null;
    if (data.type === 'page') {
      Drupal.acquiaLiftVariations.app.appModel.setModelMode(true);
      if (data.variationIndex && data.variationIndex >= 0) {
        variation = new Drupal.acquiaLiftVariations.models.PageVariationModel({
          variationIndex: data.variationIndex,
          agentName: data.agentName,
          selector: data.selector
        });
      }
    } else {
      Drupal.acquiaLiftVariations.app.appModel.setModelMode(false);
      if (data.variationIndex && data.variationIndex != -1) {
        variation = new Drupal.acquiaLiftVariations.models.ElementVariationModel({
          optionId: data.variationIndex,
          agentName: data.agentName,
          osid: data.osid
        });
      }
    }
    Drupal.acquiaLiftVariations.app.appModel.set('variation', variation);
    Drupal.acquiaLiftVariations.app.appModel.set('editMode', true);

    // Generate required event data for details form.
    var editEvent = {};
    editEvent.data = {
      anchor: $selector.get(0),
      id: data.variationType,
      limitByChildrenType: variationTypeData.limitByChildrenType,
      name: variationTypeData.name,
      selector: data.selector
    };
    if (data.osid) {
      editEvent.data.osid = data.osid;
    }

    // Open the view.
    Drupal.acquiaLiftVariations.app.appView.openExistingTypeDialog(editEvent);
  }

  /**
   * Helper function to initialize the application.
   */
  function initializeApplication() {
    // Initialize Backbone application.
    if (!Drupal.acquiaLiftVariations.app.appModel) {
      Drupal.acquiaLiftVariations.app.appModel = new Drupal.acquiaLiftVariations.models.AppModel();
    }
    if (!Drupal.acquiaLiftVariations.app.appView) {
      Drupal.acquiaLiftVariations.app.appView = new Drupal.acquiaLiftVariations.views.AppView({
        model: Drupal.acquiaLiftVariations.app.appModel,
        $el: $('body')
      });
    }
  }

  /**
   * Add an event listener for a page variation mode trigger request.
   *
   * This utilizes the custom toggle command in order to allow front-end and
   * back-end requests for the functionality to be handled the same way.
   */
  $(document).on('acquiaLiftPageVariationModeTrigger', function(e, data) {
    data['type'] = 'page';
    var response = {
      data: data
    };
    Drupal.ajax.prototype.commands.acquia_lift_variation_toggle(Drupal.ajax, response, 200);
  });

  /**
   * Add an event listener for an element variation set mode trigger request.
   *
   * This utilizes the custom toggle command in order to allow front-end
   * and back-end requests for the functionality to be handled the same way.
   */
  $(document).on('acquiaLiftElementVariationModeTrigger', function(e, data) {
    data['type'] = 'element';
    var response = {
      data: data
    };
    Drupal.ajax.prototype.commands.acquia_lift_variation_toggle(Drupal.ajax, response, 200);
  });

  /**
   * Add an event listener to open up a specific variation details form
   * for adding or editing an existing variation.
   *
   * Data is an object with the following keys:
   * - variationType: The type of variation, e.g., editText, addClass, etc.
   * - selector: The selector for the affected DOM element.
   * - osid: The option set id for the parent option set.
   * - variationIndex: (Optional) The choice id for the option to edit.
   */
  $(document).on('acquiaLiftElementVariationEdit', function(e, data) {
    data['type'] = 'element';
    var response = {
      data: data
    };
    Drupal.ajax.prototype.commands.acquia_lift_variation_edit(Drupal.ajax, response, 200);
  });


}(Drupal.jQuery, Drupal));


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

/**
 * @file acquia_lift.variations.collections.js
 * 
 * Backbone collections used for the variations application.
 */
(function($, Drupal, Backbone, _) {

  Drupal.acquiaLiftVariations.collections = Drupal.acquiaLiftVariations.collections || {
    ElementVariationCollection: Backbone.Collection.extend({
      model: Drupal.acquiaLiftVariations.models.VariationTypeModel,

      /**
       * Returns a filtered collection with only those variation types that
       * are relevent to the current element.
       *
       * For example, this is where it is determined that editText can only be
       * displayed based on particular child nodes.
       */
      applicableToElement: function ($element) {
        // Get all the node types of the children for the element.
        var childrenNodeTypes = _.pluck($element.find('*'), 'nodeType');

        return _(this.filter(function(data) {
          var limitByChildrenType = data.get('limitByChildrenType');

          // If there is a limit on the children type, make sure that every
          // child passes the test.
          if (limitByChildrenType && !isNaN(limitByChildrenType)) {
            var childMatch = _.every(childrenNodeTypes, function(type) {return type == limitByChildrenType});
            // Special limitations by node type.
            switch (parseInt(limitByChildrenType)) {
              case 3: {
                // Text nodes only - check for text within the parent element
                // if no child elements.
                return childrenNodeTypes.length == 0 ? $element.get(0).textContent.length > 0 : childMatch;
                break;
              }
              default: {
                return childMatch;
              }
            }
          }
          // No limits in place so include by default.
          return true;
        }))
      }
    })
  };

}(Drupal.jQuery, Drupal, Backbone, _));

/**
 * @file acquia_lift.variations.theme.js
 *
 * Theme functions used by the variations application.
 */
(function($, Drupal) {

  /**
   * Theme function to generate the title for a variations contextual menu.
   * @param options
   *   An object of options with a key for elementType.
   */
  Drupal.theme.acquiaLiftVariationsMenuTitle = function (options) {
    return '<h2>&lt;' + options.elementType + '&gt;</h2>';
  }

  /**
   * A theme function to generate the HTML for a single menu item link.
   *
   * @param object item
   *   An object with the following keys:
   *   - id: The type of menu option
   *   - name:  The label to display for this menu option
   */
  Drupal.theme.acquiaLiftVariationsMenuItem = function (item) {
    return '<a href="#" data-id="' + item.id + '">' + item.name + '</a>';
  }

  /**
   * Theme function to generate the title element for a variation type form.
   *
   * @param object item
   *   An object with the following keys:
   *   - elementType: the type of element that is being action on.
   *   - variationType: the type of variation to apply to the element.
   */
  Drupal.theme.acquiaLiftVariationsTypeFormTitle = function (item) {
    return '<h2>' + item.variationType + ': ' + '&lt;' + item.elementType + '&gt;</h2>';
  }

}(Drupal.jQuery, Drupal));

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

/**
 * @file acquia_lift.variations.editInContext.js
 * 
 * Provides the personalize elements edit in context functionality that allows
 * manipulation of the DOM for easier variation creation/editing.
 */
(function($, Drupal, Dialog, Backbone, _) {

  /**
   * Define editInContext behaviors to define what happens when creating
   * a particular personalize_element page variation in context.
   */
  Drupal.acquiaLiftVariations.personalizeElements = Drupal.acquiaLiftVariations.personalizeElements || {};

  /**
   * Whenever a variation type form is complete, call the personalize elements
   * editInContext callbacks.
   */
  $(document).on('acquiaLiftVariationTypeForm', function(e, type, selector, $input) {
    if ($input.val().length > 0) {
      // Don't replace any existing content in the input field.
      return;
    }
    if (Drupal.acquiaLiftVariations.personalizeElements.hasOwnProperty(type)
      && Drupal.acquiaLiftVariations.personalizeElements[type].hasOwnProperty('editInContext')
      && typeof Drupal.acquiaLiftVariations.personalizeElements[type].editInContext === 'function') {
      Drupal.acquiaLiftVariations.personalizeElements[type].editInContext(selector, $input);
    }
  });

  /****************************************************************
   *
   *          E D I T  H T M L
   *
   ****************************************************************/
  Drupal.acquiaLiftVariations.personalizeElements.editHtml = {
    getOuterHtml: function($element) {
      if ($element.length > 1) {
        $element = $element.first();
      }
      // jQuery doesn't have an outerHTML so we need to clone the child and
      // give it a parent so that we can call that parent's html function.
      // This ensures we get only the html of the $selector and not siblings.
      var $element = $element.clone().wrap('<div>').parent();
      // Remove any extraneous acquia lift / visitor actions stuff.
      var removeClasses = new RegExp(Drupal.settings.visitor_actions.ignoreClasses, 'g');
      var removeId = new RegExp(Drupal.settings.visitor_actions.ignoreIds);
      var removeTags = 'script';
      var removeAttributes = ['data-personalize'];

      // Remove any invalid ids.
      $element.find('[id]').filter(function() {
        return removeId.test(this.id);
      }).removeAttr('id');

      // Remove any classes that are marked for ignore.
      $element.find('[class]').each(function() {
        var stripClasses = this.className.match(removeClasses) || [];
        $(this).removeClass(stripClasses.join(' '));
        if (this.className.length == 0) {
          $(this).removeAttr('class');
        }
      });

      // Strip out any tags and styles if configured to do so.
      if (Drupal.settings.acquia_lift.edit_in_context_html_strip) {
        // Remove any styling added directly from jQuery.
        $element.find('[style]').removeAttr('style');

        // Remove any inappropriate tags
        $element.find(removeTags).remove();
      }

      // Remove any data attributes.
      _.each(removeAttributes, function(attr) {
        $element.find('[' + attr + ']').removeAttr(attr);
      });

      // Now return the cleaned up html.
      return $element.html();
    },
    editInContext : function(selector, $contentInput) {
      var editString = this.getOuterHtml($(selector));
      $contentInput.val(editString);
    }
  };

  /****************************************************************
   *
   *          E D I T  T E X T
   *
   ****************************************************************/
  Drupal.acquiaLiftVariations.personalizeElements.editText = {
    editInContext : function(selector, $contentInput) {
      var editString = $(selector).text();
      $contentInput.val(editString);
    }
  };

}(Drupal.jQuery, Drupal, Drupal.visitorActions.ui.dialog, Backbone, _));

/**
 * @file Override CTools modal.js in order to provide dynamic sizing
 * capabilities.  Whenever possible, the original functionality is preserved.
 *
 * These capabilities are being added to CTools and at that time this will no
 * longer be necessary.
 *
 * @see  https://www.drupal.org/node/1294478
 */

(function ($, Drupal) {
  // Make sure our objects are defined.
  Drupal.CTools = Drupal.CTools || {};
  Drupal.CTools.Modal = Drupal.CTools.Modal || {};

  /**
   * Display the modal
   */
  Drupal.CTools.Modal.show = function(choice) {
    var opts = {};

    if (choice && typeof choice == 'string' && Drupal.settings[choice]) {
      // This notation guarantees we are actually copying it.
      $.extend(true, opts, Drupal.settings[choice]);
    }
    else if (choice) {
      $.extend(true, opts, choice);
    }

    var defaults = {
      modalTheme: 'CToolsModalDialog',
      throbberTheme: 'CToolsModalThrobber',
      animation: 'show',
      animationSpeed: 'fast',
      modalSize: {
        type: 'scale',
        width: .8,
        height: .8,
        addWidth: 0,
        addHeight: 0,
        // How much to remove from the inner content to make space for the
        // theming.
        contentRight: 25,
        contentBottom: 45
      },
      modalOptions: {
        opacity: .55,
        background: '#fff'
      }
    };

    var settings = {};
    $.extend(true, settings, defaults, Drupal.settings.CToolsModal, opts);

    if (Drupal.CTools.Modal.currentSettings && Drupal.CTools.Modal.currentSettings != settings) {
      Drupal.CTools.Modal.modal.remove();
      Drupal.CTools.Modal.modal = null;
    }

    Drupal.CTools.Modal.currentSettings = settings;

    if (!Drupal.CTools.Modal.modal) {
      Drupal.CTools.Modal.modal = $(Drupal.theme(settings.modalTheme));
    }

    $('#modal-title', Drupal.CTools.Modal.modal).html(Drupal.CTools.Modal.currentSettings.loadingText);
    Drupal.CTools.Modal.modalContent(Drupal.CTools.Modal.modal, settings.modalOptions, settings.animation, settings.animationSpeed);
    $('#modal-content').html(Drupal.theme(settings.throbberTheme));

    $(window).trigger('resize');

    // Position autocomplete results based on the scroll position of the modal.
    $('#modal-content').delegate('input.form-autocomplete', 'keyup', function() {
      $('#autocomplete').css('top', $(this).position().top + $(this).outerHeight() + $(this).offsetParent().filter('#modal-content').scrollTop());
    });
  };

  // The following are implementations of AJAX responder commands.

  /**
   * AJAX responder command to place HTML within the modal.
   */
  var ctoolsModalDisplay = Drupal.CTools.Modal.modal_display;
  Drupal.CTools.Modal.modal_display = function(ajax, response, status) {
    ctoolsModalDisplay(ajax, response, status);
    // Trigger a resize event to make sure modal is in the right place.
    $(window).trigger('resize');
  }


  /**
   * modalContent
   * @param content string to display in the content box
   * @param css obj of css attributes
   * @param animation (fadeIn, slideDown, show)
   * @param speed (valid animation speeds slow, medium, fast or # in ms)
   */
  Drupal.CTools.Modal.modalContent = function(content, css, animation, speed) {
    // If our animation isn't set, make it just show/pop
    if (!animation) {
      animation = 'show';
    }
    else {
      // If our animation isn't "fadeIn" or "slideDown" then it always is show
      if (animation != 'fadeIn' && animation != 'slideDown') {
        animation = 'show';
      }
    }

    if (!speed) {
      speed = 'fast';
    }

    // Build our base attributes and allow them to be overriden
    css = jQuery.extend({
      position: 'absolute',
      left: '0px',
      margin: '0px',
      background: '#000',
      opacity: '.55'
    }, css);

    // Add opacity handling for IE.
    css.filter = 'alpha(opacity=' + (100 * css.opacity) + ')';
    content.hide();

    // if we already ahve a modalContent, remove it
    if ( $('#modalBackdrop')) $('#modalBackdrop').remove();
    if ( $('#modalContent')) $('#modalContent').remove();

    // Get our dimensions

    // Get the docHeight and (ugly hack) add 50 pixels to make sure we dont have a *visible* border below our div
    var docHeight = $(document).height() + 50;
    var docWidth = $(document).width();
    var winHeight = $(window).height();
    var winWidth = $(window).width();
    if( docHeight < winHeight ) docHeight = winHeight;

    // Create our divs
    $('body').append('<div id="modalBackdrop" style="z-index: 1000; display: none;"></div><div id="modalContent" style="z-index: 1001; position: absolute;">' + $(content).html() + '</div>');

    setSize = function(context) {
      var width = 0;
      var height = 0;

      if (Drupal.CTools.Modal.currentSettings.modalSize.type == 'scale') {
        width = $(window).width() * Drupal.CTools.Modal.currentSettings.modalSize.width;
        height = $(window).height() * Drupal.CTools.Modal.currentSettings.modalSize.height;
      } else {
        width = Drupal.CTools.Modal.currentSettings.modalSize.width;
        height = Drupal.CTools.Modal.currentSettings.modalSize.height;
      }
      if (Drupal.CTools.Modal.currentSettings.modalSize.type == 'dynamic') {
        // Use the additionol pixels for creating the width and height.
        $('div.ctools-modal-content', context).css({
          'min-width': Drupal.CTools.Modal.currentSettings.modalSize.width,
          'min-height': Drupal.CTools.Modal.currentSettings.modalSize.height,
          'width': 'auto',
          'height': 'auto'
        });
        $('#modalContent').css({'width': 'auto'});
      } else {
        // Use the additional pixels for creating the width and height.
        $('div.ctools-modal-content', context).css({
          'width': width + Drupal.CTools.Modal.currentSettings.modalSize.addWidth + 'px',
          'height': height + Drupal.CTools.Modal.currentSettings.modalSize.addHeight + 'px'
        });
        $('#modalContent', context).css({
          'width': width + Drupal.CTools.Modal.currentSettings.modalSize.addWidth + 'px',
          'height': height + Drupal.CTools.Modal.currentSettings.modalSize.addHeight + 'px'
        });
        $('div.ctools-modal-content .modal-content', context).css({
          'width': (width - Drupal.CTools.Modal.currentSettings.modalSize.contentRight) + 'px',
          'height': (height - Drupal.CTools.Modal.currentSettings.modalSize.contentBottom) + 'px'
        });
      }
    }

    setSize(document);

    // Keyboard and focus event handler ensures focus stays on modal elements only
    modalEventHandler = function( event ) {
      target = null;
      if ( event ) { //Mozilla
        target = event.target;
      } else { //IE
        event = window.event;
        target = event.srcElement;
      }

      var parents = $(target).parents().get();
      for (var i = 0; i < parents.length; ++i) {
        var position = $(parents[i]).css('position');
        if (position == 'absolute' || position == 'fixed') {
          return true;
        }
      }
      if( $(target).filter('*:visible').parents('#modalContent').size()) {
        // allow the event only if target is a visible child node of #modalContent
        return true;
      }
      if ( $('#modalContent')) $('#modalContent').get(0).focus();
      return false;
    };
    $('body').bind( 'focus', modalEventHandler );
    $('body').bind( 'keypress', modalEventHandler );

    // Create our content div, get the dimensions, and hide it
    var modalContent = $('#modalContent').css('top','-1000px');
    var mdcTop = Math.max($(document).scrollTop() + ( winHeight / 2 ) - (  modalContent.outerHeight() / 2), 10);
    var mdcLeft = Math.max(( winWidth / 2 ) - ( modalContent.outerWidth() / 2), 10);

    $('#modalBackdrop').css(css).css('top', 0).css('height', docHeight + 'px').css('width', docWidth + 'px').show();
    modalContent.css({top: mdcTop + 'px', left: mdcLeft + 'px'}).hide()[animation](speed, function () { /* $(window).trigger('resize'); */ });

    // Bind a click for closing the modalContent
    modalContentClose = function(){close(); return false;};
    $('.close').bind('click', modalContentClose);

    // Bind a keypress on escape for closing the modalContent
    modalEventEscapeCloseHandler = function(event) {
      if (event.keyCode == 27) {
        close();
        return false;
      }
    };

    $(document).bind('keydown', modalEventEscapeCloseHandler);

    // Close the open modal content and backdrop
    function close() {
      // Unbind the events
      $(window).unbind('resize',  modalContentResize);
      $('body').unbind( 'focus', modalEventHandler);
      $('body').unbind( 'keypress', modalEventHandler );
      $('.close').unbind('click', modalContentClose);
      $('body').unbind('keypress', modalEventEscapeCloseHandler);
      $(document).trigger('CToolsDetachBehaviors', $('#modalContent'));

      // Set our animation parameters and use them
      if ( animation == 'fadeIn' ) animation = 'fadeOut';
      if ( animation == 'slideDown' ) animation = 'slideUp';
      if ( animation == 'show' ) animation = 'hide';

      // Close the content
      modalContent.hide()[animation](speed);

      // Remove the content
      $('#modalContent').remove();
      $('#modalBackdrop').remove();
    };

    // Move and resize the modalBackdrop and modalContent on resize of the window
    modalContentResize = function(e){
      // When creating the modal, it actually exists only in a theoretical
      // place that is not in the DOM.  But once the modal exists, it is in the
      // DOM so the context must be set appropriately.
      var context = e ? document : Drupal.CTools.Modal.modal;

      setSize(context);

      // Get our heights
      var docHeight = $(document).height();
      var docWidth = $(document).width();
      var winHeight = $(window).height();
      var winWidth = $(window).width();
      if( docHeight < winHeight ) docHeight = winHeight;

      // Get where we should move content to
      var modalContent = $('#modalContent');

      var height = Math.max(modalContent.outerHeight(), $('div.ctools-modal-content', context).outerHeight());
      var width = Math.max(modalContent.outerWidth(), $('div.ctools-modal-content', context).outerWidth());

      var mdcTop = Math.max($(document).scrollTop() + ( winHeight / 2 ) - (  height / 2), 10);
      var mdcLeft = Math.max(( winWidth / 2 ) - ( width / 2), 10);

      // Apply attributes to fix the position of the modal relative to current
      // position of page. This is required when the modal is larger than the
      // browser window. This enables the modal to scroll with the rest of the
      // page, rather than remaining centered in the page whilst scrolling.
      if (height > $(window).height()) {
        if (e.type === 'resize') {
          // Is a resize event so get the position of top relative to current
          // position of document in browser window.
          mdcTop = 10 + $(document).scrollTop();
        } else if (e.type === 'scroll') {
          // Is a scroll event so mantain to current position of the modal
          // relative to page.
          var modalOffSet = modalContent.offset();
          mdcTop = modalOffSet.y;
        }
      }

      // Apply the changes
      $('#modalBackdrop').css({'height': winHeight + 'px', 'width': winWidth + 'px', 'top': $(document).scrollTop()}).show();
      modalContent.css('top', mdcTop + 'px').css('left', mdcLeft + 'px').show();
    };
    $(window).bind('resize', modalContentResize);
    $(window).bind('scroll', modalContentResize);

    $('#modalContent').focus();
  };

  var ctoolsUnmodalContent = Drupal.CTools.Modal.unmodalContent;
  Drupal.CTools.Modal.unmodalContent = function (content, animation, speed) {
    ctoolsUnmodalContent(content, animation, speed);
    $(window).unbind('scroll', modalContentResize);
  }

  Drupal.ajax.prototype.commands.modal_display = Drupal.CTools.Modal.modal_display;
  Drupal.ajax.prototype.commands.modal_dismiss = Drupal.CTools.Modal.modal_dismiss;

})(Drupal.jQuery, Drupal);

//# sourceMappingURL=acquia_lift.flow.js.map