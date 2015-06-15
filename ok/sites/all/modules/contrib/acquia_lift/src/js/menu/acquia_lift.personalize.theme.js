/**
 * Theme functions for the unified navigation bar tray.
 */

(function(Drupal, $) {

  var navbarMenuClassName = Drupal.settings.acquia_lift.menuClass;

  /**
   * Replaces spaces and underscored with dashes in a string.
   *
   * @param string str
   *   The string to be cleaned.
   *
   * @return str
   *   The updated string.
   */
  function formatClass (str) {
    if (!str) return '';
    return str.trim().replace(/[\s\_]+/g, '-');
  }

  /**
   * Returns an href with the preview parameter for the option set option.
   *
   * The URL fragment is maintained.
   *
   * @param mixed options
   *   An array of options for preselection or a single options object.
   *   Each option object has the following keys:
   *   - osID: The ID of the option set.
   *   - os: The option set object.
   *
   * @return string
   */
  function generateHref (options) {
    // Removes leading '/', '?' and '#' characters from a string.
    var pathRegex = /^(?:[\/\?\#])*(.*)/;
    var base = Drupal.settings.basePath + Drupal.settings.pathPrefix;
    var path = location.pathname && pathRegex.exec(location.pathname)[1] || '';
    var param = Drupal.settings.personalize.optionPreselectParam;

    var href = base + path + '?' + param + '=';
    if (!(options instanceof Array)) {
      options = [options];
    }
    var params = [];
    var osids = [];
    _.each(options, function (element, index, list) {
      osids.push(element.osID);
      params.push(element.osID + '--' + element.id);
    });
    href += params.join();

    // Now we need to add on any other Option Sets for which a preview option
    // had been selected so that we can preview more than one at a time.
    var existingSelection = decodeURI(window.location.search.replace(new RegExp("^(?:.*[&\\?]" + encodeURI(param).replace(/[\.\+\*]/g, "\\$&") + "(?:\\=([^&]*))?)?.*$", "i"), "$1"));
    if (existingSelection.length > 0) {
      Drupal.acquiaLiftUI.utilities.looper(existingSelection.split(','), function (str, key) {
        // Exclude any preselection for the Option Set we're generating a link for.
        var existingOsid = str.split('--')[0];
        if (osids.indexOf(existingOsid) == -1) {
          href += ',' + str;
        }
      });
    }

    return href;
  }

  /**
   * Returns HTML for a count display element.
   */
  Drupal.theme.acquiaLiftCount = function (options) {
    return '<i class="acquia-lift-personalize-type-count acquia-lift-empty"><span>0</span>&nbsp;</i>';
  };

  /**
   * Returns the HTML for the selected context label.
   *
   * @param $options
   * - label: The label of the selected context
   * - category: The type of context
   */
  Drupal.theme.acquiaLiftSelectedContext = function (options) {
    var label = options.category + ': ';
    label += '<span class="acquia-lift-active">' + options.label + '</span>';
    return '<span class="acquia-lift-active-container">' + label + '</span>';
  }

  /**
   * Throbber.
   */
  Drupal.theme.acquiaLiftThrobber = function () {
    return '<div class="ajax-progress ajax-progress-throbber"><div class="throbber">&nbsp;</div></div>';
  };

  /**
   * Returns an empty campaign menu item.
   */
  Drupal.theme.acquiaLiftPersonalizeNoMenuItem = function (options) {
    var attrs = [
      'class="acquia-lift-menu-disabled"'
    ];

    var item = '\n<span ' + attrs.join(' ') + '>\n';
    item += Drupal.t('No @type', {'@type': options.type}) + '\n';
    item += '</span>\n';

    return item;
  }

  /**
   * Themes a list of page variations for a campaign.
   *
   * @param MenuCampaignModel model
   *   The campaign model to create page variations display for.
   */
  Drupal.theme.acquiaLiftPageVariationsItem = function (model) {
    var optionSets = model.get('optionSets');
    var variations = optionSets.getVariations();

    if (variations.length == 0) {
      return '';
    }

    var attrs = [
      'class="acquia-lift-preview-page-variation acquia-lift-content-variation navbar-menu-item"' +
      'data-acquia-lift-personalize-agent="' + model.get('name') + '"'
    ];
    var item = '';
    item += '<span ' + attrs.join(' ') + '>';
    item += Drupal.t('Variations');
    item += '</span>\n';

    item += '<ul class="' + navbarMenuClassName + '">' + "\n";
    _.each(variations, function (variation, index, list) {
      item += Drupal.theme('acquiaLiftPreviewPageVariationMenuItem', variation);
    });
    item += '</ul>\n';
    return item;
  }

  /*****************************************
   * C A M P A I G N S
   *****************************************/

  /**
   * Returns a menu item that contains a link to a campaign.
   *
   * @param object options
   *   Keys:
   *   - id: The id of the campaign.
   *   - label: The label of the campaign.
   *   - href: The edit link to the campaign.
   *
   * @return string
   */
  Drupal.theme.acquiaLiftPersonalizeCampaignMenuItem = function (options) {
    var editAttrs = [
      'class="acquia-lift-campaign-edit acquia-lift-menu-link"',
      'title="' + Drupal.t('Edit the @campaign campaign', {'@campaign': options.link.label}) + '"',
      'href="' + options.edit.href + '"'
    ];

    var linkAttrs = [
      'class="acquia-lift-campaign acquia-lift-campaign--' + formatClass(options.link.id) + ' visitor-actions-ui-ignore"',
      'href="' + options.link.href + '"',
      'title="' + Drupal.t('Preview the @campaign campaign', {'@campaign': options.link.label}) + '"',
      'data-acquia-lift-personalize-agent="' + options.link.id + '"',
      'aria-role="button"',
      'aria-pressed="false"'
    ];

    var item = '<div class="acquia-lift-menu-item clearfix"><a ' + linkAttrs.join(' ') + '>' + options.link.label + '</a>';
    item += '<a ' + editAttrs.join(' ') + '>' + Drupal.t('Edit') + '</a>\n';
    item += '</div>\n';
    //item += '<a ' + linkAttrs.join(' ') + '>' + options.link.label + '</a>\n';

    return item;
  };

  /*****************************************
   * P A G E  V A R I A T I O N S
   *****************************************/

  /**
   * Themes a list item for a page variation within the list of variations.
   *
   * @param object variation
   *   The variation details including:
   *   - agent: the name of the agent/campaign for this variation
   *   - options: an array of variation options
   *   - index: the variation index value
   *   - label: the variation label
   */
  Drupal.theme.acquiaLiftPreviewPageVariationMenuItem = function (variation) {
    var item = '';
    var hrefOptions = [];
    _.each(variation.options, function (option, index, list) {
      hrefOptions.push({
        osID: option.osid,
        id: option.option.option_id
      });
    });
    var attrs = [
      'class="acquia-lift-preview-option acquia-lift-preview-page-variation--' + variation.original_index + ' visitor-actions-ui-ignore"',
      'href="' + generateHref(hrefOptions) + '"',
      'data-acquia-lift-personalize-page-variation="' + variation.original_index + '"',
      'aria-role="button"',
      'aria-pressed="false"'
    ];

    var renameHref = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/acquia_lift/pagevariation/rename/' + variation.agent + '/' + variation.original_index + '/nojs';
    var renameAttrs = [
      'class="acquia-lift-variation-rename acquia-lift-menu-link ctools-use-modal ctools-modal-acquia-lift-style"',
      'title="' + Drupal.t('Rename Variation #@num', {'@num': variation.index}) + '"',
      'data-acquia-lift-personalize-page-variation="' + variation.original_index + '"',
      'aria-role="button"',
      'aria-pressed="false"',
      'href="' + renameHref + '"'
    ];

    var deleteHref = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/acquia_lift/pagevariation/delete/' + variation.agent + '/' + variation.original_index + '/nojs';
    var deleteAttrs = [
      'class="acquia-lift-variation-delete acquia-lift-menu-link ctools-use-modal ctools-modal-acquia-lift-style"',
      'title="' + Drupal.t('Delete Variation #@num', {'@num': variation.index}) + '"',
      'data-acquia-lift-personalize-page-variation="' + variation.original_index + '"',
      'aria-role="button"',
      'aria-pressed="false"',
      'href="' + deleteHref + '"'
    ];

    item += '<li>\n<div class="acquia-lift-menu-item clearfix" data-acquia-lift-personalize-agent="' + variation.agent + '">';
    item += '<a ' + attrs.join(' ') + '>' + Drupal.checkPlain(variation.label) + '</a> \n';
    if (variation.index > 0) {
      item += '<a ' + deleteAttrs.join(' ') + '>' + Drupal.t('Delete') + '</a>\n';
      item += '<a ' + renameAttrs.join(' ') + '>' + Drupal.t('Rename') + '</a>\n';
    }
    item += '</div></li>';

    return item;
  }

  /**
   * Themes a list item for a new page variation that has not yet been saved.
   *
   * @param variation_number
   *   The number to display for this variation.  -1 is passed to indicate a
   *   temporary control variation option display.
   */
  Drupal.theme.acquiaLiftNewVariationMenuItem = function (variation_number) {
    var isControl = variation_number == -1;
    var attrs = [
      'class="acquia-lift-preview-option acquia-lift-page-variation-new',
      'aria-role="button"',
      'aria-pressed="false"'
    ];
    var variationLabel = isControl ? Drupal.t(Drupal.settings.personalize.controlOptionLabel) : Drupal.t('Variation #@varnum', {'@varnum': variation_number});
    if (isControl) {
      attrs.push('data-acquia-lift-personalize-page-variation="control"');
    } else {
      attrs.push('data-acquia-lift-personalize-page-variation="new"');
    }

    var item = '';
    item += '<li>\n<a ' + attrs.join(' ') + '>\n';
    item += variationLabel + '\n';
    item += '</a>\n</li>\n';

    return item;
  }

  /**
   * Returns the HTML for the page variation edit toggle link.
   */
  Drupal.theme.acquiaLiftPageVariationToggle = function () {
    var label = Drupal.t('Toggle variation mode');
    return '<a class="acquia-lift-page-variation-toggle" id="acquia-lift-menu-page-variation-toggle">' + label + '</a>';
  }

  /*****************************************
   * O P T I O N  S E T S
   *****************************************/

  /**
   * Returns a list item that contains links to preview option set options.
   *
   * @param object options
   *   Keys:
   *   - osID: The ID of the option set.
   *   - os: The option set object.
   *   - os.label: The label of the option set.
   *   - os.agent: The campaign/agent to which this option set belongs.
   *
   * @return string
   */
  Drupal.theme.acquiaLiftOptionSetItem = function (options) {
    var attrs = [
      'class="acquia-lift-preview-option-set acquia-lift-content-variation navbar-menu-item acquia-lift-preview-option-set-' + formatClass(options.osID)  + '"',
      'data-acquia-lift-personalize-id="' + options.osID + '"',
      'data-acquia-lift-personalize-agent="' + options.os.agent + '"'
    ];
    var item = '';
    item += '<span ' + attrs.join(' ') + '>' + Drupal.checkPlain(options.os.label) + '</span>';
    item += Drupal.theme('acquiaLiftOptionSetMenu', options);
    return item;
  };

  /**
   * Returns a menu item that contains links to preview option set options.
   *
   * @param object options
   *   Keys:
   *   - osID: The ID of the option set.
   *   - os: The option set object.
   *   - os.option_id: The ID of an option set option.
   *   - os.option_label: The label of an option set option.
   *   - os.deletable: Boolean indicating if the option is deletable from the
   *     menu.
   *   - os.editable: Boolean indicating if the option is editable from the menu.
   *
   * @return string
   */
  Drupal.theme.acquiaLiftOptionSetMenu = function (options) {
    var menu = '<ul class="' + navbarMenuClassName + '">' + "\n";
    var osID = options.osID;
    var os = options.os;
    var os_selector = os.selector;
    options.os.options.each(function(model) {
      menu += Drupal.theme('acquiaLiftPreviewOptionMenuItem', {
        id: model.get('option_id'),
        label: model.get('option_label'),
        osID: osID,
        osSelector: os_selector,
        showDelete: os.deletable,
        showEdit: os.editable
      });
    });
    if (os.plugin === 'elements') {
      menu += '<li>';
      menu += '<a href="' + Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/personalize/variations/add/nojs"';
      menu += ' class="acquia-lift-variation-add acquia-lift-menu-link" title="' + Drupal.t('Add variation') + '" aria-role="button" aria-pressed="false">';
      menu += Drupal.t('Add variation');
      menu += '</a></li>';
    }
    menu += '</ul>\n';
    return menu;
  };

  /**
   * Returns a menu item that contains a link to an option set option.
   *
   * @param object options
   *   Keys:
   *   - id: The id of the option set option.
   *   - label: The label of the option set option.
   *   - osID: The ID of the option set.
   *   - osSelector: The selector representing the option set.
   *   - showDelete: Indicates if the delete option should be available for this
   *     particular item.
   *   - showEdit: Indicates if the edit option should be available for this
   *     item.
   *
   * @return string
   */
  Drupal.theme.acquiaLiftPreviewOptionMenuItem = function (options) {
    var item = '';
    var ariaAttrs = [
      'aria-role="button"',
      'aria-pressed="false"'
    ];
    // Prepare the selector string to be passed as a data attribute.
    var selector = options.osSelector.replace(/\"/g, '\'');
    var previewAttrs = [
      'class="acquia-lift-preview-option acquia-lift-preview-option--' + formatClass(options.id) + ' visitor-actions-ui-ignore"',
      'href="' + generateHref(options) + '"',
      'data-acquia-lift-personalize-option-set="' + options.osID + '"',
      'data-acquia-lift-personalize-option-set-selector="' + selector + '"',
      'data-acquia-lift-personalize-option-set-option="' + options.id + '"'
    ].concat(ariaAttrs);

    var deleteHref = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/acquia_lift/variation/delete/' + options.osID + '/' + options.id + '/nojs';
    var deleteAttrs = [
      'class="acquia-lift-variation-delete acquia-lift-menu-link ctools-use-modal ctools-modal-acquia-lift-style"',
      'title="' + Drupal.t('Delete variation') + '"',
      'href="' + deleteHref + '"',
      'data-acquia-lift-personalize-option-set-option="' + options.id + '"'
    ].concat(ariaAttrs);

    var editHref = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/personalize/variations';
    var editAttrs = [
      'class="acquia-lift-variation-edit acquia-lift-menu-link"',
      'data-acquia-lift-personalize-option-set-option="' + options.id + '"',
      'title="' + Drupal.t('Edit variation') + '"',
      'href="' + editHref + '"'
    ].concat(ariaAttrs);

    item += '<li>\n<div class="acquia-lift-menu-item clearfix" data-acquia-lift-personalize-option-set="' + options.osID + '">';
    item += '<a ' + previewAttrs.join(' ') + '>' + options.label + '</a> \n';

    if (options.id !== Drupal.settings.personalize.controlOptionName) {
      if (options.showDelete) {
        item += '<a ' + deleteAttrs.join(' ') + '>' + Drupal.t('Delete') + '</a>\n';
      }
      if (options.showEdit) {
        item += '<a ' + editAttrs.join(' ') + '>' + Drupal.t('Edit') + '</a>\n';
      }
    }
    item += '</div></li>';
    return item;
  };

  /*****************************************
   * G O A L S
   *****************************************/

  /**
   * Returns the HTML for the goals list for a campaign.
   *
   * @param MenuCampaignModel model
   *   The campaign model to create goals display for.
   * @param object actions
   *   An object of all actions keyed by the action machine name.
   */
  Drupal.theme.acquiaLiftCampaignGoals = function (model, actions) {
    var goals = model.get('goals');
    var html = '<ul class="' + navbarMenuClassName + '">';

    if (goals.length == 0) {
      html += '<li>';
      html += Drupal.theme('acquiaLiftPersonalizeNoMenuItem', {
        type: 'goals'
      });
      html += '</li>';
      return html;
    }
    goals.each(function (goalModel) {
      var goalId = goalModel.get('id');
      var custom = actions.hasOwnProperty(goalId);
      html += '<li>';
      html += Drupal.theme('acquiaLiftPersonalizeGoal', {
        campaignID: model.get('name'),
        name: goalId,
        label: goalModel.get('name'),
        custom: custom
      });
      html += '</li>';
    });
    html += '</ul>';
    return html;
  }


  /**
   * Returns a campaign goal menu item.
   *
   * @param object options
   *   Keys:
   *   - campaignID: The ID of the campaign for these goals.
   *   - name: The goal ID.
   *   - label: The goal label.
   *   - custom: Boolean to indicate if a goal is custom or defined in code.
   *
   * @return string
   */
  Drupal.theme.acquiaLiftPersonalizeGoal = function (options) {
    var campaignID = options.campaignID;
    var item = '';

    var attrs = [
      'class="acquia-lift-goal acquia-lift-goal--' + formatClass(campaignID) + ' visitor-actions-ui-ignore"',
      'data-acquia-lift-personalize-agent="' + campaignID + '"',
      'data-acquia-lift-personalize-goal="' + options.name + '"'
    ];

    var renameHref = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/acquia_lift/goal/rename/' + options.name + '/nojs';
    var renameAttrs = [
      'class="acquia-lift-goal-rename acquia-lift-menu-link ctools-use-modal ctools-modal-acquia-lift-style"',
      'title="' + Drupal.t('Rename goal') + '"',
      'aria-role="button"',
      'aria-pressed="false"',
      'href="' + renameHref + '"'
    ];

    var deleteHref = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/acquia_lift/goal/delete/' + options.campaignID + '/' + options.name + '/nojs';
    var deleteAttrs = [
      'class="acquia-lift-goal-delete acquia-lift-menu-link ctools-use-modal ctools-modal-acquia-lift-style"',
      'title="' + Drupal.t('Delete goal') + '"',
      'aria-role="button"',
      'aria-pressed="false"',
      'href="' + deleteHref + '"'
    ];

    item += '<div class="acquia-lift-menu-item clearfix">\n';
    item += '<span ' + attrs.join(' ') + '>' + Drupal.t('@text', {'@text': options.label}) + '</span>\n';
    item += '<a ' + deleteAttrs.join(' ') + '>' + Drupal.t('Delete') + '</a>\n';
    if (options.custom) {
      item += '<a ' + renameAttrs.join(' ') + '>' + Drupal.t('Rename') + '</a>\n';
    }
    item += '</div>';
    return item;
  };

}(Drupal, Drupal.jQuery));
