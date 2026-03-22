cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0169 OLD)

project(polyglot_parser C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

# core library
FetchContent_Declare(
  tree_sitter
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter
  GIT_TAG master
)
FetchContent_MakeAvailable(tree_sitter)

# 3 grammars for the PoC
FetchContent_Declare(
  tree_sitter_python
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-python
  GIT_TAG master
)  # py

FetchContent_Declare( 
  tree_sitter_javascript
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-javascript
  GIT_TAG master
) # js

FetchContent_Declare( 
  tree_sitter_ruby
  GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-ruby
  GIT_TAG master
) # rb

# populating to get directories
FetchContent_Populate(tree_sitter_python)
FetchContent_Populate(tree_sitter_javascript)
FetchContent_Populate(tree_sitter_ruby)


include_directories(
  ${tree_sitter_SOURCE_DIR}/lib/include
  ${tree_sitter_python_SOURCE_DIR}/src
  ${tree_sitter_javascript_SOURCE_DIR}/src
  ${tree_sitter_ruby_SOURCE_DIR}/src
)

# defining my sources
set(SOURCES
  src/main.c
  src/parser.c

  adapters/adapters.c
  adapters/python_adapter.c
  adapters/js_adapter.c
  adapters/ruby_adapter.c

  ir/ir.c
  exporter/mc_export.c

  graph/graph.c
)

# setting the grammer 
set(GRAMMAR_SOURCES
  ${tree_sitter_python_SOURCE_DIR}/src/parser.c
  ${tree_sitter_python_SOURCE_DIR}/src/scanner.c

  ${tree_sitter_javascript_SOURCE_DIR}/src/parser.c
   ${tree_sitter_javascript_SOURCE_DIR}/src/scanner.c

  ${tree_sitter_ruby_SOURCE_DIR}/src/parser.c
  ${tree_sitter_ruby_SOURCE_DIR}/src/scanner.c

  ${tree_sitter_SOURCE_DIR}/lib/src/lib.c
)

add_executable(polyglot_parser
  ${SOURCES}
  ${GRAMMAR_SOURCES}
)

target_include_directories(polyglot_parser PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
  ${CMAKE_CURRENT_SOURCE_DIR}/adapters
  ${CMAKE_CURRENT_SOURCE_DIR}/ir
  ${CMAKE_CURRENT_SOURCE_DIR}/exporter
  ${CMAKE_CURRENT_SOURCE_DIR}/graph
)