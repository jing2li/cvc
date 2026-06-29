cmake_minimum_required(VERSION 3.24)

project(lhpc-aff
  DESCRIPTION "LHPC AFF I/O library"
  HOMEPAGE_URL "https://github.com/usqcd-software/aff"
  VERSION "0.0.1"
  LANGUAGES C CXX)

include(FetchContent)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# fetch the upstream, hand-rolled-Makefile-only AFF sources into a
# subdirectory so they can be re-cloned freely without touching this
# wrapper CMakeLists.txt
set(AFF_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/AFF)
FetchContent_Declare(
  aff_upstream
  GIT_REPOSITORY https://github.com/usqcd-software/aff.git
  SOURCE_DIR ${AFF_SRC_DIR})
FetchContent_MakeAvailable(aff_upstream)

set(AFF_LIB_DIR ${AFF_SRC_DIR}/lib)

# upstream's lib/Makefile builds the public "lhpc-aff.h" interface header
# by concatenating interface.prologue, a fixed list of internal headers
# (with their header-guard MARK_ lines stripped), then interface.epilogue.
# Replicate that here instead of invoking the upstream Makefile directly.
set(AFF_INTERFACE_PARTS aff.h node.h tree.h stable.h treap.h md5.h coding.h
                        alloc.h)
set(AFF_INTERFACE_HEADER ${CMAKE_CURRENT_BINARY_DIR}/lhpc-aff.h)

set(AFF_INTERFACE_PART_FILES)
foreach(_part ${AFF_INTERFACE_PARTS})
  list(APPEND AFF_INTERFACE_PART_FILES ${AFF_LIB_DIR}/${_part})
endforeach()

add_custom_command(
  OUTPUT ${AFF_INTERFACE_HEADER}
  COMMAND ${CMAKE_COMMAND}
          -DAFF_LIB_DIR=${AFF_LIB_DIR}
          -DOUTPUT_HEADER=${AFF_INTERFACE_HEADER}
          -P ${CMAKE_CURRENT_SOURCE_DIR}/generate_interface.cmake
  DEPENDS ${AFF_LIB_DIR}/interface.prologue ${AFF_LIB_DIR}/interface.epilogue
          ${AFF_INTERFACE_PART_FILES}
  COMMENT "Generating lhpc-aff.h interface header")

add_custom_target(lhpc-aff-interface DEPENDS ${AFF_INTERFACE_HEADER})

list(APPEND AFF_SRC
  # treap.sources
  ${AFF_LIB_DIR}/treap-cmp.c
  ${AFF_LIB_DIR}/treap-insert.c
  ${AFF_LIB_DIR}/treap-fini.c
  ${AFF_LIB_DIR}/treap-init.c
  ${AFF_LIB_DIR}/treap-lookup.c
  ${AFF_LIB_DIR}/treap-print.c
  # stable.sources
  ${AFF_LIB_DIR}/stable-fini.c
  ${AFF_LIB_DIR}/stable-index.c
  ${AFF_LIB_DIR}/stable-print.c
  ${AFF_LIB_DIR}/stable-foreach.c
  ${AFF_LIB_DIR}/stable-init.c
  ${AFF_LIB_DIR}/stable-insert.c
  ${AFF_LIB_DIR}/stable-iblock.c
  ${AFF_LIB_DIR}/stable-lookup.c
  ${AFF_LIB_DIR}/stable-name.c
  ${AFF_LIB_DIR}/stable-id.c
  # tree.sources
  ${AFF_LIB_DIR}/tree-fini.c
  ${AFF_LIB_DIR}/tree-foreach.c
  ${AFF_LIB_DIR}/tree-iblock.c
  ${AFF_LIB_DIR}/tree-index.c
  ${AFF_LIB_DIR}/tree-init.c
  ${AFF_LIB_DIR}/tree-insert.c
  ${AFF_LIB_DIR}/tree-lookup.c
  ${AFF_LIB_DIR}/tree-print.c
  ${AFF_LIB_DIR}/tree-root.c
  # node.sources
  ${AFF_LIB_DIR}/node-foreach.c
  ${AFF_LIB_DIR}/node-id.c
  ${AFF_LIB_DIR}/node-name.c
  ${AFF_LIB_DIR}/node-parent.c
  ${AFF_LIB_DIR}/node-type.c
  ${AFF_LIB_DIR}/node-size.c
  ${AFF_LIB_DIR}/node-offset.c
  ${AFF_LIB_DIR}/node-assign.c
  ${AFF_LIB_DIR}/node-chdir.c
  ${AFF_LIB_DIR}/node-cda.c
  ${AFF_LIB_DIR}/node-cdv.c
  ${AFF_LIB_DIR}/node-cd.c
  # md5.sources
  ${AFF_LIB_DIR}/md5.c
  # coding.sources
  ${AFF_LIB_DIR}/encode-type.c
  ${AFF_LIB_DIR}/encode-u32.c
  ${AFF_LIB_DIR}/encode-u64.c
  ${AFF_LIB_DIR}/encode-double.c
  ${AFF_LIB_DIR}/decode-type.c
  ${AFF_LIB_DIR}/decode-u32.c
  ${AFF_LIB_DIR}/decode-u64.c
  ${AFF_LIB_DIR}/decode-double.c
  # aff.sources
  ${AFF_LIB_DIR}/aff-signature.c
  ${AFF_LIB_DIR}/aff-version.c
  ${AFF_LIB_DIR}/aff-namecheck.c
  ${AFF_LIB_DIR}/aff-alloc.c
  ${AFF_LIB_DIR}/waff-init.c
  ${AFF_LIB_DIR}/waff-fini.c
  ${AFF_LIB_DIR}/waff-errstr.c
  ${AFF_LIB_DIR}/waff-clearerr.c
  ${AFF_LIB_DIR}/waff-root.c
  ${AFF_LIB_DIR}/waff-stable.c
  ${AFF_LIB_DIR}/waff-tree.c
  ${AFF_LIB_DIR}/waff-mkdir.c
  ${AFF_LIB_DIR}/waff-mkpath.c
  ${AFF_LIB_DIR}/waff-putchar.c
  ${AFF_LIB_DIR}/waff-putint.c
  ${AFF_LIB_DIR}/waff-putdouble.c
  ${AFF_LIB_DIR}/waff-putcomplex.c
  ${AFF_LIB_DIR}/raff-init.c
  ${AFF_LIB_DIR}/raff-fini.c
  ${AFF_LIB_DIR}/raff-check.c
  ${AFF_LIB_DIR}/raff-errstr.c
  ${AFF_LIB_DIR}/raff-clearerr.c
  ${AFF_LIB_DIR}/raff-root.c
  ${AFF_LIB_DIR}/raff-stable.c
  ${AFF_LIB_DIR}/raff-tree.c
  ${AFF_LIB_DIR}/raff-chdir.c
  ${AFF_LIB_DIR}/raff-chpath.c
  ${AFF_LIB_DIR}/raff-getchar.c
  ${AFF_LIB_DIR}/raff-getint.c
  ${AFF_LIB_DIR}/raff-getdouble.c
  ${AFF_LIB_DIR}/raff-getcomplex.c
  ${AFF_LIB_DIR}/separator.c
  # io.sources
  ${AFF_LIB_DIR}/io.c
  # cpp.sources
  ${AFF_LIB_DIR}/rwaff.cpp
  ${AFF_LIB_DIR}/mdarray.cpp)

if (BUILD_SHARED_LIBS)
  add_library(lhpc-aff SHARED ${AFF_SRC})
else ()
  add_library(lhpc-aff STATIC ${AFF_SRC})
endif ()

add_dependencies(lhpc-aff lhpc-aff-interface)

target_include_directories(lhpc-aff
  PRIVATE ${AFF_LIB_DIR}
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# matches upstream's config/linux build flags
target_compile_definitions(lhpc-aff PRIVATE
  HAS_INTEGRAL_OFF_T
  _POSIX_C_SOURCE
  _LARGEFILE_SOURCE
  _FILE_OFFSET_BITS=64)
target_compile_features(lhpc-aff PRIVATE c_std_99)
