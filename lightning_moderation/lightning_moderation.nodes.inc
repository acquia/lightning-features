<?php

/**
 * @file
 *  Migration Base for Moderated Nodes.
 */

class LightningModerationNodes extends ImportBaseNodes {

  public function __construct($arguments) {
    parent::__construct($arguments);

    if (isset($arguments['path'])) {
      $import_path = $arguments['path'];
    }
    else {
      $import_path = drupal_get_path('module', 'lightning_moderation') . '/import/lightning_moderation.nodes.csv';
    }
    $this->source = new MigrateSourceCSV($import_path, $this->csvcolumns(), array('header_rows' => 1));
    
    // Workbench Moderation
    $this->addFieldMapping('workbench_moderation_state_new', 'state')->defaultValue('published');
    // Created Date
    $this->addFieldMapping('created', 'created')->defaultValue(strtotime("now"));
  }

  function csvcolumns() {
    $columns[0] = array('title', 'Title');
    $columns[1] = array('body', 'Body');
    $columns[2] = array('state', 'State');
    return $columns;
  }

}

