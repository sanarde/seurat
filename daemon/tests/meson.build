check_inc = include_directories('../../engine/tests')
test_data_dir = meson.project_source_root() / 'engine/tests/data'

foreach name : ['config', 'state', 'socket', 'wallpaper', 'watch']
  test_exe = executable(
    'test_' + name,
    'test_' + name + '.c',
    c_args: seuratd_c_args + ['-DTEST_DATA_DIR="' + test_data_dir + '"'],
    include_directories: [seuratd_src_inc, check_inc],
    link_with: seuratd_core,
    dependencies: seurat_dep,
  )
  test('daemon-' + name, test_exe)
endforeach
