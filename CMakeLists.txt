cmake_minimum_required(VERSION 3.16)
project(KungFuChess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /EHsc)
endif()

include(FetchContent)

set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)

set(IXWEBSOCKET_INSTALL OFF CACHE BOOL "" FORCE)
set(USE_ZLIB OFF CACHE BOOL "" FORCE)
set(USE_TLS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    ixwebsocket
    GIT_REPOSITORY https://github.com/machinezone/ixwebsocket.git
    GIT_TAG v11.4.5
)

FetchContent_MakeAvailable(nlohmann_json ixwebsocket)

FetchContent_Declare(
    sqlite3_amalgamation
    GIT_REPOSITORY https://github.com/azadkuh/sqlite-amalgamation.git
    GIT_TAG master
)
FetchContent_MakeAvailable(sqlite3_amalgamation)

add_library(sqlite3 STATIC ${sqlite3_amalgamation_SOURCE_DIR}/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${sqlite3_amalgamation_SOURCE_DIR})
if(MSVC)
    target_compile_options(sqlite3 PRIVATE /W0)
endif()

set(KUNGFU_LOGIC_DIR ${CMAKE_SOURCE_DIR}/src/logic)
set(KUNGFU_UI_DIR ${CMAKE_SOURCE_DIR}/src/ui)
set(KUNGFU_SERVER_DIR ${CMAKE_SOURCE_DIR}/src/server)
set(KUNGFU_TESTS_DIR ${CMAKE_SOURCE_DIR}/src/tests)
set(KUNGFU_LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

set(MODEL_SRCS
    src/logic/model/Board.cpp
    src/logic/model/GameSnapshot.cpp
    src/logic/model/GameStats.cpp
    src/logic/model/Piece.cpp
    src/logic/model/Position.cpp
)
set(RULES_SRCS
    src/logic/rules/PieceRules.cpp
    src/logic/rules/RuleEngine.cpp
)
set(REALTIME_SRCS
    src/logic/realtime/Motion.cpp
    src/logic/realtime/RealTimeArbiter.cpp
)
set(ENGINE_SRCS src/logic/engine/GameEngine.cpp)
set(INPUT_SRCS
    src/logic/input/BoardMapper.cpp
    src/logic/input/Controller.cpp
)
set(IO_SRCS
    src/logic/io/BoardParser.cpp
    src/logic/io/BoardPrinter.cpp
)
set(NETWORK_SHARED_SRCS
    src/logic/network/JsonCodec.cpp
    src/logic/network/MatchmakingQueue.cpp
)
set(NETWORK_SERVER_SRCS
    ${NETWORK_SHARED_SRCS}
    src/server/network/Lobby.cpp
    src/server/network/NetworkBridge.cpp
    src/server/network/WebSocketServer.cpp
    src/server/storage/EloRating.cpp
    src/server/storage/PasswordHasher.cpp
    src/server/storage/UserDatabase.cpp
)
set(NETWORK_CLIENT_SRCS
    ${NETWORK_SHARED_SRCS}
    src/ui/network/RemoteGameSession.cpp
    src/ui/network/WebSocketClient.cpp
)
set(SHELL_SRCS src/ui/shell/HomeScreen.cpp)
set(TEXTTEST_SRCS src/logic/texttests/ScriptRunner.cpp)
set(VIEW_SRCS
    src/ui/view/ImageVIew.cpp
    src/ui/view/Renderer.cpp
    lib/CTD26/cpp/src/img.cpp
    src/ui/view/animation/AnimationConfig.cpp
    src/ui/view/animation/PieceAnimator.cpp
    src/ui/view/animation/AnimatorRegistry.cpp
)
set(CORE_SRCS
    ${MODEL_SRCS}
    ${RULES_SRCS}
    ${REALTIME_SRCS}
    ${ENGINE_SRCS}
    ${INPUT_SRCS}
    ${IO_SRCS}
)

set(OPENCV_DIR ${KUNGFU_LIB_DIR}/OpenCV_451)
set(OPENCV_INC ${OPENCV_DIR}/include)
set(OPENCV_LIB ${OPENCV_DIR}/bin/opencv_world451.lib)
set(OPENCV_DLL ${OPENCV_DIR}/bin/opencv_world451.dll)
set(CTD26_IMG_INC ${KUNGFU_LIB_DIR}/CTD26/cpp/src)

function(kungfu_target_include target)
    target_include_directories(${target} PRIVATE
        ${KUNGFU_LOGIC_DIR}
        ${KUNGFU_UI_DIR}
        ${KUNGFU_SERVER_DIR}
    )
endfunction()

function(kungfu_target_include_tests target)
    kungfu_target_include(${target})
    target_include_directories(${target} PRIVATE ${KUNGFU_TESTS_DIR})
endfunction()

function(kungfu_link_opencv target)
    target_include_directories(${target} PRIVATE ${OPENCV_INC} ${CTD26_IMG_INC})
    target_link_libraries(${target} PRIVATE ${OPENCV_LIB})
    if(EXISTS ${OPENCV_DLL})
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${OPENCV_DLL} $<TARGET_FILE_DIR:${target}>
        )
    endif()
endfunction()

function(kungfu_link_network target)
    target_link_libraries(${target} PRIVATE ixwebsocket nlohmann_json::nlohmann_json)
    if(WIN32)
        target_link_libraries(${target} PRIVATE ws2_32)
    endif()
endfunction()

function(kungfu_link_storage target)
    target_link_libraries(${target} PRIVATE sqlite3)
endfunction()

function(kungfu_add_test name test_src)
    add_executable(${name} ${test_src} ${ARGN})
    kungfu_target_include_tests(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_executable(kungfu src/ui/main.cpp ${CORE_SRCS} ${TEXTTEST_SRCS} ${SHELL_SRCS} ${NETWORK_CLIENT_SRCS})
kungfu_target_include(kungfu)
kungfu_link_network(kungfu)

add_executable(kungfu_gui
    src/ui/gui_main.cpp
    ${MODEL_SRCS}
    src/logic/input/BoardMapper.cpp
    src/ui/input/RemoteController.cpp
    ${NETWORK_CLIENT_SRCS}
    ${SHELL_SRCS}
    ${VIEW_SRCS}
)
kungfu_target_include(kungfu_gui)
kungfu_link_opencv(kungfu_gui)
kungfu_link_network(kungfu_gui)

add_executable(kungfu_server src/server/server_main.cpp ${CORE_SRCS} ${NETWORK_SERVER_SRCS})
kungfu_target_include(kungfu_server)
kungfu_link_network(kungfu_server)
kungfu_link_storage(kungfu_server)

enable_testing()

kungfu_add_test(test_piece_rules src/tests/test_piece_rules.cpp
    ${MODEL_SRCS} ${RULES_SRCS} ${IO_SRCS})
kungfu_add_test(test_board_mapper src/tests/test_board_mapper.cpp
    ${MODEL_SRCS} src/logic/input/BoardMapper.cpp)
kungfu_add_test(test_board_parser src/tests/test_board_parser.cpp
    ${MODEL_SRCS} ${IO_SRCS})
kungfu_add_test(test_script_runner src/tests/test_script_runner.cpp
    ${CORE_SRCS} ${TEXTTEST_SRCS})
kungfu_add_test(test_game_engine src/tests/test_game_engine.cpp ${CORE_SRCS})
kungfu_add_test(test_event_bus src/tests/test_event_bus.cpp ${CORE_SRCS})
kungfu_add_test(test_home_screen src/tests/test_home_screen.cpp
    ${SHELL_SRCS} ${NETWORK_CLIENT_SRCS} src/logic/model/Position.cpp src/logic/model/GameStats.cpp)
kungfu_link_network(test_home_screen)
kungfu_add_test(test_lobby src/tests/test_lobby.cpp src/server/network/Lobby.cpp)
kungfu_add_test(test_matchmaking src/tests/test_matchmaking.cpp src/logic/network/MatchmakingQueue.cpp)
kungfu_add_test(test_elo_rating src/tests/test_elo_rating.cpp src/server/storage/EloRating.cpp)
kungfu_add_test(test_user_database src/tests/test_user_database.cpp
    src/server/storage/UserDatabase.cpp src/server/storage/PasswordHasher.cpp src/server/storage/EloRating.cpp)
kungfu_link_storage(test_user_database)
kungfu_add_test(test_json_codec src/tests/test_json_codec.cpp
    ${MODEL_SRCS} ${RULES_SRCS} ${REALTIME_SRCS} ${ENGINE_SRCS} ${IO_SRCS} src/logic/network/JsonCodec.cpp)
target_link_libraries(test_json_codec PRIVATE nlohmann_json::nlohmann_json)
kungfu_add_test(test_board src/tests/test_board.cpp ${MODEL_SRCS} ${IO_SRCS})
kungfu_add_test(test_rule_engine src/tests/test_rule_engine.cpp
    ${MODEL_SRCS} ${RULES_SRCS} ${IO_SRCS})
kungfu_add_test(test_piece_animator src/tests/test_piece_animator.cpp
    src/ui/view/animation/AnimationConfig.cpp src/ui/view/animation/PieceAnimator.cpp)
