
#-------------------------------
# SOURCE FILES CONFIGURATION
#-------------------------------

#set(UNITTESTS_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/unittests)

#-------------------------------
# GENERAL COMPILER CONFIGURATION
#-------------------------------

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

#----------------------------------
# BUILD INFORMATION
#----------------------------------

# Turn off -isystem gcc option that CMake uses for imported
# targets even when INTERFACE_INCLUDE_DIRECTORIES is used.
set(CMAKE_NO_SYSTEM_FROM_IMPORTED "1")

add_executable(client client.cpp)
target_link_libraries(client ${SYSTEMD_LIBRARIES})

add_executable(server server.cpp)
target_link_libraries(server ${SYSTEMD_LIBRARIES})

add_executable(both both.cpp)
target_link_libraries(both ${SYSTEMD_LIBRARIES})
