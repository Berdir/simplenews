
/**
 * Set text of Save button dependent on the selected send option.
 */
Drupal.behaviors.simplenewsCommandSend = function (context) {
  // Workaround for a bug of Firefox that might change radio buttons on page reload.
  // @see http://www.ryancramer.com/journal/entries/radio_buttons_firefox
  $('.simplenews-command-send').attr('autocomplete', 'off');

  var simplenewsSendButton = function () {
    switch ($(".simplenews-command-send :radio:checked").val()) {
      case '0':
        $('#edit-submit').attr({value: Drupal.t('Save')});
        break;
      case '1':
        $('#edit-submit').attr({value: Drupal.t('Save and send')});
        break;
      case '2':
        $('#edit-submit').attr({value: Drupal.t('Save and send test')});
        break;
    }
  }
  
  // Update send button at page load and when a send option is selected.
  $(function() { simplenewsSendButton(); });
  $(".simplenews-command-send").click( function() { simplenewsSendButton(); });
  
  
}
