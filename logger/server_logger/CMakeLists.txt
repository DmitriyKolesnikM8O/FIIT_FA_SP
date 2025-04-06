# add_subdirectory(tests)
add_subdirectory(tests)

add_library(
        mp_os_lggr_srvr_lggr
        src/server_logger.cpp
        src/server_logger_builder.cpp)

target_include_directories(
        mp_os_lggr_srvr_lggr
        PUBLIC
        ./include
        ${CMAKE_SOURCE_DIR}/external)
target_link_libraries(
        mp_os_lggr_srvr_lggr
        PUBLIC
        mp_os_cmmn)
target_link_libraries(
        mp_os_lggr_srvr_lggr
        PUBLIC
        mp_os_lggr_lggr)
target_link_libraries(
        mp_os_lggr_srvr_lggr
        PUBLIC
        nlohmann_json::nlohmann_json)
# Удаляем линковку с httplib, так как это header-only библиотека
# target_link_libraries(
#        mp_os_lggr_srvr_lggr
#        PUBLIC
#        httplib)
