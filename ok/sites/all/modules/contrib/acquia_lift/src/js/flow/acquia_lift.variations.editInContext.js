/**
 * @file acquia_lift.variations.editInContext.js
 * 
 * Provides the personalize elements edit in context functionality that allows
 * manipulation of the DOM for easier variation creation/editing.
 */
(function($, Drupal, Dialog, Backbone, _) {

  /**
   * Define editInContext behaviors to define what happens when creating
   * a particular personalize_element page variation in context.
   */
  Drupal.acquiaLiftVariations.personalizeElements = Drupal.acquiaLiftVariations.personalizeElements || {};

  /**
   * Whenever a variation type form is complete, call the personalize elements
   * editInContext callbacks.
   */
  $(document).on('acquiaLiftVariationTypeForm', function(e, type, selector, $input) {
    if ($input.val().length > 0) {
      // Don't replace any existing content in the input field.
      return;
    }
    if (Drupal.acquiaLiftVariations.personalizeElements.hasOwnProperty(type)
      && Drupal.acquiaLiftVariations.personalizeElements[type].hasOwnProperty('editInContext')
      && typeof Drupal.acquiaLiftVariations.personalizeElements[type].editInContext === 'function') {
      Drupal.acquiaLiftVariations.personalizeElements[type].editInContext(selector, $input);
    }
  });

  /****************************************************************
   *
   *          E D I T  H T M L
   *
   ****************************************************************/
  Drupal.acquiaLiftVariations.personalizeElements.editHtml = {
    getOuterHtml: function($element) {
      if ($element.length > 1) {
        $element = $element.first();
      }
      // jQuery doesn't have an outerHTML so we need to clone the child and
      // give it a parent so that we can call that parent's html function.
      // This ensures we get only the html of the $selector and not siblings.
      var $element = $element.clone().wrap('<div>').parent();
      // Remove any extraneous acquia lift / visitor actions stuff.
      var removeClasses = new RegExp(Drupal.settings.visitor_actions.ignoreClasses, 'g');
      var removeId = new RegExp(Drupal.settings.visitor_actions.ignoreIds);
      var removeTags = 'script';
      var removeAttributes = ['data-personalize'];

      // Remove any invalid ids.
      $element.find('[id]').filter(function() {
        return removeId.test(this.id);
      }).removeAttr('id');

      // Remove any classes that are marked for ignore.
      $element.find('[class]').each(function() {
        var stripClasses = this.className.match(removeClasses) || [];
        $(this).removeClass(stripClasses.join(' '));
        if (this.className.length == 0) {
          $(this).removeAttr('class');
        }
      });

      // Strip out any tags and styles if configured to do so.
      if (Drupal.settings.acquia_lift.edit_in_context_html_strip) {
        // Remove any styling added directly from jQuery.
        $element.find('[style]').removeAttr('style');

        // Remove any inappropriate tags
        $element.find(removeTags).remove();
      }

      // Remove any data attributes.
      _.each(removeAttributes, function(attr) {
        $element.find('[' + attr + ']').removeAttr(attr);
      });

      // Now return the cleaned up html.
      return $element.html();
    },
    editInContext : function(selector, $contentInput) {
      var editString = this.getOuterHtml($(selector));
      $contentInput.val(editString);
    }
  };

  /****************************************************************
   *
   *          E D I T  T E X T
   *
   ****************************************************************/
  Drupal.acquiaLiftVariations.personalizeElements.editText = {
    editInContext : function(selector, $contentInput) {
      var editString = $(selector).text();
      $contentInput.val(editString);
    }
  };

}(Drupal.jQuery, Drupal, Drupal.visitorActions.ui.dialog, Backbone, _));
