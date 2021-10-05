cmake_minimum_required(VERSION 3.15)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/midi_file)

add_library(Gwidi_Importer ${CMAKE_CURRENT_LIST_DIR}/GwidiData.cc)
target_link_libraries(Gwidi_Importer midifile)

get_target_property(midifile_includes midifile INCLUDE_DIRECTORIES)
target_include_directories(Gwidi_Importer PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${midifile_includes})
