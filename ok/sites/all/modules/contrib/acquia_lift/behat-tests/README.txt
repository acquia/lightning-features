The Acquia Lift module includes Behat tests to validate the unified toolbar administration experience.
These tests utilize the Behat Drupal Extension: http://behat-drupal-extension.readthedocs.org/

The system requirements are documented here:
  http://behat-drupal-extension.readthedocs.org/en/3.0/requirements.html

To set up your environment to run tests locally:

1.  Download the Selenium stand-alone server JAR file:
    http://selenium-release.storage.googleapis.com/2.45/selenium-server-standalone-2.45.0.jar

2.  Selenium version 2.45 is only compatible with Firefox 33, but not 34 or above. It is also recommended
    that you use a dedicated Firefox for your Selenium stand-alone server. You can download Firefox 33 at
    the following link:

    https://ftp.mozilla.org/pub/mozilla.org/firefox/releases/33.1.1/mac/en-US/

3.  Install Composer:
    https://getcomposer.org/doc/00-intro.md#system-requirements

4.  Install project dependencies into a "vendor" folder
    - Navigate to the behat-tests folder
    - Type: composer install

5.  Copy behat.template.yml and rename it to behat.yml.

6.  Update behat.yml settings to match your environment.  You will need to at least adjust base_url and
    drupal_root. Additionally you can copy over any setting from behat.common.yml and change it to match
    your configuration such as a different CSS selector for a particular region.

To run tests:

1.  Start the Selenium server
    Type: java -jar [DIRECTORY PATH]/selenium-server-standalone-2.45.0.jar -Dwebdriver.firefox.bin="[DIRECTORY PATH]/Firefox.app/Contents/MacOS/firefox-bin" &

2.  Navigate to the behat-tests directory and type:
    bin/behat
