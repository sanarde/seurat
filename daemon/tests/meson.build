check_inc = include_directories('../../engine/tests')

foreach name : ['config', 'state', 'socket']
  test_exe = executable(
    'test_' + name,
    'test_' + name + '.c',
    c_args: seuratd_c_args,
    include_directories: [seuratd_src_inc, check_inc],
    link_with: seuratd_core,
    dependencies: seurat_dep,
  )
  test('daemon-' + name, test_exe)
endforeach
