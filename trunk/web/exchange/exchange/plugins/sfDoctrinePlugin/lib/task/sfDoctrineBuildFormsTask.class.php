<?php

/*
 * This file is part of the symfony package.
 * (c) 2007-2008 Jonathan H. Wage <jonwage@gmail.com>
 * 
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

require_once(dirname(__FILE__).'/sfDoctrineBaseTask.class.php');

/**
 * Create form classes for the current model.
 *
 * @package    symfony
 * @subpackage command
 * @author     Jonathan H. Wage <jonwage@gmail.com>
 * @version    SVN: $Id: sfDoctrineBuildFormsTask.class.php 8512 2008-04-17 18:06:12Z fabien $
 */
class sfDoctrineBuildFormsTask extends sfDoctrineBaseTask
{
  /**
   * @see sfTask
   */
  protected function configure()
  {
    $this->addArguments(array(
      new sfCommandArgument('application', sfCommandArgument::REQUIRED, 'The application name'),
    ));

    $this->addOptions(array(
      new sfCommandOption('env', null, sfCommandOption::PARAMETER_REQUIRED, 'The environment', 'dev'),
      new sfCommandOption('connection', null, sfCommandOption::PARAMETER_REQUIRED, 'The connection name', 'doctrine'),
      new sfCommandOption('model-dir-name', null, sfCommandOption::PARAMETER_REQUIRED, 'The model dir name', 'model'),
      new sfCommandOption('form-dir-name', null, sfCommandOption::PARAMETER_REQUIRED, 'The form dir name', 'form'),
    ));

    $this->aliases = array('doctrine-build-forms');
    $this->namespace = 'doctrine';
    $this->name = 'build-forms';
    $this->briefDescription = 'Creates form classes for the current model';

    $this->detailedDescription = <<<EOF
The [doctrine:build-forms|INFO] task creates form classes from the schema:

  [./symfony doctrine:build-forms|INFO]

The task read the schema information in [config/doctrine/*.yml|COMMENT] from 
the project and all installed plugins.

The task use the [doctrine|COMMENT] connection as defined in [config/databases.yml|COMMENT].
You can use another connection by using the [--connection|COMMENT] option:

  [./symfony doctrine:build-forms frontend --connection="name"|INFO]

The model form classes files are created in [lib/form/doctrine|COMMENT].

This task never overrides custom classes in [lib/form/doctrine|COMMENT].
It only replaces base classes generated in [lib/form/doctrine/base|COMMENT].
EOF;
  }

  /**
   * @see sfTask
   */
  protected function execute($arguments = array(), $options = array())
  {
    $buildModel = new sfDoctrineBuildModelTask($this->dispatcher, $this->formatter);
    $buildModel->setCommandApplication($this->commandApplication);
    $buildModel->run();

    $this->logSection('doctrine', 'generating form classes');

    $databaseManager = new sfDatabaseManager($this->configuration);

    $generatorManager = new sfGeneratorManager($this->configuration);

    $generatorManager->generate('sfDoctrineFormGenerator', array(
      'connection'     => $options['connection'],
      'model_dir_name' => $options['model-dir-name'],
      'form_dir_name'  => $options['form-dir-name'],
    ));
  }
}