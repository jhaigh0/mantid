include(GenerateExportHeader)

set(SRC_FILES
    src/napi.cpp
    src/napi4.cpp
    src/napi5.cpp
    src/NexusClasses.cpp
    src/NeXusException.cpp
    src/NeXusFile.cpp
    src/nxstack.cpp
    src/stptok.cpp
)

set(INC_FILES
    inc/MantidLegacyNexus/NeXusFile_fwd.h
    inc/MantidLegacyNexus/napi.h
    inc/MantidLegacyNexus/napi4.h
    inc/MantidLegacyNexus/napi5.h
    inc/MantidLegacyNexus/napiconfig.h
    inc/MantidLegacyNexus/napi_internal.h
    inc/MantidLegacyNexus/NexusClasses.h
    inc/MantidLegacyNexus/NeXusException.hpp
    inc/MantidLegacyNexus/NeXusFile.hpp
    inc/MantidLegacyNexus/nx_stptok.h
    inc/MantidLegacyNexus/nxstack.h
)

# Add the target for this directory
add_library(LegacyNexus ${SRC_FILES} ${INC_FILES})
add_library(Mantid::LegacyNexus ALIAS LegacyNexus)

target_include_directories(
  LegacyNexus PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/Mantid>
)

# Set the name of the generated library
set_target_properties(LegacyNexus PROPERTIES OUTPUT_NAME MantidLegacyNexus COMPILE_DEFINITIONS IN_MANTID_NEXUSCPP)
# H5_BUILT_AS_DYNAMIC_LIB required https://github.com/conda-forge/hdf5-feedstock/issues/58
target_compile_definitions(LegacyNexus PUBLIC -DH5_BUILT_AS_DYNAMIC_LIB)
target_link_libraries(LegacyNexus PUBLIC ${HDF4_DF_LIBRARY} ${HDF4_MFHDF_LIBRARY} ${HDF5_LIBRARIES})

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set_target_properties(LegacyNexus PROPERTIES INSTALL_RPATH "@loader_path/../MacOS;@loader_path/../Frameworks")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set_target_properties(LegacyNexus PROPERTIES INSTALL_RPATH "\$ORIGIN/../${LIB_DIR}")
endif()

# Add to the 'Framework' group in VS
set_property(TARGET LegacyNexus PROPERTY FOLDER "MantidFramework")

# Auto-generate exports header
target_include_directories(
  LegacyNexus PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/Framework/LegacyNexus> $<INSTALL_INTERFACE:include/Mantid>
)
generate_mantid_export_header(LegacyNexus FALSE)

# ######### TESTS
set(TEST_PREFIX "LegacyNexusTest")

# #### HDF5 The test doesn't want to work under windows
add_executable(hdf4_test_hdf5 EXCLUDE_FROM_ALL test/napi_test.cpp test/napi_test_util.cpp test/napi_test_util.h)
target_link_libraries(hdf4_test_hdf5 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-C-HDF5-test" COMMAND hdf4_test_hdf5)

add_executable(
  hdf4_attra_test_hdf5 EXCLUDE_FROM_ALL test/napi_attra_test.cpp test/napi_test_util.cpp test/napi_test_util.h
)
target_link_libraries(hdf4_attra_test_hdf5 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-C-HDF5-attra-test" COMMAND hdf4_attra_test_hdf5)

add_executable(hdf4_test_cpp-hdf5 EXCLUDE_FROM_ALL test/napi_test_cpp.cpp test/napi_test_util.cpp test/napi_test_util.h)
target_link_libraries(hdf4_test_cpp-hdf5 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-Cpp-HDF5-test" COMMAND hdf4_test_cpp-hdf5)

# #### HDF4
add_executable(hdf4_test_hdf4 EXCLUDE_FROM_ALL test/napi_test.cpp test/napi_test_util.cpp test/napi_test_util.h)
target_link_libraries(hdf4_test_hdf4 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-C-HDF4-test" COMMAND hdf4_test_hdf4)

add_executable(
  hdf4_attra_test_hdf4 EXCLUDE_FROM_ALL test/napi_attra_test.cpp test/napi_test_util.cpp test/napi_test_util.h
)
target_link_libraries(hdf4_attra_test_hdf4 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-C-HDF4-attra-test" COMMAND hdf4_attra_test_hdf4)
set_tests_properties("${TEST_PREFIX}-C-HDF4-attra-test" PROPERTIES WILL_FAIL TRUE)

add_executable(hdf4_test_cpp-hdf4 EXCLUDE_FROM_ALL test/napi_test_cpp.cpp test/napi_test_util.cpp test/napi_test_util.h)
target_link_libraries(hdf4_test_cpp-hdf4 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-Cpp-HDF4-test" COMMAND hdf4_test_cpp-hdf4)

# #### test for unlimited dimensions
add_executable(hdf4_test_nxunlimited EXCLUDE_FROM_ALL test/test_nxunlimited.cpp)
target_link_libraries(hdf4_test_nxunlimited LegacyNexus)
add_test(NAME "${TEST_PREFIX}-C-test-nxunlimited" COMMAND hdf4_test_nxunlimited)

# #### leak tests

add_executable(hdf4_leak_test1 EXCLUDE_FROM_ALL test/leak_test1.cpp test/napi_test_util.cpp test/napi_test_util.h)
target_link_libraries(hdf4_leak_test1 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-C-leak-test-1" COMMAND hdf4_leak_test1)

add_executable(hdf4_leak_test2 EXCLUDE_FROM_ALL test/leak_test2.cpp)
target_link_libraries(hdf4_leak_test2 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-Cpp-leak-test-2" COMMAND hdf4_leak_test2)

add_executable(hdf4_leak_test3 EXCLUDE_FROM_ALL test/leak_test3.cpp)
target_link_libraries(hdf4_leak_test3 LegacyNexus)
add_test(NAME "${TEST_PREFIX}-Cpp-leak-test-3" COMMAND hdf4_leak_test3)

# #### phony target to drag in all tests
add_custom_target(
  "${TEST_PREFIX}"
  DEPENDS hdf4_test_hdf5
          hdf4_attra_test_hdf5
          hdf4_test_cpp-hdf5
          hdf4_test_hdf4
          hdf4_attra_test_hdf4
          hdf4_test_cpp-hdf4
          hdf4_test_nxunlimited
          hdf4_leak_test1
          hdf4_leak_test2
          hdf4_leak_test3
)

add_dependencies(FrameworkTests "${TEST_PREFIX}")

# ######### Installation settings
set(TARGET_EXPORT_NAME "MantidLegacyNexusTargets")
mtd_install_framework_lib(TARGETS LegacyNexus INSTALL_EXPORT_FILE EXPORT_NAME ${TARGET_EXPORT_NAME})
