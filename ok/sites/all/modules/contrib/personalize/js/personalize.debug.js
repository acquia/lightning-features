(function ($, Drupal) {

  Drupal.personalizeDebug = (function() {

    function getMessageType(code) {
      if (code < 3000) {
        return 'OK';
      }
      if (code < 4000) {
        return 'WARNING';
      }
      return 'ERROR';
    }

    var debuggedMessages = [];

    return {
      /**
       * Outputs the passed in message.
       *
       * Checks first whether the same message has previously been output.
       *
       * @param message
       *   The message to output.
       * @param code
       *   THe message code.
       *
       * @todo Make this accept message parameters and use Drupal.t to translate.
       */
      'log': function(message, code) {
        if (debuggedMessages.indexOf(message) == -1) {
          var messageType = getMessageType(code);
          console.log(messageType + ': ' + message + ' [Code ' + code + ']');
          debuggedMessages.push(message);
        }
      }
    };
  })();

})(Drupal.jQuery, Drupal);
