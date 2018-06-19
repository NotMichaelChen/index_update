project('index', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=3'])

add_global_arguments('-I../src', language : 'cpp')

cxx = meson.get_compiler('cpp')
tacopie_dep = cxx.find_library('tacopie')

cppredis = dependency('cpp_redis')
thread_dep = dependency('threads')

src = [
    'src/index.cpp',
    'src/lexicon.cpp',
    'src/morph.cpp',
    'src/posting.cpp',
    'src/redis.cpp',
    'src/sparse_lexicon.cpp',
    'src/static_index.cpp',
    'src/util.cpp',
    'src/doc_analyzer/analyzer.cpp',
    'src/doc_analyzer/Matcher/block.cpp',
    'src/doc_analyzer/Matcher/blockmatching.cpp',
    'src/doc_analyzer/Matcher/distancetable.cpp',
    'src/doc_analyzer/Matcher/graph.cpp',
    'src/doc_analyzer/Matcher/matcher.cpp',
    'src/doc_analyzer/Matcher/stringencoder.cpp',
    'src/doc_analyzer/Matcher/translate.cpp',
    'src/document_readers/RAWreader.cpp',
    'src/document_readers/WETreader.cpp',
    'src/query_processing/DAAT.cpp',
    'src/query_processing/query_primitive_low.cpp',
    'src/query_processing/query_primitive.cpp',
    'src/query_processing/ranking_functions/BM25.cpp',
    'src/static_functions/bytesIO.cpp',
    'src/static_functions/compression.cpp',
    'src/static_functions/postingIO.cpp',
    'src/static_functions/compression_functions/varbyte.cpp',
    'src/script_engine/parse_engine.cpp',
    'src/Structures/translationtable.cpp',
    'src/Structures/documentstore.cpp',
]

src_test = [
    'src/tests/test_main.cpp',
    'src/tests/test_stringencoder.cpp',
    'src/tests/test_matcher.cpp',
    'src/tests/blocktest.cpp',
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
