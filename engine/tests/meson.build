foreach name : ['color', 'cam16', 'hct', 'palette', 'scheme']
  test_exe = executable(
    'test_' + name,
    'test_' + name + '.c',
    include_directories: seurat_src_inc,
    dependencies: [seurat_dep, m_dep],
  )
  test(name, test_exe)
endforeach
