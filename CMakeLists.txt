cmake_minimum_required(VERSION 3.15)
project(gwidi_importer)

add_subdirectory(external/midi_file)

set(CMAKE_CXX_STANDARD 20)

add_library(gwidi_importer GwidiData.cc)
target_link_libraries(gwidi_importer midifile)
get_target_property(midifile_includes midifile INCLUDE_DIRECTORIES)
target_include_directories(gwidi_importer PUBLIC ${midifile_includes})

add_executable(gwidi_importer_main main.cpp)
target_link_libraries(gwidi_importer_main gwidi_importer)



include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


enable_testing()

add_executable(
        test_gwidi
        tests/Test_GwidiData.cc
)
target_link_libraries(
        test_gwidi
        gwidi_importer
        gtest_main
)
target_include_directories(test_gwidi PUBLIC ${CMAKE_CURRENT_LIST_DIR})

include(GoogleTest)
gtest_discover_tests(test_gwidi)




# Copy <filename> to build directory
set(copy_source_dir "${CMAKE_SOURCE_DIR}/assets/test_data")
set(copy_dest_dir "${CMAKE_BINARY_DIR}/Build/test_data/$<CONFIG>")
set(copy_file_name "simple.mid")
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${copy_dest_dir}
)
add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${copy_source_dir}/${copy_file_name}" "${copy_dest_dir}/${copy_file_name}"
        COMMENT "Copying ${copy_file_name} to build directory"
)