QUnit.test("test explicit targeting logic", function( assert ) {
  expect(6);
  // Add settings for a targeting agent with some fixed targeting rules on its single
  // option set.
  var agentName = 'my-test-agent',
    decisionName = 'my-decision',
    enabledContexts = {
      'some_plugin': {
        'some-context': 'some-context',
        'other-context': 'other-context'
      }
    },
    options = [
      {
        'option_id': 'first-option',
        'option_label': 'First Option'
      },
      {
        'option_id': 'second-option',
        'option_label': 'Second Option'
      },
      {
        'option_id': 'third-option',
        'option_label': 'Third Option'
      }
    ],
    targeting = [
      {
        'option_id': 'second-option',
        // Add fixed targeting rules such that this option should be shown if two
        // feature strings are present.
        'targeting_features': [
          "some-context::some-value",
          "other-context::ss-other"
        ],
        'targeting_strategy': 'AND'
      },
      {
        'option_id': 'third-option',
        // Add fixed targeting rules such that this option should be shown if one of
        // two feature strings is present.
        'targeting_features': [
          "some-context::some-value",
          "other-context::ss-ohai"
        ],
        'targeting_strategy': 'OR'
      }
    ];
  addLiftTargetToDrupalSettings(agentName, enabledContexts, decisionName, 'osid-1', options, targeting);

  // Now request decisions from that agent to test its behavior with different contexts.
  var evaluatedVisitorContexts = {},
    choices = {},
    fallbacks = {};
  choices[decisionName] = ['first-option', 'second-option', 'third-option'];
  fallbacks[decisionName] = 0;
  // Try first with no visitor contexts present, we should get the first (fallback) option.
  Drupal.personalize.agents.acquia_lift_target.getDecisionsForPoint(agentName, evaluatedVisitorContexts, choices, decisionName, fallbacks,     callback = function(decisions) {
    assert.ok(decisions.hasOwnProperty(decisionName));
    assert.equal(decisions[decisionName], 'first-option');
  });

  // Now try with contexts that should satisfy the rules for the second option.
  evaluatedVisitorContexts = {
    'some-context': [
      'some-value',
      'sc-some'
    ],
    'other-context': [
      'other-value',
      'ss-other'
    ]
  };
  Drupal.personalize.agents.acquia_lift_target.getDecisionsForPoint(agentName, evaluatedVisitorContexts, choices, decisionName, fallbacks,     callback = function(decisions) {
    assert.ok(decisions.hasOwnProperty(decisionName));
    assert.equal(decisions[decisionName], 'second-option');
  });

  // Now try with contexts that only partially satisfy the rules for the second option, but
  // fully satisfy the rules for the third option.
  evaluatedVisitorContexts = {
    'some-context': [
      'some-value',
      'sc-some'
    ],
    'other-context': [
      'my-other-value'
    ]
  };
  Drupal.personalize.agents.acquia_lift_target.getDecisionsForPoint(agentName, evaluatedVisitorContexts, choices, decisionName, fallbacks,     callback = function(decisions) {
    assert.ok(decisions.hasOwnProperty(decisionName));
    assert.equal(decisions[decisionName], 'third-option');
  });
});

QUnit.test("test nesting logic", function( assert ) {
  expect(6);
  // Add settings for a targeting agent with a test nested in it..
  var agentName = 'my-parent-agent',
      decisionName = 'my-decision',
      enabledContexts = {
        'some_plugin': {
          'some-context': 'some-context',
          'other-context': 'other-context'
        }
      },
      options = [
        {
          'option_id': 'first-option',
          'option_label': 'First Option'
        },
        {
          'option_id': 'second-option',
          'option_label': 'Second Option'
        },
        {
          'option_id': 'third-option',
          'option_label': 'Third Option'
        }
      ],
      subOS = '123',
      targeting = [
        {
          // If this rule matches then a nested option set decides between the
          // first and second options.
          'osid': subOS,
          'targeting_features': [
            "some-context::some-value",
            "other-context::ss-other"
          ],
          'targeting_strategy': 'AND'
        },
        {
          // If this rule matches then the third option is shown.
          'option_id': 'third-option',
          'targeting_features': [
            "some-context::some-value",
            "other-context::ss-ohai"
          ],
          'targeting_strategy': 'OR'
        }
      ];
  addLiftTargetToDrupalSettings(agentName, enabledContexts, decisionName, 'osid-1', options, targeting);
  // Now add the acquia_lift_target settings for the nested option set.
  var sub_agent_name = 'my-nested-agent';
  Drupal.settings.acquia_lift_target = Drupal.settings.acquia_lift_target || {};
  Drupal.settings.acquia_lift_target.agent_map = Drupal.settings.acquia_lift_target.agent_map || {};
  Drupal.settings.acquia_lift_target.agent_map[sub_agent_name] = {
    'type': 'acquia_lift'
  };
  var subOs_str = 'osid-' + 123;
  Drupal.settings.acquia_lift_target.option_sets = Drupal.settings.acquia_lift_target.option_sets || {};
  Drupal.settings.acquia_lift_target.option_sets[subOs_str] = {
    'agent': sub_agent_name,
    'data': [],
    'decision_name': subOs_str,
    'decision_point': subOs_str,
    'executor': 'show',
    'label': 'My Sub OS',
    'mvt': null,
    'option_names': ['first-option', 'second-option'],
    'options': options.slice(0,2),
    'targeting': {},
    'osid': subOs_str,
    'plugin': 'my_os_plugin',
    'selector': '.some-class',
    'stateful': 0,
    'winner': null
  };
  Drupal.personalize.agents.acquia_lift = Drupal.personalize.agents.acquia_lift || {};
  // Mock the acquia_lift testing agent to just make it return the second option.
  Drupal.personalize.agents.acquia_lift.getDecisionsForPoint = function(agentName, evaluatedVisitorContexts, choices, decisionName, fallbacks, callback) {
    var selection = {};
    selection[subOs_str] = 'second-option';
    callback(selection);
  };

  // Now request decisions from the parent agent.
  var evaluatedVisitorContexts = {},
      choices = {},
      fallbacks = {};
  choices[decisionName] = ['first-option', 'second-option', 'third-option'];
  fallbacks[decisionName] = 0;
  // Try first with no visitor contexts present, we should get the first (fallback) option.
  Drupal.personalize.agents.acquia_lift_target.getDecisionsForPoint(agentName, evaluatedVisitorContexts, choices, decisionName, fallbacks, function(decisions) {
    assert.ok(decisions.hasOwnProperty(decisionName));
    assert.equal(decisions[decisionName], 'first-option');
  });

  // Now try with contexts that should satisfy the rules for the nested option set.
  evaluatedVisitorContexts = {
    'some-context': [
      'some-value',
      'sc-some'
    ],
    'other-context': [
      'other-value',
      'ss-other'
    ]
  };
  Drupal.personalize.agents.acquia_lift_target.getDecisionsForPoint(agentName, evaluatedVisitorContexts, choices, decisionName, fallbacks, function(decisions) {
    assert.ok(decisions.hasOwnProperty(decisionName));
    assert.equal(decisions[decisionName], 'second-option');
  });

  // Now try with contexts that satisfy the rules for the third option.
  evaluatedVisitorContexts = {
    'some-context': [
      'some-value',
      'sc-some'
    ],
    'other-context': [
      'my-other-value'
    ]
  };
  Drupal.personalize.agents.acquia_lift_target.getDecisionsForPoint(agentName, evaluatedVisitorContexts, choices, decisionName, fallbacks, function(decisions) {
    assert.ok(decisions.hasOwnProperty(decisionName));
    assert.equal(decisions[decisionName], 'third-option');
  });
});

/**
 * Adds settings for the required targeting agent set-up to Drupal.settings.
 */
function addLiftTargetToDrupalSettings(agent_name, enabled_contexts, decision_name, osid, options_array, targeting) {

  Drupal.settings.personalize.agent_map = Drupal.settings.personalize.agent_map || {};
  Drupal.settings.personalize.agent_map[agent_name] = {
    'active': 1,
    'cache_decisions': false,
    'enabled_contexts': enabled_contexts,
    'type': 'acquia_lift_target'
  };

  var option_names = [];
  for (var i in options_array) {
    if (options_array.hasOwnProperty(i)) {
      option_names.push(options_array[i].option_id);
    }
  }
  Drupal.settings.personalize.option_sets = Drupal.settings.personalize.option_sets || {};
  Drupal.settings.personalize.option_sets[osid] = {
    'agent': agent_name,
    'data': [],
    'decision_name': decision_name,
    'decision_point': decision_name,
    'executor': 'show',
    'label': 'My Lift Target',
    'mvt': null,
    'option_names': option_names,
    'options': options_array,
    'targeting': targeting,
    'osid': osid,
    'plugin': 'my_os_plugin',
    'selector': '.some-class',
    'stateful': 0,
    'winner': null
  };
}
