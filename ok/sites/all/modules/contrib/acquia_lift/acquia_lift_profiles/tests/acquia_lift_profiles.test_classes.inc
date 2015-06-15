<?php

require_once(dirname(__FILE__) . '/../../tests/acquia_lift.test_classes.inc');

/**
 * Classes used for testing.
 */
class DummyALProfilesHttpClient extends DummyAcquiaLiftHttpClient {

  /**
   * Prepopulate some dummy data for the API to return.
   *
   * @var array
   */
  protected $data = array('segments' => array('segment1', 'segment2'), 'events' => array());

  /**
   * Implements AcquiaLiftDrupalHttpClientInterface::get().
   */
  public function get($uri = null, $headers = null, array $options = array())
  {
    $this->logRequest('get', $uri, $headers, $options);
    $data = array();
    if (strpos($uri, 'segments') !== FALSE) {
      $data = isset($this->data['segments']) ? $this->data['segments'] : array();
    }
    elseif (strpos($uri, 'events') !== FALSE) {
      $data = isset($this->data['events']) ? $this->data['events'] : array();
    }
    return $this->generateDummyResponse($data);
  }
}
