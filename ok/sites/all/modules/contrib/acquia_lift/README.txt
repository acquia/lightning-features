*******************************************************************************

Acquia Lift


Description:
-------------------------------------------------------------------------------

Acquia Lift is both a suite of modules and a third-party service which powers
rich personalization, testing and targeting services on Drupal websites. This
module provides the interaction, and has dependencies on all other modules
necessary to use the Acquia Lift service.


Installation & Use:
-------------------------------------------------------------------------------
Please contact Acquia to gain access to the Acquia Lift service. Portions of
this module will not work fully without a valid API key.

1. Download and extract the following modules into separate directories in your
   docroot/sites/all/modules/ directory:
    - http://drupal.org/project/acquia_lift
    - http://drupal.org/project/libraries
    - http://drupal.org/project/personalize
    - http://drupal.org/project/visitor_actions

2. Download and extract the following JavaScript libraries into separate
   directories in the docroot/sites/all/libraries directory:
    - backbone.js - /backbone directory (IMPORTANT: Minimum version 1.5)
    - chosen.js (and associated files) - /chosen directory
    - sparkline.js - /sparkline directory
    - underscore.js - /underscore directory (IMPORTANT: Minimum version 1.0)

3. (Optional) If you want to use the Mobile Friendly Navigation Toolbar module
   with your website, which we recommend for ease of use, download and extract
   the Mobile Friendly Navigation Toolbar module from drupal.org into your
   docroot/sites/all/modules directory. (http://drupal.org/project/navbar)

4. (Optional) If you install the Mobile Friendly Navigation Toolbar module,
   download and extract the modernizr.js JavaScript library into the
   docroot/sites/all/libraries/modernizr directory.

NOTE
If you create a custom modernizr.js script (for example,
modernizr.custom.22529.js), be sure to rename the file to modernizr.js for
your website.

5. Sign in to your website as an administrator, and then click Modules in the
   admin menu to view the Modules page. Select the Acquia Lift check box.

6. (Optional) If you want to use the Mobile Friendly Navigation Toolbar with
   your website, select the Mobile Friendly Navigation Toolbar check box and
   clear the Toolbar check box. The Toolbar module is not compatible with the
   Mobile Friendly Navigation Toolbar module.

7. Click Save configuration to save your module settings.

Your website now contains the modules and files required to use Acquia Lift,
and you can connect the modules to the Acquia Lift service.

For configuration instructions, best practices and more, please visit:
http://docs.acquia.com/lift

Current maintainers:
-------------------------------------------------------------------------------
 * Katherine Bailey (katbailey) - http://drupal.org/user/172987
 * Dave Ingram (Dave.Ingram) - http://drupal.org/user/352282