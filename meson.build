project('ascetic', 'c',
  version: '20260309',
  default_options: [
    'c_std=gnu23',
  ],
)

# App metadata
app_name      = 'Ascetic'
app_target    = 'ascetic'
app_id        = 'com.gmail.trainraider7.' + app_target
app_prefix    = '/' + app_id.replace('.', '/')
app_author    = 'Robert Rapier'
app_copyright = 'Copyright (C) 2026'
app_comment   = 'A minimal web browser'
app_version   = meson.project_version()

# Dependencies
gtk4         = dependency('gtk4')
libadwaita   = dependency('libadwaita-1')
webkitgtk    = dependency('webkitgtk-6.0')
ada          = dependency('ada')
libpsl       = dependency('libpsl')

deps = [gtk4, libadwaita, webkitgtk, ada, libpsl]

# Blueprint compiler
blueprint_compiler = find_program('blueprint-compiler')

# Source files
src_dir = 'source'
data_dir = src_dir + '/data'
blp_dir = src_dir + '/blueprints'

c_sources = files(
  src_dir + '/main.c',
  src_dir + '/uri.c',
  src_dir + '/version.c',
  src_dir + '/settings_page.c',
  src_dir + '/webview.c',
  src_dir + '/app_window.c',
)

# Blueprint compilation
blueprint_files = [
  'settings_page',
  'app_window',
]

ui_files = []
foreach blp : blueprint_files
  ui_files += custom_target(
    blp + '.ui',
    input:   files(blp_dir + '/' + blp + '.blp'),
    output:  blp + '.ui',
    command: [blueprint_compiler, 'compile', '--output', '@OUTPUT@', '@INPUT@'],
  )
endforeach

# gresource.xml from template
gresource_xml = custom_target(
  'data.gresource.xml',
  input:   files(data_dir + '/data.gresource.xml.pre'),
  output:  'data.gresource.xml',
  command: ['sed', 's:__PREFIX__:' + app_prefix + ':', '@INPUT@'],
  capture: true,
)

# glib-compile-resources
glib_compile_resources = find_program('glib-compile-resources')

icon_svg         = files(data_dir + '/icon.svg')
block_images_json = files(data_dir + '/block-images.json')

data_c = custom_target(
  'data.c',
  input:   [gresource_xml] + ui_files + icon_svg + block_images_json,
  output:  'data.c',
  command: [
    glib_compile_resources,
    '--sourcedir', meson.current_build_dir(),
    '--sourcedir', meson.current_source_dir() + '/' + data_dir,
    '--generate-source',
    '@INPUT0@',
    '--target', '@OUTPUT@',
  ],
)

data_h = custom_target(
  'data.h',
  input:   [gresource_xml] + ui_files + icon_svg + block_images_json,
  output:  'data.h',
  command: [
    glib_compile_resources,
    '--sourcedir', meson.current_build_dir(),
    '--sourcedir', meson.current_source_dir() + '/' + data_dir,
    '--generate-header',
    '@INPUT0@',
    '--target', '@OUTPUT@',
  ],
)

# Executable
ascetic = executable(
  app_target,
  sources: c_sources + [data_c, data_h],
  dependencies: deps,
  c_args: [
    '-DVERSION="' + app_version + '"',
    '-DNAME="' + app_name + '"',
    '-DAPP_ID="' + app_id + '"',
    '-DAPP_PREFIX="' + app_prefix + '"',
    '-DAUTHOR="' + app_author + '"',
    '-DCOPYRIGHT="' + app_copyright + '"',
    '-DTARGET="' + app_target + '"',
  ],
  link_args: ['-rdynamic'],
  install: true,
)

# Desktop file
desktop_conf = configuration_data()
desktop_conf.set('VERSION',   app_version)
desktop_conf.set('NAME',      app_name)
desktop_conf.set('APP_ID', app_id)
desktop_conf.set('EXEC',      get_option('prefix') + '/bin/' + app_target)
desktop_conf.set('COMMENT',   app_comment)
desktop_conf.set('ICON',      get_option('prefix') + '/share/icons/hicolor/scalable/apps/' + app_target + '.svg')
desktop_conf.set('CATEGORIES','GNOME;GTK;Network;WebBrowser;')

desktop_file = configure_file(
  input:        'ascetic.desktop.in',
  output:       app_id + '.desktop',
  configuration: desktop_conf,
  install_dir:  'share/applications',
)

service_file = configure_file(
  input:         'ascetic.service.in',
  output:        app_id + '.service',
  configuration: desktop_conf,
  install_dir:   'share/dbus-1/services',
)

# Icon
install_data(
  data_dir + '/icon.svg',
  install_dir: 'share/icons/hicolor/scalable/apps',
  rename: app_target + '.svg',
)
