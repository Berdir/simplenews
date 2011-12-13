<?php

/**
 * @file
 * Contains SimplenewsSource interface and implementations.
 */

/**
 * The source used to build a newsletter mail.
 */
interface SimplenewsSourceInterface {

  function prepareMail();

  /**
   * Returns the mail headers.
   *
   * @param $headers
   *   The default mail headers.
   *
   * @return
   *   Mail headers as an array.
   */
  function getHeaders(array $headers);

  /**
   * Returns the mail subject.
   */
  function getSubject();

  /**
   * Returns the mail body.
   */
  function getBody();

  /**
   * Returns the mail footer.
   */
  function getFooter();

  /**
   * Returns the mail format.
   *
   * @return
   *   The mail format as string, either 'plain' or 'html'.
   */
  function getFormat();

  /**
   * Returns the recipent of this newsletter mail.
   *
   * @return
   *   The recipient mail address(es) of this newsletter as a string.
   */
  function getRecipient();

  /**
   * The language that should be used for this newsletter mail.
   */
  function getLanguage();

  /**
   * Returns an array of attachments for this newsletter mail.
   *
   * @return
   *   An array of managed file objects with properties uri, filemime and so on.
   */
  function getAttachments();

  /**
   * Returns the token context to be used with token replacements.
   *
   * @return
   *   An array of objects as required by token_replace().
   */
  function getTokenContext();

  /**
   * Returns the mail key to be used for drupal_mail().
   *
   * @return
   *   The mail key, either test or node.
   */
  function getKey();

  /**
   * Returns the formatted from mail address.
   */
  function getFromFormatted();

  /**
   * Returns the plain mail address.
   */
  function getFromAddress();
}

interface SimplenewsSourceCacheInterface {

  function __construct(SimplenewsSourceInterface $source);

  function get($group, $key);

  function set($group, $key, $data);
}

/**
 * Abstract base class for simplenews sources.
 */
abstract class SimplenewsSource implements SimplenewsSourceInterface {

  protected $category;
  protected $subscriber;
  protected $key;

  /**
   * Cache implementation used for this source.
   *
   * @var SimplenewsSourceCacheInterface
   */
  protected $cache;

  protected $spool_list;

  protected $spool_data;

  public function __construct($spool_list) {
    $this->spool_list = $spool_list;
  }

  public function prepareMail() {
    $this->spool_data = next($this->spool_list);

    // If we're done, return false.
    if (!$this->spool_data) {
      return FALSE;
    }

    $this->newsletter = simplenews_newsletter_load($this->spool_data->nid);
    $this->category = simplenews_category_load($this->newsletter->tid);
    $this->setSubscriber($this->spool_data->data, isset($this->spool_data->mail) ? $this->spool_data->mail : NULL);
    $this->initCache($this->spool_data);
  }

  protected function initCache($spool_data) {
    $class = variable_get('simplenews_source_cache', 'SimplenewsSourceCacheBuild');
    $this->cache = new $class($this);
  }

  /**
   * Returns the corresponding category.
   *
   * @return
   *   The simplenews category object.
   */
  public function getCategory() {
    return $this->category;
  }

  /**
   * Set the active subscriber.
   *
   * If NULL, the subscriber object will be loaded or created based on the mail
   * address.
   *
   * @todo: This behavior should probably be removed.
   *
   * @param $subscriber
   *   The subscriber object.
   * @param $mail
   *   Optionally an email address that can be used to load the subscriber.
   *
   */
  public function setSubscriber($subscriber, $mail = NULl) {
    if ($subscriber) {
      $this->subscriber = $subscriber;
    }
    elseif (!empty($mail)) {
      $this->subscriber = simplenews_get_subscription(_simplenews_user_load($mail));
    }
  }

  /**
   * Return the subscriber object.
   */
  public function getSubscriber() {
    return $this->subscriber;
  }

  /**
   * Implements SimplenewsSourceInterface::getHeaders().
   */
  public function getHeaders(array $headers) {

    // If receipt is requested, add headers.
    if ($this->category->receipt) {
      $headers['Disposition-Notification-To'] = $from;
      $headers['X-Confirm-Reading-To'] = $from;
    }

    // Add priority if set.
    switch ($this->category->priority) {
      case SIMPLENEWS_PRIORITY_HIGHEST:
        $headers['Priority'] = 'High';
        $headers['X-Priority'] = '1';
        $headers['X-MSMail-Priority'] = 'Highest';
        break;
      case SIMPLENEWS_PRIORITY_HIGH:
        $headers['Priority'] = 'urgent';
        $headers['X-Priority'] = '2';
        $headers['X-MSMail-Priority'] = 'High';
        break;
      case SIMPLENEWS_PRIORITY_NORMAL:
        $headers['Priority'] = 'normal';
        $headers['X-Priority'] = '3';
        $headers['X-MSMail-Priority'] = 'Normal';
        break;
      case SIMPLENEWS_PRIORITY_LOW:
        $headers['Priority'] = 'non-urgent';
        $headers['X-Priority'] = '4';
        $headers['X-MSMail-Priority'] = 'Low';
        break;
      case SIMPLENEWS_PRIORITY_LOWEST:
        $headers['Priority'] = 'non-urgent';
        $headers['X-Priority'] = '5';
        $headers['X-MSMail-Priority'] = 'Lowest';
        break;
    }

    // Add general headers
    $headers['Precedence'] = 'bulk';
    return $headers;
  }

  /**
   * Implements SimplenewsSourceInterface::getTokenContext().
   */
  function getTokenContext() {
    return array(
      'category' => $this->getCategory(),
      'account' => $this->getSubscriber(),
    );
  }

  /**
   * Set the mail key.
   */
  function setKey($key) {
    $this->key = $key;
  }

  /**
   * Implements SimplenewsSourceInterface::getKey().
   */
  function getKey() {
    return $this->key;
  }

  /**
   * Implements SimplenewsSourceInterface::getFromFormatted().
   */
  function getFromFormatted() {
    $name = $this->getCategory()->from_name;

    // Windows based PHP systems don't accept formatted emails.
    if (substr(PHP_OS, 0, 3) == 'WIN') {
      return $this->getFromAddress();
    } else {
      return '"' . mime_header_encode($name) . '" <' . $this->getFromAddress() . '>';
    }

    return $formatted_address;
  }

  /**
   * Implements SimplenewsSourceInterface::getFromAddress().
   */
  function getFromAddress() {
    return $this->getCategory()->from_address;
  }

  /**
   * Implements SimplenewsSourceInterface::getRecipient().
   */
  function getRecipient() {
    return $this->getSubscriber()->mail;
  }

  /**
   * Implements SimplenewsSourceInterface::getFormat().
   */
  function getFormat() {
    return $this->getCategory()->format;
  }

  /**
   * Implements SimplenewsSourceInterface::getLanguage().
   */
  function getLanguage() {
    return $this->getSubscriber()->language;
  }

}

/**
 * Simplenews source implementation based on nodes.
 */
class SimplenewsSourceNode extends SimplenewsSource {

  /**
   * The node object.
   */
  protected $node;

  /**
   * The cached build render array.
   */
  protected $build;

  /**
   * Set the active node.
   */
  public function prepareMail() {
    parent::prepareMail();

    // Reset data from previous mail.
    $this->build = NULL;

    $node = node_load($spool_data->nid);
    if (!$node) {
      throw new SimplenewsSourceNodeDoesNotExistException('Newsletter not sent: newsletter issue does not exist (nid = @nid).', array('@nid' => $nid));
    }
    $this->setSourceNode($node);
  }

  /**
   * Set the source node.
   *
   * If translation module is enabled, get the translated node of the target
   * language, if available.
   *
   * @param $node
   *   The node object to be set as source.
   */
  protected function setSourceNode($node) {
    $langcode = $this->getLanguage();
    $nid = $node->nid;
    if (module_exists('translation')) {
      // If the node has translations and a translation is required
      // the equivalent of the node in the required language is used
      // or the base node (nid == tnid) is used.
      if ($tnid = $node->tnid) {
        if ($langcode != $node->language) {
          $translations = translation_node_get_translations($tnid);
          // A translation is available in the preferred language.
          if ($translation = $translations[$langcode]) {
            $nid = $translation->nid;
            $langcode = $translation->language;
          } else {
            // No translation found which matches the preferred language.
            foreach ($translations as $translation) {
              if ($translation->nid == $tnid) {
                $nid = $tnid;
                $langcode = $translation->language;
                break;
              }
            }
          }
        }
      }
    }
    // If a translation of the node is used, load this node.
    if ($nid != $node->nid) {
      $this->node = node_load($nid);
    } else {
      $this->node = $node;
    }
  }

  /**
   * Overrides SimplenewsSource::getTokenContext().
   *
   * Add the node object to the context.
   */
  function getTokenContext() {
    $context = parent::getTokenContext();
    $context['node'] = $this->node;
    return $context;
  }

  /**
   * Implements SimplenewsSourceInterface::getSubject().
   */
  function getSubject() {
    // Build email subject and perform some sanitizing.
    $subject = token_replace($this->getCategory()->email_subject, $this->getTokenContext(), array('sanitize' => FALSE));

    // Line breaks are removed from the email subject to prevent injection of
    // malicious data into the email header.
    $subject = str_replace(array("\r", "\n"), '', $subject);
    return $subject;
  }

  /**
   * Overwrites the current content language for i18n_select.
   */
  protected function setContentLanguage($language) {
    if (module_exists('i18n_select')) {
      $this->original_language = $GLOBALS['language_content'];
      $languages = language_list();
      $GLOBALS['language_content'] = $languages[$language];
    }
  }

  /**
   * Reset the content language back to the previous value.
   */
  protected function resetContentLanguage() {
    if (module_exists('i18n_select')) {
      $GLOBALS['language_content'] = $this->original_language;
    }
  }

  /**
   * Build the node object.
   *
   * The resulting build array is cached as it is used in multiple places.
   */
  protected function build() {
    if (!empty($this->build)) {
      return $this->build;
    }

    // Set the active language to the node's language.
    // This needs to be done as otherwise the language used to send the mail
    // is the language of the user logged in.
    $this->setContentLanguage($this->node->language);

    // Build message body
    // @todo restore the format selection.
    //$build = node_view($node, 'email_' . $category->format);
    // Supported view modes: 'email_plain', 'email_html', 'email_textalt'
    $build = node_view($this->node, 'email_plain');
    unset($build['#theme']);

    foreach (field_info_instances('node', $this->node->type) as $field_name => $field) {
      if (isset($build[$field_name])) {
        $build[$field_name]['#theme'] = 'simplenews_field';
      }
    }

    $this->resetContentLanguage();

    $this->build = $build;
    return $build;
  }

  /**
   * Build the themed newsletter body.
   */
  protected function buildBody() {
    if ($cache = $this->cache->get('build', 'body')) {
      return $cache;
    }
    $body = theme('simplenews_newsletter_body', array('build' => $this->build(), 'category' => $this->getCategory(), 'language' => $this->getLanguage()));
    $this->cache->set('build', 'body', $body);
    return $body;
  }

  /**
   * Implements SimplenewsSourceInterface::getBody().
   */
  public function getBody() {
    $body = $this->buildBody();

    // Build message body, replace tokens.
    $body = token_replace($body, $this->getTokenContext(), array('sanitize' => FALSE));

    // Convert to plain text if required.
    if ($this->getFormat() == 'plain') {
      $body = simplenews_html_to_text($body, $this->getCategory()->hyperlinks);
    }
    return $body;
  }

  /**
   * Builds the themed footer.
   */
  protected function buildFooter() {
    if ($cache = $this->cache->get('build', 'footer')) {
      return $cache;
    }

    // Build and buffer message footer
    $footer = theme('simplenews_newsletter_footer', array(
        'build' => $this->build(),
        'category' => $this->getCategory(),
        'context' => $this->getTokenContext(),
        'key' => $this->getKey(),
        'language' => $this->getLanguage(),
        )
    );
    $this->cache->set('build', 'footer', $footer);
    return $footer;
  }

  /**
   * Implements SimplenewsSourceInterface::getFooter().
   */
  function getFooter() {
    if ($cache = $this->cache->get('final', 'footer')) {
      return $cache;
    }

    if ($this->category->opt_inout != SIMPLENEWS_OPT_INOUT_HIDDEN) {
      $final_footer = token_replace($this->buildFooter(), $this->getTokenContext(), array('sanitize' => FALSE));
    }

    $this->cache->set('build', 'footer', $final_footer);
    return $final_footer;
  }

  /**
   * Implements SimplenewsSourceInterface::getAttachments().
   */
  function getAttachments() {
    if ($cache = $this->cache->get('data', 'attachments')) {
      return $cache;
    }

    $attachments = array();
    $build = $this->build();
    $fids = array();
    foreach (field_info_instances('node', $this->node->type) as $field_name => $field_instance) {
      // @todo: Find a better way to support more field types.
      // Only add fields of type file which are enabled for the current view
      // mode as attachments.
      $field = field_info_field($field_name);
      if ($field['type'] == 'file' && isset($build[$field_name])) {

        if ($items = field_get_items('node', $node, $field_name)) {
          foreach ($items as $item) {
            $fids[] = $item['fid'];
          }
        }
      }
    }
    if (!empty($fids)) {
      $attachments = file_load_multiple($fids);
    }

    $this->cache->set('data', 'attachments', $attachments);
    return $attachments;
  }

}

class SimplenewsSourceCacheNone implements SimplenewsSourceCacheInterface {

  public function __construct(SimplenewsSourceInterface $source) {
    // Do nothing.
  }

  public function get($group, $key) {
    return NULL;
  }

  public function set($group, $key, $data) {
    // Do nothing.
  }

}

class SimplenewsSourceCacheBuild implements SimplenewsSourceCacheInterface {

  /**
   * The simplenews source for which this cache is used.
   *
   * @var SimplenewsSourceInterface
   */
  protected $source;

  protected $cid;

  protected static $cache = array();

  public function __construct(SimplenewsSourceInterface $source) {
    $this->source = $source;
  }

  protected function getCid() {
    if (empty($this->cid)) {
      $context = $this->source->getTokenContext();
      $this->cid = '';
      if (!empty($context['node'])) {
        $this->cid .= $context['node']->nid . ':';
      }
      $this->cid .= $this->source->getLanguage() . ':';
    }
    return $this->cid;
  }

  public function get($group, $key) {
    // Only cache data and build information.
    if (!in_array($group, array('data', 'build'))) {
      return;
    }

    if (isset(self::$cache[$this->getCid()][$group][$key])) {
      return self::$cache[$this->getCid()][$group][$key];
    }
  }

  public function set($group, $key, $data) {
    // Only cache data and build information.
    if (!in_array($group, array('data', 'build'))) {
      return;
    }

    self::$cache[$this->getCid()][$group][$key] = $data;
  }

}

class SimplenewsSourceNodeDoesNotExistException extends Exception {
  public function __construct() {

  }
}