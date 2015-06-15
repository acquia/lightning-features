<?php
/**
 * @file
 * Provides test classes for Acquia Lift
 */

/**
 * Classes used for testing.
 */
class DummyAcquiaLiftHttpClient implements AcquiaLiftDrupalHttpClientInterface {

  /**
   * Stores all requests that have been received.
   *
   * @var array
   *   An array of requests.
   */
  protected static $requests_received;

  /**
   * Whether or not this http client should return 500 errors.
   *
   * @var bool
   */
  protected $broken;

    /**
     * The type of breakage to simulate, i.e. client or server side.
     *
     * @var string
     */
  protected $breakageType;

  /**
   * An array of data simulating resources than can be returned.
   *
   * @var array
   */
  protected $data = array();

  /**
   * Generates a dummy response based on the passed in data.
   *
   * @param array $data
   *   An array of data for the response.
   * @return stdClass
   *   An object representing a response from the server.
   */
  protected function generateDummyResponse($data) {
    $response = new stdClass();
    $response->code = 200;
    $response->status_message = 'OK';
    if ($this->broken) {
      if ($this->breakageType == 'client') {
        $response->code = 400;
        $response->status_message = 'Bad request';
      }
      else {
        $response->code = 500;
        $response->status_message = 'Internal Server Error';
      }
    }
    $response->data = drupal_json_encode($data);
    return $response;
  }

  /**
   * Constructor
   *
   * @param bool $broken
   *   Whether or not this http client should just get 500 errors.
   * @param array $data
   *   An array of dummy data that can be returned in responses.
   */
  public function __construct($broken = FALSE, $data = array(), $simulate_client_side_breakage = FALSE) {
    $this->broken = $broken;
    $this->breakageType = $simulate_client_side_breakage ? 'client' : 'server';
    $this->data += $data;
  }

  /**
   * Logs the request internally.
   *
   * @param $type
   *   The type of request, e.g. 'get'
   * @param $uri
   *   The uri of the request.
   * @param $headers
   *   The array of headers.
   * @param $options
   *   An array of options
   * @param null $body
   *   (optional) The body of the request.
   */
  protected function logRequest($type, $uri, $headers, $options, $body = NULL) {
    self::$requests_received[] = array(
      'type' => $type,
      'uri' => $uri,
      'headers' => $headers,
      'options' => $options,
      'body' => $body
    );
  }

  /**
   * Returns all requests that have been made to this client.
   *
   * @return array
   *   An array of requests
   */
  public static function getLoggedRequests() {
    return self::$requests_received;
  }

  public static function clearLoggedRequests() {
    self::$requests_received = array();
  }

  /**
   * Returns the expected data array for a given uri.
   *
   * @param string $uri
   *   An absolute url for an API endpoint, e.g. http://example.com/owner-code/list-agents
   * @return array
   *   An array of data to be returned in the response.
   */
  protected function getDataForURI($uri, $headers = array()) {
    $parsed = parse_url($uri);
    $path_parts = explode('/', $parsed['path']);
    // The first element of the $path_parts array will be an empty string and the second
    // will be the owner code, which we don't need here.
    $path_parts = array_slice($path_parts, 2);
    switch($path_parts[0]) {
      case 'list-agents':
        return array(
          'data' => array(
            'agents' => isset($this->data['agents']) ? $this->data['agents'] : array()
          )
        );
      case 'transforms-options':
        return array(
          'data' => array(
            'options' => isset($this->data['options']) ? $this->data['options'] : array()
          )
        );
      case '-':
        // For some reason the endpoint for getting targeting values is '/-/potential-targeting'
        if ($path_parts[1] != 'potential-targeting') {
          return array();
        }
        if (isset($this->data['features'])) {
          return array(
            'data' => array(
              'potential' => array(
                'features' => $this->data['features']
              )
            )
          );
        }
        if (isset($this->data['reports'])) {
          // Grab the agent name from the querystring.
          $query_params = explode('&', $parsed['query']);
          $first_param = explode('=', reset($query_params));
          if ($first_param[0] !== 'agent') {
            return array();
          }
          $agent_name = $first_param[1];
          if (isset($this->data['reports'][$agent_name]['context-filters'])) {
            return $this->data['reports'][$agent_name]['context-filters'];
          }
        }
        return array(
          'data' => array(
            'potential' => array(
              'features' => array()
            )
          )
        );
      case 'agent-api':
        $agent_name = $path_parts[1];
        if (count($path_parts) == 2) {
          return array(
            'machine-name' => $agent_name,
            'status' => PERSONALIZE_STATUS_NOT_STARTED,
          );
        }
        switch ($path_parts[2]) {
          case 'points':
            // This request must be for the list of points for an agent, or the list of decisions
            // for a point, or the list of choices for a decision. Use a regex to figure out which.
            $matches = array();
            $pattern = '/agent\-api\/[a-zA-Z0-9-_]+(\/points\/[a-zA-Z0-9-_]+\/decisions(\/[a-zA-Z0-9-_]+\/choices)?)?/';
            if (!preg_match($pattern, $parsed['path'], $matches)) {
              return array();
            }
            switch (count($matches)) {
              case 1:
                // This is a request for points for an agent.
                return isset($this->data['points'][$agent_name]) ? $this->data['points'][$agent_name] : array();
              case 2:
                // This is a request for decisions for a point.
                $point_name = $path_parts[3];
                return isset($this->data['decisions'][$agent_name][$point_name]) ? $this->data['decisions'][$agent_name][$point_name] : array();
              case 3:
                // This is a request for choices for a decision.
                $point_name = $path_parts[3];
                $decision_name = $path_parts[5];
                return isset($this->data['choices'][$agent_name][$point_name][$decision_name]) ? $this->data['choices'][$agent_name][$point_name][$decision_name] : array();
            }
            return array();
          case 'goals':
            return isset($this->data['goals'][$agent_name]) ? $this->data['goals'][$agent_name] : array();
        }
        break;
    }
    // Reports (unfortunately not all of which have the same url pattern)
    if (strpos($parsed['path'], 'report/status') !== FALSE) {
      $query_params = explode('&', $parsed['query']);
      $first = explode('=', reset($query_params));
      if ($first[0] !== 'codes') {
        return array();
      }
      $agent_names = explode(',', $first[1]);
      $agent_name = reset($agent_names);
      return isset($this->data['reports'][$agent_name]['agent-status']) ? $this->data['reports'][$agent_name]['agent-status'] : array();
    }
    if (isset($path_parts[1]) && $path_parts[1] == 'report') {
      $matches = array();
      $pattern = '/([a-zA-Z0-9-_]+)\/report\/(confidence|targeting\-features|learning)/';
      if (!preg_match($pattern, $parsed['path'], $matches)) {
        return array();
      }
      $agent_name = $matches[1];
      $report_type = $matches[2];
      if ($report_type == 'confidence' && isset($headers['x-mpath-point'])) {
        // We're getting a confidence report for a particular decision point.
        if (isset($this->data['reports'][$agent_name]['confidence'])) {
          $confidence_report = $this->data['reports'][$agent_name]['confidence'];
          foreach ($confidence_report['data']['items'] as $i => $item) {
            if ($item['point'] !== $headers['x-mpath-point']) {
              unset($confidence_report['data']['items'][$i]);
            }
          }
          $confidence_report['data']['items'] = array_values($confidence_report['data']['items']);
          return $confidence_report;
        }
      }
      return isset($this->data['reports'][$agent_name][$report_type]) ? $this->data['reports'][$agent_name][$report_type] : array();
    }
    return array();
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::get().
   */
  public function get($uri = null, $headers = null, array $options = array())
  {
    $this->logRequest('get', $uri, $headers, $options);
    $data = $this->getDataForURI($uri, $headers);
    return $this->generateDummyResponse($data);
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::put().
   */
  public function put($uri = null, $headers = null, $body = null, array $options = array())
  {
    $this->logRequest('put', $uri, $headers, $options, $body);
    return $this->generateDummyResponse(array('status' => 'ok'));
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::post().
   */
  public function post($uri = null, $headers = null, $body = null, array $options = array())
  {
    $this->logRequest('post', $uri, $headers, $options, $body);
    return $this->generateDummyResponse(array('status' => 'ok'));
  }

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::delete().
   */
  public function delete($uri = null, $headers = null, $body = null, array $options = array())
  {
    $this->logRequest('delete', $uri, $headers, $options, $body);
    return $this->generateDummyResponse(array('status' => 'ok'));
  }

}

class AcquiaLiftTestLogger implements PersonalizeLoggerInterface {
  protected static $logs = array();
  protected $output = FALSE;

  public function __construct($output = FALSE) {
    $this->output = $output;
  }

  public function log($level, $message, array $context = array())
  {
    foreach ($context as $key => $value) {
      if (strpos($message, '{' . $key . '}') !== FALSE) {
        $message = str_replace('{' . $key . '}', $value, $message);
      }
    }
    if ($this->output && function_exists('drupal_set_message')) {
      drupal_set_message($message, $level);
    }
    self::$logs[] = array(
      'level' => $level,
      'message' => $message,
    );
  }

  public function clearLogs() {
    self::$logs = array();
  }

  public function getLogs() {
    return self::$logs;
  }
}

/**
 * Class AcquiaLiftTestReports
 *
 * Helper class providing static methods that return basic reports.
 */
class AcquiaLiftTestReports {

  public static function getBasicConfidenceReport($agent_name) {
    $report = array (
      'agent' => $agent_name,
      'dateFrom' => '2014-08-08',
      'dateThru' => '2014-08-08',
      'data' =>
        array (
          'items' =>
            array (
              0 =>
                array (
                  'owner' => 'test-owner-code',
                  'agent' => $agent_name,
                  'point' => 'osid-1',
                  'choice' => 'osid-1:option-1',
                  'seg' => '(none)',
                  'feature' => '(none)',
                  'goal' => NULL,
                  'count' => 0,
                  'val' => 0,
                  'valSq' => 0,
                  'goals' => 0,
                  'goalsSq' => 0,
                  'date' => '2014-08-08',
                  'totals' =>
                    array (
                      'count' => 0,
                      'goals' => 0,
                      'val' => 0,
                      'goalsPerDecision' => 0,
                      'valPerDecision' => 0,
                      'valPerGoal' => 0,
                    ),
                  'label' => 'option-1',
                  'bLo' => 0,
                  'bHi' => 0,
                  'vVari' => 0,
                  'vMean' => 0,
                  'stdDev' => 0,
                  'tScore' => 0,
                  'dgFree' => 0,
                  'pValue' => -1,
                  'signif' => false,
                  'confidence' => '0.0',
                  'confidenceLevel' => 0,
                  'lift' =>
                    array (
                      'default' => 0,
                      'random' => 0,
                    ),
                  'ts' => 1407456000,
                ),
              1 =>
                array (
                  'owner' => 'test-owner-code',
                  'agent' => $agent_name,
                  'point' => 'osid-1',
                  'choice' => 'osid-1:option-2',
                  'seg' => '(none)',
                  'feature' => '(none)',
                  'goal' => NULL,
                  'count' => 0,
                  'val' => 0,
                  'valSq' => 0,
                  'goals' => 0,
                  'goalsSq' => 0,
                  'date' => '2014-08-08',
                  'totals' =>
                    array (
                      'count' => 0,
                      'goals' => 0,
                      'val' => 0,
                      'goalsPerDecision' => 0,
                      'valPerDecision' => 0,
                      'valPerGoal' => 0,
                    ),
                  'label' => 'option-2',
                  'bLo' => 0,
                  'bHi' => 0,
                  'vVari' => 0,
                  'vMean' => 0,
                  'stdDev' => 0,
                  'tScore' => 0,
                  'dgFree' => 0,
                  'pValue' => -1,
                  'signif' => false,
                  'confidence' => '0.0',
                  'confidenceLevel' => 0,
                  'lift' =>
                    array (
                      'default' => 0,
                      'random' => 0,
                    ),
                  'ts' => 1407456000,
                ),
            ),
          'bLo' => NULL,
          'bHi' => NULL,
        ),
    );
    return $report;
  }

  /**
   * Generates confidence report data based on the passed in parameters.
   *
   * @param $agent_name
   *   The name of the agent to generate a report for.
   *
   * @param array $choices
   *   An array of options for which data should be generated, keyed by decision point
   *   name and with an array of all possible winning options or combinations of options
   *   as values. For multiple decisions at one point, i.e. an MVT, the choices should be
   *   flattened into decision combinations, e.g. 'dec-1:option-a,dec-2:option-b'. E.g.
   *   array(
   *     'my-first-decision-point' => array(
   *       'my-first-decision:option-A',
   *       'my-first-decision:option-B'
   *     ),
   *     'my-MVT-decision-point' => array(
   *       'dec-1:option-a,dec-2:option-a',
   *       'dec-1:option-a,dec-2:option-b',
   *       'dec-1:option-b,dec-2:option-a',
   *       'dec-1:option-b,dec-2:option-b',
   *      )
   *    )
   * @param array $winners
   *   Optionally specify a winning choice or combination for each decision point. The
   *   report data generated will then be such that that option or combination represents
   *   a winner.
   * @return array
   */
  public static function generateConfidenceReportWithWinners($agent_name, $choices, $winners = array()) {
    $today = time();
    $date_from = date('Y-m-d', $today);
    $date_to = date('Y-m-d', $today);
    $items = array();

    foreach ($choices as $point => $options) {
      // Start with dummy values which we'll just increment for each choice.
      $blo = 0.016;
      $vmean = 0.017;
      $bhi = 0.018;
      $num_decisions = 10;
      $num_goals = 100;
      foreach ($options as $i => $choice) {
        $items[$choice] = array(
          'owner' => 'test-owner-code',
          'agent' => $agent_name,
          'point' => $point,
          'choice' => $choice,
          'count' => $num_decisions + (10 * $i),
          'bLo' => $blo + (.001 * $i),
          'bHi' => $bhi + (.001 * $i),
          'vMean' => $vmean + (.001 * $i),
        );
      }
      if (isset($winners[$point])) {
        // For the winning option, we need to make sure the vMean value is higher
        // than the others.
        $winning_blo = $vmean + (.001 * (count($options) + 1));
        $winning_vmean = $winning_blo + .001;
        $winning_vhi = $winning_vmean + .001;
        $items[$winners[$point]]['bLo'] = $winning_blo;
        $items[$winners[$point]]['vMean'] = $winning_vmean;
        $items[$winners[$point]]['bHi'] = $winning_vhi;
      }
    }
    $report = array (
      'agent' => $agent_name,
      'dateFrom' => $date_from,
      'dateThru' => $date_to,
      'data' =>
        array (
          'items' => array_values($items),
          'bLo' => NULL,
          'bHi' => NULL,
        ),
    );
    return $report;
  }

  public static function getBasicTargetingReport($agent_name) {
    $report = array (
      'agent' => $agent_name,
      'dateFrom' => '2014-08-08',
      'dateThru' => '2014-08-08',
      'data' =>
        array (
          'items' =>
            array (
            ),
          'totals' =>
            array (
            ),
        ),
    );
    return $report;
  }

  public static function getBasicStatusReport($agent_name) {
    $report = array (
      'data' =>
        array (
          $agent_name =>
            array (
              'totals' =>
                array (
                  'goals' =>
                    array (
                      'count' => 0,
                    ),
                  'sessions' =>
                    array (
                      'count' => 0,
                    ),
                ),
              'dates' =>
                array (
                  0 =>
                    array (
                      'date' => 1407542400,
                      'sessionCount' => 0,
                      'goalCount' => 0,
                      'goalValue' => 0,
                      'liftOverDefaultUsingGoals' => 0,
                      'liftOverDefaultUsingValue' => 0,
                      'liftOverRandomUsingGoals' => 0,
                      'liftOverRandomUsingValue' => 0,
                      'goalsPerSession' => 0,
                      'valuePerSession' => 0,
                      'valuePerGoal' => 0,
                      'liftOverDefaultUsingGoalsToDate' => 0,
                      'liftOverRandomUsingGoalsToDate' => 0,
                      'liftOverDefaultUsingValueToDate' => 0,
                      'liftOverRandomUsingValueToDate' => 0,
                    ),
                ),
              'trends' =>
                array (
                  'dates' =>
                    array (
                      0 => 1407542400,
                    ),
                  'sessionCount' =>
                    array (
                      0 => 0,
                    ),
                  'goalCount' =>
                    array (
                      0 => 0,
                    ),
                  'goalValue' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverDefaultUsingGoals' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverDefaultUsingValue' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverRandomUsingGoals' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverRandomUsingValue' =>
                    array (
                      0 => 0,
                    ),
                  'goalsPerSession' =>
                    array (
                      0 => 0,
                    ),
                  'valuePerSession' =>
                    array (
                      0 => 0,
                    ),
                  'valuePerGoal' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverDefaultUsingGoalsToDate' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverRandomUsingGoalsToDate' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverDefaultUsingValueToDate' =>
                    array (
                      0 => 0,
                    ),
                  'liftOverRandomUsingValueToDate' =>
                    array (
                      0 => 0,
                    ),
                ),
              'today' =>
                array (
                  'date' => 1407542400,
                  'sessionCount' => 0,
                  'goalCount' => 0,
                  'goalValue' => 0,
                  'liftOverDefaultUsingGoals' => 0,
                  'liftOverDefaultUsingValue' => 0,
                  'liftOverRandomUsingGoals' => 0,
                  'liftOverRandomUsingValue' => 0,
                  'goalsPerSession' => 0,
                  'valuePerSession' => 0,
                  'valuePerGoal' => 0,
                  'liftOverDefaultUsingGoalsToDate' => 0,
                  'liftOverRandomUsingGoalsToDate' => 0,
                  'liftOverDefaultUsingValueToDate' => 0,
                  'liftOverRandomUsingValueToDate' => 0,
                ),
              'described' => '<p>Chooses <strong>adaptively</strong> from two options, <strong>test</strong> and <strong>some-custom-block</strong>.</p>
',
            ),
        ),
    );
    return $report;
  }

  public static function getBasicContextFilters() {
    $filters = array (
      'data' =>
        array (
          'potential' =>
            array (
              'features' =>
                array (
                  0 =>
                    array (
                      'code' => '(none)',
                      'name' => '(Any Visitor)',
                    ),
                ),
            ),
        ),
    );
    return $filters;
  }
}
