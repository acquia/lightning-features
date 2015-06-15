(function ($) {

  /**
   * Add the 'chosen' behavior to multiselect elements.
   */
  Drupal.behaviors.acquia_lift_admin = {
    attach: function (context, settings) {
      $('.acquia-lift-chosen-select', context).each(function() {
        var chosenWidth = '940px';
        if ($(this).hasClass('acquia-lift-chosen-select-half')) {
          chosenWidth = '470px';
        }
        var options = {
          width: chosenWidth
        };
        $(this).chosen(options);
      });

      // Listener for context selection.
      $contextSelect = $('.acquia-lift-report-section .acquia-lift-report-context-select', context);
      if ($contextSelect.length > 0) {
        $contextSelect.once().chosen().change(function(e) {
          var selected = $(this).val(),
            num = selected ? selected.length : 0,
            selectors = [],
            $report = $(this).parents('.acquia-lift-report-section').find('table tbody');

          // If nothing is selected, then show all.
          if (num == 0) {
            $report.find('tr').show();
            return;
          }
          for (var i=0; i<num; i++) {
            selectors.push('tr[data-acquia-lift-feature="' + selected[i] + '"]');
          }
          var selector = selectors.join(', ');
          $report.find('tr').not(selector).hide();
          $report.find(selector).show();
        });
      }
    }
  };

  /**
   * Campaign edit form behaviors.
   */
  Drupal.behaviors.acquiaLiftCampaignEdit = {
    attach: function (context, settings) {
      // Add a handler to the "Reset data" button to warn the user before resetting the data.
      $('#personalize-acquia-lift-reset input[type="submit"]').once('acquia-lift-reset').each(function() {
        // Overwrite beforeSubmit of the ajax event.
        Drupal.ajax[this.id].options.beforeSubmit = function(form_values, $element, options) {
          if (confirm(Drupal.t('This action will delete all existing data for this campaign and cannot be undone. Are you sure you want to continue?'))) {
            return true;
          } else {
            return false;
          }
        }
      });
    }
  }

  /**
   * Adjust fills for percentage line fill components.
   */
  Drupal.behaviors.acquia_lift_percentage_label = {
    attach: function(context, settings) {
      $('.acquia-lift-distribution .distribution-graphic .fill', context).once().each(function() {
        var percent = $(this).attr('data-acquia-lift-fill');
        if (isNaN(parseFloat(percent))) {
          return;
        }
        $(this).css('width', percent + '%');
      });
    }
  }

  /**
   * Adjusts the display of high-low components.
   */
  Drupal.behaviors.acquia_lift_high_low = {
    attach: function(context, settings) {
      // Determine the min and max bounds for the component.
      var maxBound = 0;
      var minBound = NaN; // Don't set this to 0 or it will always be the min.
      $('.acquia-lift-hilo-bounds').each(function() {
        var lo = parseFloat($(this).attr('data-acquia-lift-low'));
        if (isNaN(minBound)) {
          minBound = lo;
        } else {
          minBound = Math.min(minBound, lo);
        }
        maxBound = Math.max(maxBound, parseFloat($(this).attr('data-acquia-lift-high')));
      });

      // Now adjust the bounds display for each high-low component.
      $('.acquia-lift-hilo-estimate', context).each(function() {
        var $bounds = $('.acquia-lift-hilo-bounds', this);
        var high = parseFloat($bounds.attr('data-acquia-lift-high'));
        var low = parseFloat($bounds.attr('data-acquia-lift-low'));
        var estimate = parseFloat($('.acquia-lift-badge', this).text());
        if (isNaN(high) || isNaN(low) || isNaN(estimate) || (high === 0 && low === 0)) {
          $bounds.css('height', 0);
          return;
        }
        var scale = $(this).width() / (maxBound - minBound);
        var scaleLow = (low - minBound) * scale;
        var scaleHigh = (high - minBound) * scale;
        var width = Math.max((scaleHigh - scaleLow), 20);
        $bounds.css('width', width + 'px');
        $bounds.css('left', scaleLow + 'px');
      });
    }
  }

  /**
   * Remove any duplicated message display areas.
   */
  Drupal.behaviors.acquiaLiftDSM = {
    attach: function(context, settings) {
      $newMessages = $('div.messages', context);
      if ($newMessages.length === 0) {
        return;
      }
      $priorMessages = $('div.messages').not($newMessages).hide();
    }
  }
})(Drupal.jQuery);
