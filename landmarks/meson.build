project('index', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=3'])

add_global_arguments('-I../src', language : 'cpp')

cxx = meson.get_compiler('cpp')
tacopie_dep = cxx.find_library('tacopie')

cppredis = dependency('cpp_redis')
thread_dep = dependency('threads')

# find [directory] -name ["pattern"]
src = [
    'src/document_readers/WETreader.cpp',
    'src/redis_interfaces/documentstore.cpp',
    'src/doc_parsing/docparser.cpp',
    'src/doc_parsing/transform.cpp',
    'src/doc_parsing/diff.cpp',
    'src/doc_parsing/stringencoder.cpp',
    'src/landmark/landmarkdirectory.cpp',
    'src/landmark/landmarkarray.cpp',
    'src/index/posting.cpp',
    'src/index/lexicon.cpp',
    'src/index/index.cpp',
    'src/index/static_index.cpp',
    'src/index/dynamic_index.cpp',
    'src/index/sparse_lexicon.cpp',
    'src/static_functions/compression_functions/varbyte.cpp',
    'src/static_functions/compression.cpp',
    'src/static_functions/bytesIO.cpp',
    'src/static_functions/postingIO.cpp',
    'src/utility/timer.cpp',
    'src/utility/morph.cpp',
    'src/utility/util.cpp',
    'src/utility/redis.cpp',
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
