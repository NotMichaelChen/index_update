project('index', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=3'])

add_global_arguments('-I../src', language : 'cpp')

cxx = meson.get_compiler('cpp')
tacopie_dep = cxx.find_library('tacopie')

cppredis = dependency('cpp_redis')
thread_dep = dependency('threads')

src = [
'src/redis_interfaces/documentstore.cpp',
'src/doc_parsing/docparser.cpp',
'src/doc_parsing/transform.cpp',
'src/doc_parsing/diff.cpp',
'src/doc_parsing/stringencoder.cpp',
'src/posting.cpp',
'src/main.cpp',
'src/lexicon.cpp',
'src/index.cpp',
'src/tests/testdriver.cpp',
'src/tests/doc_parsing/difftest.cpp',
'src/landmark/landmarkdirectory.cpp',
'src/landmark/landmarkarray.cpp',
'src/utility/util.cpp',
]

dep = [
    cppredis,
    tacopie_dep,
    thread_dep,
]

executable('index', src, dependencies : dep)
