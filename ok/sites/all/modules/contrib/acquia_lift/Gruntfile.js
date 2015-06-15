module.exports = function(grunt) {
  require('time-grunt')(grunt);

  var reportjs = [
        'src/js/reports/Rickshaw.Graph.Axis.LabeledY.js',
        'src/js/reports/Rickshaw.Graph.ClickDetail.js',
        'src/js/reports/Rickshaw.Graph.TableLegend.js',
        'src/js/reports/acquia_lift.liftgraph.jquery.js',
        'src/js/reports/acquia_lift.reports.js'
      ];

  var flowjs = [
        'src/js/flow/acquia_lift.modal.js',
        'src/js/flow/acquia_lift.variations.js',
        'src/js/flow/acquia_lift.variations.models.js',
        'src/js/flow/acquia_lift.variations.collections.js',
        'src/js/flow/acquia_lift.variations.theme.js',
        'src/js/flow/acquia_lift.variations.views.js',
        'src/js/flow/acquia_lift.variations.editInContext.js',
        'src/js/flow/acquia_lift.ctools.modal.js'
      ];

  var goalqueuejs = [
        'src/js/agent/acquia_lift.utility.queue.js',
        'src/js/agent/acquia_lift.agent.goal_queue.js',
      ];

  var unibarjs = [
        'src/js/menu/acquia_lift.personalize.theme.js',
        'src/js/menu/acquia_lift.personalize.js',
        'src/js/menu/acquia_lift.personalize.models.js',
        'src/js/menu/acquia_lift.personalize.collections.js',
        'src/js/menu/acquia_lift.personalize.views.js',
        'src/js/menu/acquia_lift.personalize.factories.js',
        'src/js/menu/acquia_lift.personalize.commands.js',
        'src/js/menu/acquia_lift.personalize.behaviors.js'
      ];

  // Project configuration.
  grunt.initConfig({
    pkg: grunt.file.readJSON('package.json'),
    autoprefixer: {
      css: {
        options: {
          // Uncomment the map setting to enable sourcemaps.
          // map: true
        },
        src: 'css/**/*.css'
      }
    },
    concat: {
      options: {
        sourceMap: true,
        separator: "\n"
      },
      reports: {
        src: reportjs,
        dest: 'js/acquia_lift.reports.js'
      },
      help: {
        src: ['src/js/help/acquia_lift.help.js'],
        dest: 'js/acquia_lift.help.js'
      },
      flow: {
        src: flowjs,
        dest: 'js/acquia_lift.flow.js'
      },
      agent: {
        src: goalqueuejs,
        dest: 'js/acquia_lift.goals_queue.js'
      },
      unibar: {
        src: unibarjs,
        dest: 'js/acquia_lift.personalize.js'
      }
    },
    concurrent: {
      all: ['style', 'script', 'test']
    },
    // Can only test those QUnit tests that do not require Drupal interaction.
    qunit: {
      all: ['qunit/core_personalization.html']
    },
    sass: {
      dist: {
        options: {
          // Comment out the sourcemap setting to enable sourcemaps.
          sourcemap: 'none',
          style: 'expanded'
        },
        files: {
          'css/acquia_lift.help.css': 'src/css/acquia_lift.help.scss',
          'css/acquia_lift.reports.css': 'src/css/acquia_lift.reports.scss',
          'css/acquia_lift.navbar.css': 'src/css/acquia_lift.navbar.scss',
          'css/acquia_lift.navbar_1-5.css': 'src/css/acquia_lift.navbar_1-5.scss'
        }
      }
    },
    watch: {
      sass: {
        files: 'src/css/**/*.scss',
        tasks: ['style']
      },
      scripts: {
        files: 'src/js/**/*.js',
        tasks: ['script']
      },
      gruntfile: {
        files: 'Gruntfile.js',
        tasks: ['default']
      }
    }
  });

  // Load the plugin that provides the "uglify" task.
  grunt.loadNpmTasks('grunt-autoprefixer');
  grunt.loadNpmTasks('grunt-concurrent');
  grunt.loadNpmTasks('grunt-contrib-concat');
  grunt.loadNpmTasks('grunt-contrib-sass');
  grunt.loadNpmTasks('grunt-contrib-watch');
  grunt.loadNpmTasks('grunt-contrib-qunit');

  // Default task(s).
  grunt.registerTask('default', ['concurrent:all']);
  grunt.registerTask('style', ['sass', 'autoprefixer']);
  grunt.registerTask('script', ['concat']);
  grunt.registerTask('test', ['qunit']);
};
