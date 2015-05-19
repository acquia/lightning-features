<?php

/**
 * @file
 *  Migrations for Fieldable Panel Panes.
 */

class LightningFPPContent extends Migration {

  public function __construct($arguments) {
    parent::__construct($arguments);
    $this->description = t('Import Content FPPs.');

    // Create a map object for tracking the relationships between source rows
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'Title' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => FALSE,
        )
      ),
      array(
        'title' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => FALSE,
        )
      )
    );

    // Create a MigrateSource object.
    if (isset($arguments['path'])) {
      $import_path = $arguments['path'];
    }
    else {
      $import_path = drupal_get_path('module', 'lightning_fpp') . '/import/lightning_fpp.fpp.content.csv';
    }
    $this->source = new MigrateSourceCSV($import_path, array(), array('header_rows' => 1));
    $this->destination = new MigrateDestinationFieldablePanelsPanes('content');

    $source_migration = isset($arguments['source_migration']) ? $arguments['source_migration'] : 'LightningFPPContentNodes';
    $this->addFieldMapping('field_content', 'Content')->separator(',')->sourceMigration($source_migration);

    // Add optional mapping for UUID
    $this->addFieldMapping('uuid', 'UUID');

    $this->addFieldMapping('title', 'Title');
    $this->addFieldMapping('category', 'Category')->defaultValue('Reusable panes');

    $this->addFieldMapping('reusable', 'Reusable')->defaultValue(TRUE);
  }

}
