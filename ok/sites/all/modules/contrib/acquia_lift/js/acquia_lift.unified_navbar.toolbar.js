/**
 * @file
 * acquia_lift.toolbar.unified_navbar.js
 *
 * Provides functionality to integrate the "toolbar" module with the Acquia
 * Lift unified navigation bar.
 */
(function (Drupal, $, debounce, displace) {
  Drupal.navbar = Drupal.navbar || {};
  Drupal.behaviors.acquiaLiftUnifiedNavbarIntegration = {
    attach: function (context, settings) {
      var self = this;
      // Make the "Acquia Lift" top level navigation item toggle the
      // unified navbar.
      var $anchor = this.getAdminMenu().once('acquiaLiftOverride').find('a[href*="/admin/acquia_lift"]');
      if ($anchor.length && Drupal.navbar.hasOwnProperty('toggleUnifiedNavbar')) {
        $anchor.bind('click.acquiaLiftOverride', function (event) {
          event.stopPropagation();
          event.preventDefault();
          // Toggle the Acquia Lift unified navigation.
          Drupal.navbar.toggleUnifiedNavbar();
          self.updateUnifiedToolbarPosition(null, false);
        });
      }

      $('#navbar-administration').once('acquiaLiftToolbarIntegration', function() {
        // Update the padding offset of the unified navbar when the toolbar
        // height changes or re-orients.
        $(window).bind('resize.acquiaLiftToolbarResize', debounce(self.updateUnifiedToolbarPosition, 200));
        $(document).bind('drupalNavbarOrientationChange', self.updateUnifiedToolbarPosition);
        // Call it once to set the initial position.
        self.updateUnifiedToolbarPosition(null, false);
      });
    },

    /**
     * Called when the window resizes to recalculate the placement of the
     * unified toolbar beneath the main toolbar (which could have resized).
     *
     * Note that only the window broadcast resize events (not divs).
     *
     * @param e
     *   The event object.
     * @param dispatch
     *   True to dispatch displacement changes, false to ignore.
     */
    updateUnifiedToolbarPosition: function(e, dispatch) {
      var heightCss = self.getAdminMenu().css('height');
      // @todo: doesn't seem right to adjust all three.
      $('body.navbar-horizontal #toolbar + #navbar-administration.navbar-oriented').css('top', heightCss);
      $('body.navbar-horizontal #toolbar + #navbar-administration.navbar-oriented .navbar-bar').css('top', heightCss);
      $('body #toolbar + #navbar-administration.navbar-oriented .navbar-tray').css('top', heightCss);
      $('#navbar-bar.navbar-bar').attr('data-offset-top', parseInt(heightCss));
      dispatch = typeof(dispatch) == 'undefined' ? true : dispatch;
      if (dispatch) {
        displace(true);
      }
    },

    // Helper method to get admin menu container,
    // override it to provide support of custom admin menu.
    // By default supports https://drupal.org/project/admin_menu, https://drupal.org/project/admin
    // and default toolbar core module.
    // https://drupal.org/project/navbar is supported @see acquia_lift.navbar.js
    getAdminMenu: function () {
      return $('#toolbar');
    }
  }
  var self = Drupal.behaviors.acquiaLiftUnifiedNavbarIntegration;

}(Drupal, Drupal.jQuery, Drupal.debounce, Drupal.displace));
