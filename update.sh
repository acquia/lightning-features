#!/usr/bin/env bash

# --------------------------------------------------------------
# Ensure you are using the latest version of Lightning Features.
# --------------------------------------------------------------
# This script can only be run from the directory:
# `/profiles/[my-profile]/modules/contrib/lightning_features`
# It overwrites existing Libraries in the directory:
# `/profiles/[my-profile]/libraries`
# And it overwrites existing contrib modules in the directory:
# `/profiles/[my-profile]/modules/contrib`
# --------------------------------------------------------------

# Usage: sh update.sh  
drush make --no-core lightning_features.make -y
cp -fR sites/all/libraries/* ../../../libraries/.
cp -fR sites/all/modules/contrib/* ../.
rm -rf sites
