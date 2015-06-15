/**
 * @file page_variations.tests.js
 */

var $ = Drupal.jQuery;

QUnit.module("Acquia Lift page variation models");

QUnit.test('Default values', function (assert) {
  expect(7);

  var appModel = new Drupal.acquiaLiftVariations.models.AppModel();
  assert.equal(appModel.get('editMode'), true, 'App model edit mode default set correctly.');
  var valueAppModel = new Drupal.acquiaLiftVariations.models.AppModel({editMode: false});
  assert.equal(valueAppModel.get('editMode'), false, 'App model value for edit mode set correctly.');

  var elemModel = new Drupal.acquiaLiftVariations.models.ElementVariationModel();
  assert.ok(elemModel.cid, 'Element variaton model created successfully.');

  var formModel = new Drupal.acquiaLiftVariations.models.VariationTypeFormModel();
  assert.equal(formModel.get('type'), null, 'Form model type default set correctly.');
  assert.equal(formModel.get('typeLabel'), null, 'Form model type label default set correctly.');
  var valueFormModel = new Drupal.acquiaLiftVariations.models.VariationTypeFormModel({
    type: 'addClass',
    typeLabel: 'Add CSS class'
  });
  assert.equal(valueFormModel.get('type'), 'addClass', 'Form model type set correctly.');
  assert.equal(valueFormModel.get('typeLabel'), 'Add CSS class', 'Form model type label set correctly.');
});

QUnit.module("Acquia Lift page variation views", {
  setup: function() {
    Drupal.settings = Drupal.settings || {};
    Drupal.settings.basePath = '/';
    Drupal.settings.pathPrefix = '';
    Drupal.settings.personalize_elements = Drupal.settings.personalize_elements || {};
    Drupal.settings.personalize_elements.contextualVariationTypes = Drupal.settings.personalize_elements.contextualVariationTypes || {
      'addCss': {
        name: 'Add CSS class'
      },
      'editHtml': {
        name: 'Edit HTML'
      },
      'editText': {
        name: 'Edit text',
        limitByChildrenType: 3
      },
      'appendHtml': {
        name: 'Append HTML'
      },
      'prependHtml': {
        name: 'Prepend HTML'
      }
    };
    Drupal.settings.visitor_actions = Drupal.settings.visitor_actions || {};
    Drupal.settings.visitor_actions.currentPath = 'node';
  }
});

QUnit.test('Application view', function (assert) {
  var $view = $('#watch');
  var $anchor = $('#mousehere');

  var appModel = new Drupal.acquiaLiftVariations.models.AppModel();
  var appView = new Drupal.acquiaLiftVariations.views.AppView({
    model: appModel,
    el: $view[0]
  });

  expect(15);
  assert.ok($view.DOMSelector("isWatching"), 'View has been initialized as a DOM selector.');
  $anchor.trigger('mousemove');
  $anchor.trigger('click');
  assert.deepEqual(appView.anchor, $anchor[0], 'Anchor has been saved in the application view.');
  assert.ok($anchor.hasClass('acquia-lift-page-variation-item'), 'Anchor element has been highlighted.');
  assert.ok(appView.contextualMenuModel.get('active'), 'Contextual model has been created as is active.');
  assert.equal(appView.contextualMenuModel.get('selector'), '#mousehere', 'Contextual model has been created form the correct element.');

  var data = {
    id: 'addClass',
    name: 'Add CSS class',
    selector: '#mousehere',
    anchor: $anchor[0]
  }
  // @todo: this will throw errors because we are not coming from a Drupal path and therefore there is no existing base theme.
  // This prevents any additional tests being run on the variation type form dialog.
  appView.createVariationTypeDialog({data: data});
  assert.equal(appView.variationTypeFormModel.get('selector'), data.selector, 'Variation type form model selector set successfully.');
  assert.ok(appView.variationTypeFormModel.get('id'), 'Variation type form model ID set successfully.');
  assert.equal(appView.variationTypeFormModel.get('formPath'), Drupal.settings.basePath + Drupal.settings.pathPrefix + 'admin/structure/acquia_lift/variation/addClass', 'Variation type form model form path set successfully.');
  assert.equal(appView.variationTypeFormModel.get('type'), data.id, 'Variation type form model type set successfully.');
  assert.equal(appView.variationTypeFormModel.get('typeLabel'), data.name, 'Variation type form model type label set successfully.');
  assert.ok(appView.variationTypeFormModel.get('active'), 'Variation type form model set to active.');

  appModel.set('editMode', false);
  assert.ok(!$view.DOMSelector("isWatching"), "View is no longer watching when model is taken out of edit mode.");
  assert.ok(!$anchor.hasClass('acquia-lift-page-variation-item'), 'Anchor element highlight has been removed.');
  assert.ok(!appView.contextualMenuModel, 'Contextual menu has been removed.');
  assert.ok(!appView.variationTypeFormModel, 'Contextual menu has been removed.');
});

QUnit.asyncTest('Contextual menu view', function (assert) {
  QUnit.start();
  expect(11);
  var anchorSelector = '#mousehere';
  var $anchor = $(anchorSelector);
  var id = 'testing-' + new Date().getTime();
  var model = new Drupal.visitorActions.ui.dialog.models.DialogModel({
    selector: anchorSelector,
    id: id
  });
  var view = new Drupal.acquiaLiftVariations.views.PageVariationMenuView({
    el: $anchor[0],
    model: model
  });
  model.set('active', true);
  var $dialog = $('#' + id + '-dialog');
  assert.equal($dialog.length, 1, 'Dialog created with specified id.');
  assert.ok($dialog.hasClass(view.className), 'Dialog created with view class.');
  assert.equal($dialog.find('.visitor-actions-ui-dialog-content').length, 1, 'Dialog content created.');

  var dialogHtml = $dialog.find('.visitor-actions-ui-dialog-content').html();
  var dialogTitle = Drupal.theme.acquiaLiftPageVariationsMenuTitle({elementType: 'DIV'});
  assert.equal(dialogHtml.search(dialogTitle), 0, 'Dialog title created and at start of content.');
  assert.ok(view.list instanceof Drupal.acquiaLiftVariations.views.PageVariationMenuListView, 'Dialog list component is a PageVariationMenuListView.');
  assert.equal($dialog.find('.visitor-actions-ui-dialog-content ul.acquia-lift-page-variation-list li').length, 5, 'Dialog list is rendered with one list item per variation type.');

  QUnit.stop();
  // Callback for individual type click.
  Backbone.on('acquiaLiftPageVariationType', function(e) {
    QUnit.start();
    assert.equal(e.data.anchor.id, 'mousehere', 'Variation type selected triggered with correct anchor element.');
    assert.equal(e.data.selector, anchorSelector, 'Variation type selected triggered with correct anchor element.');
    assert.equal(e.data.id, 'addCss', 'Variation type selected with correct variation type id.');
    assert.equal(e.data.name, 'Add CSS class', 'Variation type selected with correction variation type name.');
    Backbone.off('acquiaLiftPageVariationType');
    QUnit.stop();
    // Give the dialog time to close.
    setTimeout(function() {
      QUnit.start();
      $dialog = $('#' + id + '-dialog');
      assert.equal($dialog.length, 0, 'Contextual menu view has been removed.');
    }, 1000)
  })
  $dialog.find('.visitor-actions-ui-dialog-content ul.acquia-lift-page-variation-list li:first-child a').trigger('click');
})

QUnit.asyncTest('Contextual menu limited by children node types', function (assert) {
  QUnit.start();
  expect(11);
  var anchorSelector = '#another';
  var $anchor = $(anchorSelector);
  var id = 'testing-' + new Date().getTime();
  var model = new Drupal.visitorActions.ui.dialog.models.DialogModel({
    selector: anchorSelector,
    id: id
  });
  var view = new Drupal.acquiaLiftVariations.views.PageVariationMenuView({
    el: $anchor[0],
    model: model
  });
  model.set('active', true);
  var $dialog = $('#' + id + '-dialog');
  assert.equal($dialog.length, 1, 'Dialog created with specified id.');
  assert.ok($dialog.hasClass(view.className), 'Dialog created with view class.');
  assert.equal($dialog.find('.visitor-actions-ui-dialog-content').length, 1, 'Dialog content created.');

  var dialogHtml = $dialog.find('.visitor-actions-ui-dialog-content').html();
  var dialogTitle = Drupal.theme.acquiaLiftPageVariationsMenuTitle({elementType: 'DIV'});
  assert.equal(dialogHtml.search(dialogTitle), 0, 'Dialog title created and at start of content.');
  assert.ok(view.list instanceof Drupal.acquiaLiftVariations.views.PageVariationMenuListView, 'Dialog list component is a PageVariationMenuListView.');
  assert.equal($dialog.find('.visitor-actions-ui-dialog-content ul.acquia-lift-page-variation-list li').length, 4, 'Dialog list is rendered with one list item per variation type.');

  QUnit.stop();
  // Callback for individual type click.
  Backbone.on('acquiaLiftPageVariationType', function(e) {
    QUnit.start();
    assert.equal(e.data.anchor.id, 'another', 'Variation type selected triggered with correct anchor element.');
    assert.equal(e.data.selector, anchorSelector, 'Variation type selected triggered with correct anchor element.');
    assert.equal(e.data.id, 'addCss', 'Variation type selected with correct variation type id.');
    assert.equal(e.data.name, 'Add CSS class', 'Variation type selected with correction variation type name.');
    Backbone.off('acquiaLiftPageVariationType');
    QUnit.stop();
    // Give the dialog time to close.
    setTimeout(function() {
      QUnit.start();
      $dialog = $('#' + id + '-dialog');
      assert.equal($dialog.length, 0, 'Contextual menu view has been removed.');
    }, 1000)
  })
  $dialog.find('.visitor-actions-ui-dialog-content ul.acquia-lift-page-variation-list li:first-child a').trigger('click');
})
