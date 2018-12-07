project('index', 'cpp', default_options : ['cpp_std=c++14', 'warning_level=3'])

add_global_arguments('-I../src', language : 'cpp')

cxx = meson.get_compiler('cpp')
tacopie_dep = cxx.find_library('tacopie')

cppredis = dependency('cpp_redis')
thread_dep = dependency('threads')

# find [directory] -name ["pattern"]
src = [
    'src/doc_analyzer/analyzer.cpp',
    'src/doc_analyzer/Matcher/graph.cpp',
    'src/doc_analyzer/Matcher/block.cpp',
    'src/doc_analyzer/Matcher/translate.cpp',
    'src/doc_analyzer/Matcher/blockmatching.cpp',
    'src/doc_analyzer/Matcher/distancetable.cpp',
    'src/doc_analyzer/Matcher/matcher.cpp',
    'src/doc_analyzer/Matcher/stringencoder.cpp',
    'src/dumps/redis_dump.cpp',
    'src/document_readers/WETreader.cpp',
    'src/document_readers/RAWreader.cpp',
    'src/script_engine/commands.cpp',
    'src/script_engine/parse_engine.cpp',
    'src/Structures/translationtable.cpp',
    'src/Structures/documentstore.cpp',
    'src/index/posting.cpp',
    'src/index/lexicon.cpp',
    'src/index/query_processing/DAAT.cpp',
    'src/index/query_processing/query_primitive.cpp',
    'src/index/query_processing/query_primitive_low.cpp',
    'src/index/query_processing/ranking_functions/BM25.cpp',
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
]

src_test = [
    'src_test/test_stringencoder.cpp',
    'src_test/blocktest.cpp',
    'src_test/test_matcher.cpp',
    'src_test/test_main.cpp',
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
