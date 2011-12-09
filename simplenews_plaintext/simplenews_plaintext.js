(function ($) {

	// Defines an ajax command to replace content inside 
	// the plaintext field after serverside processing
	Drupal.ajax.prototype.commands.replacecontent = function (ajax, response, status) {
	  $('#' + response.trigger).parent().find('textarea').val(response.plaintext);
	}

})(jQuery);
