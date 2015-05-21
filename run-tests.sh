#!/usr/bin/env bash

# Enables composer manager and behatrunner then runs tests.
# Requires @site-url argument.

# Usage:  sh run-tests.sh [http://lightning.local]
echo "Running tests on environment: $1"
drush vset composer_manager_autobuild_file 0
drush vset composer_manager_autobuild_packages 0
drush en composer_manager -vy
drush composer-manager install -n --prefer-source
drush vset composer_manager_autobuild_file 1
drush vset composer_manager_autobuild_packages 1
drush en behatrunner -l $1 -vy
drush cc all
drush brun -l $1 -vy
