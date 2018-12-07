project('index', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=3'])

add_global_arguments('-I../src', language : 'cpp')

cxx = meson.get_compiler('cpp')
tacopie_dep = cxx.find_library('tacopie')

cppredis = dependency('cpp_redis')
thread_dep = dependency('threads')

# find [directory] -name ["pattern"]
src = [
    'src/redis_interfaces/documentstore.cpp',
    'src/doc_parsing/docparser.cpp',
    'src/doc_parsing/transform.cpp',
    'src/doc_parsing/diff.cpp',
    'src/doc_parsing/stringencoder.cpp',
    'src/posting.cpp',
    'src/lexicon.cpp',
    'src/index.cpp',
    'src/landmark/landmarkdirectory.cpp',
    'src/landmark/landmarkarray.cpp',
    'src/utility/util.cpp',
]

src_test = [
    'src_test/test_main.cpp',
    'src_test/test_diff.cpp',
]

should_test = get_option('test')
if should_test
    src += src_test
else
    src += 'src/main.cpp'
endif

dep = [
    cppredis,
    tacopie_dep,
    thread_dep,
]

executable('index', src, dependencies : dep)
