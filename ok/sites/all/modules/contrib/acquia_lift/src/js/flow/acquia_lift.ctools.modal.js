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
