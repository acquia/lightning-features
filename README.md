lightning-features
==================
[![Build Status](https://travis-ci.org/acquia/lightning-features.svg?branch=7.x-1.x)](https://travis-ci.org/acquia/lightning-features)

Decoupled components used by the Lightning distribution for Drupal.

Using the lightning_features module in your own install profile's is recommended.

FAQ

- If you are not using a module and no lightning_features modules inuse require it as a dependency, it could be deleted from your project with a patch for lightning_features.make
- You will not see any performance decrease on your delivery if you leave uninstalled modules in your profiles directory.
- We recommend evaluating each lightning_* module for your own usage and only using the things you deem necessary.

See lightning_features.make for a detailed list of dependent modules and libraries.

### Updating
If you downloaded the Lightning Demo from http://drupal.org/project/lightning

`cd profiles/lightning/modules/contrib/lightning_features`

`sh update.sh`

If you built your own profile using Lightning Features: you must keep lightning_features and the contrib it provides in profiles/[my-profile]/modules/contrib otherwise the update.sh script will not run correctly.

`cd profiles/[my-profile]/modules/contrib/lightning_features`

`sh update.sh`

### Running tests

Move the tests folder to your Drupal docroot.

  ``mv lightning_features/tests docroot/tests``

Install the drupal-extension for mink/behat from the tests folder.

  ``cd tests && composer install``

Set up the behat.local.yml file replacing ``BASE_URL`` with your own url.

  ``cp behat.local.example.yml behat.local.yml``

Check that behat is installed and running under the dev profile.

  ``bin/behat --help --profile=dev``

Execute the Lightning Features scenarios using the "dev" Behat profile.

  ``bin/behat --tags=lightning --profile=dev``

#### Behat tests tagged with @lightning

##### article.feature

* Article content type exists
* Creating an article as admin
* Article content type has fields provided by feature

##### breakpoints.feature

* Checks to ensure breakpoints were created

##### cache.feature

* Checks to ensure Views Content Cache is enabled

##### filter.feature

* Text formats exist
* Text formats have appropriate feature-defined filters

##### forms.feature

* Webform type present
* Can create Webform
* Check basic Webform fields

##### fpp.feature

* Checks for FPP types
* Checks for fields on FPP types

##### identifiers.feature

* Checks to ensure taxonomy exists

##### landing.feature

* Ensure landing page content type exists
* Create landing page content
* Landing Page fields

##### letterbox.feature

* Checks for letterbox breakpoints

##### moderation.feature

* Ensure moderation steps are created

##### page.feature

* Ensure page content type exists
* Create page content
* Page fields

##### roles.feature

* Role exists: curator
* Role exists: reviewer
* Role exists: marketer

##### sps.feature

* Checks for Article field collection

##### tags.feature
* Tags taxonomy vocabulary exists

##### wysiwyg.feature

* Check Linkit exists
* Linkit sanity check
