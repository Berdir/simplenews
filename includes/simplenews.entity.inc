<?php

/**
 * @file
 * Seimplenews entities definitions.
 *
 * @ingroup simplenews
 */

/**
 * Newsletter class.
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

  protected function defaultLabel() {
    return $this->name;
  }

  protected function defaultUri() {
    return array('path' => 'admin/config/services/simplenews/categories/' . $this->identifier() . '/edit');
  }

}

?>
