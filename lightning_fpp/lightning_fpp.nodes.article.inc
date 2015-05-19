<?php

/**
 * @file
 *  Example migration to test LightningContentFPP migration class.
 */

class LightningFPPContentNodes extends ImportBaseNodes {

  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import example Lightning nodes for testing FPPs.');
    $import_path = drupal_get_path('module', 'lightning_fpp') . '/import/';

    // Create a MigrateSource object.
    $this->source = new MigrateSourceCSV($import_path . 'lightning_fpp.nodes.article.csv', $this->csvcolumns(), array('header_rows' => 1));
    $this->destination = new MigrateDestinationNode('article');

    // Set body:format
    $this->addFieldMapping('body:format')->defaultValue('full_html');
  }

  function csvcolumns() {
    $columns[0] = array('title', 'Title');
    $columns[1] = array('body', 'Body');
    return $columns;
  }

}
