add_library(cprime_ast STATIC
  dump.cpp)
target_link_libraries(cprime_ast
  PUBLIC
    cprime_headers
    cprime_support
    magic_enum::magic_enum)

add_executable(cprime_ast_unittests
  list_unittest.cpp)
target_link_libraries(cprime_ast_unittests
  PRIVATE
    cprime_ast
    Catch2::Catch2WithMain)
catch_discover_tests(cprime_ast_unittests)
