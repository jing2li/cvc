cmake_minimum_required(VERSION 3.24)

include(CMakeDependentOption)
include(CheckIncludeFile)
include(TestBigEndian)
include(FetchContent)

project(KQED
  DESCRIPTION "QED kernel code for computing the HLBL contribution to the muon g-2"
  HOMEPAGE_URL "https://github.com/RJHudspith/KQED"
  VERSION "0.0.1"
  LANGUAGES C CXX)

option(KQED_USE_CUDA "Enable CUDA support" ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (KQED_USE_CUDA)
  enable_language(CUDA)
  find_package(CUDAToolkit)
endif()

# fetch the upstream, autotools-only KQED sources into a subdirectory so
# they can be re-cloned freely without touching this wrapper CMakeLists.txt
set(KQED_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/KQED)
FetchContent_Declare(
  kqed_upstream
  GIT_REPOSITORY https://github.com/RJHudspith/KQED.git
  GIT_TAG main
  SOURCE_DIR ${KQED_SRC_DIR})
FetchContent_MakeAvailable(kqed_upstream)

# KQED.h unconditionally includes "config.h", normally produced by its
# autotools build. Generate an equivalent here instead of running autoreconf.
check_include_file(immintrin.h HAVE_IMMINTRIN_H)
check_include_file(omp.h HAVE_OMP_H)
check_include_file(sys/time.h HAVE_SYS_TIME_H)
check_include_file(time.h HAVE_TIME_H)
test_big_endian(WORDS_BIGENDIAN)

# io.c resolves its precomputed kernel data (taylorx_cksum.bin, etc.) as
# LIB_PATH + "/PRECOMP/<file>". Point it at the fetched source tree, where
# the PRECOMP directory already lives, so no install step is required.
set(KQED_LIB_PATH "${KQED_SRC_DIR}")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/config.h @ONLY)

list(APPEND KQED_SRC
  KQED/src/LIBFILES/all_kernels.c
  KQED/src/LIBFILES/cheby.c
  KQED/src/LIBFILES/chnr_dV.c
  KQED/src/LIBFILES/chnr_dS.c
  KQED/src/LIBFILES/chnr_dT.c
  KQED/src/LIBFILES/con_kernel.c
  KQED/src/LIBFILES/getff-new.c
  KQED/src/LIBFILES/init.c
  KQED/src/LIBFILES/io.c
  KQED/src/LIBFILES/kernels.c
  KQED/src/LIBFILES/pi_pert.c
  KQED/src/LIBFILES/QED_kernel.c
  KQED/src/LIBFILES/QED_kernel_xy0.c
  KQED/src/LIBFILES/simd.c
  KQED/src/LIBFILES/SYMXY.c
  KQED/src/LIBFILES/SYMXY0.c
  KQED/src/LIBFILES/Tabd.c
  KQED/src/LIBFILES/sub_kernel.c
  KQED/src/UTILS/GLU_timer.c
  KQED/src/UTILS/GLU_bswap.c
  KQED/src/UTILS/crc32c.c
  KQED/src/UTILS/corr_malloc.c)

# KQED.cu is a "unity build": it #includes all the individual per-kernel
# .cu files itself (cheby.cu, chnr_d*.cu, kernels.cu, con_kernel.cu,
# sub_kernel.cu, all_kernels.cu, SYMXY*.cu, QED_kernel*.cu, pi_pert.cu,
# getff-new.cu, Tabd.cu), so those must NOT also be compiled as separate
# translation units here or every symbol they define ends up duplicated
# across object files, which nvlink rejects.
list(APPEND KQED_SRC_CUDA
  KQED/src_cuda/LIBFILES/KQED.cu
  KQED/src_cuda/LIBFILES/init.c
  KQED/src_cuda/LIBFILES/io.c
  KQED/src_cuda/UTILS/GLU_timer.c
  KQED/src_cuda/UTILS/GLU_bswap.c
  KQED/src_cuda/UTILS/crc32c.c)

list(APPEND kqed_libs "KQED")

if (KQED_USE_CUDA)
  list(APPEND kqed_libs "KQED_cuda")
endif()

if (BUILD_SHARED_LIBS)
  add_library(KQED SHARED ${KQED_SRC})
else ()
  add_library(KQED STATIC ${KQED_SRC})
endif()

# KQED and KQED_cuda both publicly expose identically-named headers with
# different (host-only vs __device__-qualified) declarations. A consumer
# linking both (e.g. cvc, which needs host decls in its .cpp files and
# device decls in its .cu files) must not get both dumped into one flat
# include path, or whichever wins first breaks the other language's TUs.
# Keep each target's own headers unconditionally visible to its own
# sources (PRIVATE), but gate what gets propagated to consumers
# (INTERFACE) by the consuming source file's compile language.
target_include_directories(KQED
  PRIVATE ${KQED_SRC_DIR}/src/HEADERS
  INTERFACE $<$<NOT:$<COMPILE_LANGUAGE:CUDA>>:${KQED_SRC_DIR}/src/HEADERS>
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

if (KQED_USE_CUDA)
  if (BUILD_SHARED_LIBS)
    add_library(KQED_cuda SHARED ${KQED_SRC_CUDA})
  else ()
    add_library(KQED_cuda STATIC ${KQED_SRC_CUDA})
  endif()

  target_include_directories(KQED_cuda
    PRIVATE ${KQED_SRC_DIR}/src_cuda/HEADERS
    INTERFACE $<$<COMPILE_LANGUAGE:CUDA>:${KQED_SRC_DIR}/src_cuda/HEADERS>
    PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
  # upstream annotates __device__ functions with KQED_PRIVATE but never
  # defines it; the CPU headers show the equivalent declarations plain, so
  # it must expand to nothing (and non-static linkage is required anyway
  # for these split, relocatable-device-code translation units to link)
  target_compile_definitions(KQED_cuda PRIVATE KQED_PRIVATE=)
  # io.c's cudaMalloc(&Grid->TX, ...) calls implicitly convert double** to
  # void**, legal in the C KQED was written against; GCC 14 made that a
  # hard error by default, so relax it back to a warning here.
  target_compile_options(KQED_cuda PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wno-error=incompatible-pointer-types>)
  target_link_libraries(KQED_cuda PUBLIC CUDA::cudart CUDA::cuda_driver)
  set_target_properties(KQED_cuda PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
    POSITION_INDEPENDENT_CODE ON)
endif ()
