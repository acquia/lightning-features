<?php

/**
 * @file acquia_lift.ui.inc
 * Provides functions needed for the front-end UI.
 */

/**
 * Menu callback; Provide the top-level access point.
 */
function acquia_lift_root_page() {
  drupal_goto('admin/structure/personalize');
}

/**
 * Sends the Acquia Lift control menus.
 */
function acquia_lift_controls_assets_callback() {
  // The Acquia Lift module is responsible for assembling menu items into a
  // single menu in the navbar.
  $menu = menu_tree_all_data('acquia-lift-controls');

  $response = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('acquia-lift-controls'),
    ),
    'personalization' => menu_tree_output($menu),
  );
  return $response;
}

/**
 * Attaches the front-end controls to the page.
 *
 * @param $page
 *   The render array of the page.
 */
function acquia_lift_build_page(&$page) {
  // Attach the editor app toggle code on all non-admin pages.
  // A special case is made for the block demo page which is not properly
  // marked as an admin page by path_is_admin().
  $is_admin = TRUE;
  if (!path_is_admin(current_path()) && preg_match('/^admin\/structure\/block\/demo\//', current_path()) == 0) {
    $is_admin = FALSE;
    // If the user is personalizing content in context, provide a DSM with actions
    // to personalize or cancel out of this 'mode'.
    if (acquia_lift_nav_message_is_set()) {
      drupal_set_message(acquia_lift_edit_mode_personalize_in_context(), 'acquia-lift-personalize');
    }
  }

  // Attach client-side controls for managing personalized content.
  _acquia_lift_navigation_attach_assets($page['page_top'], $is_admin);

  // Necessary ctools integration for modal windows.  These are used only for
  // administrative functionality.
  if (user_access('manage personalized content')) {
    ctools_include('modal');
    ctools_include('ajax');
    ctools_modal_add_js();

    // Have to add styling here.  When added as part of a library it is always
    // added before ctools and therefore cannot override styles.
    $page['page_top']['#attached']['library'][] = array('acquia_lift', 'acquia_lift.modal');
    ctools_add_css('acquia_lift.ctools.modal', 'acquia_lift');

    // Load data about active campaigns.
    $include_status = variable_get('acquia_lift_unibar_allow_status_change', TRUE);
    $settings['acquia_lift']['campaigns'] = acquia_lift_get_campaign_details($include_status);
    $settings['acquia_lift']['allowStatusChange'] = (bool) $include_status;
    // Load data about custom defined visitor actions.
    $actions = visitor_actions_custom_load_multiple();
    $settings['acquia_lift']['customActions'] = $actions;

    $settings['acquia_lift']['dom_selector_ignore'] = acquia_lift_generate_ignore_selector();
    $settings['acquia_lift']['edit_in_context_html_strip'] = variable_get('acquia_lift_html_context_strip', 1);

    $pending_agents = acquia_lift_get_pending_agents();
    if (!empty($pending_agents)) {
      $settings['acquia_lift']['pendingMessage'][] = t('The following campaigns are being synced to Lift and will be started shortly: %agents', array('%agents' => implode(', ', $pending_agents)));
      $page['page_top']['#attached']['library'][] = array('acquia_lift', 'acquia_lift.messagebox');
    }
    // Add any pending messages from the query string.
    $params = drupal_get_query_parameters();
    if (!empty($params['liftpm']) && strstr($params['liftpm'], '|') !== FALSE) {
      list($type, $details) = explode('|', $params['liftpm']);
      if ($type === 'new_block') {
        $settings['acquia_lift']['pendingMessage'][] = t('Created the new %block_title personalized block. The block will not appear on your website until you add the block to a region on the !blocks page.', array(
          '%block_title' => $details,
          '!blocks' => l('Structure > Blocks', 'admin/structure/blocks'),
        ));
      }
    }
    drupal_add_js($settings, 'setting');
  }
}

/**
 * Attaches the jQuery "chosen" behavior to the the passed in element.
 *
 * @param array $element
 *   An array representing a multi-select form element.
 * @param array $classes
 *   An array of classes to be included on this element.
 */
function acquia_lift_chosenify_element(&$element, $classes = array()) {
  $chosen_path = libraries_get_path('chosen');
  $classes[] = 'acquia-lift-chosen-select';

  $options = array(
    'scope' => 'footer',
    'defer' => TRUE,
  );
  $element['#attributes'] = array(
    'class' => $classes,
    'data-placeholder' => t('Choose a context...'),
  );
  $element['#attached'] = array(
    'js' => array(
      $chosen_path . '/chosen.jquery.min.js' => array('group' => 'JS_LIBRARY'),
      drupal_get_path('module', 'acquia_lift') . '/js/acquia_lift.admin.js' => $options,
    ),
    'css' => array(
      $chosen_path . '/chosen.css' => array(),
    )
  );
}

/**
 * Generates a selector for all of the regions of this page to ignore when
 * selecting elements from the DOM.
 */
function acquia_lift_generate_ignore_selector() {
  global $theme;
  $classes = array();
  $all_regions = system_region_list($theme, REGIONS_ALL);
  $visible_regions = system_region_list($theme, REGIONS_VISIBLE);
  $invisible_regions = array_diff($all_regions, $visible_regions);
  foreach ($invisible_regions as $region_name => $region_label) {
    $classes[] = drupal_region_class($region_name);
  }
  return $classes;
}

/**
 * =======================================================================
 *  F U N C T I O N S  B A S E D  O N  V I S I T O R  A C T I O N S  U I
 * =======================================================================
 */


/**
 * Builds controls for an in-context personalization mode.
 *
 * Based on visitor_actions_ui_edit_mode_action_in_context().
 *
 * @return String
 *   The rendered string for display.
 */
function acquia_lift_edit_mode_personalize_in_context() {
  $build = array(
    'message' => array(
      '#markup' => '<p>' . t('To personalize content in context, navigate to the page where the content you wish to personalize is, then press the %link link.', array('%link' => 'Personalize')) . '</p>',
    ),
    'actions' => array(
      '#theme' => 'acquia_lift_edit_mode_personalize_in_context_links',
      '#attributes' => array(
        'class' => array('visitor-actions-ui-actions'),
      ),
      'launch' => array(
        '#type' => 'link',
        '#title' => t('Personalize'),
        '#href' => drupal_get_normal_path('admin/structure/personalize'),
        '#options' => array(
          'attributes' => array(
            'title' => t('Highlight content that can be personalized'),
            'role' => 'button',
            'aria-pressed' => 'false',
            'class' => array('acquia-lift-ignore', 'acquia-lift-personalize-trigger'),
            'data-acquia-lift-personalize-mode' => 'content-variation',
          ),
        ),
      ),
      'cancel' => array(
        '#type' => 'link',
        '#title' => t('Cancel'),
        '#href' => drupal_get_normal_path('acquia_lift/personalize_in_context/stop'),
        '#options' => array(
          'attributes' => array(
            'title' => t('Home page'),
            'class' => array('acquia-lift-ignore'),
          ),
        ),
      ),
    ),
  );

  // Allow other modules to modify the message markup.
  drupal_alter('acquia_lift_edit_mode_actions', $build);
  return drupal_render($build);
}

/**
 * Toggles acquia_lift edit mode.
 */
function acquia_lift_edit_mode_toggle() {
  if (isset($_SESSION['acquia_lift_edit_mode'])) {
    unset($_SESSION['acquia_lift_edit_mode']);
  }
  else {
    $_SESSION['acquia_lift_edit_mode'] = 1;
  }
}

/**
 * Disables edit mode and returns to the stored destination.
 */
function acquia_lift_edit_mode_disable() {
  unset($_SESSION['acquia_lift_edit_mode']);
  $goto = '';
  $options = array();
  if (isset($_SESSION['acquia_lift_personalize_edit_mode_return'])) {
    $goto = $_SESSION['acquia_lift_personalize_edit_mode_return'];
    if (strpos($goto, '#overlay=') === 0) {
      // If the original path was in the overlay, make sure we go back
      // to the overlay.
      $path = substr($goto, strlen('#overlay='));
      $goto = current_path();
      $options = array('fragment' => 'overlay=' . $path);
    }
    unset($_SESSION['acquia_lift_personalize_edit_mode_return']);
    // The above session variable trumps any destination that has been
    // set so we need to make sure drupal_goto() behaves correctly.
    if (isset($_GET['destination'])) {
      unset($_GET['destination']);
    }
  }
  elseif (isset($_GET['destination'])) {
    $goto = $_GET['destination'];
    unset($_GET['destination']);
  }
  if (acquia_lift_nav_message_is_set()) {
    acquia_lift_unset_nav_message();
  }
  drupal_goto($goto, $options);
}

/**
 * Page callback to start personalization edit mode.
 */
function personalize_in_context_start() {
  if (isset($_GET['destination'])) {
    $_SESSION['acquia_lift_personalize_edit_mode_return'] = $_GET['destination'];
    unset($_GET['destination']);
  }
  acquia_lift_set_nav_message();
  drupal_goto();
}

/**
 * Sets the instructional message for users to navigate to where they
 * wish to add an action.
 */
function acquia_lift_set_nav_message() {
  $_SESSION['acquia_lift_personalize_set_message'] = 1;
}

/**
 * Checks whether the instructional navigation message is set.
 *
 * @return bool
 *   TRUE if the message is set, FALSE otherwise.
 */
function acquia_lift_nav_message_is_set() {
  return isset($_SESSION['acquia_lift_personalize_set_message']);
}


/**
 * Unsets the instructional navigation message.
 */
function acquia_lift_unset_nav_message() {
  unset($_SESSION['acquia_lift_personalize_set_message']);
}

/**
 * =======================================================================
 *  F U N C T I O N S  B A S E D  O N  N A V B A R
 * These functions are only used/useful when navbar is not included.
 * =======================================================================
 */
/**
 * Builds the unified navbar as a structured array ready for drupal_render().
 *
 * @param array $element
 *   A renderable array
 *
 * @return
 *   A renderable array.
 *
 * @see navbar_pre_render().
 * @see acquia_lift_page_build().
 */
function acquia_lift_navbar_ui_pre_render($element) {
  // Define the breakpoints to switch from vertical to horizontal
  // navbar presentation.
  $breakpoints = array(
    'narrow' => 'only screen and (min-width: 16.5em)',
    'standard' => 'only screen and (min-width: 38.125em)',
    'wide' => 'only screen and (min-width: 50em)',
  );
  // Allow for altering of the breakpoints.
  drupal_alter('acquia_lift_breakpoints', $breakpoints);

  if (!empty($breakpoints)) {
    $element['#attached']['js'][] = array(
      'data' => array(
        'acquia_lift' => array(
          'unified_navbar' => array(
            'breakpoints' => $breakpoints,
          ),
        ),
      ),
      'type' => 'setting',
    );
  }

  // Get the navigation items as defined for the navbar implementation.
  $items = acquia_lift_navbar();
  // Sort the children.
  uasort($items, 'element_sort');

  // Merge in the original navbar values.
  $element = array_merge($element, $items);
  // Render the children.
  $element['#children'] = drupal_render_children($element);

  return $element;
}

/**
 * Provides markup for associating a tray trigger with a tray element.
 *
 * A tray is a responsive container that wraps renderable content. Trays present
 * content well on small and large screens alike.
 *
 * @param array $element
 *   A renderable array.
 *
 * @return
 *   A renderable array.
 *
 * @see navbar_pre_render_item().
 */
function acquia_lift_navbar_ui_pre_render_item($element) {
  // Assign each item a unique ID.
  $id = drupal_html_id('navbar-item');

  // If tray content is present, markup the tray and its associated trigger.
  if (!empty($element['tray'])) {
    // Provide attributes for the tray theme wrapper.
    $attributes = array(
      'id' => $id . '-tray',
      'data-navbar-tray' => $id . '-tray',
      'aria-owned-by' => $id,
    );

    // Merge in module-provided attributes.
    if (!isset($element['tray']['#wrapper_attributes'])) {
      $element['tray']['#wrapper_attributes'] = array();
    }
    $element['tray']['#wrapper_attributes'] += $attributes;
    $element['tray']['#wrapper_attributes']['class'][] = 'navbar-tray';

    if (!isset($element['tray']['#theme_wrappers'])) {
      $element['tray']['#theme_wrappers'] = array();
    }
    // Add the standard theme_wrapper for trays.
    array_unshift($element['tray']['#theme_wrappers'], 'acquia_lift_navbar_tray_wrapper');
    // If a #heading is provided for the tray, provided a #theme_wrapper
    // function to append it.
    array_unshift($element['tray']['#theme_wrappers'], 'acquia_lift_navbar_tray_heading_wrapper');
  }
  return $element;
}
