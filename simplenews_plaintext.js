(function ($) {

  // Defines an ajax command to replace content inside
  // the plaintext field after serverside processing
  Drupal.ajax.prototype.commands.replacecontent = function (ajax, response, status) {
    console.log(response);
    console.log($('#' + response.trigger).parent());
    $('#' + response.trigger).parent().find('textarea').val(response.plaintext);
  }

})(jQuery);
