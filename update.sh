#!/usr/bin/env bash

drush make --no-core lightning_features.make -y
cp -R sites/all/libraries/* ../../../libraries/.
cp -R sites/all/modules/contrib/* ../.
rm -rf sites
