<?php
/**
 * This class has been auto-generated by the Doctrine ORM Framework
 */
class AddThemeThemeGroup extends Doctrine_Migration
{
	public function up()
	{
		$this->createTable('theme_theme_group', array (
  'id' => 
  array (
    'primary' => true,
    'autoincrement' => true,
    'type' => 'integer',
    'length' => 4,
  ),
  'name' => 
  array (
    'type' => 'string',
    'length' => 255,
  ),
  'theme_id' => 
  array (
    'type' => 'integer',
    'length' => 4,
  ),
  'theme_group_id' => 
  array (
    'type' => 'integer',
    'length' => 4,
  ),
  'created_at' => 
  array (
    'type' => 'timestamp',
    'length' => 25,
  ),
  'updated_at' => 
  array (
    'type' => 'timestamp',
    'length' => 25,
  ),
), array (
  'indexes' => 
  array (
  ),
  'primary' => 
  array (
    0 => 'id',
  ),
));
	}

	public function down()
	{
		$this->dropTable('theme_theme_group');
	}
}