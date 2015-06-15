<?php
/**
 * @file
 * Provides an agent type for Acquia Lift
 */

class AcquiaLiftAPI implements PersonalizeLoggerAwareInterface, AcquiaLiftReportDataSourceInterface {

  const API_URL = 'api.lift.acquia.com';

  const EXPLORATION_RATE_RANDOM = 1;

  const FEATURE_STRING_REPLACE_PATTERN = '[^A-Za-z0-9_-]';

  const FEATURE_STRING_MAX_LENGTH = 50;

  const FEATURE_STRING_SEPARATOR_MUTEX = ':';

  const FEATURE_STRING_SEPARATOR_NONMUTEX = '::';

  const VALID_NAME_MATCH_PATTERN = '[A-Za-z0-9_-]';

  const ENABLED_STATUS = 'enabled';

  const PAUSED_STATUS = 'paused';

  const STOPPED_STATUS = 'stopped';

  const PROVISIONAL_STATUS = 'provisional';

  /**
   * The Acquia Lift API key to use.
   *
   * @var string
   */
  protected $api_key;

  /**
   * The Acquia Lift admin key to use.
   *
   * @var string
   */
  protected $admin_key;

  /**
   * The Acquia Lift owner code to use.
   *
   * @var string
   */
  protected $owner_code;

  /**
   * The Acquia Lift API url to use.
   *
   * @var string
   */
  protected $api_url;

  /**
   * An http client for making calls to Acquia Lift.
   *
   * @var AcquiaLiftDrupalHttpClientInterface
   */
  protected $httpClient;

  /**
   * The singleton instance.
   *
   * @var AcquiaLiftAPI
   */
  private static $instance;

  protected $logger = NULL;

  /**
   * Singleton factory method.
   *
   * @return AcquiaLiftAPI
   */
  public static function getInstance($account_info) {
    if (empty(self::$instance)) {
      if (drupal_valid_test_ua()) {
        self::setTestInstance();
        return self::$instance;
      }
      foreach (array('api_key', 'admin_key', 'owner_code') as $key) {
        if (!isset($account_info[$key])) {
          throw new AcquiaLiftCredsException('Acquia Lift account info is not complete.');
        }
      }
      if (!self::codeIsValid($account_info['owner_code'])) {
        throw new AcquiaLiftCredsException('Acquia Lift owner code is invalid.');
      }

      $api_url = self::API_URL;
      $needs_scheme = TRUE;
      if (!empty($account_info['api_url'])) {
        if (!valid_url($account_info['api_url'])) {
          throw new AcquiaLiftCredsException('Acquia Lift API URL is not a valid URL.');
        }
        $api_url = $account_info['api_url'];
        $needs_scheme = strpos($api_url, '://') === FALSE;
      }
      if ($needs_scheme) {
        global $is_https;
        // Use the same scheme for Acquia Lift as we are using here.
        $url_scheme = ($is_https) ? 'https://' : 'http://';
        $api_url = $url_scheme . $api_url;
      }
      if (substr($api_url, -1) === '/') {
        $api_url = substr($api_url, 0, -1);
      }

      self::$instance = new self($account_info['api_key'], $account_info['admin_key'], $account_info['owner_code'], $api_url);
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
   * Returns an AcquiaLiftAPI instance with dummy creds and a dummy HttpClient.
   *
   * This is used during simpletest web tests.
   */
  public static function setTestInstance($broken_http_client = FALSE, $simulate_client_side_breakage = FALSE) {
    module_load_include('inc', 'acquia_lift', 'tests/acquia_lift.test_classes');
    self::$instance = new self('test-api-key', 'test-admin-key', 'test-owner-code', 'http://api.example.com');
    // This method is only ever called within the context of a simpletest web
    // test, so the call to variable_get() is ok here.
    $test_data = variable_get('acquia_lift_web_test_data', array());
    self::$instance->setHttpClient(new DummyAcquiaLiftHttpClient($broken_http_client, $test_data, $simulate_client_side_breakage));
    self::$instance->setLogger(new AcquiaLiftTestLogger(FALSE));
  }

  /**
   * Maps the passed in status code to one consumable by the Lift service.
   *
   * If the passed in status code is already a string status code that's
   * consumable by Lift it just returns that code. If it's a numeric code
   * defined in personalize module it maps it to the corresponding string
   * code. If it's neither, an exception is thrown.
   *
   * @param $status
   *   The status to map.
   * @return string status
   *   The status code consumable by Lift.
   * @throws AcquiaLiftException
   */
  public static function mapStatus($status) {
    $status_map = array(
      PERSONALIZE_STATUS_NOT_STARTED => self::PAUSED_STATUS,
      PERSONALIZE_STATUS_PAUSED => self::PAUSED_STATUS,
      PERSONALIZE_STATUS_RUNNING => self::ENABLED_STATUS,
      PERSONALIZE_STATUS_COMPLETED => self::STOPPED_STATUS
    );
    // Maybe we already have a Lift status string.
    if (in_array($status, $status_map)) {
      return $status;
    }
    // Otherwise map it to a Lift status.
    if (!isset($status_map[$status])) {
      throw new AcquiaLiftException('Unknown agent status: ' . $status);
    }
    return $status_map[$status];
  }

  /**
   * Whether the passed in status code represents a successful response.
   *
   * @param $code
   *   The status code.
   * @return bool
   *   TRUE if the code represents a client error, FALSE otherwise.
   */
  public static function isSuccessful($code) {
    return $code >= 200 && $code < 300;
  }

  /**
   * Whether the passed in status code represents a client side error.
   *
   * @param $code
   *   The status code.
   * @return bool
   *   TRUE if the code represents a client error, FALSE otherwise.
   */
  public static function isClientError($code) {
    return $code >= 400 && $code < 500;
  }

  /**
   * Whether the passed in status code represents a server side error.
   *
   * @param $code
   *   The status code.
   * @return bool
   *   TRUE if the code represents a server error, FALSE otherwise.
   */
  public static function isServerError($code) {
    return $code >= 500 && $code < 600;
  }

  /**
   * Maps the passed in response code to an exception class to use.
   *
   * If the response code passed in constitutes a successful response,
   * NULL is returned. If it does not constitute a 400 or 500 error,
   * NULL is returned.
   *
   * @param $code
   *   The response code to map.
   * @return null|string
   *   The exception class to use or NULL.
   */
  protected static function mapBadResponseToExceptionClass($code) {
    if (self::isSuccessful($code)) {
      return NULL;
    }
    if (self::isClientError($code)) {
      switch ($code) {
        case 404:
          return 'AcquiaLiftNotFoundException';
        case 403:
          return 'AcquiaLiftForbiddenException';
        default:
          return 'AcquiaLiftClientErrorException';
      }
    }
    elseif (self::isServerError($code)) {
      return 'AcquiaLiftServerErrorException';
    }
    return NULL;
  }

  /**
   * Determines whether the passed in string is valid as an owner code.
   *
   * @param $str
   *   The string to check
   * @return bool
   *   Returns FALSE if the string contains any invalid characters, TRUE
   *   otherwise.
   */
  public static function codeIsValid($str) {
    return (bool) preg_match('/^' . self::VALID_NAME_MATCH_PATTERN . '+$/', $str);
  }

  /**
   * Returns a "clean" version of the passed in string.
   *
   * @param $str
   *   The string to be cleaned.
   * @return string
   *   The clean string.
   */
  public static function cleanFeatureString($str) {
    $string = preg_replace('/' . self::FEATURE_STRING_REPLACE_PATTERN . '/', '-', $str);
    // Get rid of instances of multiple '-' characters after replacement.
    $string = preg_replace('/\-{2,}/', '-', $string);
    return $string;
  }

  /**
   * Private constructor as this is a singleton.
   *
   * @param string $api_key
   *   A string representing an Acquia Lift API key.
   * @param string $admin_key
   *   A string representing an Acquia Lift admin key.
   * @param string $owner_code
   *   A string representing an Acquia Lift owner code.
   * @param string $api_url
   *   A string representing an Acquia Lift API url.
   */
  private function __construct($api_key, $admin_key, $owner_code, $api_url) {
    $this->api_key = $api_key;
    $this->admin_key = $admin_key;
    $this->owner_code = $owner_code;
    $this->api_url = $api_url;
  }

  /**
   * Accessor for the api_key property.
   *
   * @return string
   */
  public function getApiKey() {
    return $this->api_key;
  }

  /**
   * Accessor for the admin_key property.
   *
   * @return string
   */
  public function getAdminKey() {
    return $this->admin_key;
  }

  /**
   * Accessor for the owner_code property.
   *
   * @return string
   */
  public function getOwnerCode() {
    return $this->owner_code;
  }

  /**
   * Accessor for the api_url property.
   *
   * @return string
   */
  public function getApiUrl() {
    return $this->api_url;
  }

  /**
   * Returns an http client to use for Acquia Lift calls.
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
   * Setter for the httpClient property.
   *
   * @param AcquiaLiftDrupalHttpClientInterface $client
   *   The http client to use.
   */
  public function setHttpClient(AcquiaLiftDrupalHttpClientInterface $client) {
    $this->httpClient = $client;
  }

  /**
   * Returns the fully qualified URL to use to connect to API.
   *
   * This function handles personalizing the endpoint to the client by
   * handling owner code and API keys.
   *
   * @param $path
   *   The $path to the endpoint at the API base url.
   * @param $admin
   *   Boolean indicating whether to use admin key (true) or runtime (false).
   */
  protected function generateEndpoint($path, $admin = TRUE) {
    $endpoint = $this->api_url . '/';
    $endpoint .= $this->owner_code;
    if (substr($path, 0, 1) !== '/') {
      $endpoint .= '/';
    }
    $endpoint .= $path;
    // Append api key.
    if (strpos($endpoint, '?')) {
      $endpoint .= '&';
    }
    else {
      $endpoint .= '?';
    }
    $key = $admin ? $this->getAdminKey() : $this->getApiKey();
    $endpoint .= "apikey=$key";
    return $endpoint;
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

  /**
   * Tests the connection to Acquia Lift.
   *
   * @return bool
   *   TRUE if the connection succeeded, FALSE otherwise.
   */
  public function pingTest() {
    // We use the list-agents endpoint for our ping test, in the absence of
    // an endpoint specifically provided for this purpose.
    $url = $this->generateEndpoint("/list-agents");
    $admin_response = $this->httpClient()->get($url, array('Accept' => 'application/json'));
    $url = $this->generateEndpoint('/ping-test-agent/expire', FALSE);
    $runtime_response = $this->httpClient()->post($url, array('Accept' => 'application/json'));
    return $admin_response->code == 200 && $runtime_response->code != 403;
  }

  /**
   * Saves an agent to Acquia Lift.
   *
   * @param $machine_name
   *   The machine of the agent.
   * @param $label
   *   The human-readable name of the agent.
   * @param $decision_style
   *   The decision style to use, either 'random' or 'adaptive'
   * @param $status
   *   The status to set the agent to.
   * @param $control_rate
   *   A number between 0 and 1 inclusive representing the percentage to use as a
   *   control group.
   * @param $explore_rate
   *   A number between 0 and 1 inclusive representing the percentage to use as
   *   the continuous experiment group.
   * @param $sticky
   *   Boolean indicating whether decisions made by the agent should stick for each
   *   visitor.
   * @return boolean
   *   TRUE if the agent has been saved to Acquia Lift, FALSE otherwise.
   */
  public function saveAgent($machine_name, $label, $decision_style, $status = 'enabled', $control_rate = 0.1, $explore_rate = .2, $sticky = TRUE) {
    // The selection-mode argument can only be "random" or "adaptive", so coerce the
    // $decision_mode variable if it's somehow something else.
    if ($decision_style !== "random") {
      $decision_style = "adaptive";
    }
    $status = self::mapStatus($status);
    $url = $this->generateEndpoint("/agent-api/$machine_name");
    $agent = array(
      'name' => $label,
      'selection-mode' => $decision_style,
      'status' => $status,
      'control-rate' => $control_rate,
      'explore-rate' => $explore_rate,
      'decision-stickiness' => $sticky ? 'session' : 'none'
    );
    $response = $this->httpClient()->put($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'), $agent);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $machine_name);
    $success_msg = 'The campaign {agent} was pushed to Acquia Lift';
    $fail_msg = 'The campaign {agent} could not be pushed to Acquia Lift';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Updates the status of an agent in Acquia Lift.
   *
   * @param $machine_name
   *   The machine of the agent.
   * @param $status
   *   The status to set the agent to.
   * @throws AcquiaLiftException
   */
  public function updateAgentStatus($machine_name, $status) {
    $lift_status_code = self::mapStatus($status);
    $url = $this->generateEndpoint("/agent-api/$machine_name");
    $agent = array(
      'status' => $lift_status_code,
    );
    $response = $this->httpClient()->put($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'), $agent);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $machine_name);
    $success_msg = 'The new status of campaign {agent} was pushed to Acquia Lift';
    $fail_msg = 'The new status of campaign {agent} could not be pushed to Acquia Lift';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Figures out the correct exception to throw and throws it.
   *
   * @param $response_code
   *   The response code, e.g. 404.
   * @param $fail_msg
   *   The message to use for the exception, and for logging if $log_failure it TRUE.
   * @param array $vars
   *   Variables to pass to the message.
   * @param bool $log_failure
   *   Whether failures should be logged or not
   */
  public function handleBadResponse($response_code, $fail_msg, $vars = array(), $log_failure = TRUE) {
    if ($exception_class = self::mapBadResponseToExceptionClass($response_code)) {
      if ($log_failure) {
        $this->logger()->log(PersonalizeLogLevel::ERROR, $fail_msg, $vars);
      }
      throw new $exception_class(t($fail_msg, $vars));
    }
    if (self::isSuccessful($response_code)) {
      // If the response wasn't actually bad but we still got here somehow, just
      // log a warning and return.
      if ($log_failure) {
        $this->logger()->log(PersonalizeLogLevel::WARNING, $fail_msg, $vars);
      }
      return;
    }
    // Otherwise throw a generic exception.
    throw new AcquiaLiftException($fail_msg);
  }

  /**
   * Saves a decision point for an agent.
   *
   * @param $agent_name
   *   The name of the agent to save the point on.
   * @param $point
   *   The name of the decision point.
   */
  public function savePoint($agent_name, $point_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/$point_name");
    $response = $this->httpClient()->put($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'));
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'decpoint' => $point_name);
    $success_msg = 'The point {decpoint} was pushed to the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not save the point {decpoint} to the Acquia Lift campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Saves a decision for an agent.
   *
   * @param $agent_name
   *   The name of the agent the decision belongs to.
   * @param $point
   *   The name of the decision point that the decision belongs to.
   * @param $decision_name
   *   The name of the decision to save.
   */
  public function saveDecision($agent_name, $point_name, $decision_name, $data = array()) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/$point_name/decisions/$decision_name");
    $response = $this->httpClient()->put($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'), $data);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'decpoint' => $point_name, 'decname' => $decision_name);
    $success_msg = 'The decision {decname} for point {decpoint} was pushed to the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not save decision {decname} for point {decpoint} to the Acquia Lift campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Saves a choice for a decision.
   *
   * @param $agent_name
   *   The name of the agent the decision belongs to.
   * @param $point
   *   The name of the decision point containing the decision the choice
   *   belongs to.
   * @param $decision_name
   *   The name of the decision that the choice belongs to.
   * @param $choice
   *   The name of the choice to save.
   */
  public function saveChoice($agent_name, $point_name, $decision_name, $choice, $data = array()) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/$point_name/decisions/$decision_name/choices/$choice");
    $response = $this->httpClient()->put($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'), $data);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'decpoint' => $point_name, 'choicename' => $decision_name . ': ' . $choice);
    $success_msg = 'The decision choice {choicename} for point {decpoint} was pushed to the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not save decision choice {choicename} for point {decpoint} to the Acquia Lift campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Resets the data for an agent.
   *
   * @param $agent_name
   */
  public function resetAgentData($agent_name) {
    $url = $this->generateEndpoint("/$agent_name/data");
    $response = $this->httpClient()->delete($url);
    $vars = array('agent' => $agent_name);
    $success_msg = 'The data for Acquia Lift campaign {agent} was reset';
    $fail_msg = 'Could not reset data for Acquia Lift campaign {agent}';
    if ($response->code == 200) {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Deletes an agent.
   *
   * @param $agent_name
   *   The name of the agent to delete.
   */
  public function deleteAgent($agent_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name");
    $response = $this->httpClient()->delete($url);
    $vars = array('agent' => $agent_name);
    $success_msg = 'The Acquia Lift campaign {agent} was deleted';
    $fail_msg = 'Could not delete Acquia Lift campaign {agent}';
    if ($response->code == 200) {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Deletes an entire decision point from an agent.
   *
   * @param $agent_name
   *   The name of the agent to delete the point from.
   * @param $point
   *   The name of the decision point to delete.
   */
  public function deletePoint($agent_name, $point_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/$point_name");
    $response = $this->httpClient()->delete($url);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'decpoint' => $point_name);
    $success_msg = 'The decision point {decpoint} was deleted from the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not delete decision point {decpoint} from the Acquia Lift campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Deletes a decision from an agent.
   *
   * @param $agent_name
   *   The name of the agent to delete the decision from.
   * @param $point
   *   The name of the decision point that the decision belongs to.
   * @param $decision_name
   *   The name of the decision to delete.
   */
  public function deleteDecision($agent_name, $point_name, $decision_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/$point_name/decisions/$decision_name");
    $response = $this->httpClient()->delete($url);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'decpoint' => $point_name, 'decname' => $decision_name);
    $success_msg = 'The decision {decname} for point {decpoint} was deleted from the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not delete decision {decname} for point {decpoint} from the Acquia Lift campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Deletes a choice from a decision.
   *
   * @param $agent_name
   *   The name of the agent to delete the choice from.
   * @param $point
   *   The name of the decision point containing the decision from which the
   *   choice is to be deleted.
   * @param $decision_name
   *   The name of the decision that the choice belongs to.
   * @param $choice
   *   The name of the choice to delete.
   */
  public function deleteChoice($agent_name, $point_name, $decision_name, $choice) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/$point_name/decisions/$decision_name/choices/$choice");
    $response = $this->httpClient()->delete($url);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'decpoint' => $point_name, 'choicename' => $decision_name . ': ' . $choice);
    $success_msg = 'The decision choice {choicename} for point {decpoint} was deleted from the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not delete decision choice {choicename} for point {decpoint} from the Acquia Lift campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->logger()->log(PersonalizeLogLevel::ERROR, $fail_msg, $vars);
      throw new AcquiaLiftException($fail_msg);
    }
  }

  /**
   * Saves a goal for an agent.
   *
   * @param $agent_name
   *   The agent the goal belongs to.
   * @param $goal_name
   *   The name of the goal.
   * @param array $data
   *   Array containing further information about the goal.
   */
  public function saveGoal($agent_name, $goal_name, $data = array()) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/goals/$goal_name");
    $response = $this->httpClient()->put($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'), $data);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'goalname' => $goal_name);
    $success_msg = 'The goal {goalname} was pushed to the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not save the goal {goalname} to the Acquia Lift campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Deletes a goal from an agent.
   *
   * @param $agent_name
   *   The agent to delete the goal from.
   * @param $goal_name
   *   The name of the goal.
   */
  public function deleteGoal($agent_name, $goal_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/goals/$goal_name");
    $response = $this->httpClient()->delete($url);
    $vars = array('agent' => $agent_name, 'goalname' => $goal_name);
    $success_msg = 'The goal {goalname} was deleted from the Acquia Lift campaign {agent}';
    $fail_msg = 'Could not delete the goal {goalname} from the Acquia Lift campaign {agent}';
    if ($response->code == 200) {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Retrieves the specified agent from Acquia Lift.
   *
   * @param $machine_name
   *   The machine name of the agent to retrieve.
   *
   * @return bool|array
   *   An array representing the agent or FALSE if none was found.
   */
  public function getAgent($machine_name) {
    $url = $this->generateEndpoint("/agent-api/$machine_name");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));
    if ($response->code == 200) {
      return json_decode($response->data, TRUE);
    }
    else {
      $this->handleBadResponse($response->code, 'Could not retrieve agent from Acquia Lift', array(), FALSE);
    }
    return FALSE;
  }

  /**
   * Gets a list of goals for the specified agent.
   *
   * @param $agent_name
   *   The name of the agent.
   * @return bool|mixed
   *   An array of goal names or FALSE if an error occurs.
   */
  public function getGoalsForAgent($agent_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/goals");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));
    if ($response->code == 200) {
      return json_decode($response->data, TRUE);
    }
    else {
      $this->handleBadResponse($response->code, 'Could not retrieve goals from Acquia Lift', array(), FALSE);
    }
    return FALSE;
  }

  /**
   * Gets a list of decision points for the specified agent.
   *
   * @param $agent_name
   *   The name of the agent.
   * @return bool|mixed
   *   An array of point names or FALSE if an error occurs.
   */
  public function getPointsForAgent($agent_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));

    if ($response->code == 200) {
      return json_decode($response->data, TRUE);
    }
    else {
      $this->handleBadResponse($response->code, 'Could not retrieve decision points from Acquia Lift', array(), FALSE);
    }
    return FALSE;
  }

  /**
   * Gets a list of decisions for the specified agent and
   * decision point.
   *
   * @param $agent_name
   *   The name of the agent.
   * @param $point_name
   *   The name of the decision point.
   * @return bool|mixed
   *   An array of decision names or FALSE if an error occurs.
   */
  public function getDecisionsForPoint($agent_name, $point_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/{$point_name}/decisions");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));

    if ($response->code == 200) {
      return json_decode($response->data, TRUE);
    }
    else {
      $this->handleBadResponse($response->code, 'Could not retrieve decisions from Acquia Lift', array(), FALSE);
    }
    return FALSE;
  }

  /**
   * Gets a list of choices for the specified agent, decision
   * point and decision.
   *
   * @param $agent_name
   *   The name of the agent.
   * @param $point_name
   *   The name of the decision point.
   * @param $decision_name
   *   The name of the decision.
   * @return bool|mixed
   *   An array of choices or FALSE if an error occurs.
   */
  public function getChoicesForDecision($agent_name, $point_name, $decision_name) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/{$point_name}/decisions/$decision_name/choices");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));

    if ($response->code == 200) {
      return json_decode($response->data, TRUE);
    }
    else {
      $this->handleBadResponse($response->code, 'Could not retrieve choices from Acquia Lift', array(), FALSE);
    }
    return FALSE;
  }

  /**
   * Retrieves a list of existing agents from Acquia Lift.
   *
   * @return array
   *   An associative array whose keys are agent names and values are objects
   *   representing agents.
   * @throws AcquiaLiftException
   */
  public function getExistingAgents() {
    $url = $this->generateEndpoint("/list-agents");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));
    if ($response->code == 200) {
      $response = json_decode($response->data, TRUE);
      if (!isset($response['data']['agents'])) {
        return array();
      }
      $existing_agents = array();
      foreach ($response['data']['agents'] as $agent) {
        $existing_agents[$agent['code']] = $agent;
      }
      return $existing_agents;
    }
    else {
      $this->handleBadResponse($response->code, 'Error retrieving agent list from Acquia Lift');
    }
    return array();
  }

  /**
   * Retrieves the list of available source data options from Acquia Lift.
   * @return mixed
   * @throws AcquiaLiftException
   */
  public function getTransformOptions() {
    $url = $this->generateEndpoint("/transforms-options");
    // Use a timeout of 8 seconds for retrieving the transform options.
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));
    if ($response->code == 200) {
      $response = json_decode($response->data, TRUE);
      return $response['data']['options'];
    }
    else {
      $this->handleBadResponse($response->code, 'Error retrieving list of transforms options');
    }
    return FALSE;
  }

  /**
   * Saves a targeting rule with automatic features to Acquia Lift for a particular agent.
   *
   * @param $agent_name
   *   The name of the agent the rule is for.
   * @param $auto_features
   *   The list of automatic targeting features to use.
   */
  public function saveAutoTargetingRule($agent_name, $auto_features) {
    $rule_name = $agent_name . '-auto-targeting';
    $url = $this->generateEndpoint("/transform-rule");
    foreach ($auto_features as &$feature) {
      $feature = '#' . $feature;
    }
    $body = array(
      'code' => $rule_name,
      'status' => 1,
      'agents' => array($agent_name),
      'when' => array(),
      'apply' => array(
        'feature' => implode(',', $auto_features),
      )
    );
    $response = $this->httpClient()->post($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'), $body);
    $vars = array('agent' => $agent_name);
    $success_msg = 'The targeting rule for campaign {agent} was saved successfully';
    $fail_msg = 'The targeting rule could not be saved for campaign {agent}';
    if ($response->code == 200) {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Deletes the auto targeting rule for the specified agent.
   *
   * @param $agent_name
   *   The name of the agent whose targeting rule is to be deleted.
   */
  public function deleteAutoTargetingRule($agent_name) {
    $rule_name = $agent_name . '-auto-targeting';
    $url = $this->generateEndpoint("/transform-rule/$rule_name");
    $response = $this->httpClient()->delete($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'));
    $vars = array('agent' => $agent_name);
    $success_msg = 'The targeting rule for campaign {agent} was deleted successfully';
    $fail_msg = 'The targeting rule could not be deleted for campaign {agent}';
    if ($response->code == 200) {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Returns whether or not a targeting rule exists for the given agent.
   *
   * @param $agent_name
   *   The name of the agent to check.
   * @return bool
   *   TRUE if the specified agent has a targeting rule set up, FALSE otherwise.
   */
  public function hasAutoTargetingRule($agent_name) {
    $rule_name = $agent_name . '-auto-targeting';
    $url = $this->generateEndpoint("/transforms-list");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));
    if ($response->code != 200) {
      $this->handleBadResponse($response->code, 'Problem retrieving auto-targeting rules');
      return FALSE;
    }

    $response = json_decode($response->data, TRUE);
    if (!isset($response['data']) || !isset($response['data']['transforms'])) {
      return FALSE;
    }
    foreach ($response['data']['transforms'] as $rule) {
      if ($rule['code'] == $rule_name) {
        return TRUE;
      }
    }
    return FALSE;
  }

  /**
   * Gets a list of all possible values for the features that have been set.
   *
   * @param $agent_name
   *   The name of the agent to get targeting values for.
   * @return array
   *   An associative array of targeting values structured as follows:
   *    array(
   *      'data' => array(
   *        'potential' => array(
   *          'features' => array(
   *              // A line like this for every possible value
   *              array('code' => 'AFG', 'name' => 'Afghanistan'),
   *           )
   *         )
   *       )
   *     )
   * @throws AcquiaLiftException
   */
  public function getPotentialTargetingValues($agent_name) {
    $url = $this->generateEndpoint("/-/potential-targeting?agent={$agent_name}&include-current=true");
    $headers = array('Accept' => 'application/json');
    $response = $this->httpClient()->get($url, $headers);
    if ($response->code != 200) {
      $this->handleBadResponse($response->code, 'Problem retrieving potential targeting values');
      return array();
    }
    return json_decode($response->data, TRUE);
  }

  /**
   * Gets array of possible targeting values in the format expected by the plugin.
   *
   * @param $agent_name
   *   The name of the agent to get targeting values for.
   * @param $mutex_separator
   *   The string used as a separator for mutually exclusive context values. This
   *   parameter is provided so that the logic can be unit tested.
   * @param $non_mutex_separator
   *   The string used as a separator for non-mutually exclusive context values. This
   *   parameter is provided so that the logic can be unit tested.
   * @return array
   *   An associative array of targeting values with feature names as keys and an
   *   associative array structured as follows for each value:
   *    array(
   *      'mutex' => TRUE, // If values for this feature type are mutually exclusive.
   *      'values' => array(
   *        'some-feature-value-code' => 'Some feature value friendly name'
   *       )
   *     )
   */
  public function getPossibleValues($agent_name, $mutex_separator = NULL, $non_mutex_separator = NULL) {
    $possible_values = array();
    // We make these separators injectable so that we can write unit tests to test the
    // logic here.
    if (empty($mutex_separator)) {
      $mutex_separator = self::FEATURE_STRING_SEPARATOR_MUTEX;
    }
    if (empty($non_mutex_separator)) {
      $non_mutex_separator = self::FEATURE_STRING_SEPARATOR_NONMUTEX;
    }
    $result = $this->getPotentialTargetingValues($agent_name);
    if (isset($result['data']['potential']['features']) && !empty($result['data']['potential']['features'])) {
      // To determine whether values are mutually exclusive we need to check whether
      // they contain the separator that designates them as such, but the separator that
      // designates them as non-mutex could be contained in this so we need some logic
      // here to figure out how to check for this.
      $check = $non_mutex_separator;
      if (strpos($mutex_separator, $non_mutex_separator) !== FALSE) {
        // The mutex separator contains the non-mutex separator so we should use that
        // for our check.
        $check = $mutex_separator;
      }
      foreach ($result['data']['potential']['features'] as $feature) {
        $code = $feature['code'];
        // This logic is seriously gnarly. The absence of the string we check for signifies
        // something different depending on what that string was. E.g. if we are checking for
        // the presence of the non-mutex separator, then its absence means we are dealing with
        // mutex values.
        $mutex = (strpos($code, $check) === FALSE) ? $check === $non_mutex_separator : $check === $mutex_separator;
        $separator = $mutex ? $mutex_separator : $non_mutex_separator;
        $separated = explode($separator, $code);
        if (count($separated) !== 2) {
          continue;
        }
        list($name, $value) = $separated;
        $name = trim($name);
        $value = trim($value);
        $friendly_name = isset($feature['typeName']) ? $feature['typeName'] : $name;

        if (!isset($possible_values[$name])) {
          $possible_values[$name] = array(
            'value type' => 'predefined',
            'mutex' => $mutex,
            'friendly name' => $friendly_name,
            'values' => array(),
          );
        }
        $possible_values[$name]['values'][$value] = $feature['name'];
      }
    }
    return $possible_values;
  }

  /**
   * Saves a mapping of targeting features to options for explicit targeting.
   *
   * @param $agent_name
   *   The name of the agent this mapping is for.
   * @param $point_name
   *   The decision point this mapping is for.
   * @param $map
   *   An array of associative arrays with teh following keys:
   *   - feature A string corresponding to a feature name
   *   - decision A string in the form decision_name:option_id
   * @return array
   *   An array containing the response from Acquia Lift.
   * @throws AcquiaLiftException
   */
  public function saveFixedTargetingMapping($agent_name, $point_name, $map) {
    $url = $this->generateEndpoint("/agent-api/$agent_name/points/$point_name/fixed-targeting");
    $response = $this->httpClient()->put($url, array('Content-Type' => 'application/json; charset=utf-8', 'Accept' => 'application/json'), $map);
    $data = json_decode($response->data, TRUE);
    $vars = array('agent' => $agent_name, 'decpoint' => $point_name);
    $success_msg = 'The fixed targeting mapping for point {decpoint} was successfully saved for campaign {agent}';
    $fail_msg = 'The fixed targeting mapping for point {decpoint} could not be saved for campaign {agent}';
    if ($response->code == 200 && $data['status'] == 'ok') {
      $this->logger()->log(PersonalizeLogLevel::INFO, $success_msg, $vars);
    }
    else {
      $this->handleBadResponse($response->code, $fail_msg, $vars);
    }
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getTargetingImpactReport().
   */
  public function getTargetingImpactReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL) {
    $date_str = $this->getDateString($date_start, $date_end);
    $url = $this->generateEndpoint("/{$agent_name}/report/targeting-features{$date_str}");
    $headers = array('Accept' => 'application/json');
    if ($point !== NULL) {
      $headers['x-mpath-point'] = $point;
    }
    $response = $this->httpClient()->get($url, $headers);
    if ($response->code != 200) {
      $this->handleBadResponse($response->code, 'Problem retrieving targeting impact report.');
      return array();
    }
    return json_decode($response->data, TRUE);
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getAgentStatusReport().
   */
  public function getAgentStatusReport($agent_names, $num_days = NULL) {
    $codes = implode(',', $agent_names);
    $days = (is_null($num_days) || !is_numeric($num_days)) ? '' : '&days=' . $num_days;
    $url = $this->generateEndpoint("/report/status?codes={$codes}{$days}");
    $response = $this->httpClient()->get($url, array('Accept' => 'application/json'));
    if ($response->code != 200) {
      $this->handleBadResponse($response->code, 'Problem retrieving status report.');
      return array();
    }
    return json_decode($response->data, TRUE);
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getConfidenceReport().
   */
  public function getConfidenceReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL, $options = array()) {
    $default_options = array(
      'confidence-measure' => 0.95,
      'aggregated-over-dates' => true,
      'features' => NULL,
    );
    $options = array_merge($default_options, $options);
    $date_str = $this->getDateString($date_start, $date_end);
    if ($options['features'] === 'all') {
      $features = '';
    }
    else {
      $features = is_array($options['features']) ? implode(',', $options['features']) : "(none)";
    }
    unset($options['features']);

    $url = $this->generateEndpoint("/{$agent_name}/report/confidence{$date_str}?features=$features");
    foreach ($options as $param => $value) {
      $param_value = is_bool($value) ? var_export($value, TRUE) : (string)$value;
      $url .= "&$param=" . $param_value;
    }
    $headers = array('Accept' => 'application/json');
    if ($point !== NULL) {
      $headers['x-mpath-point'] = $point;
    }
    // Use a timeout of 8 seconds for retrieving the transform options.
    $response = $this->httpClient()->get($url, $headers);
    if ($response->code != 200) {
      $this->handleBadResponse($response->code, 'Problem retrieving confidence report.');
      return array();
    }
    return json_decode($response->data, TRUE);
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getContextFilters().
   */
  public function getContextFilters($agent_name) {
    return $this->getPotentialTargetingValues($agent_name);
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getRawLearningReport().
   */
  public function getRawLearningReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL) {
    $date_str = $this->getDateString($date_start, $date_end);
    $url = $this->generateEndpoint("/{$agent_name}/report/learning{$date_str}");
    $headers = array('Accept' => 'application/json');
    if ($point !== NULL) {
      $headers['x-mpath-point'] = $point;
    }
    // Use a timeout of 8 seconds for retrieving the transform options.
    $response = $this->httpClient()->get($url, $headers);

    if ($response->code != 200) {
      $this->handleBadResponse($response->code, 'Problem retrieving learning report.');
      return array();
    }
    return json_decode($response->data, TRUE);
  }

  /**
   * Returns the number or runtime API calls that were made during the specified period.
   *
   * @param $date_start
   *   The start date in the format YYYY-MM-DD.
   * @param $date_end
   *   The end date in the format YYYY-MM-DD.
   * @return int
   */
  public function getAPICallsForPeriod($date_start, $date_end) {
    $date_str = $this->getDateString($date_start, $date_end);
    $url = $this->generateEndpoint("/-/report/system-usage{$date_str}");
    $headers = array('Accept' => 'application/json');
    $response = $this->httpClient()->get($url, $headers);
    if ($response->code != 200) {
      $this->handleBadResponse($response->code, 'Problem retrieving API call counts.');
      return array();
    }
    $result = json_decode($response->data, TRUE);
    if (!isset($result['data']) || !isset($result['data'][0]) || !isset($result['data'][0]['calls'])) {
      return array();
    }
    return $result['data'][0]['calls'];
  }

  /*
   * Converts an associative array of call counts to a count of the total number of
   * calls. Calls categorized as "other" (reporting, admin calls) are excluded by
   * default.
   */
  protected function convertCallCountsToTotalCount($counts, $exclude = array('other')) {
    $total_count = 0;
    foreach ($counts as $type => $count) {
      if (in_array($type, $exclude)) {
        continue;
      }
      $total_count += $count;
    }
    return $total_count;
  }

  /**
   * Returns the counts of API calls for the month prior to the date provided.
   *
   * @param $timestamp
   *   The timestamp representing the date from which to calculate the previous
   *   month's API calls.
   *
   * @return array
   *   An associative array with type of call as keys and counts for each type
   *   as values, e.g.
   *   array(
   *     'decisions' => 1000,
   *     'goals' => 100,
   *     'expires' => 2,
   *     'webactions' => 0,
   *     'other' => 10
   *   )
   */
  public function getCallsForPreviousMonth($timestamp) {
    $date = getdate($timestamp);
    $current_month = $date['mon'];
    $current_month_year = $last_month_year = $date['year'];

    if ($current_month == 1) {
      $last_month = 12;
      $last_month_year = $current_month_year - 1;
    }
    else {
      $last_month = $current_month - 1;
      if ($last_month < 10) {
        $last_month = '0' . $last_month;
      }
    }
    // Get a timestamp for the first of the month in question.
    $ts_last_month = strtotime("$last_month/01/$last_month_year");
    // Use this timestamp to get the number of days in that month.
    $num_days_last_month = date('t', $ts_last_month);
    $date_start = $last_month_year . '-' . $last_month . '-01';
    $date_end = $last_month_year . '-' . $last_month . '-' . $num_days_last_month;
    $calls_last_month = $this->getAPICallsForPeriod($date_start, $date_end);
    return $calls_last_month;
  }

  /**
   * Returns the total number of runtimeAPI calls for the month prior to the date
   * provided.
   *
   * @param $timestamp
   *   The timestamp representing the date from which to calculate the previous
   *   month's API calls.

   * @return int
   */
  public function getTotalRuntimeCallsForPreviousMonth($timestamp) {
    $calls_last_month = $this->getCallsForPreviousMonth($timestamp);
    return $this->convertCallCountsToTotalCount($calls_last_month);
  }

  /**
   * Returns counts of API calls from the 1st to the date provided.
   *
   * @param $timestamp
   *   The timestamp representing the date up to which to show API calls
   *   from the start of that month. For example, passing in a timestamp
   *   representing the date October 17th 2013 would return the number of
   *   API calls made from October 1st 2013 to October 17th 2013.
   *
   * @return array
   *   An associative array with type of call as keys and counts for each type
   *   as values, e.g.
   *   array(
   *     'decisions' => 1000,
   *     'goals' => 100,
   *     'expires' => 2,
   *     'webactions' => 0,
   *     'other' => 10
   *   )
   */
  public function getCallsForMonthToDate($timestamp) {
    $date_start = date('Y', $timestamp) . '-' . date('m', $timestamp) . '-01';
    $date_end = date('Y-m-d', $timestamp);
    $calls_this_month = $this->getAPICallsForPeriod($date_start, $date_end);
    return $calls_this_month;
  }

  /**
   * Returns the total number of runtimeAPI calls for the month prior to the date
   * provided.
   *
   * @param $timestamp
   *   The timestamp representing the date from which to calculate the previous
   *   month's API calls.

   * @return int
   */
  public function getTotalRuntimeCallsForMonthToDate($timestamp) {
    $calls_last_month = $this->getCallsForMonthToDate($timestamp);
    return $this->convertCallCountsToTotalCount($calls_last_month);
  }

  /**
   * Returns a unique agent name based on the name passed in.
   *
   * Checks existing agents in Acquia Lift and adds a suffix if the
   * passed in name already exists. Also ensures the name is within
   * the smaller of Acquia Lift's max length restriction and the
   * passed in max length restriction.
   *
   * @param $agent_name
   *   The desired agent name.
   *
   * @param $max_length
   *   The max length restriction other than that imposed by Acquia
   *   Lift itself. The function will use the smaller of the two
   *   max length restrictions.

   * @return string
   *   A machine-readable name for the agent that does not exist yet
   *   in Acquia Lift.
   */
  public function ensureUniqueAgentName($agent_name, $max_length) {
    if ($max_length > self::FEATURE_STRING_MAX_LENGTH) {
      $max_length = self::FEATURE_STRING_MAX_LENGTH;
    }
    $agent_name = substr($agent_name, 0, $max_length);

    $existing = $this->getExistingAgents();
    $index = 0;
    $suffix = '';
    while(in_array($agent_name . $suffix, array_keys($existing))) {
      $suffix = '-' . $index;
      while (strlen($agent_name . $suffix) > $max_length) {
        $agent_name = substr($agent_name, 0, -1);
      }
      $index++;
    }
    return $agent_name . $suffix;
  }

  /**
   * Returns the timeframe portion of a report API url for the specified dates.
   *
   * @param $date_start
   *   The start date in the format YYYY-MM-DD or null to use today's date.
   * @param null $date_end
   *   The end date in the format YYYY-MM-DD or null for a single date.
   * @return string
   *   A string in the format /{start-date}/{end-date}
   */
  protected function getDateString($date_start, $date_end) {
    if ($date_start === NULL || !preg_match('/\d{4}\-\d{2}\-\d{2}/', $date_start)) {
      $date_start = date('Y-m-d');
    }
    $date_str = '/' . $date_start;
    if ($date_end !== NULL && preg_match('/\d{4}\-\d{2}\-\d{2}/', $date_end)) {
      $date_str .= '/' . $date_end;
    }
    return $date_str;
  }

}

interface AcquiaLiftReportDataSourceInterface {
  /**
   * Returns a confidence report for the specified agent and timeframe.
   *
   * @param $agent_name
   *   The name of the agent.
   * @param string $date_start
   *   The start date in the format YYYY-MM-DD or null to use today's date.
   * @param string $date_end
   *   The end date in the format YYYY-MM-DD or null to get a report for just
   *   a single day.
   * @param string $point
   *   An optional decision point to limit the report to.
   * @param array $options
   *   An array of report options to be passed to Acquia Lift where the keys
   *   are the parameter names and the values are the parameter values.
   *   Support includes:
   *     - features: An array of features to include in the report.
   *       Defaults to "none" but can also be set to "all" for all features.
   *     - confidence-measure: The confidence measure to use between 0 and 1.
   *     - aggregated-over-dates: Aggregates the data over the time span or
   *       reports on individual dates
   * @return array
   *   The report as an associative array
   *
   * @throws AcquiaLiftException
   */
  public function getConfidenceReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL, $options = array());

  /**
   * Returns a targeting impact report for the specified agent and timeframe.
   *
   * @param $agent_name
   *   The name of the agent.
   * @param string $date_start
   *   The start date in the format YYYY-MM-DD or null to use today's date.
   * @param string $date_end
   *   The end date in the format YYYY-MM-DD or null to get a report for just
   *   a single day.
   * @param string $point
   *   An optional decision point to limit the report to.
   * @return array
   *   The report as an associative array
   *
   * @throws AcquiaLiftException
   */
  public function getTargetingImpactReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL);

  /**
   * Returns status reports for the specified agents and number of days.
   *
   * @param $agent_names
   *   An array of agent names to return status reports for.
   * @param null $num_days
   *   Number of days to return reports for, or NULL to get the default
   *   14 days.
   *
   * @return array
   *   The report as an associative array
   *
   * @throws AcquiaLiftException
   */
  public function getAgentStatusReport($agent_names, $num_days = NULL);

  /**
   * Returns raw data about the accumulated value of options for the specified
   * agent.
   *
   * @param $agent_name
   *   The name of the agent.
   * @param string $date_start
   *   The start date in the format YYYY-MM-DD or null to use today's date.
   * @param string $date_end
   *   The end date in the format YYYY-MM-DD or null to get a report for just
   *   a single day.
   * @param string $point
   *   An optional decision point to limit the report to.
   * @return array
   *   The report as an associative array
   *
   * @todo Add support for the other optional parameters, i.e. 'confidence-
   *   measure', 'comparison-decision' and 'use-bonferroni'
   *
   * @throws AcquiaLiftException
   */
  public function getRawLearningReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL);


  /**
   * Returns the context values to use in a report.
   * @param $agent_name
   */
  public function getContextFilters($agent_name);

}

class AcquiaLiftReportDataFromFile implements AcquiaLiftReportDataSourceInterface {

  /**
   * The path to the file that acts as the reporting source.
   *
   * @var string
   */
  protected $fileLocation;

  /**
   * An array of reports to act as a request length storage for the reports.
   *
   * @var array
   */
  protected $reports;

  public function __construct($file_location, AcquiaLiftReportCacheInterface $cache = NULL) {
    $this->fileLocation = $file_location;
    $this->cache = $cache;
  }

  /**
   * Retrieves a particular report for the specifed agent.
   *
   * @param $agent_name
   *   The name of the agent to retrieve a report for.
   * @param $report_name
   *   The name of teh report to retrieve, one of 'targeting-impact', 'confidence',
   *   'agent-status', 'raw-learning', or 'context-filters'.
   * @return array
   *   An array representing the report.
   */
  protected function getReport($agent_name, $report_name) {

    // First see if we already have the reports in memory.
    if ($this->reports) {
      return isset($this->reports[$report_name]) ? $this->reports[$report_name] : array();
    }
    // Check the cache if available
    if ($this->cache && $reports = $this->cache->getCachedReports($agent_name)) {
      $this->reports = $reports;
      return isset($reports[$report_name]) ? $reports[$report_name] : array();
    }
    // Otherwise resort to reading from a file.
    else {
      $file_name = $this->fileLocation;
      if (strpos($file_name, '://') === FALSE) {
        // If not given full URL, assume it's an absolute path for
        // the current site.
        global $base_url;
        $file_name = $base_url . $file_name;
      }

      if ($str = file_get_contents($file_name)) {
        $parsed = json_decode($str, TRUE);
        $this->reports = $parsed['reports'];
        if ($this->cache) {
          $this->cache->cacheReports($agent_name, $this->reports);
        }
        return isset($this->reports[$report_name]) ? $this->reports[$report_name] : array();
      }
    }
    return array();
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getTargetingImpactReport().
   */
  public function getTargetingImpactReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL) {
    return $this->getReport($agent_name, 'targeting-impact');
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getAgentStatusReport().
   */
  public function getAgentStatusReport($agent_names, $num_days = NULL) {
    $agent_name = reset($agent_names);
    return $this->getReport($agent_name, 'agent-status');
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getConfidenceReport().
   */
  public function getConfidenceReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL, $options = array()) {
    $report_name = 'confidence';
    if (!empty($options['goal'])) {
      // There are two sample goal reports included in the test file structure.
      // Pick one at random to show for the selected goal.
      $report_name .= '_goal' . rand(1, 2);
    }
    if (isset($options['aggregated-over-dates']) && $options['aggregated-over-dates'] === FALSE) {
      $report_name .= '_detail';
    }
    return $this->getReport($agent_name, $report_name);
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getRawLearningReport().
   */
  public function getRawLearningReport($agent_name, $date_start = NULL, $date_end = NULL, $point = NULL) {
    return $this->getReport($agent_name, 'raw-learning');
  }

  /**
   * Implements AcquiaLiftReportDataSourceInterface::getContextFilters().
   */
  public function getContextFilters($agent_name) {
    $context_filters = $this->getReport($agent_name, 'context-filters');
    return !empty($context_filters) ? $context_filters : array(
      'data' => array(
        'potential' => array(
          'features' => array()
        )
      )
    );
  }
}

interface AcquiaLiftReportCacheInterface {
  /**
   * Retrieve reports from the cache for a particular agent.
   *
   * @param $agent_name
   *   The name of the agent to retrieve reports for.
   * @return mixed
   *   An array representing the reports or NULL of not found.
   */
  public function getCachedReports($agent_name);

  /**
   * Store reports for a particular agent in the cache.
   *
   * @param $agent_name
   *   The name of the agent to store reports for.
   * @param $data
   *   An array representing the reports.
   */
  public function cacheReports($agent_name, $data);
}

/**
 * Class AcquiaLiftReportCache
 *
 * An OOP wrapper around cache_get() and cache_set().
 */
class AcquiaLiftReportCache implements AcquiaLiftReportCacheInterface {
  /**
   * Holds the reports retrieved from the cache.
   *
   * @var array
   */
  protected $cache = array();

  /**
   * The cache bin to use.
   *
   * @var string
   */
  protected $bin = 'cache_acquia_lift_reports';

  /**
   * Implements AcquiaLiftReportCacheInterface::getCachedReports().
   */
  public function getCachedReports($agent_name) {
    if (!isset($this->cache[$agent_name])) {
      if ($get = cache_get($agent_name, $this->bin)) {
        $this->cache[$agent_name] = $get->data;
      }
      else {
        $this->cache[$agent_name] = FALSE;
      }
    }
    return $this->cache[$agent_name];
  }

  /**
   * Implements AcquiaLiftReportCacheInterface::cacheReports().
   */
  public function cacheReports($agent_name, $data) {
    cache_set($agent_name, $data, $this->bin);
  }
}

/**
 * A simple interface for http clients.
 *
 * The point of this is to make our AcquiaLiftAPI class unit testable. It
 * is modeled after Guzzle's ClientInterface. The AcquiaLiftAPI class relies
 * on an object implementing this interface to make calls to the Acquia Lift
 * service. For tests we can pass in a dummy http client so that no calls
 * are actually made.
 *
 * This also makes our code more maintainable between D7 and D8.
 *
 * @see tests/acquia_lift.test
 */
interface AcquiaLiftDrupalHttpClientInterface {

  /**
   * Create a GET request for the client
   *
   * @param string|array     $uri     Resource URI
   * @param array|Collection $headers HTTP headers
   * @param array            $options Options to apply to the request. For BC compatibility, you can also pass a
   *                                  string to tell Guzzle to download the body of the response to a particular
   *                                  location. Use the 'body' option instead for forward compatibility.
   */
  public function get($uri = null, $headers = null, array $options = array());

  /**
   * Create a PUT request for the client
   *
   * @param string|array                        $uri     Resource URI
   * @param array|Collection                    $headers HTTP headers
   * @param string|resource|EntityBodyInterface $body    Body to send in the request
   * @param array                               $options Options to apply to the request
   */
  public function put($uri = null, $headers = null, $body = null, array $options = array());

  /**
   * Create a POST request for the client
   *
   * @param string|array                                $uri      Resource URI
   * @param array|Collection                            $headers  HTTP headers
   * @param array|Collection|string|EntityBodyInterface $postBody POST body. Can be a string, EntityBody, or
   *                                                    associative array of POST fields to send in the body of the
   *                                                    request. Prefix a value in the array with the @ symbol to
   *                                                    reference a file.
   * @param array                                       $options Options to apply to the request
   */
  public function post($uri = null, $headers = null, $body = null, array $options = array());
  /**
   * Create a DELETE request for the client
   *
   * @param string|array                        $uri     Resource URI
   * @param array|Collection                    $headers HTTP headers
   * @param string|resource|EntityBodyInterface $body    Body to send in the request
   * @param array                               $options Options to apply to the request
   */
  public function delete($uri = null, $headers = null, $body = null, array $options = array());
}

/**
 * This is just an OOP wrapper around drupal_http_request.
 */
class AcquiaLiftDrupalHttpClient implements AcquiaLiftDrupalHttpClientInterface {

  // Some constants for our timeout values.
  const REQUEST_TIMEOUT_VALUE_GET = 8.0;
  const REQUEST_TIMEOUT_VALUE_DEFAULT = 15.0;

  protected function encodeBody($body) {
    if (is_string($body)) {
      $data = $body;
    }
    else {
      $data = drupal_json_encode($body);
    }
    return $data;
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::get().
   */
  public function get($uri = NULL, $headers = NULL, array $options = array()) {
    $headers = $headers ? $headers : array();
    $options += array(
      'timeout' => self::REQUEST_TIMEOUT_VALUE_GET
    );
    $options = array('method' => 'GET', 'headers' => $headers) + $options;
    return drupal_http_request($uri, $options);
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::put().
   */
  public function put($uri = NULL, $headers = NULL, $body = NULL, array $options = array()) {
    $data = ($body === NULL ? NULL : $this->encodeBody($body));
    $headers = $headers ? $headers : array();
    $options += array(
      'timeout' => self::REQUEST_TIMEOUT_VALUE_DEFAULT
    );
    $options = array('method' => 'PUT', 'data' => $data, 'headers' => $headers) + $options;
    return drupal_http_request($uri, $options);
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::post().
   */
  public function post($uri = NULL, $headers = NULL, $body = NULL, array $options = array()) {
    $data = $body ? $this->encodeBody($body) : NULL;
    $headers = $headers ? $headers : array();
    $options += array(
      'timeout' => self::REQUEST_TIMEOUT_VALUE_DEFAULT
    );
    $options = array('method' => 'POST', 'data' => $data, 'headers' => $headers) + $options;
    return drupal_http_request($uri, $options);
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::delete().
   */
  public function delete($uri = null, $headers = null, $body = null, array $options = array()) {
    $data = $body ? $this->encodeBody($body) : NULL;
    $headers = $headers ? $headers : array();
    $options += array(
      'timeout' => self::REQUEST_TIMEOUT_VALUE_DEFAULT
    );
    $options = array('method' => 'DELETE', 'data' => $data, 'headers' => $headers) + $options;
    return drupal_http_request($uri, $options);
  }
}

/**
 * Class AcquiaLiftQueue
 *
 * This class is used for queueing http requests to Acquia Lift. It
 * prevents duplicate items from being added during a request, which
 * can happen when there are multiple calls to the agent save or option
 * set save or delete functions.
 */
class AcquiaLiftQueue extends SystemQueue {

  public static $items = array();

  public static $retries = array();

  const MAX_RETRIES = 15;

  /**
   * Takes necessary steps when an item has failed permanently.
   *
   * @param $item
   */
  public static function handleFailedItem($item) {
    if (!isset($item->data['agent'])) {
      return;
    }
    personalize_pause_if_running($item->data['agent']);
  }

  /**
   * Overrides SystemQueue::createItem().
   *
   * Prevents duplicate requests from being queued up during the same
   * page request.
   *
   * @param $data
   * @return bool|void
   */
  public function createItem($data) {
    if (isset($data['hash'])) {
      if (in_array($data['hash'], self::$items)) {
        return;
      }
      else {
        self::$items[] = $data['hash'];
        unset($data['hash']);
      }
    }
    parent::createItem($data);
  }

  /**
   * Overrides SystemQueue::releaseItem().
   *
   * Imposes a maximum number of retries for released items.
   *
   * @param $data
   * @return bool|void
   */
  public function releaseItem($item) {
    if (!isset(self::$retries[$item->item_id])) {
      self::$retries[$item->item_id] = 0;
    }
    elseif(self::$retries[$item->item_id] >= self::MAX_RETRIES) {
      self::handleFailedItem($item);
      $this->deleteItem($item);
      return;
    }
    parent::releaseItem($item);
    self::$retries[$item->item_id]++;
  }

  /**
   * Overrides SystemQueue::deleteQueue().
   */
  public function deleteQueue() {
    parent::deleteQueue();
    self::$items = array();
  }
}

class AcquiaLiftException extends Exception {}
class AcquiaLiftServerErrorException extends AcquiaLiftException {}
class AcquiaLiftClientErrorException extends AcquiaLiftException {}
class AcquiaLiftNotFoundException extends AcquiaLiftClientErrorException {}
class AcquiaLiftForbiddenException extends AcquiaLiftClientErrorException {}
class AcquiaLiftCredsException extends AcquiaLiftException {}
