/**
 * @file
 * Use Qtip to show help information in a tooltip.
 */
(function ($, Drupal) {
  "use strict";

  /**
   * Attach tooltips to elements with help data.
   */
  Drupal.behaviors.acquiaLiftHelp = {
    attach: function (context, settings) {
      $('[data-help-tooltip]').each(function() {
        $(this).once('acquia-lift-help', function() {
          var text = $(this).attr('data-help-tooltip'),
              trigger = $('<span class="acquia-lift-help">'),
              positionTooltip = 'bottomMiddle';

          // The tooltip will run out of room on the right.
          if ($(this).is(':last-child')) {
            positionTooltip = 'bottomRight';
          }

          $(trigger).qtip({
            content: text,
            position: {
              corner: {
                target: 'topMiddle',
                tooltip: positionTooltip
              }
            },
            adjust: {
              screen: true
            },
            style: {
              tip: true,
            }
          });
          $(this).append(trigger);
        })
      });
    }
  }
}(Drupal.jQuery, Drupal));

//# sourceMappingURL=acquia_lift.help.js.map