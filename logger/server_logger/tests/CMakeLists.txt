add_executable(
        mp_os_lggr_srvr_lggr_tests
        server_logger_tests.cpp
        server.cpp  # Добавляем server.cpp
)

target_include_directories(
        mp_os_lggr_srvr_lggr_tests
        PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/crow-src/include
        ${CMAKE_SOURCE_DIR}/external)  # Добавляем путь к external директории с httplib.h

target_compile_definitions(
        mp_os_lggr_srvr_lggr_tests
        PRIVATE
        BOOST_ASIO_ENABLE_OLD_SERVICES
        BOOST_ALLOW_DEPRECATED_HEADERS)

target_link_libraries(
        mp_os_lggr_srvr_lggr_tests
        PRIVATE
        gtest_main
        crow)

target_link_libraries(
        mp_os_lggr_srvr_lggr_tests
        PRIVATE
        mp_os_lggr_srvr_lggr)

add_executable(
        serv_test
        server.cpp
        server.h
        server_test.cpp)

target_include_directories(
        serv_test
        PRIVATE
        ${CMAKE_BINARY_DIR}/_deps/crow-src/include)

target_compile_definitions(
        serv_test
        PRIVATE
        BOOST_ASIO_ENABLE_OLD_SERVICES
        BOOST_ALLOW_DEPRECATED_HEADERS)

target_link_libraries(
        serv_test
        PRIVATE
        gtest_main
        crow)

target_link_libraries(
        serv_test
        PRIVATE
        mp_os_lggr_srvr_lggr)