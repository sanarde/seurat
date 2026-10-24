test_data_dir = meson.current_source_dir() / 'data'

foreach name : ['color', 'cam16', 'hct', 'palette', 'scheme', 'image', 'quantize', 'score', 'source']
  test_exe = executable(
    'test_' + name,
    'test_' + name + '.c',
    c_args: '-DTEST_DATA_DIR="' + test_data_dir + '"',
    include_directories: seurat_src_inc,
    dependencies: [seurat_dep, m_dep],
  )
  test(name, test_exe)
endforeach
