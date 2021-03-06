cmake_minimum_required(VERSION 3.1)

project(matrix_structs CXX)

option(BUILD_TESTS "Build tests" OFF)
option(BUILD_DOC "Build documentation" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(LIBRARY_NAME matrix_structs)

find_package(Doxygen)

if(DOXYGEN_FOUND AND BUILD_DOC)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile.out)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    message("Doxygen build started")

    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
        -std=c++1z \
        -Wall \
        -Wextra \
        -Werror \
        -pipe \
        -pedantic \
        -Wunreachable-code")

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        execute_process(COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)

        if (GCC_VERSION VERSION_GREATER 4.9)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always" )
        endif()
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always" )
    endif()
endif()

file(DOWNLOAD
    "https://github.com/nlohmann/json/releases/download/v3.0.0/json.hpp" ${PROJECT_SOURCE_DIR}/deps/json.hpp
    EXPECTED_MD5 "513357d232a0185a3d8d0f3c665744fb")

file(DOWNLOAD
    "https://github.com/mpark/variant/releases/download/v1.3.0/variant.hpp" ${PROJECT_SOURCE_DIR}/deps/variant.hpp
    EXPECTED_MD5 "be0ce322cdd408e1b347b9f1d59ea67a")

include_directories(deps)
include_directories(include)

set(SRC
    src/events.cpp
    src/requests.cpp

    src/events/aliases.cpp
    src/events/avatar.cpp
    src/events/canonical_alias.cpp
    src/events/common.cpp
    src/events/create.cpp
    src/events/guest_access.cpp
    src/events/history_visibility.cpp
    src/events/join_rules.cpp
    src/events/member.cpp
    src/events/name.cpp
    src/events/pinned_events.cpp
    src/events/power_levels.cpp
    src/events/redaction.cpp
    src/events/topic.cpp

    src/events/messages/audio.cpp
    src/events/messages/emote.cpp
    src/events/messages/file.cpp
    src/events/messages/image.cpp
    src/events/messages/notice.cpp
    src/events/messages/text.cpp
    src/events/messages/video.cpp

    src/responses/common.cpp
    src/responses/login.cpp
    src/responses/messages.cpp
    src/responses/profile.cpp
    src/responses/sync.cpp
    src/responses/version.cpp)

add_library(${LIBRARY_NAME} ${SRC})

if (BUILD_TESTS)
    enable_testing()

    file(COPY fixtures DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    add_executable(identifiers tests/identifiers.cpp)
    target_link_libraries(identifiers ${LIBRARY_NAME} ${GTEST_BOTH_LIBRARIES})

    add_executable(events tests/events.cpp)
    target_link_libraries(events ${LIBRARY_NAME} ${GTEST_BOTH_LIBRARIES})

    add_executable(messages tests/messages.cpp)
    target_link_libraries(messages ${LIBRARY_NAME} ${GTEST_BOTH_LIBRARIES})

    add_executable(responses tests/responses.cpp)
    target_link_libraries(responses ${LIBRARY_NAME} ${GTEST_BOTH_LIBRARIES})

    add_test(MatrixIdentifiers identifiers)
    add_test(StateEvents events)
    add_test(RoomEvents messages)
    add_test(Responses responses)
endif()
