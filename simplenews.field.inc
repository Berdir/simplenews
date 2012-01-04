<?php

/**
 * @file
 *
 * Defines a text widget for content types that are used as simplenews newsletters.
 * This can only be used by newsletters that are sent as html mails and use a specified
 * field for their plaintext content. The widget consists of a textarea and an additional
 * 'process' button which allows a serverside conversion of the typed text into plaintext.
 *
 * Notes
 * Don't use "Filtered text" with the plaintext field.
 * @todo try to avoid "filtered text".
 *
 */

/**
 * Implements hook_field_widget_info().
 */
function simplenews_field_widget_info() {
  return array(
    'simplenews_plaintext_textarea' => array(
      'label' => t('Textarea for Plaintext (multiple rows)'),
      'field types' => array('text_long'),
      'settings' => array('rows' => 20),
    ),
  );
}

/*
 * Implements hook_field_create_instance().
 *
 * Enables the display style 'email_plain' and sets the
 * display of the created field in 'email_plain' to hidden.
 */

function simplenews_field_create_instance($instance) {

  if ($instance['widget']['type'] == 'simplenews_plaintext_textarea') {
    $bundle_settings = array(
      'view_modes' => array(
        'email_plain' => array('custom_settings' => TRUE),
        'email_html' => array('custom_settings' => TRUE),  
      ),
    );
    field_bundle_settings('node', $instance['bundle'], $bundle_settings);

    $instance['display']['email_plain'] = array(
      'label' => 'hidden',
      'type' => 'hidden',
    );
    $instance['display']['email_html'] = array(
      'label' => 'hidden',
      'type' => 'hidden',
    );
    field_update_instance($instance);
  }
}

/**
 * Implements hook_field_widget_settings_form().
 */
function simplenews_field_widget_settings_form($field, $instance) {

  $widget = $instance['widget'];
  $settings = $widget['settings'];

  $form['rows'] = array(
    '#type' => 'textfield',
    '#title' => t('Rows'),
    '#default_value' => $settings['rows'],
    '#required' => TRUE,
    '#element_validate' => array('element_validate_integer_positive'),
  );

  return $form;
}

/**
 * Implements hook_field_widget_form().
 *
 * @todo: maybe integrate the elements of the widget into a fieldset for clearer visual presentation
 */
function simplenews_field_widget_form(&$form, &$form_state, $field, $instance, $langcode, $items, $delta, $element) {

  $widget = array();

  $widget['value'] = array(
    '#type' => 'textarea',
    '#title' => $instance['label'],
    '#default_value' => isset($items[$delta]['value']) ? $items[$delta]['value'] : NULL,
    '#rows' => $instance['widget']['settings']['rows'],
    '#attributes' => array('class' => array('text-full')),
    '#prefix' => '<div id="plaintext-container">',
    '#suffix' => '</div>',
  );

  $jspath = drupal_get_path('module', 'simplenews_plaintext') . '/simplenews_plaintext.js';
  $widget['#attached']['js'] = array($jspath);

  $widget['process_trigger'] = array(
    '#type' => 'submit',
    '#value' => t('Generate plaintext'),
    '#ajax' => array(
      'callback' => 'simplenews_process_plaintext_value',
      'progress' => array(
        'type' => 'progress',
      ),
    ),
  );

  $widget = $element + $widget;

  return $widget;
}

function simplenews_process_plaintext_value($form, $form_state) {

  // Build the node to get all necessary field values.
  $node = $form_state['node'];
  entity_form_submit_build_entity('node', $node, $form, $form_state);
  node_submit($node);

  // Generate default plaintext variant.
  module_load_include('inc', 'simplenews', 'includes/simplenews.mail');
  $source = new SimplenewsSourceNode($node, $subscriber);
  $plaintext = simplenews_html_to_text($text);

  // send the plaintext value to a client side js command
  return array(
    '#type' => 'ajax',
    '#commands' => array(
      array(
        'command' => 'replacecontent',
        'plaintext' => $plaintext,
        'trigger' => $form_state['triggering_element']['#id'],
      ),
      // Add the status messages inside the new content's wrapper element, so that
      // on subsequent Ajax requests, it is treated as old content.
      ajax_command_prepend(NULL, theme('status_messages'))
    ),
  );
}
