add_library(cprime_semantics STATIC
  semantics.cpp)
target_link_libraries(cprime_semantics
  PUBLIC
    cprime_headers
    cprime_support
    cprime_source
    cprime_diagnostics
    cprime_ast)

add_executable(cprime_semantics_file_test
  semantics_file_test.cpp)
target_link_libraries(cprime_semantics_file_test
  PRIVATE
    cprime_support
    cprime_source
    cprime_diagnostics
    cprime_ast
    cprime_parse
    cprime_semantics
    cprime_testing)
