configure_file(googletest-download/CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/src/googletest-download" )
execute_process(COMMAND "${CMAKE_COMMAND}" --build .
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/src/googletest-download" )


# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This adds
# the following targets: gtest, gtest_main, gmock
# and gmock_main
add_subdirectory("${CMAKE_BINARY_DIR}/src/googletest-src"
                 "${CMAKE_BINARY_DIR}/src/googletest-build")
add_subdirectory(iptux)
add_subdirectory(main)
