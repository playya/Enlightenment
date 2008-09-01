<?php

/**
 * This class has been auto-generated by the Doctrine ORM Framework
 */
abstract class BaseUser extends sfDoctrineRecord
{

  public function setTableDefinition()
  {
    $this->setTableName('user');
    $this->hasColumn('id', 'integer', 4, array('primary' => true, 'autoincrement' => true));
    $this->hasColumn('name', 'string', 255);
    $this->hasColumn('email', 'string', 255);
    $this->hasColumn('password', 'string', 255);
    $this->hasColumn('role', 'integer', 4);
    $this->hasColumn('hash', 'string', 50);
    $this->hasColumn('api_key', 'string', 50);
    $this->hasColumn('active', 'boolean', null);
    $this->hasColumn('created_at', 'timestamp', null);
    $this->hasColumn('updated_at', 'timestamp', null);
  }

  public function setUp()
  {
    parent::setUp();
    $this->hasMany('Application as Applications', array('local' => 'id',
                                                        'foreign' => 'user_id'));

    $this->hasMany('Comment as Comments', array('local' => 'id',
                                                'foreign' => 'user_id'));

    $this->hasMany('Madule as Madules', array('local' => 'id',
                                              'foreign' => 'user_id'));

    $this->hasMany('Rating as Ratings', array('local' => 'id',
                                              'foreign' => 'user_id'));

    $this->hasMany('Theme as Themes', array('local' => 'id',
                                            'foreign' => 'user_id'));
  }

}
