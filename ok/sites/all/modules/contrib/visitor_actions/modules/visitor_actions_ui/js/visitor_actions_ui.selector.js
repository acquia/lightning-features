/**
 * @file visitor_actions_ui.selector.js
 */
(function ($, Drupal) {
  Drupal.utilities = Drupal.utilities || {};

  /**
   * Determines the unique selector for an element.
   *
   * @param element
   *   A DOM element to find the selector for.
   * @param ignoreId
   *   An ID that should not be used when generating the selector.  Can be a
   *   string for an exact match, or a regular expression object.
   * @param ignoreClasses
   *   Classes that should be ignored when generating the selector.  Can be a
   *   string for exact match (space delimited for multiple classes) or a regular
   *   expression object that returns a match for each class name.
   *   *-processed classes are ignored automatically.
   */
  Drupal.utilities.getSelector = Drupal.utilities.getSelector || function (element, ignoreId, ignoreClasses) {

    // Convert the ignoreID  and ignoreClasses to regular expressions if only
    // strings passed in.
    ignoreId = typeof ignoreId === 'undefined' ? '' : ignoreId;
    ignoreClasses = typeof ignoreClasses === 'undefined' ? '' : ignoreClasses;
    if (typeof ignoreId === 'string' && notEmpty(ignoreId)) {
      ignoreId = new RegExp('^' + ignoreId + '$', i);
    }
    if (typeof ignoreClasses === 'string' && notEmpty(ignoreClasses)) {
      temp = ignoreClasses.split(' ');
      ignoreClasses = '';
      for (var i=0; i<temp.length; i++) {
        ignoreClasses += '(' + temp[i] + ')';
        if (i < (temp.length-1)) {
          ignoreClasses += '|';
        }
      }
      ignoreClasses = new RegExp(ignoreClasses);
    }

    /**
     * Utility function to test if a string value empty.
     *
     * @param stringValue
     *   String value to test
     * @returns {boolean}
     *   True if not empty, false if null or empty.
     */
    function notEmpty(stringValue) {
      return (stringValue != null ? stringValue.length : void 0) > 0;
    };

    /**
     * Removes any attributes that should be ignored for an element.
     *
     * @param element
     *   A DOM element to manipulate.
     * @param ignoreId
     *   An ID that should not be used.  Can be a string for an exact match, or a
     *   regular expression object.
     * @param ignoreClasses
     *   Classes that should be ignored.  Can be a string for exact match (space
     *   delimited for multiple classes) or a regular expression object.
     *   *-processed classes are ignored automatically.
     * @return an array with 2 keys
     *   1 - any removed id
     *   2 - the list of removed classes (space delimited)
     */
    function removeIgnoreAttributes(element, ignoreId, ignoreClasses) {
      var tempId = '',
        tempClasses,
        tempClassNames;

      // Pull out any ids to be ignored.
      if (ignoreId instanceof RegExp && notEmpty(element.id) && ignoreId.test(element.id)) {
        tempId = element.id;
        element.id = '';
      }
      // Remove any classes to be ignored.
      if (ignoreClasses instanceof RegExp) {
        tempClasses = element.className.match(ignoreClasses);
      }
      // Remove any visitorActionsUI classes or *-processed classes.
      tempClasses = tempClasses instanceof Array ? tempClasses : [];
      tempClassNames = tempClasses.join(' ');
      $(element).removeClass(tempClassNames);
      return [tempId, tempClassNames];
    }

    /**
     * Restore any removed classes or ids to the element.
     *
     * @param element
     *   A DOM element to be restored.
     * @param restoreId
     *   The id attributes for the element.
     * @param restoreClasses
     *   Any classes to be restored.
     * @return
     *   The updated DOM element for chaining.
     */
    function restoreIgnoredAttributes(element, restoreId, restoreClasses) {
      if (restoreId.length) {
        element.id = restoreId;
      }
      $(element).addClass(restoreClasses);
      return element;
    }

    /**
     * Indicates whether the selector string represents a unique DOM element.
     *
     * @param String selector
     *   A string selector that can be used to query a DOM element.
     *
     * @return Boolean
     *   Whether or not the selector string represents a unique DOM element.
     */
    function isUniquePath (selector) {
      return $(selector).length === 1;
    }

    /**
     * Creates a selector from the element's id attribute.
     *
     * Temporary IDs created by the module are excluded.
     *
     * @param DOM element
     *
     * @return String
     *   An id selector or an empty string.
     */
    function applyID (element) {
      // Don't return any id to apply if it matches the ignore id regex.
      if (ignoreId instanceof RegExp && notEmpty(element.id) && ignoreId.test(element.id)) {
        return '';
      }
      return notEmpty(element.id) ? '#' + element.id : '';
    }

    /**
     * Creates a selector from classes on the element.
     *
     * Classes with known functional components like the word 'active' are
     * excluded because these often denote state, not identity.
     *
     * @param DOM element
     *
     * @return String
     *   A selector of classes or an empty string.
     */
    function applyClasses (element) {
      var selector = '';
      // Try to make a selector from the element's classes.
      var classes = element.className || '';
      // Filter out classes that might represent state.
      var removeClasses = ['active','enabled','disabled','first','last','only','collapsed','open','clearfix','processed'];
      if (classes.length > 0) {
        if (ignoreClasses instanceof RegExp) {
          removeClasses = removeClasses.concat(classes.match(ignoreClasses));
        }
        classes = classes.split(/\s+/);
        classes = _.reject(classes, function (cl) {
          var removeReg = new RegExp(removeClasses.join('|'));
          return removeReg.test(cl);
        });
        if (classes.length > 0) {
          return '.' + classes.join('.');
        }
      }
      return selector;
    }

    /**
     * Finds attributes on the element and creates a selector from them.
     *
     * @param DOM element
     *
     * @return String
     *   A selector of attributes or an empty string.
     */
    function applyAttributes (element) {
      var selector = '';
      var attributes = ['href', 'type'];
      var value;
      // Try to make a selector from the element's classes.
      for (var i = 0, len = attributes.length; i < len; i++) {
        value = element.attributes[attributes[i]] && element.attributes[attributes[i]].value;
        if (value) {
          // If the attr is href and it points to a specific user account,
          // just tack on the attr name and not the value.
          if (attributes[i] === 'href' && /user\/\d+/.test(value)) {
            selector += '[' + attributes[i] + ']'
          }
          else {
            selector += '[' + attributes[i] + '="' + value + '"]';
          }
        }
      }
      return selector;
    }

    /**
     * Generates a more complicated selector based on the element's position
     * in the DOM.
     *
     * While not guaranteed to be unique, it is very close.
     *
     * @param DOM element
     * @returns string
     *   A selector for the element.
     */
    function generateUniqueSelector(element) {
      var invalidParents = ['#document', 'HTML', 'BODY'];
      var hasParent = element.parentNode != null && invalidParents.indexOf(element.parentNode.nodeName) == -1;

      var selector = nthChild(element);
      if (hasParent) {
        return Drupal.utilities.getSelector(element.parentNode, ignoreId, ignoreClasses) + " > " + selector;
      }
      return selector;
    }

    /**
     * Determines the element selector for a child element based on element
     * name.

     * @param element
     *   The element to use for determination
     * @returns string
     *   The selector string to use
     */
    function nthChild(element) {
      if (element == null || element.ownerDocument == null || element === document || element === document.body || element === document.head) {
        return "";
      }
      var parent = element.parentNode || null;
      if (parent) {
        var nthStack = [];
        var num = parent.childNodes.length;
        for (var i = 0; i < num; i++) {
          var nthName = parent.childNodes[i].nodeName.toLowerCase();
          if (nthName === "#text") {
            continue;
          }
          nthStack.push(nthName);
          if (parent.childNodes[i] === element) {
            if (nthStack.length > 1) {
              nthStack[0] += ":first-child";
            }
            return nthStack.join(" + ");
          }
        }
      }
      return element.nodeName.toLowerCase();
    }

    /**
     * Creates a simple selector using id, classes and attributes.
     *
     * It is possible that the selector will not be unique if there is no
     * unique description using only ids, classes and attributes of an
     * element that exist on the page already.
     *
     * @param DOM element
     *
     * @return String
     *   A unique selector for the element.
     */
    function generateSimplifiedSelector (element) {
      var selector = '';
      var scopeSelector = '';
      var pseudoUnique = false;
      var firstPass = true;

      do {
        scopeSelector = '';
        // Try to apply an ID.
        if ((scopeSelector = applyID(element)).length > 0) {
          selector = scopeSelector + ' ' + selector;
          // Assume that a selector with an ID in the string is unique.
          break;
        }

        // Try to apply classes.
        if (!pseudoUnique && (scopeSelector = applyClasses(element)).length > 0) {
          // If the classes don't create a unique path, tack them on and
          // continue.
          selector = scopeSelector + ' ' + selector;
          // If the classes do create a unique path, mark this selector as
          // pseudo unique. We will keep attempting to find an ID to really
          // guarantee uniqueness.
          if (isUniquePath(selector)) {
            pseudoUnique = true;
          }
        }

        // Process the original element.
        if (firstPass) {
          // Try to add attributes.
          if ((scopeSelector = applyAttributes(element)).length > 0) {
            // Do not include a space because the attributes qualify the
            // element. Append classes if they exist.
            selector = scopeSelector + selector;
          }

          // Add the element nodeName.
          selector = element.nodeName.toLowerCase() + selector;

          // The original element has been processed.
          firstPass = false;
        }

        // Try the parent element to apply some scope.
        element = element.parentNode;
      } while (element && element.nodeType === 1 && element.nodeName !== 'BODY' && element.nodeName !== 'HTML');

      return selector.trim();
    }

    var removed = removeIgnoreAttributes(element, ignoreId, ignoreClasses);
    var removedId = removed[0];
    var removedClasses = removed[1];

    selector = generateSimplifiedSelector(element);
    if (!isUniquePath(selector)) {
      // generate the selector based on nth child which will produce a longer
      // and crazier looking selector but far more likely to be unique.
      selector = generateUniqueSelector(element);
    }

    restoreIgnoredAttributes(element, removedId, removedClasses);
    return selector;
  }
}(Drupal.jQuery, Drupal));
