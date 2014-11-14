api = 2
core = 7.x

projects[better_formats][version] = "1.0-beta1"
projects[better_formats][type] = "module"
projects[better_formats][subdir] = "contrib"

projects[bean][version] = "1.x-dev"
projects[bean][type] = "module"
projects[bean][subdir] = "contrib"
projects[bean][download][type] = "git"
projects[bean][download][revision] = "2d0f262"
projects[bean][download][branch] = "7.x-1.x"

projects[bean_tax][version] = "2.3"
projects[bean_tax][type] = "module"
projects[bean_tax][subdir] = "contrib"

projects[behatrunner][version] = "1.x-dev"
projects[behatrunner][type] = "module"
projects[behatrunner][subdir] = "contrib"
projects[behatrunner][download][type] = "git"
projects[behatrunner][download][revision] = "3d74d9b"
projects[behatrunner][download][branch] = "7.x"
; Remove PHP memory_limit ini_set on hook_install as it causes failures
; http://drupal.org/node/2360825
projects[behatrunner][patch][2360825] = "http://drupal.org/files/issues/behatrunner-memory_limit-0.patch"

projects[breakpoints][version] = "1.3"
projects[breakpoints][type] = "module"
projects[breakpoints][subdir] = "contrib"

projects[ckeditor][version] = "1.x-dev"
projects[ckeditor][type] = "module"
projects[ckeditor][subdir] = "contrib"
projects[ckeditor][download][type] = "git"
projects[ckeditor][download][revision] = "875a4b1"
projects[ckeditor][download][branch] = "7.x-1.x"
; CKEditor accomodate latest Media changes
; http://drupal.org/node/2159403
projects[ckeditor][patch][2159403] = "http://drupal.org/files/issues/make_ckeditor_plugin-2159403-107.patch"

projects[collections][version] = "1.x-dev"
projects[collections][type] = "module"
projects[collections][subdir] = "contrib"
projects[collections][download][type] = "git"
projects[collections][download][revision] = "b4e8212"
projects[collections][download][branch] = "7.x-1.x"

projects[colorbox][version] = "2.x-dev"
projects[colorbox][type] = "module"
projects[colorbox][subdir] = "contrib"
projects[colorbox][download][type] = "git"
projects[colorbox][download][revision] = "ce90f5d"
projects[colorbox][download][branch] = "7.x-1.x"

projects[composer_manager][version] = "1.7"
projects[composer_manager][type] = "module"
projects[composer_manager][subdir] = "contrib"

projects[context_admin][version] = "1.x-dev"
projects[context_admin][type] = "module"
projects[context_admin][subdir] = "contrib"
projects[context_admin][download][type] = "git"
projects[context_admin][download][revision] = "15a8390"
projects[context_admin][download][branch] = "7.x-1.x"

projects[ctools][version] = "1.x-dev"
projects[ctools][type] = "module"
projects[ctools][subdir] = "contrib"
projects[ctools][download][type] = "git"
projects[ctools][download][revision] = "ae66d65"
projects[ctools][download][branch] = "7.x-1.x"

projects[date][version] = "2.8"
projects[date][type] = "module"
projects[date][subdir] = "contrib"

projects[demonstratie_panels][version] = "1.x-dev"
projects[demonstratie_panels][type] = "module"
projects[demonstratie_panels][subdir] = "contrib"

projects[defaultconfig][version] = "1.x-dev"
projects[defaultconfig][subdir] = "module"
projects[defaultconfig][subdir] = "contrib"
projects[defaultconfig][download][type] = "git"
projects[defaultconfig][download][revision] = "a62d216"
projects[defaultconfig][download][branch] = "7.x-1.x"
; Fatal error when adding a permission that doesn't exist
; http://drupal.org/node/2008178
projects[defaultconfig][patch][2008178] = "http://drupal.org/files/issues/defaultconfig-rebuild-2008178-9.patch"

projects[devel][version] = "1.3"
projects[devel][type] = "module"
projects[devel][subdir] = "contrib"

projects[diff][version] = "3.2"
projects[diff][type] = "module"
projects[diff][subdir] = "contrib"

projects[ember_support][version] = "1.x-dev"
projects[ember_support][type] = "module"
projects[ember_support][subdir] = "contrib"
projects[ember_support][download][type] = "git"
projects[ember_support][download][branch] = "7.x-1.x"

projects[entity][version] = "1.x-dev"
projects[entity][type] = "module"
projects[entity][subdir] = "contrib"
projects[entity][download][type] = "git"
projects[entity][download][revision] = "4d2cc6f"
projects[entity][download][branch] = "7.x-1.x"

projects[entityreference][version] = "1.x-dev"
projects[entityreference][type] = "module"
projects[entityreference][subdir] = "contrib"
projects[entityreference][download][type] = "git"
projects[entityreference][download][revision] = "dc4196b"
projects[entityreference][download][branch] = "7.x-1.x"

projects[entityreference_prepopulate][version] = "1.5"
projects[entityreference_prepopulate][type] = "module"
projects[entityreference_prepopulate][subdir] = "contrib"

projects[escape_admin][version] = "1.x-dev"
projects[escape_admin][type] = "module"
projects[escape_admin][subdir] = "contrib"
projects[escape_admin][download][type] = "git"
projects[escape_admin][download][revision] = "ecd3f58"
projects[escape_admin][download][branch] = "7.x-1.x"

projects[fape][version] = "1.2"
projects[fape][type] = "module"
projects[fape][subdir] = "contrib"

projects[features][version] = "2.x-dev"
projects[features][type] = "module"
projects[features][subdir] = "contrib"
projects[features][download][type] = "git"
projects[features][download][revision] = "9f4ecc7"
projects[features][download][branch] = "7.x-2.x"

projects[field_group][version] = "1.x-dev"
projects[field_group][type] = "module"
projects[field_group][subdir] = "contrib"
projects[field_group][download][type] = "git"
projects[field_group][download][revision] = "9cdde2b"
projects[field_group][download][branch] = "7.x-1.x"

projects[fieldable_panels_panes][version] = "1.x-dev"
projects[fieldable_panels_panes][type] = "module"
projects[fieldable_panels_panes][subdir] = "contrib"
projects[fieldable_panels_panes][download][type] = "git"
projects[fieldable_panels_panes][download][revision] = "bfef4bc"
projects[fieldable_panels_panes][download][branch] = "7.x-1.x"

projects[file_entity][version] = "2.x-dev"
projects[file_entity][type] = "module"
projects[file_entity][subdir] = "contrib"
projects[file_entity][download][type] = "git"
projects[file_entity][download][revision] = "609fa9f"
projects[file_entity][download][branch] = "7.x-2.x"

projects[file_entity_link][version] = "1.0-alpha3"
projects[file_entity_link][type] = "module"
projects[file_entity_link][subdir] = "contrib"

projects[file_image_formatters][version] = "1.1"
projects[file_image_formatters][type] = "module"
projects[file_image_formatters][subdir] = "contrib"

projects[focal_point][version] = "1.0-beta1"
projects[focal_point][type] = "module"
projects[focal_point][subdir] = "contrib"

projects[form_builder][version] = "1.x-dev"
projects[form_builder][type] = "module"
projects[form_builder][subdir] = "contrib"
projects[form_builder][download][type] = "git"
projects[form_builder][download][revision] = "3d904df"
projects[form_builder][download][branch] = "7.x-1.x"
; The form editing part rolls to hide, can not be edited
; http://drupal.org/node/1987332
projects[form_builder][patch][1987332] = "http://drupal.org/files/rolls-past-editing-form-1987332-4.patch"

projects[iib][version] = "1.x-dev"
projects[iib][type] = "module"
projects[iib][subdir] = "contrib"
projects[iib][download][type] = "git"
projects[iib][download][revision] = "17a55eb"
projects[iib][download][branch] = "7.x-1.x"
; Integrate IIB with the Navbar module and improve Toolbar integration
; http://drupal.org/node/1737036
projects[iib][patch][1737036] = "http://drupal.org/files/issues/iib-navbar-toolbar-1737036-51.patch"

projects[jquery_update][version] = "2.4"
projects[jquery_update][type] = "module"
projects[jquery_update][subdir] = "contrib"

projects[libraries][version] = "2.2"
projects[libraries][type] = "module"
projects[libraries][subdir] = "contrib"

projects[link][version] = "1.2"
projects[link][type] = "module"
projects[link][subdir] = "contrib"

projects[linkit][version] = "3.1"
projects[linkit][type] = "module"
projects[linkit][subdir] = "contrib"

projects[magic_beans][version] = "1.x-dev"
projects[magic_beans][type] = "module"
projects[magic_beans][subdir] = "contrib"
projects[magic_beans][download][type] = "git"
projects[magic_beans][download][revision] = "6c5d19e"
projects[magic_beans][download][branch] = "7.x-1.x"

projects[media][version] = "2.x-dev"
projects[media][type] = "module"
projects[media][subdir] = "contrib"
projects[media][download][type] = "git"
projects[media][download][revision] = "f5db1d1"
projects[media][download][branch] = "7.x-2.x"
; Improve UX for Media Thumbnail and Media Bulk Upload's multiform page 
; http://drupal.org/node/2166623
projects[media][patch][2166623] = "http://drupal.org/files/issues/media_bulk_upload-improve-multiform-2166623-2.patch"
; Media WYSIWYG broken Quickedit module compatibility
; http://drupal.org/node/2331293
projects[media][patch][2331293] = "http://drupal.org/files/issues/media_wysiwyg_quickedit-2331293-8.patch"

projects[media_youtube][version] = "2.x-dev"
projects[media_youtube][type] = "module"
projects[media_youtube][subdir] = "contrib"
projects[media_youtube][download][type] = "git"
projects[media_youtube][download][revision] = "187283f"
projects[media_youtube][download][branch] = "7.x-2.x"

projects[media_preview_slider][version] = "1.x-dev"
projects[media_preview_slider][type] = "module"
projects[media_preview_slider][subdir] = "contrib"
projects[media_preview_slider][download][type] = "git"
projects[media_preview_slider][download][branch] = "7.x-1.x"
projects[media_preview_slider][download][url] = "http://git.drupal.org/sandbox/Brian14/2222597.git"

projects[module_filter][version] = "2.0-alpha2"
projects[module_filter][type] = "module"
projects[module_filter][subdir] = "contrib"

projects[multiform][version] = "1.1"
projects[multiform][type] = "module"
projects[multiform][subdir] = "contrib"

projects[metatag][version] = "1.0-rc2"
projects[metatag][type] = "module"
projects[metatag][subdir] = "contrib"

projects[navbar][version] = "1.x-dev"
projects[navbar][type] = "module"
projects[navbar][subdir] = "contrib"
projects[navbar][download][type] = "git"
projects[navbar][download][revision] = "bd3389b"
projects[navbar][download][branch] = "7.x-1.x"
; Menu icons for contrib modules
; http://drupal.org/node/1954912
projects[navbar][patch][1954912] = "http://drupal.org/files/issues/navbar-contrib-icons-1954912-20.patch"
; JSON Error caused in Views when navbar.tableheader is not loaded
; http://drupal.org/node/2263205
projects[navbar][patch][2263205] = "http://drupal.org/files/issues/navbar-tableheader-views.patch"

projects[nra][version] = "1.0-alpha2"
projects[nra][type] = "module"
projects[nra][subdir] = "contrib"

projects[nra_workbench_moderation][version] = "1.x-dev"
projects[nra_workbench_moderation][type] = "module"
projects[nra_workbench_moderation][subdir] = "contrib"
projects[nra_workbench_moderation][download][type] = "git"
projects[nra_workbench_moderation][download][revision] = "9f17009"
projects[nra_workbench_moderation][download][branch] = "7.x-1.x"
; Errors when 'Status' column is built for new/unpublished items in NRA
; http://drupal.org/node/2163175
projects[nra_workbench_moderation][patch][2163175] = "http://drupal.org/files/issues/nra_workbench_moderation-no-published-state-2163175-1.patch"

projects[options_element][version] = "1.x-dev"
projects[options_element][type] = "module"
projects[options_element][subdir] = "contrib"
projects[options_element][download][type] = "git"
projects[options_element][download][revision] = "33fa8a7"
projects[options_element][download][branch] = "git"

projects[pathauto][version] = "1.2"
projects[pathauto][type] = "module"
projects[pathauto][subdir] = "contrib"

projects[panelizer][version] = "3.x-dev"
projects[panelizer][subdir] = "contrib"
projects[panelizer][download][type] = "git"
projects[panelizer][download][revision] = "c8fb90b"
projects[panelizer][download][branch] = "7.x-3.x"

projects[panels][version] = "3.x-dev"
projects[panels][type] = "module"
projects[panels][subdir] = "contrib"
projects[panels][download][type] = "git"
projects[panels][download][revision] = "bcda4a6"
projects[panels][download][branch] = "7.x-3.x"

projects[panopoly_magic][version] = "1.x-dev"
projects[panopoly_magic][type] = "module"
projects[panopoly_magic][subdir] = "contrib"
projects[panopoly_magic][download][type] = "git"
projects[panopoly_magic][download][revision] = "1135fea"
projects[panopoly_magic][download][branch] = "7.x-1.x"

projects[panopoly_theme][version] = "1.x-dev"
projects[panopoly_theme][type] = "module"
projects[panopoly_theme][subdir] = "contrib"
projects[panopoly_theme][download][type] = "git"
projects[panopoly_theme][download][revision] = "d409deb"
projects[panopoly_theme][download][branch] = "7.x-1.x"
; Remove makefile from Panopoly Theme
; http://drupal.org/node/1904766
projects[panopoly_theme][patch][1904766] = "http://drupal.org/files/issues/panopoly_theme-makefile-free-1904766-13.patch"

projects[picture][version] = "1.x-dev"
projects[picture][type] = "module"
projects[picture][subdir] = "contrib"
projects[picture][download][type] = "git"
projects[picture][download][revision] = "3d9fe6c"
projects[picture][download][branch] = "7.x-1.x"

projects[plupload][version] = "1.6"
projects[plupload][type] = "module"
projects[plupload][subdir] = "contrib"

projects[quickedit][version] = "1.x-dev"
projects[quickedit][type] = "module"
projects[quickedit][subdir] = "contrib"
projects[quickedit][download][type] = "git"
projects[quickedit][download][revision] = "28a314d"
projects[quickedit][download][branch] = "7.x-1.x"

projects[quickedit_tab][version] = "1.x-dev"
projects[quickedit_tab][type] = "module"
projects[quickedit_tab][subdir] = "contrib"

projects[responsive_preview][version] = "1.x-dev"
projects[responsive_preview][type] = "module"
projects[responsive_preview][subdir] = "contrib"
projects[responsive_preview][download][type] = "git"
projects[responsive_preview][download][revision] = "d741779"
projects[responsive_preview][download][branch] = "7.x-1.x"
; Before js processing, the phone image incorrectly positioned.
; https://drupal.org/node/2276789
projects[responsive_preview][patch][2276789] = "http://drupal.org/files/issues/responsive_preview-phone_image_incorrectly_positioned-2276789-2.patch"

projects[revision_scheduler][version] = "1.x-dev"
projects[revision_scheduler][type] = "module"
projects[revision_scheduler][subdir] = "contrib"
projects[revision_scheduler][download][type] = "git"
projects[revision_scheduler][download][revision] = "bb9fd39"
projects[revision_scheduler][download][branch] = "7.x-1.x"

projects[role_export][version] = "1.0"
projects[role_export][type] = "module"
projects[role_export][subdir] = "contrib"

projects[rules][version] = "2.7"
projects[rules][type] = "module"
projects[rules][subdir] = "contrib"

projects[simple_gmap][version] = "1.2"
projects[simple_gmap][type] = "module"
projects[simple_gmap][subdir] = "contrib"

projects[sps][version] = "1.x-dev"
projects[sps][type] = "module"
projects[sps][subdir] = "contrib"
projects[sps][download][type] = "git"
projects[sps][download][revision] = "be9bd83"
projects[sps][download][branch] = "7.x-1.x"
; UX improvements on page level IIB
; http://drupal.org/node/1733490
projects[sps][patch][1733490] = "http://drupal.org/files/sps-css-cleanup-1733490-3.patch"
; sps_entity_create() throws errors
; http://drupal.org/node/2288873
projects[sps][patch][2288873] = "http://drupal.org/files/issues/sps-undefiend-method-create-3.patch"

projects[strongarm][version] = "2.0"
projects[strongarm][type] = "module"
projects[strongarm][subdir] = "contrib"

projects[token][version] = "1.5"
projects[token][type] = "module"
projects[token][subdir] = "contrib"

projects[taxonomy_entity_index][version] = "1.0-beta7"
projects[taxonomy_entity_index][type] = "module"
projects[taxonomy_entity_index][subdir] = "contrib"

projects[ux_elements][version] = "1.x-dev"
projects[ux_elements][type] = "module"
projects[ux_elements][subdir] = "contrib"
projects[ux_elements][download][type] = "git"
projects[ux_elements][download][revision] = "87cdc5d"
projects[ux_elements][download][branch] = "master"
; PHP Fatal error: Cannot redeclare form_process_horizontal_tabs()
; http://drupal.org/node/1224568
projects[ux_elements][patch][1224568] = "http://drupal.org/files/issues/1224568-ux_elements_redeclare.patch"

projects[views][version] = "3.x-dev"
projects[views][type] = "module"
projects[views][subdir] = "contrib"
projects[views][download][type] = "git"
projects[views][download][revision] = "82634af"
projects[views][download][branch] = "7.x-3.x"

projects[views_autocomplete_filters][version] = "1.1"
projects[views_autocomplete_filters][type] = "module"
projects[views_autocomplete_filters][subdir] = "contrib"

projects[views_field_view][version] = "1.x-dev"
projects[views_field_view][type] = "module"
projects[views_field_view][subdir] = "contrib"
projects[views_field_view][download][type] = "git"
projects[views_field_view][download][revision] = "bb6cfea"
projects[views_field_view][download][branch] = "7.x-1.x"

projects[views_bulk_operations][version] = "3.x-dev"
projects[views_bulk_operations][type] = "module"
projects[views_bulk_operations][subdir] = "contrib"
projects[views_bulk_operations][download][type] = "git"
projects[views_bulk_operations][download][revision] = "3e27b0b"
projects[views_bulk_operations][download][branch] = "7.x-3.x"

projects[views_load_more][version] = "1.5"
projects[views_load_more][type] = "module"
projects[views_load_more][subdir] = "contrib"

projects[webform][version] = "4.1"
projects[webform][type] = "module"
projects[webform][subdir] = "contrib"

projects[workbench][version] = "1.x-dev"
projects[workbench][type] = "module"
projects[workbench][subdir] = "contrib"
projects[workbench][download][type] = "git"
projects[workbench][download][revision] = "6856e4a"
projects[workbench][download][branch] = "7.x-1.x"

projects[workbench_moderation][version] = "1.x-dev"
projects[workbench_moderation][type] = "module"
projects[workbench_moderation][subdir] = "contrib"
projects[workbench_moderation][download][type] = "git"
projects[workbench_moderation][download][revision] = "b38ac3e"
projects[workbench_moderation][download][branch] = "7.x-1.x"
; Panelizer is incompatible with moderation
; http://www.drupal.org/node/1402860
projects[workbench_moderation][patch][1402860] = "http://drupal.org/files/issues/workbench_moderation-panelizer_revisions-1402860-59.patch"

projects[workbench_moderation_buttons][version] = "1.x-dev"
projects[workbench_moderation_buttons][type] = "module"
projects[workbench_moderation_buttons][subdir] = "contrib"
projects[workbench_moderation_buttons][download][type] = "git"
projects[workbench_moderation_buttons][download][revision] = "4a488f7"
projects[workbench_moderation_buttons][download][branch] = "7.x-1.x"
; PHP Parse error: syntax error, unexpected '$key'
; http://drupal.org/node/2353111
projects[workbench_moderation_buttons][patch][2353111] = "http://drupal.org/files/issues/workbench_moderation_buttons-PHP_parse-2353111-1.patch"
; Form always thinks current state is "draft"
; http://drupal.org/node/2365863
projects[workbench_moderation_buttons][patch][2365863] = "http://drupal.org/files/issues/workbench_moderation_buttons-default_state-2365863-1.patch"

projects[workbench_moderation_notes][version] = "1.x-dev"
projects[workbench_moderation_notes][type] = "module"
projects[workbench_moderation_notes][subdir] = "contrib"
projects[workbench_moderation_notes][download][type] = "git"
projects[workbench_moderation_notes][download][revision] = "8e5e6f4"
projects[workbench_moderation_notes][download][branch] = "7.x-1.x"
; Incorrect status message "...no published revision of this site"
; http://drupal.org/node/2045407
projects[workbench_moderation_notes][patch][] = "http://drupal.org/files/workbench_moderation_notes-no_live_node_alert-2045407-1.patch"

projects[xautoload][version] = "4.5"
projects[xautoload][type] = "module"
projects[xautoload][subdir] = "contrib"

; Libraries
libraries[backbone][download][type] = "get"
libraries[backbone][download][url] = "https://github.com/jashkenas/backbone/archive/1.1.0.zip"

libraries[ckeditor][download][type] = "get"
libraries[ckeditor][download][url] = "http://download.cksource.com/CKEditor%20for%20Drupal/edit/ckeditor_4.4.3_edit.zip"

libraries[colorbox][download][type] = "get"
libraries[colorbox][download][url] = "https://github.com/jackmoore/colorbox/archive/1.x.zip"

libraries[jsonpath][download][type] = "get"
libraries[jsonpath][download][url] = "https://jsonpath.googlecode.com/files/jsonpath-0.8.1.php"

libraries[modernizr][download][type] = "get"
libraries[modernizr][download][url] = "https://github.com/Modernizr/Modernizr/archive/v2.7.1.zip"

libraries[plupload][download][type] = "get"
libraries[plupload][download][url] = "https://github.com/moxiecode/plupload/archive/v1.5.8.zip"
; Remove plupload library examples folder for Drupal distribution
; http://drupal.org/node/1903850
libraries[plupload][patch][1903850] = "http://drupal.org/files/issues/plupload-1_5_8-rm_examples-1903850-16.patch"

libraries[underscore][download][type] = "get"
libraries[underscore][download][url] = "https://github.com/jashkenas/underscore/archive/1.5.2.zip"
