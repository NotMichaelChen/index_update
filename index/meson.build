project('index', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=3'])

cxx = meson.get_compiler('cpp')
tacopie_dep = cxx.find_library('tacopie')

cppredis = dependency('cpp_redis')
thread_dep = dependency('threads')

src = [
    'index.cpp',
    'lexicon.cpp',
    'posting.cpp',
    'redis.cpp',
    'sparse_lexicon.cpp',
    'static_index.cpp',
    'util.cpp',
    'doc_analyzer/analyzer.cpp',
    'doc_analyzer/Matcher/block.cpp',
    'doc_analyzer/Matcher/distancetable.cpp',
    'doc_analyzer/Matcher/graph.cpp',
    'doc_analyzer/Matcher/matcher.cpp',
    'doc_analyzer/Matcher/stringencoder.cpp',
    'doc_analyzer/Matcher/translate.cpp',
    'document_readers/RAWreader.cpp',
    'document_readers/WETreader.cpp',
    'query_processing/DAAT.cpp',
    'query_processing/query_primitive_low.cpp',
    'query_processing/query_primitive.cpp',
    'query_processing/ranking_functions/BM25.cpp',
    'static_functions/bytesIO.cpp',
    'static_functions/compression.cpp',
    'static_functions/postingIO.cpp',
    'static_functions/compression_functions/varbyte.cpp',
    'script_engine/parse_engine.cpp',
    'Structures/translationtable.cpp',
    'Structures/documentstore.cpp',
]

should_test = get_option('test')
if should_test
    src += 'tests/test_main.cpp'
else
    src += 'main.cpp'
endif

dep = [
    cppredis,
    tacopie_dep,
    thread_dep,
]

executable('index', src, dependencies : dep)
