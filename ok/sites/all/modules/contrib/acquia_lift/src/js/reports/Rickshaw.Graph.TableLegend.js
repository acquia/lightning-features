/**
 * Append the legend to the first column of a table.
 * ----------------------------------------------------------------------------
 */

Rickshaw.namespace('Rickshaw.Graph.TableLegend');

Rickshaw.Graph.TableLegend = Rickshaw.Class.create(Rickshaw.Graph.Legend, {

  initialize: function(args) {
    this.element = args.element;
    this.graph = args.graph;

    this.render();

    // we could bind this.render.bind(this) here
    // but triggering the re-render would lose the added
    // behavior of the series toggle
    this.graph.onUpdate( function() {} );
  },
  render: function() {
    var $ = jQuery,
        self = this,
        $label = $(this.element).find('thead > tr > th:first-child');

    this.lines = [];

    var series = this.graph.series
      .map( function(s) { return s } );

    series.forEach( function(s) {
      self.addLine(s);
    } );
  },
  addLine: function (series) {
    var $ = jQuery,
        self = this;

    $(this.element).find('tbody > tr').each(function (index, row) {
      var $cell = $(row).find('td:first-child');

      if ($(row).find('td:first-child').text() == series.name) {

        $cell.addClass('legend line');
        if (series.disabled) {
          $cell.addClass('disabled');
        }
        if (series.className) {
          d3.select($cell[0]).classed(series.className, true);
        }

        var label = document.createElement('span');
        label.className = 'label';
        label.innerHTML = (series.shortName || series.name) + ':';

        $cell.prepend(label);

        $cell[0].series = series;

        var swatch = document.createElement('div');
        swatch.className = 'swatch';
        swatch.style.backgroundColor = series.color;

        $cell.prepend(swatch);

        if (series.noLegend) {
          $cell.css('display', 'none');
        }

        var _line = { element: $cell[0], series: series };
        if (self.shelving) {
          self.shelving.addAnchor(_line);
          self.shelving.updateBehaviour();
        }
        if (self.highlighter) {
          self.highlighter.addHighlightEvents(_line);
        }
        self.lines.push(_line);
        return $cell[0];
      }
    });
  }
});
