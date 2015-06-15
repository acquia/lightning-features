/**
 * Behaviors that should be called on every page.
 */

(function ($, Drupal) {

  Drupal.behaviors.acquia_lift_goal_queue = {
    attach: function (context, settings) {
      // Once per page, clear out the goals processing status and attempt to
      // process any goals that were left from previous page requests.
      $('body').once('acquiaLiftGoalsQueue', function () {
        Drupal.acquiaLiftUtility.GoalQueue.processQueue(true);
      });
    }
  }

})(Drupal.jQuery, Drupal);
