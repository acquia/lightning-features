/**
 * @file acquia_lift.variations.collections.js
 * 
 * Backbone collections used for the variations application.
 */
(function($, Drupal, Backbone, _) {

  Drupal.acquiaLiftVariations.collections = Drupal.acquiaLiftVariations.collections || {
    ElementVariationCollection: Backbone.Collection.extend({
      model: Drupal.acquiaLiftVariations.models.VariationTypeModel,

      /**
       * Returns a filtered collection with only those variation types that
       * are relevent to the current element.
       *
       * For example, this is where it is determined that editText can only be
       * displayed based on particular child nodes.
       */
      applicableToElement: function ($element) {
        // Get all the node types of the children for the element.
        var childrenNodeTypes = _.pluck($element.find('*'), 'nodeType');

        return _(this.filter(function(data) {
          var limitByChildrenType = data.get('limitByChildrenType');

          // If there is a limit on the children type, make sure that every
          // child passes the test.
          if (limitByChildrenType && !isNaN(limitByChildrenType)) {
            var childMatch = _.every(childrenNodeTypes, function(type) {return type == limitByChildrenType});
            // Special limitations by node type.
            switch (parseInt(limitByChildrenType)) {
              case 3: {
                // Text nodes only - check for text within the parent element
                // if no child elements.
                return childrenNodeTypes.length == 0 ? $element.get(0).textContent.length > 0 : childMatch;
                break;
              }
              default: {
                return childMatch;
              }
            }
          }
          // No limits in place so include by default.
          return true;
        }))
      }
    })
  };

}(Drupal.jQuery, Drupal, Backbone, _));
