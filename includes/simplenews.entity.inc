<?php

/**
 * @file
 * Seimplenews entities definitions.
 *
 * @ingroup simplenews
 */

/**
 * Class for simplenews_newsleter entity.
 *
 * @incroup newsletter
 */
class SimplenewsNewsletter extends Entity {

  public $newsletter_id;
  public $name;
  public $description;
  public $format;
  public $priority;
  public $receipt;
  public $from_name;
  public $email_subject;
  public $from_address;
  public $hyperlinks;
  public $new_account;
  public $opt_inout;
  public $block;
  public $weight;

  /**
   * Override Entity
   *
   * Prevent failure if the entity system ist not fully loaded.
   */
  protected function setUp() {
    // Hack to get arount the not loaded entity system.
    // TODO: fix this and remove hack.
    if (function_exists('entity_get_info')) {
      parent::setUp();
    }
  }

  /**
   * Override Entity
   *
   * Prevent failure if the entity system ist not fully loaded.
   */
  public function __sleep() {
    // TODO: fix this and remove hack.
    $vars = get_object_vars($this);
    unset($vars['entityInfo'], $vars['idKey'], $vars['nameKey'], $vars['statusKey']);
    return array_combine(array_keys($vars), (array_keys($vars)));
  }

  /**
   * Override Entity
   */
  protected function defaultUri() {
    return array('path' => 'admin/config/services/simplenews/categories/' . $this->identifier() . '/edit');
  }

}
