/**
 * @file core_personalization.tests.js
 */

var $ = Drupal.jQuery;

QUnit.module("Acquia Lift Cookie Queue", {
  setup: function() {
    initializeLiftSettings();
  }
});

QUnit.test("QueueItem unit tests", function(assert) {
  expect(10);

  var itemData = {
    'testid': 'abc',
    'testdata': 'foo'
  };
  var item = new Drupal.acquiaLiftUtility.QueueItem(itemData);
  assert.ok(item instanceof Drupal.acquiaLiftUtility.QueueItem, 'Queue item identifies itself as the correct type.');
  assert.equal(item.getId().indexOf('acquia-lift-ts-'), 0, 'Queue item has auto-generated id.');
  assert.equal(item.isProcessing(), false, 'Queue item set initially to not processing.');
  assert.deepEqual(item.getData(), itemData, 'Queue item data is set properly.');
  var queueObj = {
    'id': item.getId(),
    'data': itemData,
    'pflag': false,
    'numberTried': 0
  };
  assert.deepEqual(item.toObject(), queueObj, 'Queue item can be turned into an object.');
  queueObj.data = 'testme';
  var item2 = new Drupal.acquiaLiftUtility.QueueItem(queueObj);
  assert.ok(item.equals(item2), 'Queue item equality check passes based on id only.');
  queueObj.data = itemData;
  queueObj.id = 'newid';
  var item3 = new Drupal.acquiaLiftUtility.QueueItem(queueObj);
  assert.ok(!item.equals(item3), 'Queue item equality check passes based on id only.');
  assert.ok(!item.equals(queueObj), 'Queue item can equality check can handle invalid data.');
  item.setProcessing(true);
  assert.equal(item.isProcessing(), true, 'Queue item has processing value changed.');
  item.reset();
  assert.equal(item.isProcessing(), false, 'Queue item processing can be reset.');
});

QUnit.test("Queue unit tests", function(assert) {
  var testData1 = {'testdata': 1},
    testData2 = {'testdata': 2};

  expect(20);

  // Qunit/sinon resets all dates which affects our automatic ids.
  if (this.clock) {
    this.clock.restore();
  }

  // Start by clearing the queue so we can test.
  Drupal.acquiaLiftUtility.Queue.empty();
  assert.deepEqual(readCookieQueue(), [], 'Cookie queue is empty.');

  // Now add an item to the queue.
  Drupal.acquiaLiftUtility.Queue.add(testData1);
  var queue = readCookieQueue();
  assert.ok(queue instanceof Array, 'Cookie contents are an array.');
  assert.equal(queue.length, 1, 'Cookie queue has one item.');
  assert.deepEqual(queue[0].data, testData1, 'Item has the correct data.');
  assert.equal(queue[0].pflag, false, 'Item has been added as not in processing.');

  // Add another item to the queue.
  Drupal.acquiaLiftUtility.Queue.add(testData2);
  var queue = readCookieQueue();
  assert.ok(queue instanceof Array, 'Cookie contents are an array.');
  assert.equal(queue.length, 2, 'Cookie queue has two items.');
  assert.deepEqual(queue[1].data, testData2, 'New item has the correct data.');

  // Get the next item for processing
  var next = Drupal.acquiaLiftUtility.Queue.getNext();
  assert.ok(next instanceof Drupal.acquiaLiftUtility.QueueItem, 'Next item for processing is a QueueItem');
  assert.ok(next.isProcessing(), 'Next item has been marked as processing.');
  assert.deepEqual(next.getData(), testData1, 'Next item is the first added to the queue.');

  // Now add it back into the queue.
  Drupal.acquiaLiftUtility.Queue.add(next);
  var queue = readCookieQueue();
  assert.equal(queue.length, 2, 'The queue is still at two items.');
  assert.deepEqual(queue[0].data, testData1, 'The re-added item was set back to its original index.');
  assert.equal(queue[0].pflag, false, 'The item has had its processing status correctly reset.');

  // Now get two items for processing.
  var next1 = Drupal.acquiaLiftUtility.Queue.getNext();
  var next2 = Drupal.acquiaLiftUtility.Queue.getNext();
  var next3 = Drupal.acquiaLiftUtility.Queue.getNext();
  assert.deepEqual(next1.getData(), testData1, 'The first item returned was the first added.');
  assert.deepEqual(next2.getData(), testData2, 'The second item returned was the second added.');
  assert.ok(next3 == null, 'There was no third item returned for processing.');

  // Remove an item from the queue.
  Drupal.acquiaLiftUtility.Queue.remove(next2);
  var queue = readCookieQueue();
  assert.equal(queue.length, 1, 'The queue now has one item for processing.');
  assert.deepEqual(queue[0].data, testData1, 'The correct item remained in the queue.');

  // Reset the queue.
  Drupal.acquiaLiftUtility.Queue.reset();
  var queue = readCookieQueue();
  assert.equal(queue[0].pflag, false, 'The remaining item has been reset.');
});

QUnit.test("Goals queue", function(assert) {
  // Clear out the queue
  // Qunit/sinon resets all dates which affects our automatic ids.
  if (this.clock) {
    this.clock.restore();
  }

  expect(26);

  // Create a fake request for the goals api call.
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  // Start by clearing the queue so we can test.
  Drupal.acquiaLiftUtility.Queue.empty();
  assert.deepEqual(readCookieQueue(), [], 'Cookie queue is empty.');

  var agentName = 'test-agent';
  var testGoal1 = {
    reward: 1,
    goal: 'goal1'
  };
  var testGoal2 = {
    reward: 2,
    goal: 'goal2'
  };
  var testGoal3 = {
    reward: 0,
    goal: 'goal3'
  };

  // Spy on the queue to see that the correct functions are called.
  sinon.spy(Drupal.acquiaLiftUtility.Queue, 'add');
  sinon.spy(Drupal.acquiaLiftUtility.Queue, 'remove');

  // Add a goal to the goals queue without processing.
  Drupal.acquiaLiftUtility.GoalQueue.addGoal(agentName, testGoal1, false);
  var queueData = {
    'a': agentName,
    'o': testGoal1
  };
  var queueData2 = {
    'a': agentName,
    'o': testGoal2
  };
  var queueData3 = {
    'a': agentName,
    'o': testGoal3
  };
  // Get the queue item for assertions and then clear out the processing status.
  var item = Drupal.acquiaLiftUtility.Queue.getNext();
  Drupal.acquiaLiftUtility.Queue.reset();

  // Verify the add process.
  assert.ok(Drupal.acquiaLiftUtility.Queue.add.calledWith(queueData), 'The queue add method was called with queue data.');

  // Now go ahead and process the queue.
  Drupal.acquiaLiftUtility.GoalQueue.processQueue();

  // For now just check that a request was made to the right path since we are
  // testing the queue process here.  Tests specific to goals processing with
  // handle deeper checkers of parameters, etc.
  assert.equal(sinon.requests.length, 1, 'The api was called once.');

  var parsed = parseUri(sinon.requests[0].url);
  assert.equal(parsed.host, 'api.example.com', 'API host is correct.');
  assert.equal(parsed.path, '/someOwner/' + agentName + '/goal/' + testGoal1.goal, 'API path is correct.');

  sinon.requests[0].respond(200, { "Content-Type": "application/json" }, '{"agent": "' + agentName + '", "session": "some-session-ID", "reward":"' + testGoal1.reward + '", "goal":"' + testGoal1.goal + '"}');
  assert.equal(Drupal.acquiaLiftUtility.Queue.remove.callCount, 1, 'The remove call was made once.');
  var removeCall = Drupal.acquiaLiftUtility.Queue.remove.getCall(0);
  assert.ok(item.equals(removeCall.args[0]), 'The remove call was made with the goal item previously added.');

  // Now add a goal that results in an error from the API and verify that it
  // remains in the queue for later processing until the max retries is reached.
  Drupal.acquiaLiftUtility.GoalQueue.addGoal(agentName, testGoal2);
  assert.ok(Drupal.acquiaLiftUtility.Queue.add.calledWith(queueData2), 'The second goal was added to the queue.');
  assert.equal(sinon.requests.length, 2, 'Another api call was made.');
  sinon.requests[1].respond(500, {"Content-Type": "application/json"}, '[{"status": 500}]');
  assert.equal(Drupal.acquiaLiftUtility.Queue.remove.callCount, 1, 'The remove call was not called again.');
  var nextGoal = Drupal.acquiaLiftUtility.Queue.getNext();
  assert.equal(nextGoal.getNumberTried(), 1, 'The goal has been tried once.');
  assert.deepEqual(nextGoal.getData(), queueData2, 'The second goal is still in the queue.');

  // Execute tries 2 through 4 (max retries = 5).
  for (var i = 2; i < 5; i++) {
    Drupal.acquiaLiftUtility.GoalQueue.processQueue(true);
    var req = sinon.requests.pop();
    req.respond(500, {"Content-Type": "application/json"}, '[{"status": 500}]');
    assert.equal(Drupal.acquiaLiftUtility.Queue.remove.callCount, 1, 'The remove call was not called again.');
    var nextGoal = Drupal.acquiaLiftUtility.Queue.getNext();
    assert.equal(nextGoal.getNumberTried(), i, 'The goal has been tried ' + i + ' times.');
    assert.deepEqual(nextGoal.getData(), queueData2, 'The second goal is still in the queue.');
  }

  // The fifth failure should result in the item being removed from the queue.
  Drupal.acquiaLiftUtility.GoalQueue.processQueue(true);
  var req = sinon.requests.pop();
  req.respond(500, {"Content-Type": "application/json"}, '[{"status": 500}]');
  assert.equal(Drupal.acquiaLiftUtility.Queue.remove.callCount, 2, 'The remove call was called again.');
  var nextGoal = Drupal.acquiaLiftUtility.Queue.getNext();
  assert.ok(nextGoal == null, 'There are no more items in the queue.');

  // Now add a goal that fails and is not retryable.
  Drupal.acquiaLiftUtility.GoalQueue.addGoal(agentName, testGoal3);
  assert.ok(Drupal.acquiaLiftUtility.Queue.add.calledWith(queueData3), 'The third goal was added to the queue.');
  var request = sinon.requests.pop();
  request.respond(202, { "Content-Type": "*/*" }, '{"agent": "' + agentName + '", "session": "some-session-ID", "reward":"' + testGoal3.reward + '", "goal":"' + testGoal3.goal + '"}');
  // This should return not retryable and be deleted.
  assert.equal(Drupal.acquiaLiftUtility.Queue.remove.callCount, 3, 'The remove call was called for non-retryable goal.');
  var nextGoal = Drupal.acquiaLiftUtility.Queue.getNext();
  assert.ok(nextGoal == null, 'There are no more items in the queue.');

  // Clean up after the sinon wrappers.
  xhr.restore();
  Drupal.acquiaLiftUtility.Queue.add.restore();
  Drupal.acquiaLiftUtility.Queue.remove.restore();
});

QUnit.module("Acquia Lift service calls", {
  'setup': function() {
    initializeLiftSettings();
  }
});

QUnit.test('Make decision', function(assert) {
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];
  initializeLiftSettings();

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  var callback = sinon.spy();
  Drupal.acquiaLift.getDecision('my-agent', {'some-feature': ['some-value', 'sc-some-value'],'other-feature': ['some,value'] }, {'first-decision': ['option-1', 'option-2']}, 'my-decision-point', {'first-decision': 0}, callback);

  equal(sinon.requests.length, 1);
  var parsed = parseUri(sinon.requests[0].url);

  equal(parsed.host, 'api.example.com');
  equal(parsed.path, "/someOwner/my-agent/decisions/first-decision:option-1,option-2");
  equal(parsed.queryKey.apikey, "xyz123");
  equal(parsed.queryKey.features, "some-feature%3A%3Asome-value%2Csome-feature%3A%3Asc-some-value%2Cother-feature%3A%3Asome-value");
  equal(parsed.queryKey.session, "some-session-ID");

  requests[0].respond(200, { "Content-Type": "application/json" }, '{"decisions": {"first-decision":"option-2"}, "session": "some-session-ID"}');
  ok(callback.called);

  cleanUp();
});

QUnit.test('Multiple decisions no batching', function(assert) {
  setSessionID(true);
  testMultipleDecisionsNoBatch(assert, true);
});

QUnit.test('Multiple decisions no session', function(assert) {
  setSessionID(false);
  testMultipleDecisionsNoBatch(assert, false);
});

function testMultipleDecisionsNoBatch(assert, known) {
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  var callback = sinon.spy();
  Drupal.acquiaLift.getDecision('my-agent', {'some-feature': ['some-value', 'sc-some-value'],'other-feature': ['some,value'] }, {'first-decision': ['option-1', 'option-2']}, 'my-decision-point', {'first-decision': 0}, callback);
  requests[0].respond(200, { "Content-Type": "application/json" }, '{"decisions": {"first-decision":"option-2"}, "session": "some-session-ID"}');

  Drupal.acquiaLift.getDecision('my-agent', {'some-feature': ['some-value', 'sc-some-value'],'other-feature': ['some,value'] }, {'second-decision': ['option-1', 'option-2']}, 'other-decision-point', {'second-decision': 0}, callback);
  requests[1].respond(200, { "Content-Type": "application/json" }, '{"decisions": {"second-decision":"option-2"}, "session": "some-session-ID"}');

  equal(sinon.requests.length, 2);

  var parsedUri1 = parseUri(sinon.requests[0].url);
  equal(parsedUri1.host, 'api.example.com');
  equal(parsedUri1.path, "/someOwner/my-agent/decisions/first-decision:option-1,option-2");
  equal(parsedUri1.queryKey.apikey, "xyz123");
  equal(parsedUri1.queryKey.features, "some-feature%3A%3Asome-value%2Csome-feature%3A%3Asc-some-value%2Cother-feature%3A%3Asome-value");
  if (known) {
    equal(parsedUri1.queryKey.session, "some-session-ID");
  }

  var parsedUri2 = parseUri(sinon.requests[1].url);
  equal(parsedUri2.host, 'api.example.com');
  equal(parsedUri2.path, "/someOwner/my-agent/decisions/second-decision:option-1,option-2");
  equal(parsedUri2.queryKey.apikey, "xyz123");
  equal(parsedUri2.queryKey.features, "some-feature%3A%3Asome-value%2Csome-feature%3A%3Asc-some-value%2Cother-feature%3A%3Asome-value");
  equal(parsedUri2.queryKey.session, "some-session-ID");

  ok(callback.callCount, 2);

  cleanUp();
}

QUnit.test('Make batched decisions', function(assert) {
  setSessionID(true);
  testBatchDecisions(assert, true);
});

QUnit.test('Make batched decisions no session', function(assert) {
  setSessionID(false);
  testBatchDecisions(assert, false);
});

function testBatchDecisions(assert, known) {
  Drupal.settings.acquia_lift.batchMode = true;
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  var callback = sinon.spy();
  // Fire off two requests for decisions - only one HTTP request should be made.
  Drupal.acquiaLift.getDecision('my-agent', {'some-feature': ['some-value', 'sc-some-value'],'other-feature': ['some,value'] }, {'first-decision': ['option-1', 'option-2']}, 'my-decision-point', {'first-decision': 0}, callback);
  Drupal.acquiaLift.getDecision('my-agent', {'some-feature': ['some-value', 'sc-some-value'],'other-feature': ['some,value'] }, {'second-decision': ['option-1', 'option-2']}, 'other-decision-point', {'second-decision': 0}, callback);
  // This event lets the Lift js know there are no more decisions to wait for.
  $(document).trigger('personalizeDecisionsEnd');
  // Confirm a single request is made with the expected request body.
  assert.equal(sinon.requests.length, 1);

  // Mock a response from the server so that the request completes.
  requests[0].respond(200, { "Content-Type": "application/json" }, '[{"status": 200, "data": {"agent": "my-agent", "decisions": {"first-decision":"option-2"}, "session": "some-session-ID"}}, {"status": 200, "data": {"agent": "my-agent", "decisions": {"second-decision":"option-2"}, "session": "some-session-ID"}}]');
  var parsedUri = parseUri(sinon.requests[0].url);
  var requestBody = JSON.parse(sinon.requests[0].requestBody);
  if (known) {
    var expectedRequestBody = [
      {
        'agent': 'my-agent',
        'choices': "first-decision:option-1,option-2",
        'query': {
          '_t': 0,
          'apikey': 'xyz123',
          'features': "some-feature::some-value,some-feature::sc-some-value,other-feature::some-value",
          'point': 'my-decision-point',
          'session': 'some-session-ID'
        },
        'type': 'decisions'
      },
      {
        'agent': 'my-agent',
        'choices': "second-decision:option-1,option-2",
        'query': {
          '_t': 0,
          'apikey': 'xyz123',
          'features': "some-feature::some-value,some-feature::sc-some-value,other-feature::some-value",
          'point': 'other-decision-point',
          'session': 'some-session-ID'
        },
        'type': 'decisions'
      }
    ];
  } else {
    var expectedRequestBody = [
      {
        'agent': 'my-agent',
        'choices': "first-decision:option-1,option-2",
        'query': {
          '_t': 0,
          'apikey': 'xyz123',
          'features': "some-feature::some-value,some-feature::sc-some-value,other-feature::some-value",
          'point': 'my-decision-point'
        },
        'type': 'decisions'
      },
      {
        'agent': 'my-agent',
        'choices': "second-decision:option-1,option-2",
        'query': {
          '_t': 0,
          'apikey': 'xyz123',
          'features': "some-feature::some-value,some-feature::sc-some-value,other-feature::some-value",
          'point': 'other-decision-point'
        },
        'type': 'decisions'
      }
    ];
  }
  assert.deepEqual(requestBody, expectedRequestBody);
  assert.equal(parsedUri.host, 'api.example.com');
  assert.equal(parsedUri.path, "/someOwner/-/batch");
  if (known) {
    assert.equal(parsedUri.queryKey.session, "some-session-ID");
  }

  assert.ok(callback.called);

  cleanUp();
}

QUnit.test('Send goal', function(assert) {
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  Drupal.acquiaLift.sendGoal('my-agent', 'some-goal', 2);

  assert.equal(sinon.requests.length, 1);
  var parsed = parseUri(sinon.requests[0].url);
  assert.equal(parsed.host, 'api.example.com');
  assert.equal(parsed.path, "/someOwner/my-agent/goal/some-goal");
  assert.equal(parsed.queryKey.apikey, "xyz123");
  // For some reason the URI parser has trouble with the first param in a querystring
  assert.ok(parsed.query.indexOf('reward=2') != -1);

  requests[0].respond(200, { "Content-Type": "application/json" }, '{"agent": "my-agent", "session": "some-session-ID", "reward":2, "goal": "some-goal"}');

  cleanUp();
});

QUnit.test('Send goal in batch mode', function(assert) {
  Drupal.settings.acquia_lift.batchMode = true;
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  Drupal.acquiaLift.sendGoal('my-agent', 'some-goal', 2);

  assert.equal(sinon.requests.length, 1);
  var requestBody = JSON.parse(sinon.requests[0].requestBody);
  var expectedRequestBody = [
    {
      'agent': 'my-agent',
      'goal': "some-goal",
      'query': {
        '_t': 0,
        'apikey': 'xyz123',
        'reward': 2,
        'session': 'some-session-ID'
      },
      'type': 'goal'
    },
  ];
  assert.deepEqual(expectedRequestBody, requestBody);
  var parsedUri = parseUri(sinon.requests[0].url);
  console.log(parsedUri);

  assert.equal(parsedUri.host, 'api.example.com');
  assert.equal(parsedUri.path, "/someOwner/-/batch");

  requests[0].respond(200, { "Content-Type": "application/json" }, '[{"status": 200, "data": {"agent": "my-agent", "session": "some-session-ID", "reward":2, "goal":"some-goal"}}]');

  cleanUp();
});

QUnit.test('Make decision after goal without session', function(assert) {
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];
  initializeLiftSettings();

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  // Update so that no session id is returned from the module.
  setSessionID(false);

  // Send goal.
  Drupal.acquiaLift.sendGoal('my-agent', 'some-goal', 2);

  assert.equal(sinon.requests.length, 1);

  // Now make decision requests and ensure that the session id is stored.
  var callback = sinon.spy();
  Drupal.acquiaLift.getDecision('my-agent', {'some-feature': ['some-value', 'sc-some-value'],'other-feature': ['some,value'] }, {'first-decision': ['option-1', 'option-2']}, 'my-decision-point', {'first-decision': 0}, callback);
  requests[1].respond(200, { "Content-Type": "application/json" }, '{"decisions": {"first-decision":"option-2"}, "session": "12345678"}');

  // The second decision should include the new session returned from the server.
  Drupal.acquiaLift.getDecision('my-agent', {'some-feature': ['some-value', 'sc-some-value'],'other-feature': ['some,value'] }, {'second-decision': ['option-1', 'option-2']}, 'other-decision-point', {'second-decision': 0}, callback);
  var parsedUri2 = parseUri(sinon.requests[2].url);
  equal(parsedUri2.host, 'api.example.com');
  equal(parsedUri2.path, "/someOwner/my-agent/decisions/second-decision:option-1,option-2");
  equal(parsedUri2.queryKey.apikey, "xyz123");
  equal(parsedUri2.queryKey.features, "some-feature%3A%3Asome-value%2Csome-feature%3A%3Asc-some-value%2Cother-feature%3A%3Asome-value");
  equal(parsedUri2.queryKey.session, "12345678");

  requests[2].respond(200, { "Content-Type": "application/json" }, '{"decisions": {"second-decision":"option-2"}, "session": "12345678"}');
  equal(callback.callCount, 2);
  equal(sinon.requests.length, 3);

  cleanUp();
});

QUnit.test('Page load goals queue processing', function(assert) {
  var xhr = sinon.useFakeXMLHttpRequest();
  var requests = sinon.requests = [];
  initializeLiftSettings();

  xhr.onCreate = function (request) {
    requests.push(request);
  };

  // Add a goal to the queue without processing it.
  // Start by clearing the queue so we can test.
  Drupal.acquiaLiftUtility.Queue.empty();
  assert.deepEqual(readCookieQueue(), [], 'Cookie queue is empty.');

  var agentName = 'test-agent';
  var testGoal = {
    reward: 1,
    goal: 'goal1'
  };

  sinon.spy(Drupal.acquiaLiftUtility.GoalQueue, 'processQueue');

  // Add a goal to the goals queue without processing.
  Drupal.acquiaLiftUtility.GoalQueue.addGoal(agentName, testGoal, false);

  var queue = readCookieQueue();
  assert.equal(queue.length, 1, 'Goals queue has one item.');

  // Call the queue page processing.
  Drupal.behaviors.acquia_lift_goal_queue.attach($(document), Drupal.settings);

  // Make sure the correct URL was called.
  assert.equal(sinon.requests.length, 1);
  var parsed = parseUri(sinon.requests[0].url);
  assert.equal(parsed.host, 'api.example.com');
  assert.equal(parsed.path, "/someOwner/test-agent/goal/goal1");
  assert.equal(parsed.queryKey.apikey, "xyz123");
  assert.equal(parsed.queryKey.session, "some-session-ID");
  assert.ok(parsed.query.indexOf('reward=1') != -1);

  requests[0].respond(200, { "Content-Type": "application/json" }, '{"agent": "test-agent", "session": "some-session-ID", "reward":1, "goal": "goal1"}');

  // Make sure the cookie queue was emptied.
  assert.ok(Drupal.acquiaLiftUtility.GoalQueue.processQueue.called, 'Process queue was called');
  assert.equal(Drupal.acquiaLiftUtility.GoalQueue.processQueue.callCount, 1, 'Process queue was called only once.');

  queue = readCookieQueue();
  assert.equal(queue.length, 0, 'Goals queue is empty.');

  cleanUp();
})

// Helper for parsing the ajax request URI

// parseUri 1.2.2
// (c) Steven Levithan <stevenlevithan.com>
// MIT License

function parseUri (str) {
  var	o   = parseUri.options,
    m   = o.parser[o.strictMode ? "strict" : "loose"].exec(str),
    uri = {},
    i   = 14;

  while (i--) uri[o.key[i]] = m[i] || "";

  uri[o.q.name] = {};
  uri[o.key[12]].replace(o.q.parser, function ($0, $1, $2) {
    if ($1) uri[o.q.name][$1] = $2;
  });

  return uri;
};
parseUri.options = {
  strictMode: false,
  key: ["source","protocol","authority","userInfo","user","password","host","port","relative","path","directory","file","query","anchor"],
  q:   {
    name:   "queryKey",
    parser: /(?:^|&)([^&=]*)=?([^&]*)/g
  },
  parser: {
    strict: /^(?:([^:\/?#]+):)?(?:\/\/((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?))?((((?:[^?#\/]*\/)*)([^?#]*))(?:\?([^#]*))?(?:#(.*))?)/,
    loose:  /^(?:(?![^:@]+:[^:@\/]*@)([^:\/?#.]+):)?(?:\/\/)?((?:(([^:@]*)(?::([^:@]*))?)?@)?([^:\/?#]*)(?::(\d*))?)(((\/(?:[^?#](?![^?#\/]*\.[^?#\/.]+(?:[?#]|$)))*\/?)?([^?#\/]*))(?:\?([^#]*))?(?:#(.*))?)/
  }
};

// Helper function to read the queue cookie contents.
function readCookieQueue() {
  return $.parseJSON($.cookie('acquiaLiftQueue'));
};

// Helper function to initialize the lift API settings.
function initializeLiftSettings() {
  Drupal.acquiaLift.reset();
  Drupal.settings.personalize = Drupal.settings.personalize || {};
  Drupal.settings.acquia_lift = Drupal.settings.acquia_lift || {};
  Drupal.settings.acquia_lift.baseUrl = 'http://api.example.com';
  Drupal.settings.acquia_lift.owner = 'someOwner';
  Drupal.settings.acquia_lift.apiKey = 'xyz123';
  Drupal.settings.acquia_lift.featureStringSeparator = '::';
  Drupal.settings.acquia_lift.featureStringMaxLength = 50;
  Drupal.settings.acquia_lift.featureStringReplacePattern = '[^A-Za-z0-9_-]';
  Drupal.settings.acquia_lift.batchMode = 0;
  setSessionID(true);
}

// Helper function to mimic a known or unknown session.
function setSessionID(known) {
  if (known) {
    Drupal.personalize.initializeSessionID = function() {
      return 'some-session-ID';
    };
  } else {
    Drupal.personalize.initializeSessionID = function() { return null };
  }
}

// Helper function to clean up after tests.
function cleanUp() {
  Drupal.acquiaLift.reset();
  sinon.restore();
}
