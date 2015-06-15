(function ($, Drupal) {

  var pendingMessagesShown = false;

  Drupal.behaviors.AcquiaLiftMessageBox = {
    'attach': function(context, settings) {
      if (!settings.hasOwnProperty('acquia_lift')) {
        return;
      }
      // If any messages have been set to display on page load, show them.
      if (!pendingMessagesShown && settings.acquia_lift.pendingMessage && settings.acquia_lift.pendingMessage.length > 0) {
        showMessageBox(settings.acquia_lift.pendingMessage.join('<br />'), 0);
        pendingMessagesShown = true;
      }
    }
  };

  /**
   * JavaScript regarding the message box that is used for administrative
   * messages.
   */

  /**
   * Creates a message box if one is not present.
   *
   * @return
   *   The jQuery message box.
   */
  function createMessageBox() {
    var $messageBox = getMessageBox();
    if ($messageBox.length == 0) {
      $messageBox = $('<div id="acquia-lift-message-box"><div class="close"><a id="acquia-lift-message-box-close" href="#">' + Drupal.t('Close') + '</a></div><p class="message"></p></div>');
      $('body').prepend($messageBox);
      $messageBox.find('.close').on('click', closeMessageBox);
      // Don't close the message box if you click on it (other than close).
      $messageBox.on('click', function(e) {
        e.stopPropagation();
      });
    }
    return $messageBox;
  }

  /**
   * Close the message box.
   *
   * @param e
   *   (optional) The event that triggered the close.
   */
  function closeMessageBox(e) {
    var $messageBox = getMessageBox();
    $messageBox.animate({ height:0, opacity:0 }, "slow", function() {
      $(this).addClass('element-hidden');
      $(this).removeClass('acquia-lift-messagebox-shown');
      // Take off the height/opacity styles - only used for animation.
      $(this).removeAttr('style');
    });
    if (e && e.stopPropagation) {
      e.stopPropagation();
    }
    $(document).off('click', closeMessageBox);
  }

  /**
   * Helper function to retrieve the message box from the DOM if it exists.
   *
   * @returns jQuery match for message box selector
   */
  function getMessageBox() {
    return $('#acquia-lift-message-box');
  }

  /**
   * Shows the requested message within a message box.
   *
   * @param message
   *   The message to show.
   * @param seconds
   *   The number of seconds to show for.  If value = 0, then the message
   *   is shown until the user clicks to close it.
   */
  function showMessageBox(message, seconds) {
    var $messageBox = createMessageBox();
    $messageBox.find('.message').html(message);
    // Measure the final height while the box is still hidden.
    var fullHeight = $messageBox.outerHeight();
    // Reset the properties to animate so that it starts hidden.
    $messageBox.css('height', '0px');
    $messageBox.css('opacity', '0');
    $messageBox.removeClass('element-hidden');
    // Animate the box height and opacity to draw attention.
    $messageBox.animate({height: fullHeight + 'px', opacity: 1}, 'slow', function() {
      $(this).addClass('acquia-lift-messagebox-shown');
    });

    // Close the message box by clicking anywhere on the page.
    $(document).on('click', closeMessageBox);

    if (seconds > 0) {
      setTimeout(closeMessageBox, seconds*1000);
    }
  }

  /**
   * A Drupal AJAX command to display a message box.
   */
  Drupal.ajax.prototype.commands.acquia_lift_message_box = function (ajax, response, status) {
    response.data = response.data || {};
    var seconds = response.data.seconds || 0;
    var message = response.data.message || '';
    if (message.length >0) {
      showMessageBox(response.data.message, seconds);
    }
  };
})(Drupal.jQuery, Drupal);
