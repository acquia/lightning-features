/**
 * A generic DOM selector that allows the selection of an element and triggers
 * an event upon selection.
 *
 * Inspired by https://github.com/conductrics/dom-selector
 */
;(function ($, Utilities) {

  /**
   * jQuery plugin definition.
   */
  var pluginName = 'DOMSelector',
    indicatorClass = 'acquia-lift-active-element',
    selectorIgnoreClasses = new RegExp(Drupal.settings.visitor_actions.ignoreClasses);
    selectorIgnoreId = tipIgnoreId = new RegExp(Drupal.settings.visitor_actions.ignoreIds);

  defaults = {
      hoverClass: 'acquia-lift-dom-highlight',
      onElementSelect: function (element, selector) {
        console.log('selected: ' + selector);
      },
      onError: function (message) {
        console.log(message);
      }
    };

  function Plugin (element, options) {
    // Allow the initializer to specify the elements to watch or just default
    // to the element and all of its children.
    if (options.$watchElements) {
      this.$element = options.$watchElements;
    } else {
      var $matched;
      if ($(element).addBack) {
        $matched = $(element).find('*').addBack();
      } else {
        $matched = $(element).find('*').andSelf();
      }
      this.$element = $matched;
    }

    this.settings = $.extend({}, defaults, options);
    this._defaults = defaults;
    this._name = pluginName;
    this.init();
  }

  $.extend(Plugin.prototype, {
    /**
     * Initialization logic.
     */
    init: function() {
      this._hovered.hoverClass = this.settings.hoverClass;
      this._watching = false;
    },

    /**
     * Determine tip content for an element
     * @returns {*|HTMLElement}
     */
    getTipContent: function (element) {
      if (element.hasOwnProperty('id') && element.id.length > 0 && !tipIgnoreId.test(element.id)) {
        return element.id;
      } else {
        return '&lt;' + element.nodeName.toLowerCase() + '&gt;';
      }
    },

    /**
     * Enables DOM watching capabilities.
     *
     * @returns the current jQuery element.
     */
    startWatching: function() {
      this.$element.bind('mousemove', $.proxy(this, '_onMouseMove'));
      this.$element.bind('click', $.proxy(this, '_onClick'));
      this.$element.each(function() {
        $(this).qtip({
          content: Plugin.prototype.getTipContent(this),
          solo: true,
          position: {
            target: 'mouse',
            adjust: {
              mouse: true
            }
          },
          // Let the show event remain at mouseover to allow for deferred
          // instantiation, but handle only showing when highlighted via the
          // beforeShow callback.
          show: {
            delay: 0,
            effect: {
              type: 'show',
              length: 0
            },
            when: {
              event: 'mouseover'
            }
          },
          hide: {
            delay: 0,
            when: false,
            effect: {
              type: 'hide',
              length: 0
            },
            when: {
              event: 'mouseout'
            }
          },
          api: {
            beforeShow: function() {
              return this.elements.target.hasClass(indicatorClass);
            }
          }
        });
      });
      this._watching = true;
      return this.$element;
    },

    /**
     * Disables DOM watching capabilities.
     *
     * @returns the current jQuery element.
     */
    stopWatching: function() {
      this._hovered.unhighlight();
      this.$element.unbind('mousemove', this._onMouseMove);
      this.$element.unbind('click', this._onClick);
      // QTip has some problems fully removing itself so help it.
      // NOTE that QTips don't properly re-enable so disabling is not an option.
      this.$element.each(function() {
        if (typeof $(this).data('qtip') !== 'undefined') {
          $(this).qtip('destroy');
          $(this).unbind('.qtip');
          // Sadly just unbinding qtip namespaced events doesn't grab it all.
          // Would rather than just unbind mouseover but since this only happens
          // in administration of a variation it will do for now.
          // @todo Find a way to preserve the origin element's mouseover event.
          $(this).unbind('mouseover');
        }
      });
      $.fn.qtip.interfaces.length = 0;

      this._watching = false;
      return this.$element;
    },

    /**
     * Returns if the selector is currently watching the DOM.
     */
    isWatching: function() {
      return this._watching;
    },

    /**
     * Update the watched elements
     */
    updateElements: function($updated) {
      this.$element = $updated;
    },

    /**
     * Highlight functionality for a hovered element.
     */
    _hovered: {
      // The element that is currently hovered.
      $element: null,
      // The CSS for the element on hover.
      hoverClass: '',

      // Unhighlight the element.
      unhighlight: function() {
        if (this.$element != null) {
          this.$element.qtip("hide");
          this.$element.removeClass(indicatorClass);
          this.$element.removeClass(this.hoverClass);
        }
        return this.$element = null;
      },

      // Highlight the element.
      highlight: function() {
        if (this.$element != null) {
          this.$element.addClass(indicatorClass);
          this.$element.addClass(this.hoverClass)
          this.$element.qtip("show");
          return this.$element;
        }
      },

      // Update what element is the current for hover functionality.
      update: function(target) {
        if (target === null || typeof target === 'undefined' || (this.$element && target === this.$element[0])) {
          return;
        }
        // If the element wasn't initialized with a qTip, then it's not one of
        // the elements available for selection.
        if (!$(target).data('qtip')) {
          return;
        }
        this.unhighlight();
        this.$element = $(target);
        this.highlight();
      }
    },

    /**
     * Event listener for a mouse move event.
     *
     * Update the hover element to the current element being moused over.
     */
    _onMouseMove: function(event) {
      this._hovered.update(event.target);
    },

    /**
     * Event listener for an element click event.
     */
    _onClick: function(event) {
      var $selected = this._hovered.$element;
      var hoverClass = this._hovered.hoverClass;
      if (!$selected) {
        return;
      }
      if ($selected.length != 1) {
        this.settings.onError.call(this, Drupal.t('Invalid element selector.'));
      } else {
        // Remove the indicator class so it doesn't show in the final selector.
        $selected.removeClass(indicatorClass);
        $selected.removeClass(hoverClass);
        this.settings.onElementSelect.call(this, $selected[0], Utilities.getSelector($selected[0], selectorIgnoreId, selectorIgnoreClasses));
      }
      event.preventDefault();
      event.stopPropagation();
      event.cancelBubble = true;
      return false;
    }
  })

  /**
   * Add DOMSelector to jQuery and protect against multiple instantiations.
   */
  $.fn.DOMSelector = function (options) {
    var args = arguments;
    // Create a new plugin.
    if (options === undefined || typeof options === 'object') {
      return this.each(function () {
        // Only allow the plugin to be instantiated once.
        if (!$.data(this, 'plugin_' + pluginName)) {
          $.data(this, 'plugin_' + pluginName, new Plugin(this, options));
        }
      });
      // If the first parameter is a string and it doesn't start
      // with an underscore and isn't the init function,
      // treat this as a call to a public method.
    } else if (typeof options === 'string' && options[0] !== '_' && options !== 'init') {
      var returns;

      this.each(function () {
        var instance = $.data(this, 'plugin_' + pluginName);
        if (instance instanceof Plugin && typeof instance[options] === 'function') {
          returns = instance[options].apply( instance, Array.prototype.slice.call( args, 1 ) );
        }

        // Allow instances to be destroyed via the 'destroy' method
        if (options === 'destroy') {
          $.data(this, 'plugin_' + pluginName, null);
        }
      });

      return returns !== undefined ? returns : this;
    }
  };
})(Drupal.jQuery, Drupal.utilities);
