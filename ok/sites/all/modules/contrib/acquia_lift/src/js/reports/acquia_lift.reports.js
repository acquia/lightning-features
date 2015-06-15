/**
 * Functionality related specifically to the Acquia Lift reports.
 */

(function ($, Drupal) {
  "use strict";

  Drupal.behaviors.acquiaLiftReports = {
    attach: function (context, settings) {
      // Generate graphs and switches for the A/B statistics.
      $('.lift-statistics').once('acquiaLiftReports', function () {
        var $statistics = $(this),
            $data = $statistics.find('table[data-lift-statistics]'),
            campaign = $data.attr('data-acquia-lift-campaign'),
            $goalSelect = $('.acquia-lift-report-section-options .form-item-goal select'),
            $metricSelect = $('.acquia-lift-report-section-options .form-item-metric select'),
            metric = $metricSelect.val(),
            // Find the table column that matches the metric value.
            metricColumn = function() {
              var $heads = $data.find('thead > tr > th'),
                  column = $data.find('thead > tr > th[data-conversion-metric="' + metric + '"]')[0];

              return $heads.index(column) + 1;
            };

        // Format the initial data.
        $data.liftGraph();

        // Load a new report if the goal is changed.
        $goalSelect.change(function() {
          // Construct the GET path.
          var args = {
                campaign: campaign,
                goal: $(this).val()
              },
              path = Drupal.settings.basePath + Drupal.settings.pathPrefix + 'acquia_lift/reports/conversion?' + $.param(args);

          // Get the new report and replace the existing report(s) with it.
          $.get(path, function (html) {
            var $html = $(html);
            $statistics.find('.lift-statistic-category').remove();
            $statistics.prepend($html);
            // Simultaneously replace $data with the new table.
            $data = $html.find('table[data-lift-statistics]');
            // Make sure the proper metric column is set and render the graph.
            $data.attr('data-liftgraph-columny', metricColumn()).liftGraph();
          });
        })

        // Attach a data column to a metric option.
        // Change the data fed to the y-axis and update the graph.
        $metricSelect.change(function () {
          // Set the new metric value.
          metric = $(this).val();

          // Change the data-liftgraph-columny attribute and rebuild the graph.
          $data.attr('data-liftgraph-columny', metricColumn())
            .liftGraph('update');
        })
      });

      // Hide the submit button.
      $('.acquia-lift-report-section-options .form-submit').hide();
    }
  }

}(Drupal.jQuery, Drupal));
