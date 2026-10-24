m_dep = meson.get_compiler('c').find_library('m', required: false)

seurat_inc = include_directories('include')
seurat_src_inc = include_directories('src')

seurat_sources = files(
  'src/cam16.c',
  'src/color.c',
  'src/hct.c',
  'src/palette.c',
  'src/scheme.c',
)

libseurat = library(
  'seurat',
  seurat_sources,
  include_directories: seurat_inc,
  dependencies: m_dep,
  install: true,
)

seurat_dep = declare_dependency(
  link_with: libseurat,
  include_directories: seurat_inc,
)

install_headers(
  'include/seurat/color.h',
  'include/seurat/hct.h',
  'include/seurat/palette.h',
  'include/seurat/scheme.h',
  subdir: 'seurat',
)

subdir('tests')
