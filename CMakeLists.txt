cmake_minimum_required(VERSION 3.26)

project(cvc
        DESCRIPTION "CVC"
        LANGUAGES C CXX
        VERSION "0.0.1")

# include our cmake snippets
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(FetchContent)
# tmLQCDConfig.cmake calls find_dependency() without including this itself
include(CMakeFindDependencyMacro)

cmake_policy(SET CMP0048 NEW)

if(POLICY CMP0144)
  cmake_policy(SET CMP0144 NEW)
endif()

if(NOT DEFINED CMAKE_CUDA_STANDARD)
  set(CMAKE_CUDA_STANDARD 14)
  set(CMAKE_CUDA_STANDARD_REQUIRED ON)
endif()

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(NOT DEFINED CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED ON)
endif()

if(NOT DEFINED CMAKE_HIP_STANDARD)
  set(CMAKE_HIP_STANDARD 14)
  set(CMAKE_HIP_STANDARD_REQUIRED ON)
endif()

option(CVC_USE_CUDA "Enable CUDA support" ON)
option(CVC_USE_HIP "Enable HIP support" OFF)
option(CVC_USE_MPI "Enable MPI support" ON)
option(CVC_USE_QUDA "Enable QUDA support" ON)
option(CVC_USE_TMLQCD "Enable tmLQCD support" ON)
option(CVC_USE_OPENMP "Enable OpenMP support" ON)
option(CVC_USE_KQED "Enable KQED support" ON)
option(CVC_USE_LEMON "Enable lemon support" ON)
option(CVC_USE_LHPC_AFF "Enable LHPC AFF I/O support" ON)
option(CVC_USE_FFTW "Enable FFTW support" ON)
option(CVC_USE_HDF5 "Enable hdf5 support" ON)

find_package(PkgConfig)
if (CVC_USE_MPI)
find_package(MPI)
endif()

if (CVC_USE_CUDA)
  enable_language(CUDA)
  find_package(CUDAToolkit REQUIRED)
endif()

if (CVC_USE_HIP)
  enable_language(HIP)
endif()

if (CVC_USE_QUDA)
  find_package(QUDA)
endif()

if (CVC_USE_TMLQCD)
  find_package(tmLQCD)
endif()

if (CVC_USE_LEMON)
  FetchContent_Declare(
    lemon
    GIT_REPOSITORY https://github.com/etmc/lemon
    GIT_TAG e40cda10d18284795f840a04ecbb4d63007fee3d
    FIND_PACKAGE_ARGS NAMES lemon)
  FetchContent_MakeAvailable(lemon)
endif()

if (CVC_USE_OPENMP)
  find_package(OpenMP)
endif()

if (CVC_USE_KQED)
  # build KQED in-tree instead of requiring a pre-installed copy; deps/
  # holds the dedicated wrapper that fetches and builds the upstream sources
  set(KQED_USE_CUDA ${CVC_USE_CUDA} CACHE BOOL "" FORCE)
  add_subdirectory(deps)
endif()

if (CVC_USE_LHPC_AFF)
  # LHPC AFF has no CMake (or even autotools) build of its own, just a
  # hand-rolled Makefile; deps/aff wraps fetching and building it in-tree
  add_subdirectory(deps/aff)
endif()

if (CVC_USE_HDF5)
	find_package(HDF5 REQUIRED)
endif()

if (CVC_USE_FFTW)
  # FFTW3's own Config.cmake predates imported targets (just sets plain
  # FFTW3_* variables), and the mpi/threads variants have no CMake package
  # at all, so build our own imported targets via find_library.
  find_path(CVC_FFTW3_INCLUDE_DIR NAMES fftw3.h)
  find_library(CVC_FFTW3_LIBRARY NAMES fftw3)
  if (NOT CVC_FFTW3_INCLUDE_DIR OR NOT CVC_FFTW3_LIBRARY)
    message(FATAL_ERROR "FFTW3 was not found")
  endif()

  if (NOT TARGET cvc::fftw3)
    add_library(cvc::fftw3 INTERFACE IMPORTED)
    set_target_properties(cvc::fftw3 PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${CVC_FFTW3_INCLUDE_DIR}"
      INTERFACE_LINK_LIBRARIES "${CVC_FFTW3_LIBRARY}")
  endif()

  if (CVC_USE_MPI)
    find_library(CVC_FFTW3_MPI_LIBRARY NAMES fftw3_mpi)
    if (NOT CVC_FFTW3_MPI_LIBRARY)
      message(FATAL_ERROR "FFTW3 MPI support was not found")
    endif()
    if (NOT TARGET cvc::fftw3_mpi)
      add_library(cvc::fftw3_mpi INTERFACE IMPORTED)
      set_target_properties(cvc::fftw3_mpi PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CVC_FFTW3_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${CVC_FFTW3_MPI_LIBRARY};cvc::fftw3")
    endif()
  endif()

  if (CVC_USE_OPENMP)
    find_library(CVC_FFTW3_THREADS_LIBRARY NAMES fftw3_threads)
    if (NOT CVC_FFTW3_THREADS_LIBRARY)
      message(FATAL_ERROR "FFTW3 threads support was not found")
    endif()
    if (NOT TARGET cvc::fftw3_threads)
      add_library(cvc::fftw3_threads INTERFACE IMPORTED)
      set_target_properties(cvc::fftw3_threads PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CVC_FFTW3_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${CVC_FFTW3_THREADS_LIBRARY};cvc::fftw3")
    endif()
  endif()
endif()

# cvc calls complex LAPACK routines (zgemm, zgemv, zheev, zgesv, ...)
# directly via extern "C" declarations in iblas.h; never wired up before.
find_package(BLAS REQUIRED)
find_package(LAPACK REQUIRED)

find_package(FLEX REQUIRED)

LIST(APPEND CVC_SRC_CPP clover.cpp contract_cvc_tensor_all2all.cpp contract_cvc_tensor.cpp contract_cvc_tensor_eo_lm_factors.cpp contract_cvc_tensor_mixed.cpp contract_factorized.cpp contractions_io_3d.cpp contractions_io.cpp contract_loop.cpp cvc_geometry.cpp cvc_utils.cpp derived_quantities.cpp dml.cpp DML_crc32.cpp dummy_solver.cpp gamma.cpp gauge_io.cpp get_index.cpp getopt.cpp gluon_operators.cpp group_projection.cpp gsp.cpp gsp_recombine.cpp gsp_utils.cpp hyp_smear.cpp incomp_gamma.cpp io.cpp io_utils.cpp make_x_orbits.cpp matrix_init.cpp mpi_init.cpp prepare_propagator.cpp prepare_source.cpp project.cpp propagator_io.cpp Q_clover_phi.cpp Q_phi.cpp ranlxd.cpp ranlxs.cpp rotations.cpp scalar_products.cpp set_default.cpp smearing_techniques.cpp su3.cpp uwerr.cpp vdag_w_utils.cpp)

# global.h declares `extern const char *g_gitversion;`; the reference
# Makefile generates gitversion.cpp from the current git HEAD via a shell
# rule that reruns whenever HEAD/index change (hence it being in
# .gitignore), and links it directly into each PROGRAM rather than into the
# shared MODULES library. Replicate that with a custom command so it stays
# in sync without a full reconfigure.
find_package(Git REQUIRED)
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/gitversion.cpp"
  COMMAND ${CMAKE_COMMAND}
          -DGIT_EXECUTABLE=${GIT_EXECUTABLE}
          -DSRC_DIR=${CMAKE_SOURCE_DIR}
          -DOUT_FILE=${CMAKE_BINARY_DIR}/gitversion.cpp
          -P "${CMAKE_SOURCE_DIR}/cmake/GenerateGitVersion.cmake"
  DEPENDS "${CMAKE_SOURCE_DIR}/.git/HEAD" "${CMAKE_SOURCE_DIR}/.git/index"
  COMMENT "Generating gitversion.cpp from current git HEAD")

LIST(APPEND CVC_SRC_HLBL_KERNELS hlbl_kernels/global.cpp hlbl_kernels/kernels.cpp)
LIST(APPEND CVC_SRC_CUDA hlbl_kernels/kernels.cu)
LIST(APPEND CVC_SRC_PROGRAM apply_Dtm avgx23_invert_contract avgx_invert_contract avx23_conn_analyse avxg_analyse avxg_analyse_simple avxg_rho_analyse avxn_analyse avxn_conn_analyse calculate_gsp compact compact-block compact-block-3pt cpff_avx_fht_invert_contract cpff_fht_analyse cpff_fht_invert_contract cpff_invert_contract cpff_tbc_invert_contract cpff_threep_analyse cpff_xg_contract cpff_xg_contract_lowmem deflate disc_analyse eta_invert_contract example get_q2_list hlbl_2p2_invert_contract hlbl_invert_contract hlbl_mII_contract_test hlbl_mII_invert_contract hlbl_test_kqed hvp_analyse jj_invert_contract kaon2pt_invert_contract kaon2pt_simple_invert_contract lime2ascii lime2bin ll_caa_lma ll_lma ll_lma_recombine loop_analyse loop_extract loop_extract_plegma loop_invert_contract loops_caa_lma nn_2pt_analyse p2gg p2gg_analyse p2gg_analyse_conn p2gg_analyse_neutral p2gg_analyse_pdisc p2gg_analyse_vdisc p2gg_analyse_wdisc p2gg_caa_lma p2gg_contract p2gg_exdefl_analyse p2gg_exdefl_uwerr_analyse p2gg_factorized p2gg_invert_contract p2gg_invert_contract_local p2gg_lma p2gg_lmimp_analysis p2gg_local p2gg_mixed p2gg_noexdefl p2gg_noexdefl2 p2gg_pspace p2gg_test p2gg_tpspace p2gg_twop_invert_contract_local p2gg_xm p2gg_xspace p2gg_xspace_ama p2gg_xspace_local pppo_invert_contract pv2pt_simple_invert_contract sff_threep_analyse soet_invert_contract stout_smearing_radius test test_gm test_h5_parallel test_invert test_xchanger twop_analyse twop_analyse_wdisc twop_invert_contract twopt_extract twopt_invert_contract twopt_plegma_extract vv2pt_simple_invert_contract xg_analyse xg_exdefl_analyse)

include_directories(
  $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# cmake 4.0 uses a different syntax for the option
# -t tells flex to write to stdout, which conflicts with flex_target's own
# -o output redirection (the reference Makefile uses -t only because it
# pipes stdout to the target file itself via shell redirection, which
# flex_target does not); omit it here so the generated file actually lands
# at the OUTPUT path.
if(CMAKE_MAJOR_VERSION LESS 4)
  flex_target(cvc_input_read read_input_parser_cvc.l ${CMAKE_BINARY_DIR}/read_input_parser_cvc.cpp
    COMPILE_FLAGS "-P cvc_ -i")
else()
  flex_target(cvc_input_read read_input_parser_cvc.l ${CMAKE_BINARY_DIR}/read_input_parser_cvc.cpp
    OPTIONS "-P;cvc_;-i")
endif()


if (CVC_USE_CUDA)
  LIST(APPEND  CVC_SRC_ALL "${CVC_SRC_HLBL_KERNELS};${CVC_SRC_CUDA};${CVC_SRC_CPP}")
else()
  LIST(APPEND CVC_SRC_ALL "${CVC_SRC_HLBL_KERNELS};${CVC_SRC_CPP}")
endif()
if(BUILD_SHARED_LIBS)
  add_library(cvc SHARED "${CVC_SRC_ALL};${FLEX_cvc_input_read_OUTPUTS}")
else()
  add_library(cvc STATIC "${CVC_SRC_ALL};${FLEX_cvc_input_read_OUTPUTS}")
endif()

# define a library and add the dependencies
target_link_libraries(cvc
  PUBLIC
  $<$<BOOL:${CVC_USE_CUDA}>:CUDA::cufftw
  CUDA::cublas
  CUDA::cudart
  CUDA::cuda_driver>
  $<$<BOOL:${CVC_USE_TMLQCD}>:tmlqcd::tmlqcd>
  $<$<BOOL:${CVC_USE_LEMON}>:lemon::lemon>
  $<$<BOOL:${CVC_USE_HDF5}>:hdf5::hdf5>
  $<$<BOOL:${CVC_USE_MPI}>:MPI::MPI_C
  MPI::MPI_CXX>
  $<$<BOOL:${CVC_USE_OPENMP}>:OpenMP::OpenMP_C
  OpenMP::OpenMP_CXX>
  $<$<BOOL:${CVC_USE_KQED}>:KQED>
  $<$<AND:$<BOOL:${CVC_USE_KQED}>,$<BOOL:${CVC_USE_CUDA}>>:KQED_cuda>
  $<$<BOOL:${CVC_USE_LHPC_AFF}>:lhpc-aff>
  $<$<BOOL:${CVC_USE_FFTW}>:cvc::fftw3>
  $<$<AND:$<BOOL:${CVC_USE_FFTW}>,$<BOOL:${CVC_USE_MPI}>>:cvc::fftw3_mpi>
  $<$<AND:$<BOOL:${CVC_USE_FFTW}>,$<BOOL:${CVC_USE_OPENMP}>>:cvc::fftw3_threads>
  LAPACK::LAPACK
  BLAS::BLAS
  m)

target_compile_definitions(
  # iblas.h mangles the LAPACK/BLAS symbol names it declares (zgemm, zgemv,
  # zheev, zgesv, ...) as `s##_` only when F_ is defined; every reference
  # Makefile variant always passes -DF_ to match the trailing-underscore
  # Fortran ABI that OpenBLAS/reference LAPACK use, but the CMake port never
  # carried it over, leaving those symbols unresolved at link time.
  cvc PUBLIC F_
  $<$<BOOL:${CVC_USE_MPI}>:HAVE_MPI PARALLELTXYZ>
  $<$<BOOL:${CVC_USE_TMLQCD}>:PARALLELTXYZ HAVE_TMLQCD_LIBWRAPPER TM_USE_MPI>
  $<$<BOOL:${CVC_USE_LEMON}>:HAVE_LIBLEMON>
  $<$<BOOL:${CVC_USE_LHPC_AFF}>:HAVE_LHPC_AFF>
  $<$<BOOL:${CVC_USE_OPENMP}>:HAVE_OPENMP>
  $<$<BOOL:${CVC_USE_HDF5}>:HAVE_HDF5>
  $<$<BOOL:${CVC_USE_QUDA}>:TM_USE_QUDA>
  $<$<BOOL:${CVC_USE_CUDA}>:GPU_DIRECT_SOLVER>
  $<$<BOOL:${CVC_USE_KQED}>:HAVE_KQED>
  $<$<BOOL:${CVC_USE_HDF5}>:HAVE_HDF5>
)

set_target_properties(cvc PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION 1)

if (CVC_USE_CUDA)
  # cvc's own hlbl_kernels/kernels.cu calls device functions defined in
  # KQED_cuda's separately-compiled relocatable device code (-rdc=true);
  # matching CUDA_SEPARABLE_COMPILATION here is required for those
  # cross-translation-unit device calls to resolve at the final device
  # link step, which happens in the consuming executables below.
  set_target_properties(cvc PROPERTIES CUDA_SEPARABLE_COMPILATION ON)
endif()

foreach(_prog ${CVC_SRC_PROGRAM})
  add_executable(${_prog} "${_prog}.cpp" "${CMAKE_BINARY_DIR}/gitversion.cpp")

  target_link_libraries(${_prog} PUBLIC cvc)
  set_target_properties(
    ${_prog}
    PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    POSITION_INDEPENDENT_CODE ON
    LINKER_LANGUAGE "CXX")
  if (CVC_USE_CUDA)
    set_target_properties(${_prog} PROPERTIES
      CUDA_SEPARABLE_COMPILATION ON
      CUDA_RESOLVE_DEVICE_SYMBOLS ON)
  endif()
endforeach()

