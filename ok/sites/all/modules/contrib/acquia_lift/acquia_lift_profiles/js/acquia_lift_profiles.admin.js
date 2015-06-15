(function ($) {
  /**
   * Vertical tab display within content types.
   */
  Drupal.behaviors.AcquiaLiftProfilesAdmin = {
    attach: function (context) {
      if ($.fn.drupalSetSummary) {
        // Provide the vertical tab summaries.
        $('fieldset[id^="edit-acquia-lift-profiles"]', context).drupalSetSummary(function(context) {
          var val = 'Image thumbnail: ';
          val += $('select[name="acquia_lift_profiles[thumbnail]"] option:selected', context).text();
          return val;
        });
      }

      // Code to show/hide extra UDF mapping elements.
      $('div.acquia-lift-profiles-hidden-udfs').once(function(){
        $(this).hide();
      });
      $('input.acquia-lift-profiles-udf-show-all').once(function(){
        $(this).bind('click', function() {
          var action, text;
          if ($(this).val() == Drupal.t('Show all')) {
            action = 'show';
            text = Drupal.t('Show fewer');
          }
          else {
            action = 'hide';
            text = Drupal.t('Show all');
          }
          $(this).parent().find('div.acquia-lift-profiles-hidden-udfs')[action]();
          $(this).val(text);
          return false;
        });
      });

      // Positions the Web Admin menu link to the right.
      $('a.acquia-lift-web-admin').once(function() {
        var $parentLi = $(this).parents('li');
        var $previousLi = $parentLi.prev('li');
        $parentLi.addClass('acquia-lift-navbar-secondary');
        $previousLi.addClass('acquia-lift-navbar-marker');
      });
    }
  };

})(jQuery);
