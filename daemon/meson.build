seuratd_c_args = ['-D_GNU_SOURCE']

seuratd_src_inc = include_directories('src')

seuratd_core = static_library(
  'seuratd-core',
  'src/config.c',
  c_args: seuratd_c_args,
  include_directories: seuratd_src_inc,
)

executable(
  'seuratd',
  'src/main.c',
  c_args: seuratd_c_args,
  include_directories: seuratd_src_inc,
  link_with: seuratd_core,
  install: true,
)

subdir('tests')
