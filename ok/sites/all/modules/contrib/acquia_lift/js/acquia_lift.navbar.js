/**
 * @file
 * personalize_ui.navbar.js
 */

(function ($, Drupal) {

"use strict";

Drupal.behaviors.acquiaLiftNavbarMenu = {
  attach: function (context) {
    if ('drupalNavbarMenu' in $.fn) {
      $('.navbar-menu-acquia-lift-controls')
        .children('.menu, .navbar-menu')
        .drupalNavbarMenu({
          activeTrail: false,
          findItem: function ($list, $menu) {
            var $items = $list.children('li');
            var $wrappedItems = $list.children().not('li').children('li');
            $items = $items.add($wrappedItems);
            if ($items.length) {
              return $items;
            }
          },
          findItemElement: function ($item, $menu) {
            var $campaigns = $item.children('div.acquia-lift-menu-item');
            var $contentVariations = $item.children('.acquia-lift-content-variation');
            var $handle = $campaigns.add($contentVariations);
            if ($handle.length) {
              return $handle;
            }
          },
          findItemSubMenu: function ($item, $menu) {
            var $subMenus = $item.find('ul');
            var $wrappedSubMenus = $item.children().not('ul').children('ul');
            $subMenus = $subMenus.add($wrappedSubMenus);
            if ($subMenus.length) {
              return $subMenus;
            }
          }
        });
    }
  }
};

}(Drupal.jQuery, Drupal));
