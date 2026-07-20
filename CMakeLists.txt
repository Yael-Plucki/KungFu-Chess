cmake_minimum_required(VERSION 3.16)
project(KungFuChess LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /EHsc)
endif()

set(MODEL_SRCS
    model/Board.cpp
    model/GameSnapshot.cpp
    model/GameStats.cpp
    model/Piece.cpp
    model/Position.cpp
)
set(RULES_SRCS
    rules/PieceRules.cpp
    rules/RuleEngine.cpp
)
set(REALTIME_SRCS
    realtime/Motion.cpp
    realtime/RealTimeArbiter.cpp
)
set(ENGINE_SRCS engine/GameEngine.cpp)
set(INPUT_SRCS
    input/BoardMapper.cpp
    input/Controller.cpp
)
set(IO_SRCS
    io/BoardParser.cpp
    io/BoardPrinter.cpp
)
set(TEXTTEST_SRCS texttests/ScriptRunner.cpp)
set(VIEW_SRCS
    view/ImageVIew.cpp
    view/Renderer.cpp
    CTD26/cpp/src/img.cpp
    view/animation/AnimationConfig.cpp
    view/animation/PieceAnimator.cpp
    view/animation/AnimatorRegistry.cpp
)
set(CORE_SRCS
    ${MODEL_SRCS}
    ${RULES_SRCS}
    ${REALTIME_SRCS}
    ${ENGINE_SRCS}
    ${INPUT_SRCS}
    ${IO_SRCS}
)

set(OPENCV_DIR ${CMAKE_SOURCE_DIR}/OpenCV_451)
set(OPENCV_INC ${OPENCV_DIR}/include)
set(OPENCV_LIB ${OPENCV_DIR}/bin/opencv_world451.lib)
set(OPENCV_DLL ${OPENCV_DIR}/bin/opencv_world451.dll)

function(kungfu_target_include target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})
endfunction()

function(kungfu_link_opencv target)
    target_include_directories(${target} PRIVATE ${OPENCV_INC})
    target_link_libraries(${target} PRIVATE ${OPENCV_LIB})
    if(EXISTS ${OPENCV_DLL})
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${OPENCV_DLL} $<TARGET_FILE_DIR:${target}>
        )
    endif()
endfunction()

function(kungfu_add_test name test_src)
    add_executable(${name} ${test_src} ${ARGN})
    kungfu_target_include(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_executable(kungfu main.cpp ${CORE_SRCS} ${TEXTTEST_SRCS})
kungfu_target_include(kungfu)

add_executable(kungfu_gui gui_main.cpp ${CORE_SRCS} ${VIEW_SRCS})
kungfu_target_include(kungfu_gui)
kungfu_link_opencv(kungfu_gui)

enable_testing()

kungfu_add_test(test_piece_rules tests/test_piece_rules.cpp
    ${MODEL_SRCS} ${RULES_SRCS} ${IO_SRCS})
kungfu_add_test(test_board_mapper tests/test_board_mapper.cpp
    ${MODEL_SRCS} input/BoardMapper.cpp)
kungfu_add_test(test_board_parser tests/test_board_parser.cpp
    ${MODEL_SRCS} ${IO_SRCS})
kungfu_add_test(test_script_runner tests/test_script_runner.cpp
    ${CORE_SRCS} ${TEXTTEST_SRCS})
kungfu_add_test(test_game_engine tests/test_game_engine.cpp ${CORE_SRCS})
kungfu_add_test(test_board tests/test_board.cpp ${MODEL_SRCS} ${IO_SRCS})
kungfu_add_test(test_rule_engine tests/test_rule_engine.cpp
    ${MODEL_SRCS} ${RULES_SRCS} ${IO_SRCS})
kungfu_add_test(test_piece_animator tests/test_piece_animator.cpp
    view/animation/AnimationConfig.cpp view/animation/PieceAnimator.cpp)
