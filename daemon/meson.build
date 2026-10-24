dbus_dep = dependency('dbus-1')

seuratd_c_args = ['-D_GNU_SOURCE']

seuratd_src_inc = include_directories('src')

seuratd_core = static_library(
  'seuratd-core',
  'src/bus.c',
  'src/config.c',
  'src/socket.c',
  'src/state.c',
  'src/wallpaper.c',
  'src/watch.c',
  c_args: seuratd_c_args,
  include_directories: seuratd_src_inc,
  dependencies: [seurat_dep, dbus_dep],
)

executable(
  'seuratd',
  'src/main.c',
  c_args: seuratd_c_args + [
    '-DSEURATD_WALLPAPER_PATH="' + get_option('wallpaper_path') + '"',
  ],
  include_directories: seuratd_src_inc,
  link_with: seuratd_core,
  dependencies: [seurat_dep, dbus_dep],
  install: true,
)

subdir('tests')
