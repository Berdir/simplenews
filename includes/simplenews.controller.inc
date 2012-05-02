<?php

/**
 * @file
 * Seimplenews entity controllers definitions.
 *
 * @ingroup simplenews
 */
class SimplenewsNewsletterController extends EntityAPIController {

  public function create(array $values = array()) {
    $values += array(
      'name' => '',
      'description' => '',
      'format' => variable_get('simplenews_format', 'plain'),
      'priority' => variable_get('simplenews_priority', SIMPLENEWS_PRIORITY_NONE),
      'receipt' => variable_get('simplenews_receipt', 0),
      'from_name' => variable_get('simplenews_from_name', variable_get('site_name', 'Drupal')),
      'email_subject' => '[[simplenews-category:name]] [node:title]',
      'from_address' => variable_get('simplenews_from_address', variable_get('site_mail', ini_get('sendmail_from'))),
      'hyperlinks' => 1,
      'new_account' => 'none',
      'opt_inout' => 'double',
      'block' => 1,
      'weight' => '0',
    );
    return parent::create($values);
  }

  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    parent::delete($ids, $transaction);

    foreach ($ids as $id) {
      // Delete subscriptions
      simplenews_subscription_delete(array('tid' => $id));
      drupal_set_message(t('All subscriptions to newsletter %newsletter have been deleted.', array('%newsletter' => _simplenews_newsletter_name($id))));

      // Delete subscription block
      db_delete('block')
        ->condition('module', 'simplenews')
        ->condition('delta', $id)
        ->execute();
    }
  }
}

?>
