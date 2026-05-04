cmake_minimum_required(VERSION 3.24)
project(corpus)

# Set the C++ standard to use
#set(CMAKE_CXX_STANDARD 17)

# Add a definition for the GRAMMAR_END_OF_TOKEN_MARKER macro
#add_definitions(-DGRAMMAR_END_OF_TOKEN_MARKER=" ")
#add_definitions(-DGRAMMAR_END_OF_TOKEN_MARKER_SIZE=1)
#add_definitions(-D_SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRICATION_WARNING)

# ------------
#  corpus.exe
# ------------
# Add the source file
set(SOURCES_CORPUS main.cpp ./../../sundry/cooked_read.cc)
add_executable(corpus ${SOURCES_CORPUS})

# Add a definition for the GRAMMAR_END_OF_TOKEN_MARKER macro for the replika target
target_compile_definitions(corpus PRIVATE GRAMMAR_END_OF_TOKEN_MARKER=" ")
target_compile_definitions(corpus PRIVATE GRAMMAR_END_OF_TOKEN_MARKER_SIZE=1)

target_compile_definitions(corpus PRIVATE _TEST_CORPUS_HH)
target_compile_definitions(corpus PRIVATE TEST_SKIP_GRAM_PAIRS_HH)