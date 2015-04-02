api = 2
core = 7.x

projects[acquia_lift][version] = "1.0-rc11"
projects[acquia_lift][type] = "module"
projects[acquia_lift][subdir] = "contrib"
; Lift forces max width on .ctools-modal-content but not .modal-content
; http://drupal.org/node/2406855
projects[acquia_lift][patch][2406855] = "http://drupal.org/files/issues/acquia-lift-ctools-modal-content-css-0.patch"

projects[personalize][version] = "1.0-rc9"
projects[personalize][type] = "module"
projects[personalize][subdir] = "contrib"
; Personalized Fields break preview inside Panels
; http://drupal.org/node/2303111
projects[personalize][patch][2303111] = "http://drupal.org/files/issues/personalize-panels-fix-2303111-4.patch"

projects[visitor_actions][version] = "1.0-rc4"
projects[visitor_actions][type] = "module"
projects[visitor_actions][subdir] = "contrib"

; Libraries
libraries[d3][destination] = "libraries"
libraries[d3][download][type] = "get"
libraries[d3][download][url] = https://github.com/mbostock/d3/releases/download/v3.4.11/d3.zip

libraries[qtip][destination] = "libraries"
libraries[qtip][download][type] = "get"
libraries[qtip][download][url] = "https://raw.githubusercontent.com/Craga89/qTip1/master/1.0.0-rc3/jquery.qtip-1.0.0-rc3.min.js"

libraries[rickshaw][destination] = "libraries"
libraries[rickshaw][download][type] = "get"
libraries[rickshaw][download][url] = https://github.com/shutterstock/rickshaw/archive/v1.5.0.zip
