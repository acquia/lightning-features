<?php
/**
 * @file acquia_lift_profiles.drush.inc
 * Provides Acquia Lift Profiles drush commands.
 */

/**
 * Implements hook_drush_command().
 */
function acquia_lift_profiles_drush_command() {
  $items['acquia-lift-profiles-get-segments'] = array (
    'description' => 'List segments',
    'aliases' => array('alp-seg'),
    'arguments' => array(
      'accessKey' => 'The access key to use (optional)',
      'secretKey' => 'The secret key to use (optional)',
    ),
  );
  return $items;
}

/**
 * Prints out all the segments in Acquia Lift Web for the configured customer.
 *
 * @param null $accessKey Optional access key to use (if null then the configured access key is used)
 * @param null $secretKey Optional secret key to use (if null then the configured secret key is used)
 */
function drush_acquia_lift_profiles_get_segments($accessKey = NULL, $secretKey = NULL) {
  drupal_set_message(T('Get Segments') );
  drupal_set_message(T('Account Name : '.variable_get('acquia_lift_profiles_account_name', '')));
  drupal_set_message(T('API URL : '.variable_get('acquia_lift_profiles_api_url', '')));
  if ( $accessKey == NULL ) {
    $accessKey = variable_get('acquia_lift_profiles_access_key', '');
  }
  if ( $secretKey == NULL ) {
    $secretKey = variable_get('acquia_lift_profiles_secret_key', '');
  }
  drupal_set_message(T('Access Key : '.$accessKey));
  drupal_set_message(T('Secret Key : '.$secretKey));
  drupal_set_message(T('Segments : '));
  try {
    $returnSegments = ALProfilesAPI::getInstance(variable_get('acquia_lift_profiles_account_name', ''), variable_get('acquia_lift_profiles_site_name', ''), variable_get('acquia_lift_profiles_api_url', ''), $accessKey, $secretKey)
      ->getSegments();
    foreach ($returnSegments as $segment) {
      drupal_set_message($segment);
    }
  } catch ( Exception $e ) {
    drupal_set_message(T($e->getMessage()));
  }
}
