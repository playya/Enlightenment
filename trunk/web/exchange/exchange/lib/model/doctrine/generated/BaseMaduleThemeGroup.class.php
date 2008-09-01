<?php

/**
 * This class has been auto-generated by the Doctrine ORM Framework
 */
abstract class BaseMaduleThemeGroup extends sfDoctrineRecord
{

  public function setTableDefinition()
  {
    $this->setTableName('madule_theme_group');
    $this->hasColumn('id', 'integer', 4, array('primary' => true, 'autoincrement' => true));
    $this->hasColumn('name', 'string', 255);
    $this->hasColumn('madule_id', 'integer', 4);
    $this->hasColumn('theme_group_id', 'integer', 4);
    $this->hasColumn('created_at', 'timestamp', null);
    $this->hasColumn('updated_at', 'timestamp', null);
  }

  public function setUp()
  {
    parent::setUp();
    $this->hasOne('Madule', array('local' => 'madule_id',
                                  'foreign' => 'id'));

    $this->hasOne('ThemeGroup', array('local' => 'theme_group_id',
                                      'foreign' => 'id'));
  }

}
