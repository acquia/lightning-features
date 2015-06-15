/**
 * @file
 * Add navigation to personalizable fields.
 */

(function ($) {
  /**
   * Namespace functions to prevent clashes.
   */
  var Personalize = Personalize || {
        fields: {
          init: {},
          collapse: {},
          navigate: {},
          update: {}
        }
      };

  /**
   * Build the navigation.
   */
  Personalize.fields.init = function (widget, settings) {
    // Collect and assemble the team.
    var actions = widget.find('.personalize-field-add').before($(Drupal.theme('personalizeFieldsNavigation'))),
        fields = widget.find('.personalize-field'),
        optionCount = fields.length,
        previousButton = widget.find('.personalize-field-previous'),
        nextButton = widget.find('.personalize-field-next'),
        current = widget.find('.personalize-field-current'),
        expandButton = widget.find('.personalize-field-expand'),
        collapseButton = widget.find('.personalize-field-collapse'),
        // Get the ID value up to a '--#' ID increment counter, taking into
        // account that the first instance does not have a '--#' counter.
        widgetID = /^.*(?=(?:--\d+))|.*$/.exec(widget.attr('id') || '') || '',
        settings = settings.personalize.fields[widgetID],
        status = $.extend({
            collapsed: 1
          },
          settings || {});

    // Retain the cleaned ID for updating the widget settings.
    widget.data('id', widgetID);

    // Update the count to accommodate for new option fields.
    status.total = optionCount;

    // Set the active item to the fist item on page load, and the last item
    // on each subsequent addition of a field. We use the presence of a
    // settings object as a flag to determine if this is the first or the N+1
    // page load.
    status.current = (settings) ? optionCount : 1;
    status.previous = status.current - 1;
    status.next = status.current + 1;

    // Hide the inactive options.
    fields.each(function (i) {
      var position = i + 1;
      $(this).addClass('personalize-field-position-' + position);
    });

    // Save the data outside the scope of this function.
    Drupal.settings.personalize.fields[widget.data('id')] = status;

    // Update navigation
    widget.data('personalizeFieldState', status);

    Personalize.fields.update(widget);

    // Bind navigation to clicks on the previous and next buttons.
    previousButton.bind('click', function(event) {
      Personalize.fields.navigate(widget, 'back');
      return false;
    });
    nextButton.bind('click', function(event) {
      Personalize.fields.navigate(widget, 'forward');
      return false;
    });

    // Bind the expand and collapse buttons to actions.
    expandButton.bind('click', function(event) {
      Personalize.fields.collapse(widget, 'expand');
      return false;
    });
    collapseButton.bind('click', function(event) {
      Personalize.fields.collapse(widget, 'collapse');
      return false;
    });
  };

  /**
   * Increment or decrement the navigation.
   */
  Personalize.fields.navigate = function (widget, direction) {
    var status = widget.data('personalizeFieldState');

    // Adjust the status and update the display.
    if (direction == 'forward' && status.next <= status.total) {
      status.previous = status.current;
      status.current = status.current + 1;
      status.next = status.current + 1;
    }
    else if (direction == 'back' && status.previous > 0) {
      status.next = status.current;
      status.current = status.current - 1;
      status.previous = status.current - 1;
    }
    Personalize.fields.update(widget);
  };

  /**
   * Collapse and expand visible options.
   */
  Personalize.fields.collapse = function (widget, direction) {
    var status = widget.data('personalizeFieldState');

    if (direction == 'collapse') {
      status.collapsed = 1;
    }
    else if (direction == 'expand') {
      status.collapsed = 0;
    }

    Personalize.fields.update(widget);
  };

  /**
   * Update the visible elements based on the navigation position.
   */
  Personalize.fields.update = function (widget) {
    var status = widget.data('personalizeFieldState'),
        previousButton = widget.find('.personalize-field-previous'),
        nextButton = widget.find('.personalize-field-next'),
        expandButton = widget.find('.personalize-field-expand'),
        collapseButton = widget.find('.personalize-field-collapse'),
        active = widget.find('.personalize-field-position-' + status.current),
        inactive = active.siblings('.personalize-field');

    // Update the collapsed status.
    if (status.collapsed) {
      // Hide inactive fields.
      active.removeClass('personalize-option-hidden').addClass('personalize-field-active');
      inactive.removeClass('personalize-field-active').addClass('personalize-option-hidden');
      collapseButton.attr('disabled', 'disabled');
      expandButton.removeAttr('disabled');

      // Add a status class to the widget for styling purposes.
      widget.addClass('personalize-fields-expanded').removeClass('personalize-fields-collapsed');

      // Disable navigation buttons if there is nowhere to go.
      if (status.previous <= 0) {
        previousButton.attr('disabled', 'disabled');
      }
      else if (status.previous > 0) {
        previousButton.removeAttr('disabled');
      }
      if (status.next > status.total || status.current == status.next) {
        nextButton.attr('disabled', 'disabled');
      }
      else if (status.next <= status.total) {
        nextButton.removeAttr('disabled');
      }
      // Changed to the counter to an 'of' counter.
      widget.find('.personalize-field-counter').html(Drupal.theme('personalizeFieldsOfCounter', {
        active: status.current,
        total: status.total
      }));
    }
    else {
      // Add a status class to the widget for styling purposes.
      widget.addClass('personalize-fields-collapsed').removeClass('personalize-fields-expanded');

      // Make all of the options visible.
      widget.find('.personalize-field').removeClass('personalize-option-hidden').addClass('personalize-field-active');
      expandButton.attr('disabled', 'disabled');
      collapseButton.removeAttr('disabled');
      previousButton.attr('disabled', 'disabled');
      nextButton.attr('disabled', 'disabled');
      // Changed to the counter to a 'total' counter.
      widget.find('.personalize-field-counter').html(Drupal.theme('personalizeFieldsTotalCounter', {
        total: status.total
      }));
    }

    Drupal.settings.personalize.fields[widget.data('id')] = status;
  };

  /**
   * Returns the navigation markup for personalizable field navigation.
   */
  Drupal.theme.personalizeFieldsNavigation = function () {
    var output = '';

    output += '<div class="personalize-field-navigation">';
    output += '<button class="personalize-field-expand">' + Drupal.t('Show all options') + '</button>';
    output += '<button class="personalize-field-collapse">' + Drupal.t('Collapse options') + '</button>';
    output += '<button class="personalize-field-previous">' + Drupal.t('Previous option') + '</button> ';
    output += '<span class="personalize-field-counter">';
    output += '</span>';
    output += '<button class="personalize-field-next">' + Drupal.t('Next option') + '</button>';
    output += '</div>';

    return output;
  }

  /**
   * Returns the string for a "# of total" counter.
   */
  Drupal.theme.personalizeFieldsOfCounter = function (options) {
    return '<strong>' + options.active + '</strong> ' + Drupal.t('of') + ' <span class="personalize-field-total">' + options.total + '</span>';
  };

  /**
   * Returns a string for a "# options" count.
   */
  Drupal.theme.personalizeFieldsTotalCounter = function (options) {
    return '<span class="personalize-field-total">' + Drupal.formatPlural(options.total, '<strong>@count</strong> option', '<strong>@count</strong> options') + '</span>';
  };

  /**
   * Looks for personalizable fields and adds a special widget
   * for configuring the options.
   */
  Drupal.behaviors.acquia_lift_fields = {
    attach: function (context, settings) {
      settings.personalize = settings.personalize || {};
      settings.personalize.fields = settings.personalize.fields || {};

      var $fieldGroups = $(context).find('div[id*="-add-more-wrapper"]')
        // The context is the field group when returned from an ajax call to
        // add another item, so add it back.
        .addBack()
        // Filter for add-more-wrappers that have the expected personalizable-
        // field parent element.
        .filter(function (index) {
          return this.parentNode && this.parentNode.className && /personalizable-field/.test(this.parentNode.className);
        })
        // When updated by an AJAX add option call, this element will be removed
        // and replaced, so filtering it with once will ensure we process
        // only fresh DOM nodes.
        .once('personalize-field');

      // Process pesonalizeable field groups.
      if ($fieldGroups.length > 0) {
        $fieldGroups.each(function () {
          Personalize.fields.init($(this), settings);
        });
      }
    }
  };

  // Get ready for jQuery 1.8.
  jQuery.fn.addBack = jQuery.fn.addBack || jQuery.fn.andSelf;

})(Drupal.jQuery);
