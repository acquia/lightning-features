/**
 * Functionality related specifically to the modal campaign management
 * procedures.
 */
(function ($, Drupal) {
  "use strict";

  Drupal.behaviors.acquiaLiftTypeModal = {
    attach: function (context, settings) {
      // Make the whole campaign type div clickable.
      $('div.ctools-modal-content .modal-content .acquia-lift-type', context).once(function() {
        $(this).on('click', function(e) {
          var $link = $(this).find('a.acquia-lift-type-select');
          // Special handling based on href values.
          if ($link.attr('href') == settings.basePath + settings.pathPrefix + 'admin/structure/visitor_actions') {
            // Trigger goals in context.
            $('#acquiaLiftVisitorActionsConnector').find('a').trigger('click');
            Drupal.CTools.Modal.dismiss();
            e.preventDefault();
            e.stopImmediatePropagation();
          } else if ($link.attr('href') == settings.basePath + settings.pathPrefix + 'admin/structure/personalize/variations/personalize-elements/add') {
            // Trigger variations in context.
            $(document).trigger('acquiaLiftElementVariationModeTrigger', [{start: true}]);
            Drupal.CTools.Modal.dismiss();
            e.preventDefault();
            e.stopImmediatePropagation();
          } else if ($link.hasClass('ctools-use-modal')) {
            // It needs to be the link that is triggered if we want CTools to
            // take over.
            if (!$(e.currentTarget).is('a')) {
              $link.trigger('click');
            }
            // Let the event bubble on to the next handler.
            return;
          } else {
            // If it's a regular link, then we also need to set the new location.
            window.location = $link.attr('href');
          }
        })
      });
      hidePageVisitorActionsButton();
      // When visitor actions is activated, remove the page actions button
      // because the user is selecting this through the modal process.
      $('body').once('acquiaLiftVisitorActionsHidePage', function() {
        $(document).bind('visitorActionsUIEditMode', function (event, isActive) {
          if (isActive) {
            hidePageVisitorActionsButton();
          }
        });
      });

      // The visitor actions ui application expects there to always be a
      // trigger link on the page, but with the modal process the trigger would
      // disappear when the modal closes.  We create a hidden trigger link
      // to handle the edit mode toggle.
      var $connector = $('#acquiaLiftVisitorActionsConnector');
      if ($connector.length == 0) {
        $('body').append('<div id="acquiaLiftVisitorActionsConnector"><a href="' + Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/visitor_actions/add" class="element-hidden">' + Drupal.t('Add goals') + '</a></div>');
        // Allow visitor actions UI to process the link.
        Drupal.attachBehaviors($('#acquiaLiftVisitorActionsConnector'));
        $(document).on('acquiaLiftVisitorActionsConnectorToggle', function(e) {
          $('#acquiaLiftVisitorActionsConnector').find('a').trigger('click');
        });
      }

      // Provide method to hide full selector in variation type details form
      // until the user selects to edit.
      // Note that the form is sent as the new context so we can't just check
      // within the context.
      // Note that the selector input may not be available if the user isn't
      // able to edit its contents.
      var $variationTypeForm = $('#acquia-lift-element-variation-details-form').not('.acquia-lift-processed');
      var $selectorInput = $variationTypeForm.find('input[name="selector"]');

      if ($variationTypeForm.length > 0 && $selectorInput.length > 0) {
        var editLink = '<a class="acquia-lift-selector-edit">' + Drupal.t('Edit selector') + '</a>';
        var $selector =  $selectorInput.closest('div');
        $variationTypeForm.parent().find('h2').append(editLink);
        $variationTypeForm.parent().find('.acquia-lift-selector-edit').on('click', function(e) {
          var newText = $(this).text() == Drupal.t('Edit selector') ? Drupal.t('Hide selector') : Drupal.t('Edit selector');
          $selector.slideToggle();
          $(this).text(newText);
        });
        $selector.hide();
        $variationTypeForm.addClass('acquia-lift-processed');
      }

      // Populate the pages input with the current page.
      // The form is sent as the context so we can't check within it.
      var $pageGoalForm = $('#acquia-lift-create-goal-type-form').not('acquia-lift-processed');
      if ($pageGoalForm.length > 0) {
        $pageGoalForm.find('input[name="pages"]').val(Drupal.settings.visitor_actions.currentPath);
        $pageGoalForm.addClass('acquia-lift-processed');
      }
    }
  };

  Drupal.behaviors.acquiaLiftOptionSetTypeList = {
    attach: function (context, settings) {
      $('#acquia-lift-option-set-type-list', context).once('acquia-lift-option-set-type-list').each(function () {
        var blockAnchor = $(this).find('a[href="' + settings.basePath + settings.pathPrefix + 'admin/structure/personalize/variations/personalize-blocks/add"]');
        // Add the current destination address to the personalize blocks anchor.
        blockAnchor.attr('href', blockAnchor.attr('href') + '?destination=' + settings.visitor_actions.currentPath);
      });
    }
  };

  function hidePageVisitorActionsButton() {
    $('#visitor-actions-ui-actionable-elements-without-identifiers').hide();
  }

  /**
   * Provide the HTML to create the modal dialog.
   */
  Drupal.theme.prototype.AcquiaLiftModalDialog = function () {
    var html = ''
    html += '  <div id="ctools-modal">'
    html += '    <div class="ctools-modal-content acquia-lift-modal">' // panels-modal-content
    html += '      <div class="modal-header">';
    html += '        <a class="close" href="#">';
    html +=            Drupal.CTools.Modal.currentSettings.closeText + Drupal.CTools.Modal.currentSettings.closeImage;
    html += '        </a>';
    html += '        <span id="modal-title" class="modal-title">&nbsp;</span>';
    html += '      </div>';
    html += '      <div id="modal-content" class="modal-content">';
    html += '      </div>';
    html += '    </div>';
    html += '  </div>';

    return html;
  }

}(Drupal.jQuery, Drupal));
