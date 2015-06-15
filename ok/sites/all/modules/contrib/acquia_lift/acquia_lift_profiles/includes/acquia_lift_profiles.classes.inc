<?php
/**
 * @file
 * Provides an agent type for Acquia Lift Profiles
 */

class ALProfilesAPI {
  /**
   * An http client for making calls to Acquia Lift Profiles.
   *
   * @var AcquiaLiftDrupalHttpClientInterface
   */
  protected $httpClient;

  /**
   * The API URL for Acquia Lift Profiles.
   *
   * @var string
   */
  protected $apiUrl;

  /**
   * The Acquia Lift Profiles account name to use.
   *
   * @var string
   */
  protected $accountName;

  /**
   * The access key to use for authorization.
   *
   * @var string
   */
  protected $accessKey;

  /**
   * The secret key to use for authorization.
   *
   * @var string
   */
  protected $secretKey;

  /**
   * The list of headers that can be used in the canonical request.
   *
   * @var array
   */
  protected $headerWhitelist = array(
    'Accept',
    'Host',
    'User-Agent'
  );

  /**
   * The logger to use for errors and notices.
   *
   * @var PersonalizeLoggerInterface
   */
  protected $logger = NULL;

  /**
   * The singleton instance.
   *
   * @var ALProfilesAPI
   */
  private static $instance;

  /**
   * Singleton factory method.
   *
   * @param $account_name
   *   The name of the Acquia Lift Profiles account.
   * @param $api_url
   *   The URL to use for API calls.
   * @param $access_key
   *   The access key to use for authorization.
   * @param $secret_key
   *   The secret key to use for authorization.
   *
   * @return ALProfilesAPI
   */
  public static function getInstance($account_name = '', $customer_site = '', $api_url = '', $access_key = '', $secret_key = '') {
    if (empty(self::$instance)) {
      if (drupal_valid_test_ua()) {
        self::setTestInstance();
        return self::$instance;
      }
      // If no account name or api url has been passed in, fallback to getting them
      // from the variables.
      if (empty($account_name)) {
        $account_name = variable_get('acquia_lift_profiles_account_name', '');
      }
      if (empty($customer_site)) {
        $customer_site = variable_get('acquia_lift_profiles_site_name', '');
      }
      if (empty($api_url)) {
        $api_url = variable_get('acquia_lift_profiles_api_url', '');
      }
      if (empty($access_key)) {
        $access_key = variable_get('acquia_lift_profiles_access_key', '');
      }
      if (empty($secret_key)) {
        $secret_key = variable_get('acquia_lift_profiles_secret_key', '');
      }
      // If either account name or API URL is still missing, bail.
      if (empty($api_url) || empty($account_name) || empty($access_key) || empty($secret_key)) {
        throw new ALProfilesCredsException('Missing acquia_lift_profiles account information.');
      }
      if (!valid_url($api_url)) {
        throw new ALProfilesCredsException('API URL is not a valid URL.');
      }
      $needs_scheme = strpos($api_url, '://') === FALSE;
      if ($needs_scheme) {
        global $is_https;
        // Use the same scheme for Acquia Lift Profiles as we are using here.
        $url_scheme = ($is_https) ? 'https://' : 'http://';
        $api_url = $url_scheme . $api_url;
      }
      if (substr($api_url, -1) === '/') {
        $api_url = substr($api_url, 0, -1);
      }
      self::$instance = new self($account_name, $customer_site, $api_url, $access_key, $secret_key);
    }
    return self::$instance;
  }

  /**
   * Resets the singleton instance.
   *
   * Used in unit tests.
   */
  public static function reset() {
    self::$instance = NULL;
  }

  /**
   * Sets the ALProfilesAPI instance with dummy creds and a dummy HttpClient.
   *
   * This is used during simpletest web tests.
   */
  public static function setTestInstance($broken_http_client = FALSE) {
    module_load_include('inc', 'acquia_lift_profiles', 'tests/acquia_lift_profiles.test_classes');
    self::$instance = new self('TESTACCOUNT', 'TESTSITE', 'http://api.example.com', 'testUser', 'testPass');
    $test_data = variable_get('acquia_lift_profiles_web_test_data', array());
    self::$instance->setHttpClient(new DummyALProfilesHttpClient($broken_http_client, $test_data));
    self::$instance->setLogger(new AcquiaLiftTestLogger(TRUE));
  }

  /**
   * Private constructor as this is a singleton.
   *
   * @param $account_name
   *   The name of the Acquia Lift Profiles account.
   * @param $api_url
   *   The URL to use for API calls.
   * @param $access_key
   *   The access key to use for authorization.
   * @param $secret_key
   *   The secret key to use for authorization.
   */
  private function __construct($account_name, $site, $api_url, $access_key, $secret_key) {
    $this->accountName = $account_name;
    $this->customerSite = $site;
    $this->apiUrl = $api_url;
    $this->accessKey = $access_key;
    $this->secretKey = $secret_key;
  }

  /**
   * Accessor for the accountName property.
   *
   * @return string
   */
  public function getAccountName() {
    return $this->accountName;
  }

  /**
   * Accessor for the customerSite property.
   *
   * @return string
   */
  public function getCustomerSite() {
    return $this->customerSite;
  }

  /**
   * Accessor for the apiUrl property.
   *
   * @return string
   */
  public function getApiUrl() {
    return $this->apiUrl;
  }

  /**
   * Returns an http client to use for Acquia Lift Profiles calls.
   *
   * @return AcquiaLiftDrupalHttpClientInterface
   */
  protected function httpClient() {
    if (!isset($this->httpClient)) {
      $this->httpClient = new AcquiaLiftDrupalHttpClient();
    }
    return $this->httpClient;
  }

  /**
   * Generates an endpoint for a particular section of the Acquia Lift Profiles API.
   *
   * @param string $path
   *   The endpoint path, e.g. 'segments' or 'events/my-event'
   * @return string
   *   The endpoint to make calls to.
   */
  protected function generateEndpoint($path) {
    return $this->apiUrl . '/dashboard/rest/' . $this->accountName . '/' . $path;
  }

  /**
   * Setter for the httpClient property.
   *
   * @param AcquiaLiftDrupalHttpClientInterface $client
   *   The http client to use.
   */
  public function setHttpClient(AcquiaLiftDrupalHttpClientInterface $client) {
    $this->httpClient = $client;
  }

  /**
   * Returns the canonical representation of a request.
   *
   * @param $method
   *   The request method, e.g. 'GET'.
   * @param $path
   *   The path of the request, e.g. '/dashboard/rest/[ACCOUNTNAME]/segments'.
   * @param array $parameters
   *   An array of request parameters.
   * @param array $headers
   *   An array of request headers.
   * @param bool $add_extra_headers
   *   Whether to add the extra headers that we know drupal_http_request will add
   *   to the request. Set to FALSE if the request will not be handled by
   *   drupal_http_request.
   *
   * @return string
   *   The canonical representation of the request.
   */
  public function canonicalizeRequest($method, $url, $parameters = array(), $headers = array(), $add_extra_headers = TRUE) {
    $parsed_url = parse_url($url);
    $str = strtoupper($method) . "\n";
    // Certain headers may get added to the actual request so we need to
    // add them here.
    if ($add_extra_headers && !isset($headers['User-Agent'])) {
      $headers['User-Agent'] = 'Drupal (+http://drupal.org/)';
    }
    if ($add_extra_headers && !isset($headers['Host'])) {
      $headers['Host'] = $parsed_url['host'] . (!empty($parsed_url['port']) ? ':' . $parsed_url['port'] : '');
    }
    // Sort all header names alphabetically.
    $header_names = array_keys($headers);
    uasort($header_names, create_function('$a, $b', 'return strtolower($a) < strtolower($b) ? -1 : 1;'));
    // Add each header (trimmed and lowercased) and value to the string, separated by
    // a colon, and with a new line after each header:value pair.
    foreach ($header_names as $header) {
      if (!in_array($header, $this->headerWhitelist)) {
        continue;
      }
      $str .= trim(strtolower($header)) . ':' . trim($headers[$header]) . "\n";
    }
    // Add the path.
    $str .= $parsed_url['path'];
    // Sort any parameters alphabetically and add them as a querystring to our string.
    if (!empty($parameters)) {
      ksort($parameters);
      $first_param = key($parameters);
      $str .= '?' . $first_param . '=' . array_shift($parameters);
      foreach ($parameters as $key => $value) {
        $str .= '&' . $key . '=' . $value;
      }
    }
    return $str;
  }

  /**
   * Returns a string to use for the 'Authorization' header.
   *
   * @return string
   */
  public function getAuthHeader($method, $path, $parameters = array(), $headers = array()) {
    $canonical = $this->canonicalizeRequest($method, $path, $parameters, $headers, is_a($this->httpClient(), 'AcquiaLiftDrupalHttpClient'));
    $hmac = base64_encode(hash_hmac('sha1', (string) $canonical, $this->secretKey, TRUE));
    return 'HMAC ' . $this->accessKey . ':' . $hmac;
  }

  /**
   * Fetches the available Segment IDs from Acquia Lift Profiles
   *
   * @return array
   *   An array of segment IDs that can be used for targeting.
   */
  public function getSegments() {
    // First get our Authorization header.
    $headers = array('Accept' => 'application/json');
    $url = $this->generateEndpoint('segments');
    $auth_header = $this->getAuthHeader('GET', $url, array(), $headers);
    $headers += array('Authorization' => $auth_header);

    $response = $this->httpClient()->get($url, $headers);
    if ($response->code != 200 || !isset($response->data)) {
      if ($response->code != 200) {
        $vars = array(
          'rest_api_status' => $response->code,
          'rest_api_status_line' => $response->status_message,);
        $fail_msg_template = 'Could not get segments from Acquia Lift Profiles "{rest_api_status} {rest_api_status_line}"';
        $this->logger()->log(PersonalizeLogLevel::ERROR, $fail_msg_template, $vars);
        $fail_msg = str_replace(array('{rest_api_status}','{rest_api_status_line}'),
          array($vars['rest_api_status'],$vars['rest_api_status_line']), $fail_msg_template);
        throw new ALProfilesException($fail_msg);
      }
      return array();
    }
    $data = json_decode($response->data, TRUE);
    if (is_array($data)) {
      $segments = array_values(array_filter($data));
      return $segments;
    }
    return array();
  }

  /**
   * Saves an event to Acquia Lift Profiles
   *
   * @param string $machine_name
   *   The canonical, machine-readable name of the event.
   * @param string $label
   *   The human-readable name of the event.
   * @param string $event_type
   *   The type of event, can be one of 'CAMPAIGN_ACTION', 'CAMPAIGN_CLICK_THROUGH',
   *   'CAMPAIGN_CONVERSION', or 'OTHER' (default).
   *
   * @throws ALProfilesException
   */
  public function saveEvent($event_name, $event_label, $event_type = 'OTHER') {
    // First get our Authorization header.
    $headers = array('Accept' => 'application/json');
    $url = $this->generateEndpoint('events/' . $event_name);
    $params = array('label' => $event_label, 'type' => $event_type);
    if (!empty($this->customerSite)) {
      $params['customerSite'] = $this->customerSite;
    }
    $auth_header = $this->getAuthHeader('PUT', $url, $params, $headers);
    $headers += array('Authorization' => $auth_header);
    $querystring = empty($this->customerSite) ? '?' : '?customerSite=' . $this->customerSite . '&';
    $querystring .= 'label=' . rawurlencode($event_label) . '&type=' . $event_type;
    $response = $this->httpClient()->put($url . $querystring, $headers);
    $vars = array('eventname' => $event_name);
    $success_msg = 'The event {eventname} has been saved to Acquia Lift Profiles';
    $fail_msg = 'Could not save event {eventname} to Acquia Lift Profiles';
    if ($response->code == 200) {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->logger()->log(PersonalizeLogLevel::ERROR, $fail_msg, $vars);
      throw new ALProfilesException($fail_msg);
    }
  }

  /**
   * Deletes an event from Acquia Lift Profiles
   *
   * @param $event_name
   *   The name of the event.
   *
   * @throws ALProfilesException
   */
  public function deleteEvent($event_name) {
    // First get our Authorization header.
    $url = $this->generateEndpoint('events/' . $event_name);

    $params = array();
    $querystring = '';
    if (!empty($this->customerSite)) {
      $params['customerSite'] = $this->customerSite;
      $querystring = '?customerSite=' . $this->customerSite;
    }
    $auth_header = $this->getAuthHeader('DELETE', $url, $params);
    $response = $this->httpClient()->delete($url . $querystring, array('Authorization' => $auth_header));
    $vars = array('eventname' => $event_name);
    $success_msg = 'The event {eventname} was deleted from Acquia Lift Profiles';
    $fail_msg = 'Could not delete event {eventname} from Acquia Lift Profiles';
    if ($response->code == 200) {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->logger()->log(PersonalizeLogLevel::ERROR, $fail_msg, $vars);
      throw new ALProfilesException($fail_msg);
    }
  }

  /**
   * Returns the logger to use.
   *
   * @return PersonalizeLoggerInterface
   */
  protected function logger() {
    if ($this->logger !== NULL) {
      return $this->logger;
    }
    return new PersonalizeLogger();
  }

  /**
   * Implements PersonalizeLoggerAwareInterface::setLogger().
   */
  public function setLogger(PersonalizeLoggerInterface $logger)
  {
    $this->logger = $logger;
  }

}

class ALProfilesException extends Exception {};
class ALProfilesCredsException extends ALProfilesException {};
