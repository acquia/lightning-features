/**
 * Add a label to the Y axis.
 * ----------------------------------------------------------------------------
 * The label will inherit the axis data name or can be explicitely defined.
 */

Rickshaw.namespace('Rickshaw.Graph.Axis.LabeledY');

Rickshaw.Graph.Axis.LabeledY = Rickshaw.Class.create(Rickshaw.Graph.Axis.Y, {
  initialize: function(args) {

    this.label = args.label || '';

    this.graph = args.graph;
    this.orientation = args.orientation || 'right';

    this.pixelsPerTick = args.pixelsPerTick || 75;
    if (args.ticks) this.staticTicks = args.ticks;
    if (args.tickValues) this.tickValues = args.tickValues;

    this.tickSize = args.tickSize || 4;
    this.ticksTreatment = args.ticksTreatment || 'plain';

    this.tickFormat = args.tickFormat || function(y) { return y };

    this.berthRate = 0.10;

    if (args.element) {

      this.element = args.element;
      this.vis = d3.select(args.element)
        .append("svg:svg")
        .attr('class', 'rickshaw_graph y_axis');

      this.element = this.vis[0][0];
      this.element.style.position = 'relative';

      this.setSize({ width: args.width, height: args.height });

    } else {
      this.vis = this.graph.vis;
    }

    var self = this;
    this.graph.onUpdate( function() { self.render() } );
  },
  _drawAxis: function(scale) {
    var axis = d3.svg.axis().scale(scale).orient(this.orientation);

    axis.tickFormat(this.tickFormat);
    if (this.tickValues) axis.tickValues(this.tickValues);

    if (this.orientation == 'left') {
      var berth = this.height * this.berthRate,
          transform = 'translate(' + this.width + ', ' + berth + ')',
          labelX = this.height / 4 * -1,
          labelY = this.width / 3,
          labelTransform = 'rotate(-90 50 50)';
    }
    else if (this.orientation == 'right') {
      var labelX = this.height / 2,
          labelY = this.width / 3 * 2,
          labelTransform = 'rotate(90 50 50)';
    }

    if (this.element) {
      this.vis.selectAll('*').remove();
    }

    this.vis
      .append("svg:g")
      .attr("class", ["y_ticks", this.ticksTreatment].join(" "))
      .attr("transform", transform)
      .call(axis.ticks(this.ticks).tickSubdivide(0).tickSize(this.tickSize));

    label = this.vis
      .append("svg:text")
      .attr('class', 'y-axis-label')
      .attr('x', labelX)
      .attr('y', labelY)
      .attr('text-anchor', 'middle')
      .attr('transform', labelTransform)
      .text(this.label);

    return axis;
  }
});
