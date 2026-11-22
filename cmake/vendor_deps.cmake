# Простой скрипт для скачивания зависимостей. Обновлять
# зависимости рекомендуется через этот скрипт.

cmake_minimum_required(VERSION 3.28)

if(NOT DEFINED CPRIME_VENDOR_DIR)
  set(CPRIME_VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
endif()

function(VendorDependency)
  cmake_parse_arguments(
    PARSE_ARGV 0 ARG
    "" "URL;URL_HASH;SOURCE_DIR" "")
  if(
      ARG_UNPARSED_ARGUMENTS OR ARG_KEYWORDS_MISSING_VALUES
      OR NOT ARG_URL
      OR NOT ARG_URL_HASH
      OR NOT ARG_SOURCE_DIR
    )
    message(FATAL_ERROR
      "VendorDependency requires URL, URL_HASH and SOURCE_DIR")
  endif()

  message(STATUS "Vendoring ${ARG_SOURCE_DIR}...")

  get_filename_component(
    _dest "${ARG_SOURCE_DIR}" ABSOLUTE
    BASE_DIR "${CPRIME_VENDOR_DIR}")

  set(_tmp "${CPRIME_VENDOR_DIR}/.vendor-tmp")
  file(REMOVE_RECURSE "${_tmp}")
  file(MAKE_DIRECTORY "${_tmp}/extracted")

  file(DOWNLOAD
    "${ARG_URL}" "${_tmp}/archive"
    EXPECTED_HASH "${ARG_URL_HASH}"
    TLS_VERIFY ON
    STATUS _status
    SHOW_PROGRESS)
  list(GET _status 0 _code)
  if(NOT _code EQUAL 0)
    file(REMOVE_RECURSE "${_tmp}")
    message(FATAL_ERROR "Download failed: ${_status}")
  endif()

  file(ARCHIVE_EXTRACT
    INPUT "${_tmp}/archive"
    DESTINATION "${_tmp}/extracted")

  # Убираем корневую папку архива, если она есть
  file(GLOB _entries LIST_DIRECTORIES TRUE "${_tmp}/extracted/*")
  list(LENGTH _entries _count)
  if(_count EQUAL 1 AND IS_DIRECTORY "${_entries}")
    set(_source "${_entries}")
  else()
    set(_source "${_tmp}/extracted")
  endif()

  file(REMOVE_RECURSE "${_dest}")
  # Если родительской директории _dest не существует, то RENAME
  # может завершиться с ошибкой. Создаём родительскую директорию,
  # чтобы избежать этого.
  cmake_path(GET _dest PARENT_PATH _dest_parent)
  file(MAKE_DIRECTORY "${_dest_parent}")
  file(RENAME "${_source}" "${_dest}")
  file(REMOVE_RECURSE "${_tmp}")
endfunction()


VendorDependency(
  SOURCE_DIR "Catch2"
  URL        "https://github.com/catchorg/Catch2/archive/refs/tags/v3.15.0.tar.gz"
  URL_HASH   "SHA256=9650c55e497759cc39b977e45524bc8acb15256061c112080916ab6cb0b1ea66")

VendorDependency(
  SOURCE_DIR "utfcpp"
  URL        "https://github.com/nemtrif/utfcpp/archive/refs/tags/v4.0.9.tar.gz"
  URL_HASH   "SHA256=397a9a2a6ed5238f854f490b0177b840abc6b62571ec3e07baa0bb94d3f14d5a")

VendorDependency(
  SOURCE_DIR "magic_enum"
  URL        "https://github.com/Neargye/magic_enum/archive/refs/tags/v0.9.7.tar.gz"
  URL_HASH   "SHA256=b403d3dad4ef542fdc3024fa37d3a6cedb4ad33c72e31b6d9bab89dcaf69edf7")

VendorDependency(
  SOURCE_DIR "cxxopts"
  URL        "https://github.com/jarro2783/cxxopts/archive/refs/tags/v3.3.1.tar.gz"
  URL_HASH   "SHA256=3bfc70542c521d4b55a46429d808178916a579b28d048bd8c727ee76c39e2072")

set(BOOST_VERSION "1.91.0")
VendorDependency(
  SOURCE_DIR "boost/config"
  URL        "https://github.com/boostorg/config/archive/refs/tags/boost-${BOOST_VERSION}.tar.gz"
  URL_HASH   "SHA256=e69618fa862927db69b4dd8e6b070c647a799b892fd0ee141d28db0dab025531")
VendorDependency(
  SOURCE_DIR "boost/assert"
  URL        "https://github.com/boostorg/assert/archive/refs/tags/boost-${BOOST_VERSION}.tar.gz"
  URL_HASH   "SHA256=9145fba14048a46c0f65e5b28e68176d568148957ebf9bdf9e82bcc5d5a703a9")
VendorDependency(
  SOURCE_DIR "boost/move"
  URL        "https://github.com/boostorg/move/archive/refs/tags/boost-${BOOST_VERSION}.tar.gz"
  URL_HASH   "SHA256=9c0c45d240bcdebc0305e19c376746be082614401769366fa99de6968e2522ac")
VendorDependency(
  SOURCE_DIR "boost/intrusive"
  URL        "https://github.com/boostorg/intrusive/archive/refs/tags/boost-${BOOST_VERSION}.tar.gz"
  URL_HASH   "SHA256=9709c94d0cb96d26c9cb325b842f300c1c067579907d34604d716f86556ae794")
