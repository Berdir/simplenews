<?php

interface SimplenewsSourceInterface {

  function getSubject();

  function getBody();

  function getHeaders(array $headers);

  function getCategory();

  function getSubscriber();

  function getAttachments();

  function getTokenContext();
  
  function setCategory($tid);
  
  function setSubscriber($subscriber, $mail = NULL);

  function setKey($key);

  function getKey();

  function getFromFormatted();

  function getFromAddress();
}

abstract class SimplenewsSource implements SimplenewsSourceInterface {

  protected $category;
  protected $subscriber;
  protected $key;

  public function setCategory($tid) {
    $this->category = simplenews_category_load($tid);
  }

  public function getCategory() {
    return $this->category;
  }

  public function setSubscriber($subscriber, $mail = NULl) {
    if ($subscriber) {
      $this->subscriber = $subscriber;
    } else {
      $this->subscriber = simplenews_get_subscription(_simplenews_user_load($mail));
    }
  }

  public function getSubscriber() {
    return $this->subscriber;
  }

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

  function getTokenContext() {
    return array(
      'category' => $this->getCategory(),
      'account' => $this->getSubscriber(),
    );
  }

  function setKey($key) {
    $this->key = $key;
  }

  function getKey() {
    return $this->key;
  }

  function getFromFormatted() {
    $name = $this->getCategory()->from_name;

    // Windows based PHP systems don't accept formatted emails.
    if (substr(PHP_OS, 0, 3) == 'WIN') {
      return $this->getFromAddress();

    }
    else {
      return '"' . mime_header_encode($name) . '" <' . $this->getFromAddress() . '>';
    }

    return $formatted_address;
  }

  function getFromAddress() {
    return $this->getCategory()->from_name;
  }
}

abstract class SimplenewsSourceNode extends SimplenewsSource {

  protected $node;
  protected $spool;
  protected $newsletter;
  protected $build;

  public function __construct($spool_data) {
    $node = node_load($spool_data->nid);

    if (!$node) {
      throw new SimplenewsSourceException('Newsletter not send: newsletter issue does not exist (nid = @nid).', array('@nid' => $nid));
    }
    $this->newsletter = simplenews_newsletter_load($node->nid);
    $this->setCategory($this->newsletter->tid);
    $this->setSubscriber($spool_data->data, isset($spool_data->mail) ? $spool_data->mail : NULL);

    $this->setSourceNode($node);
  }

  function setSourceNode($node) {
    $langcode = $this->getSubscriber()->language;
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
      // If a translation of the node is used, load this node.
      if ($nid != $node->nid) {
        $this->node = node_load($nid);
      } else {
        $this->node = $node;
      }
    }
  }

  function getTokenContext() {
    $context = parent::getTokenContext();
    $context['node'] = $this->node;
    return $context;
  }

  function getSubject() {
    // Build email subject and perform some sanitizing.
    $subject = token_replace($this->getCategory()->email_subject, $this->getTokenContext(), array('sanitize' => FALSE));

    // Line breaks are removed from the email subject to prevent injection of
    // malicious data into the email header.
    $subject = str_replace(array("\r", "\n"), '', $subject);
    return $subject;
  }

  protected function build() {
    if (!empty($this->build)) {
      return $this->build;
    }

    // Set the active language to the node's language.
    // This needs to be done as otherwise the language used to send the mail
    // is the language of the user logged in.
    // @todo Rewrite this code for drupal 7.
  //    if (module_exists('i18n')) {
  //         i18n_selection_mode('node', $node->language);
  //    }
    // Build message body
    // @todo restore the format selection.
    //$build = node_view($node, 'email_' . $category->format);
    // Supported view modes: 'email_plain', 'email_html', 'email_textalt'
    $build = node_view($this->node, 'email_plain');
    // @todo Use simplenews_newsletter_body as #theme?
    unset($build['#theme']);
    foreach (element_children($build) as $child) {
      $build[$child]['#theme'] = 'simplenews_field';
    }

    $this->build = $build;
    return $build;
  }

  public function getBody() {
    $body = theme('simplenews_newsletter_body', array('build' => $this->build(), 'category' => $this->getCategory(), 'language' => $this->getSubscriber()->language));
    return $body;
  }

  function getFooter() {
    if ($this->category->opt_inout != SIMPLENEWS_OPT_INOUT_HIDDEN) {
      // Build and buffer message footer
      $footer = theme('simplenews_newsletter_footer', array(
        'build' => $this->build(),
        'category' => $this->getCategory(),
        'context' => $this->getTokenContext(),
        'key' => $this->getKey(),
        'language' => $this->getSubscriber()->language,
        )
      );
      return $footer;
    }
  }

  function getAttachments() {
    return array();
  }

}

class SimplenewsSourceNodeCached extends SimplenewsSourceNode {

  protected static $cache = array();

}
