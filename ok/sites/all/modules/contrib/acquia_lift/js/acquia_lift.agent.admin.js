(function ($) {
  /**
   * Allows one control to adjust the percentage while another
   * displays the amount left.
   */
  Drupal.behaviors.acquiaLiftAgentPercentageControls = {
    attach: function(context, settings) {
      $('div.acquia-lift-percentage-control', context).once(function() {
        var $rest = $('.acquia-lift-percentage-rest-display', this);
        var $input = $('input.acquia-lift-percentage-control', this);

        var updateRest = function() {
          var newPercent = parseFloat($input.val());
          if (isNaN(newPercent) || newPercent < 0 || newPercent > 100) {
            $rest.text(Drupal.t('Invalid percentage'));
            $rest.addClass('error');
          } else {
            // Limit the "rest" display to 2 decimal places.
            var newRest = (100 - newPercent).toFixed(2);
            // Calling parseFloat again here to trim off any trailing zeros.
            $rest.text(parseFloat(newRest) + '%');
            $rest.removeClass('error');
          }
        }

        // Update the "rest" percentage box when percentage changes.
        $('input.acquia-lift-percentage-control', this).change(updateRest);

        // Add a timer to update the "rest" while the user is in the percentage
        // input box.
        var updateTimeout = NaN;
        function clearUpdateTimeout() {
          if (!isNaN(updateTimeout)) {
            clearInterval(updateTimeout);
            updateTimeout = NaN;
          }
        }

        $('input.acquia-lift-percentage-control', this).focusin(function(e) {
          clearUpdateTimeout();
          updateTimeout = setInterval(updateRest, 1000);
        });

        $('input.acquia-lift-percentage-control', this).focusout(clearUpdateTimeout);

        // Set initial "rest" percentage value.
        updateRest();
      })
    }
  };

  /**
   * Adds ability to collapse description for form elements.
   */
  Drupal.behaviors.acquiaLiftShowHide = {
    attach: function(context, settings) {
      var showText = Drupal.t('Info');
      var hideText = Drupal.t('Hide info');
      $('.acquia-lift-collapsible', context).once(function() {
        var $container = $(this);
        $container.prepend('<a class="acquia-lift-toggle-text" href="#"></a>');

        $('.acquia-lift-toggle-text', this).click(function(e) {
          if ($container.hasClass('acquia-lift-collapsed')) {
            // Open and show additional details.
            $('.description', $container).slideDown();
            $container.removeClass('acquia-lift-collapsed');
            $(this).text(Drupal.t('Hide info'));
          } else {
            $container.addClass('acquia-lift-collapsed');
            $('.description', $container).slideUp();
            $(this).text(Drupal.t('Info'));
          }
          return false;
        });

        // Hide descriptions if collapsed by default.
        if ($container.hasClass('acquia-lift-collapsed')) {
          $('.description', $container).hide();
          $('.acquia-lift-toggle-text', this).text(showText);
        } else {
          $('.acquia-lift-toggle-text', this).text(hideText);
        }
      });
    }
  }
})(Drupal.jQuery);
