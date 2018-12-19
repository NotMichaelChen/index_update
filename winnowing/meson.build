project('winnowing', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=3'])

add_global_arguments('-I../src', language : 'cpp')

cxx = meson.get_compiler('cpp')
tacopie_dep = cxx.find_library('tacopie')

cppredis = dependency('cpp_redis')
thread_dep = dependency('threads')

src = [
'src/main.cpp',
'src/document_readers/WETreader.cpp',
'src/utility/fs_util.cpp',
'src/utility/string_util.cpp',
'src/utility/morph.cpp',
'src/utility/md5.cpp',
'src/doc_partitioner/partitioner.cpp',
'src/index/index_builder.cpp',
'src/index/document_store.cpp',
'src/index/document_version_table.cpp',
]

dep = [
    cppredis,
    tacopie_dep,
    thread_dep,
]

executable('winnowing', src, dependencies : dep)
