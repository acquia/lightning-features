/**
 * @file acquia_lift.unified_navbar.admin_menu.js
 *
 * Provides functionality to integrate the "admin_menu" module with the Acquia
 * Lift unified navigation bar.
 */
(function (Drupal, $, debounce, displace) {
  Drupal.navbar = Drupal.navbar || {};
  Drupal.behaviors.acquiaLiftUnifiedNavbarIntegration = {
    attach: function (context, settings) {
      var self = this;
      // Make the "Acquia Lift" top level navigation item toggle the
      // unified navbar.
      $('#navbar-administration').once('acquiaLiftAdminMenuIntegration', function() {
        Drupal.navbar.calculateDynamicOffset(false);
        var adminMenu = self.getAdminMenu();
        if (adminMenu.length == 0) {
          // The admin_menu does not broadcast when it is added and does not
          // re-load Drupal behaviors.  It does, however, trigger a window resize
          // event when loaded in order to adjust placement.
          $(window).bind('resize.AcquiaLiftAdminMenuWait', self.checkForAdminMenu);
          return;
        }
        // If the admin menu is loaded then go ahead and add the unified navbar.
        self.checkForAdminMenu();
      });
    },

    /**
     * Attach the Acquia Lift navbar listeners once the admin menu is present.
     *
     * @param e
     *   The triggering event.
     */
    checkForAdminMenu: function (e) {
      var adminMenu = self.getAdminMenu();
      if (adminMenu.length == 0) {
        return;
      }
      // If the link is found and not processed, then go ahead and add
      // listeners.
      var anchorSelector = self.getAdminMenuSelector() + ' a[href*="' + Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/acquia_lift"]';
      if ($(anchorSelector).length == 0 || !Drupal.navbar.hasOwnProperty('toggleUnifiedNavbar')) {
        return;
      }
      // Must use "live" event delegation here as the admin menu can be
      // attached multiple times when pulled from cache.
      $(document).delegate(anchorSelector, 'click', function acquiaLiftClickHandler(event) {
        // Make sure this is triggered by the right link (side-effect of method
        // and multiple menus).
        if (!event.target.href || event.target.href.indexOf('admin/acquia_lift') < 0) {
          return;
        }
        event.preventDefault();
        // Toggle the Acquia Lift unified navigation.
        Drupal.navbar.toggleUnifiedNavbar();
        self.updateUnifiedToolbarPosition(null, false);
      });
      $(window).unbind('resize.AcquiaLiftAdminMenuWait', self.checkForAdminMenu);

      // Update the padding offset of the unified navbar when the admin_menu
      // height changes or re-orients.
      $(window).bind('resize.acquiaLiftAdminMenuResize', debounce(self.updateUnifiedToolbarPosition, 200));
      $(document).bind('drupalNavbarOrientationChange', self.updateUnifiedToolbarPosition);
      $(document).bind('drupalNavbarTrayActiveChange', self.updateUnifiedToolbarPosition);
      // Call it once to set the initial position.
      self.updateUnifiedToolbarPosition(null, false);
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
      var $tray = $('div#navbar-item-tray.navbar-tray-acquia-lift.navbar-tray');
      $('body.navbar-horizontal #navbar-administration.navbar-oriented').css('top', heightCss);
      $('body #navbar-administration.navbar-oriented .navbar-tray').css('top', heightCss);
      // Because the admin_menu is positioned via margins we need to ignore
      // it when specifying the displacement for body content and explicitly
      // define the displacement top based on the size of the unified navbar.
      $('#navbar-bar.navbar-bar').attr('data-offset-top', 0);
      if ($tray.hasClass('navbar-tray-horizontal') && $tray.hasClass('navbar-active')) {
        $tray.attr('data-offset-top', $tray.height());
      } else {
        $tray.removeAttr('data-offset-top');
      }
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
      return $(self.getAdminMenuSelector());
    },

    /**
     * Helper function to get the admin menu selector string.
     */
    getAdminMenuSelector: function () {
      return '#admin-menu';
    }
  }
  var self = Drupal.behaviors.acquiaLiftUnifiedNavbarIntegration;

}(Drupal, Drupal.jQuery, Drupal.debounce, Drupal.displace));
