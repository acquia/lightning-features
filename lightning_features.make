api = 2
core = 7.x

projects[better_formats][version] = "1.0-beta1"
projects[better_formats][type] = "module"
projects[better_formats][subdir] = "contrib"

projects[bean][version] = "1.9"
projects[bean][type] = "module"
projects[bean][subdir] = "contrib"
; Bean insists we use a tagged release of Entity API
; http://drupal.org/node/2465267
projects[bean][patch][2465267] = "http://drupal.org/files/issues/bean-entity-api-version-0.patch"
; Bean's panelizer integration does not correctly use views_data_alter
; http://drupal.org/node/2474631
projects[bean][patch][2474631] = "http://drupal.org/files/issues/2474631-10.patch"

projects[bean_tax][version] = "2.6"
projects[bean_tax][type] = "module"
projects[bean_tax][subdir] = "contrib"

projects[behatrunner][version] = "1.x-dev"
projects[behatrunner][type] = "module"
projects[behatrunner][subdir] = "contrib"
projects[behatrunner][download][type] = "git"
projects[behatrunner][download][revision] = "0714764"
projects[behatrunner][download][branch] = "7.x"

projects[block_class][version] = "2.x-dev"
projects[block_class][type] = "module"
projects[block_class][subdir] = "contrib"
projects[block_class][download][type] = "git"
projects[block_class][download][revision] = "f062706"
projects[block_class][download][branch] = "7.x-2.x"

projects[breakpoints][version] = "1.3"
projects[breakpoints][type] = "module"
projects[breakpoints][subdir] = "contrib"

projects[cache_warmer][version] = "4.1"
projects[cache_warmer][type] = "module"
projects[cache_warmer][subdir] = "contrib"

projects[ckeditor][version] = "1.x-dev"
projects[ckeditor][type] = "module"
projects[ckeditor][subdir] = "contrib"
projects[ckeditor][download][type] = "git"
projects[ckeditor][download][revision] = "24e805a"
projects[ckeditor][download][branch] = "7.x-1.x"

projects[classy_panel_styles][version] = "1.0-alpha1"
projects[classy_panel_styles][type] = "module"
projects[classy_panel_styles][subdir] = "contrib"

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
projects[colorbox][download][revision] = "97a4ff1"
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

projects[ctools][version] = "1.7"
projects[ctools][type] = "module"
projects[ctools][subdir] = "contrib"
; When Ctools modal is open, disable the page scroll behind the modal
; http://drupal.org/node/2475417
projects[ctools][patch][2475417] = "http://drupal.org/files/issues/fix-background-scroll-2475417-5.patch"

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
projects[defaultconfig][patch][2008178] = "http://drupal.org/files/issues/defaultconfig-rebuild-2008178-14.patch"

projects[devel][version] = "1.3"
projects[devel][type] = "module"
projects[devel][subdir] = "contrib"

projects[diff][version] = "3.2"
projects[diff][type] = "module"
projects[diff][subdir] = "contrib"

projects[draggableviews][version] = "2.x-dev"
projects[draggableviews][type] = "module"
projects[draggableviews][subdir] = "contrib"
projects[draggableviews][download][type] = "git"
projects[draggableviews][download][revision] = "4f5ac4c"
projects[draggableviews][download][branch] = "7.x-2.x"

projects[ember_support][version] = "1.x-dev"
projects[ember_support][type] = "module"
projects[ember_support][subdir] = "contrib"
projects[ember_support][download][type] = "git"
projects[ember_support][download][branch] = "7.x-1.x"

projects[entity][version] = "1.x-dev"
projects[entity][type] = "module"
projects[entity][subdir] = "contrib"
projects[entity][download][type] = "git"
projects[entity][download][revision] = "4d76c5c"
projects[entity][download][branch] = "7.x-1.x"

projects[entity_embed][version] = "1.x-dev"
projects[entity_embed][type] = "module"
projects[entity_embed][subdir] = "contrib"
projects[entity_embed][download][type] = "git"
projects[entity_embed][download][revision] = "f930e28"
projects[entity_embed][download][branch] = "7.x-1.x"

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

projects[features][version] = "2.5"
projects[features][type] = "module"
projects[features][subdir] = "contrib"

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
projects[fieldable_panels_panes][download][revision] = "f513a15"
projects[fieldable_panels_panes][download][branch] = "7.x-1.x"
; Allow a particular revision of a fieldable panels pane to be placed in a panelized page
; http://drupal.org/node/1986334
projects[fieldable_panels_panes][patch][1986334] = "http://drupal.org/files/issues/1986334-10.patch"
; Fix bug where clicking "Save and add fields" would redirect to /fields
; https://www.drupal.org/node/2477421
projects[fieldable_panels_panes][patch][2477421] = "https://www.drupal.org/files/issues/fieldable_panels_panes-save-continue-redirect-2477421-1.patch"

projects[file_entity][version] = "2.x-dev"
projects[file_entity][type] = "module"
projects[file_entity][subdir] = "contrib"
projects[file_entity][download][type] = "git"
projects[file_entity][download][revision] = "54a45a3"
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

projects[form_builder][version] = "1.7"
projects[form_builder][type] = "module"
projects[form_builder][subdir] = "contrib"

projects[httprl][version] = "1.x-dev"
projects[httprl][type] = "module"
projects[httprl][subdir] = "contrib"
projects[httprl][download][type] = "git"
projects[httprl][download][revision] = "78bc6ad"
projects[httprl][download][branch] = "7.x-1.x"

projects[iib][version] = "1.x-dev"
projects[iib][type] = "module"
projects[iib][subdir] = "contrib"
projects[iib][download][type] = "git"
projects[iib][download][revision] = "513fa9d"
projects[iib][download][branch] = "7.x-1.x"

projects[imageinfo_cache][version] = "3.5"
projects[imageinfo_cache][type] = "module"
projects[imageinfo_cache][subdir] = "contrib"

projects[import][version] = "1.x-dev"
projects[import][type] = "module"
projects[import][subdir] = "contrib"
projects[import][download][type] = "git"
projects[import][download][branch] = "7.x-1.x"

projects[inline_entity_form][version] = "1.x-dev"
projects[inline_entity_form][type] = "module"
projects[inline_entity_form][subdir] = "contrib"
projects[inline_entity_form][download][type] = "git"
projects[inline_entity_form][download][revision] = "47ffa63"
projects[inline_entity_form][download][branch] = "7.x-1.x"

projects[jquery_update][version] = "2.5"
projects[jquery_update][type] = "module"
projects[jquery_update][subdir] = "contrib"

projects[libraries][version] = "2.2"
projects[libraries][type] = "module"
projects[libraries][subdir] = "contrib"

projects[link][version] = "1.3"
projects[link][type] = "module"
projects[link][subdir] = "contrib"

projects[linkit][version] = "3.3"
projects[linkit][type] = "module"
projects[linkit][subdir] = "contrib"

projects[magic_beans][version] = "1.x-dev"
projects[magic_beans][type] = "module"
projects[magic_beans][subdir] = "contrib"
projects[magic_beans][download][type] = "git"
projects[magic_beans][download][revision] = "9113b89"
projects[magic_beans][download][branch] = "7.x-1.x"

projects[media][version] = "2.x-dev"
projects[media][type] = "module"
projects[media][subdir] = "contrib"
projects[media][download][type] = "git"
projects[media][download][revision] = "a2b8ae2"
projects[media][download][branch] = "7.x-2.x"

projects[media_oembed][version] = "2.3"
projects[media_oembed][type] = "module"
projects[media_oembed][subdir] = "contrib"

projects[media_youtube][version] = "2.0-rc5"
projects[media_youtube][type] = "module"
projects[media_youtube][subdir] = "contrib"

projects[media_preview_slider][version] = "1.x-dev"
projects[media_preview_slider][type] = "module"
projects[media_preview_slider][subdir] = "contrib"
projects[media_preview_slider][download][type] = "git"
projects[media_preview_slider][download][branch] = "7.x-1.x"
projects[media_preview_slider][download][url] = "http://git.drupal.org/sandbox/Brian14/2222597.git"

projects[metatag][version] = "1.4"
projects[metatag][type] = "module"
projects[metatag][subdir] = "contrib"

projects[module_filter][version] = "2.0"
projects[module_filter][type] = "module"
projects[module_filter][subdir] = "contrib"

projects[migrate][version] = "2.7"
projects[migrate][type] = "module"
projects[migrate][subdir] = "contrib"

projects[migrate_extras][version] = "2.5"
projects[migrate_extras][type] = "module"
projects[migrate_extras][subdir] = "contrib"
; Add support for UUID module for 7.x
; http://drupal.org/node/1870886
projects[migrate_extras][patch][1870886] = "http://drupal.org/files/migrate_extras-2.5-add_support_for_uuid-1870886-4-do-not-test.patch"

projects[multiform][version] = "1.1"
projects[multiform][type] = "module"
projects[multiform][subdir] = "contrib"

projects[navbar][version] = "1.x-dev"
projects[navbar][type] = "module"
projects[navbar][subdir] = "contrib"
projects[navbar][download][type] = "git"
projects[navbar][download][revision] = "71972ec"
projects[navbar][download][branch] = "7.x-1.x"
; Menu icons for contrib modules
; http://drupal.org/node/1954912
projects[navbar][patch][1954912] = "http://drupal.org/files/issues/navbar-contrib-icons-1954912-20.patch"

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
projects[panelizer][download][revision] = "b696d13"
projects[panelizer][download][branch] = "7.x-3.x"
; Better Revision Handling inside and outside of Workbench Moderation
; http://drupal.org/node/2457113
projects[panelizer][patch][2457113] = "http://drupal.org/files/issues/2457113-26.patch"

projects[panels][version] = "3.5"
projects[panels][type] = "module"
projects[panels][subdir] = "contrib"
; Fix IPE JS alert (Panelizer is Incompatible with Moderation)
; http://drupal.org/node/1402860#comment-9729091
projects[panels][patch][1402860] = "http://drupal.org/files/issues/panelizer_is-1402860-82-fix-ipe-end-js-alert.patch"
; IPE Insufficient for working with Panelizer Revisioning
; https://www.drupal.org/node/2462331#comment-9778921
projects[panels][patch][2462331] = "http://www.drupal.org/files/issues/2462331-7.patch"
; Allow modules to alter IPE buttons w/o hook_page_alter implementation
; https://www.drupal.org/node/2465193#comment-9791045
projects[panels][patch][2465193] = "http://www.drupal.org/files/issues/2465193-1.patch"

projects[panels_preview][version] = "1.x-dev"
projects[panels_preview][type] = "module"
projects[panels_preview][subdir] = "contrib"
projects[panels_preview][download][type] = "git"
projects[panels_preview][download][branch] = "7.x-1.x"

projects[panopoly_magic][version] = "1.20"
projects[panopoly_magic][type] = "module"
projects[panopoly_magic][subdir] = "contrib"

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

projects[plupload][version] = "1.7"
projects[plupload][type] = "module"
projects[plupload][subdir] = "contrib"

projects[quickedit][version] = "1.x-dev"
projects[quickedit][type] = "module"
projects[quickedit][subdir] = "contrib"
projects[quickedit][download][type] = "git"
projects[quickedit][download][revision] = "bafb1b5"
projects[quickedit][download][branch] = "7.x-1.x"
; Assign filter types to Entity Embed filters.
; https://www.drupal.org/node/2458517#comment-9754317
projects[quickedit][patch][2458517] = "http://drupal.org/files/issues/entity-embed-filter-types-2458517-1.patch"
; Remove data-quickedit-field-id for personalized fields.
; http://drupal.org/node/2424613#comment-9613695
projects[quickedit][patch][2424613] = "http://drupal.org/files/issues/quick_edit-personalize-2424613-4.patch"
; Don't load external CKEditor plugins until CKEditor integration improves.
; https://www.drupal.org/node/2459541
projects[quickedit][patch][2459541] = "http://drupal.org/files/issues/ignore-remove-external-plugins.patch"

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

projects[rules][version] = "2.9"
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

projects[token][version] = "1.6"
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

projects[views][version] = "3.11"
projects[views][type] = "module"
projects[views][subdir] = "contrib"
; Call to a member function init_display() on a non-object...
; http://drupal.org/node/1685144
projects[views][patch][1685114] = "http://drupal.org/files/views-1685144-localization-bug_1.patch"

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

projects[views_content_cache][version] = "3.x-dev"
projects[views_content_cache][type] = "module"
projects[views_content_cache][subdir] = "contrib"
projects[views_content_cache][download][type] = "git"
projects[views_content_cache][download][revision] = "45863c3"
projects[views_content_cache][download][branch] = "7.x-3.x"

projects[views_fieldsets][version] = "1.2"
projects[views_fieldsets][type] = "module"
projects[views_fieldsets][subdir] = "contrib"

projects[views_load_more][version] = "1.5"
projects[views_load_more][type] = "module"
projects[views_load_more][subdir] = "contrib"

projects[webform][version] = "4.8"
projects[webform][type] = "module"
projects[webform][subdir] = "contrib"

projects[workbench][version] = "1.x-dev"
projects[workbench][type] = "module"
projects[workbench][subdir] = "contrib"
projects[workbench][download][type] = "git"
projects[workbench][download][revision] = "6856e4a"
projects[workbench][download][branch] = "7.x-1.x"

projects[workbench_moderation][version] = "1.4"
projects[workbench_moderation][type] = "module"
projects[workbench_moderation][subdir] = "contrib"
; Workbench Moderation IIB Integration should be optional
; http://drupal.org/node/2462453
projects[workbench_moderation][patch][2462453] = "http://drupal.org/files/issues/workbench_moderation-iib-var-2462453-1.patch"

projects[workbench_moderation_buttons][version] = "1.x-dev"
projects[workbench_moderation_buttons][type] = "module"
projects[workbench_moderation_buttons][subdir] = "contrib"
projects[workbench_moderation_buttons][download][type] = "git"
projects[workbench_moderation_buttons][download][revision] = "0b50592"
projects[workbench_moderation_buttons][download][branch] = "7.x-1.x"

projects[workbench_moderation_notes][version] = "1.x-dev"
projects[workbench_moderation_notes][type] = "module"
projects[workbench_moderation_notes][subdir] = "contrib"
projects[workbench_moderation_notes][download][type] = "git"
projects[workbench_moderation_notes][download][revision] = "8e5e6f4"
projects[workbench_moderation_notes][download][branch] = "7.x-1.x"
; Incorrect status message "...no published revision of this site"
; http://drupal.org/node/2045407
projects[workbench_moderation_notes][patch][2045407] = "http://drupal.org/files/workbench_moderation_notes-no_live_node_alert-2045407-1.patch"

projects[xautoload][version] = "4.5"
projects[xautoload][type] = "module"
projects[xautoload][subdir] = "contrib"

projects[shortcutperrole][version] = "1.2"
projects[shortcutperrole][type] = "module"
projects[shortcutperrole][subdir] = "contrib"

; Libraries
libraries[backbone][download][type] = "get"
libraries[backbone][download][url] = "https://github.com/jashkenas/backbone/archive/1.1.0.zip"

libraries[ckeditor][download][type] = "get"
libraries[ckeditor][download][url] = "http://download.cksource.com/CKEditor%20for%20Drupal/edit/ckeditor_4.4.3_edit.zip"

libraries[ckeditor_lineutils][download][type] = "get"
libraries[ckeditor_lineutils][download][url] = "http://download.ckeditor.com/lineutils/releases/lineutils_4.4.3.zip"
libraries[ckeditor_lineutils][type] = "libraries"
libraries[ckeditor_lineutils][subdir] = "ckeditor/plugins"
libraries[ckeditor_lineutils][directory_name] = "lineutils"

libraries[ckeditor_widget][download][type] = "get"
libraries[ckeditor_widget][download][url] = "http://download.ckeditor.com/widget/releases/widget_4.4.3.zip"
libraries[ckeditor_widget][type] = "libraries"
libraries[ckeditor_widget][subdir] = "ckeditor/plugins"
libraries[ckeditor_widget][directory_name] = "widget"

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

; Lightning Lift
includes[lightning_lift] = lightning_lift/lightning_lift.make
