<?php

/**
 * @file
 * Seimplenews entity controllers definitions.
 *
 * @ingroup simplenews
 */

/**
 * Controller for simplenews_newsletter entity.
 *
 * @ingroup newsletter
 */
class SimplenewsNewsletterController extends EntityAPIController {

  /**
   * Override EntityAPIController.
   */
  public function create(array $values = array()) {
    $values += array(
      'name' => '',
      'description' => '',
      'format' => variable_get('simplenews_format', 'plain'),
      'priority' => variable_get('simplenews_priority', SIMPLENEWS_PRIORITY_NONE),
      'receipt' => variable_get('simplenews_receipt', 0),
      'from_name' => variable_get('simplenews_from_name', variable_get('site_name', 'Drupal')),
      'email_subject' => '[[simplenews-newsletter:name]] [node:title]',
      'from_address' => variable_get('simplenews_from_address', variable_get('site_mail', ini_get('sendmail_from'))),
      'hyperlinks' => 1,
      'new_account' => 'none',
      'opt_inout' => 'double',
      'block' => 1,
      'weight' => '0',
    );
    return parent::create($values);
  }

  /**
   * Override EntityAPIController.
   *
   * Make sure all subscriptions to the newsletters gets deleted.
   *
   * @param $transaction
   *   Optionally a DatabaseTransaction object to use. Allows overrides to pass
   *   in their transaction object.
   */
  public function delete($ids, DatabaseTransaction $transaction = NULL) {
    $names = array();
    foreach (simplenews_newsletter_load_multiple($ids) as $newsletter) {
      $names[$newsletter->newsletter_id] = _simplenews_newsletter_name($newsletter);
    }

    parent::delete($ids, $transaction);

    foreach ($ids as $id) {
      // Delete subscriptions
      simplenews_subscription_delete(array('newsletter_id' => $id));
      drupal_set_message(t('All subscriptions to newsletter %newsletter have been deleted.', array('%newsletter' => $names[$id])));

      // Delete subscription block
      db_delete('block')
        ->condition('module', 'simplenews')
        ->condition('delta', $id)
        ->execute();
    }
  }
}
