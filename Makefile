# KungFu Chess - MSVC build (NMake)
#
# From a regular shell:
#   build.bat [target]
#
# From "x64 Native Tools Command Prompt for VS 2022":
#   nmake [target]

PYTHON = py -3
CXX = cl
LINK = link

CXXFLAGS = /nologo /std:c++17 /W4 /EHsc /I.
LDFLAGS = /nologo

OPENCV_DIR = cpp\OpenCV_451
OPENCV_INC = $(OPENCV_DIR)\include
OPENCV_LIB = $(OPENCV_DIR)\bin\opencv_world451.lib
OPENCV_DLL = $(OPENCV_DIR)\bin\opencv_world451.dll

MODEL_SRCS = model\Board.cpp model\GameSnapshot.cpp model\Piece.cpp model\Position.cpp
RULES_SRCS = rules\PieceRules.cpp rules\RuleEngine.cpp
REALTIME_SRCS = realtime\Motion.cpp realtime\RealTimeArbiter.cpp
ENGINE_SRCS = engine\GameEngine.cpp
INPUT_SRCS = input\BoardMapper.cpp input\Controller.cpp
IO_SRCS = io\BoardParser.cpp io\BoardPrinter.cpp
TEXTTEST_SRCS = texttests\ScriptRunner.cpp
VIEW_SRCS = view\ImageVIew.cpp view\Renderer.cpp \
            view\animation\AnimationConfig.cpp view\animation\PieceAnimator.cpp \
            view\animation\AnimatorRegistry.cpp

CORE_SRCS = $(MODEL_SRCS) $(RULES_SRCS) $(REALTIME_SRCS) $(ENGINE_SRCS) $(INPUT_SRCS) $(IO_SRCS)

MODEL_OBJS = model\Board.obj model\GameSnapshot.obj model\Piece.obj model\Position.obj
RULES_OBJS = rules\PieceRules.obj rules\RuleEngine.obj
REALTIME_OBJS = realtime\Motion.obj realtime\RealTimeArbiter.obj
ENGINE_OBJS = engine\GameEngine.obj
INPUT_OBJS = input\BoardMapper.obj input\Controller.obj
IO_OBJS = io\BoardParser.obj io\BoardPrinter.obj
TEXTTEST_OBJS = texttests\ScriptRunner.obj
VIEW_OBJS = view\ImageVIew.obj view\Renderer.obj \
            view\animation\AnimationConfig.obj view\animation\PieceAnimator.obj \
            view\animation\AnimatorRegistry.obj

CORE_OBJS = $(MODEL_OBJS) $(RULES_OBJS) $(REALTIME_OBJS) $(ENGINE_OBJS) $(INPUT_OBJS) $(IO_OBJS)

TEST_TARGETS = tests\test_piece_rules.exe tests\test_board_mapper.exe tests\test_board_parser.exe \
               tests\test_script_runner.exe tests\test_game_engine.exe tests\test_board.exe \
               tests\test_rule_engine.exe tests\test_piece_animator.exe

KUNGFU = kungfu.exe
KUNGFU_GUI = kungfu_gui.exe

ALL_OBJS = $(CORE_OBJS) $(TEXTTEST_OBJS) main.obj gui_main.obj $(VIEW_OBJS)

.SUFFIXES: .cpp .obj

.cpp.obj:
	$(CXX) $(CXXFLAGS) /c /Fo$@ $<

.PHONY: all clean test gui compile-db compile_commands.json

all: $(KUNGFU) $(TEST_TARGETS)

gui: $(KUNGFU_GUI)

$(KUNGFU): $(CORE_OBJS) $(TEXTTEST_OBJS) main.obj
	$(LINK) $(LDFLAGS) /OUT:$@ $(CORE_OBJS) $(TEXTTEST_OBJS) main.obj

$(KUNGFU_GUI): $(CORE_OBJS) $(VIEW_OBJS) gui_main.obj
	$(LINK) $(LDFLAGS) /OUT:$@ $(CORE_OBJS) $(VIEW_OBJS) gui_main.obj $(OPENCV_LIB)
	copy /Y $(OPENCV_DLL) . >nul

tests\test_piece_rules.exe: tests\test_piece_rules.cpp $(MODEL_SRCS) $(RULES_SRCS) $(IO_SRCS)
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_piece_rules.cpp $(MODEL_SRCS) $(RULES_SRCS) $(IO_SRCS)

tests\test_board_mapper.exe: tests\test_board_mapper.cpp $(MODEL_SRCS) input\BoardMapper.cpp
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_board_mapper.cpp $(MODEL_SRCS) input\BoardMapper.cpp

tests\test_board_parser.exe: tests\test_board_parser.cpp $(MODEL_SRCS) $(IO_SRCS)
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_board_parser.cpp $(MODEL_SRCS) $(IO_SRCS)

tests\test_script_runner.exe: tests\test_script_runner.cpp $(CORE_SRCS) $(TEXTTEST_SRCS)
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_script_runner.cpp $(CORE_SRCS) $(TEXTTEST_SRCS)

tests\test_game_engine.exe: tests\test_game_engine.cpp $(CORE_SRCS)
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_game_engine.cpp $(CORE_SRCS)

tests\test_board.exe: tests\test_board.cpp $(MODEL_SRCS) $(IO_SRCS)
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_board.cpp $(MODEL_SRCS) $(IO_SRCS)

tests\test_rule_engine.exe: tests\test_rule_engine.cpp $(MODEL_SRCS) $(RULES_SRCS) $(IO_SRCS)
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_rule_engine.cpp $(MODEL_SRCS) $(RULES_SRCS) $(IO_SRCS)

tests\test_piece_animator.exe: tests\test_piece_animator.cpp view\animation\AnimationConfig.cpp view\animation\PieceAnimator.cpp
	$(CXX) $(CXXFLAGS) /Fe$@ tests\test_piece_animator.cpp view\animation\AnimationConfig.cpp view\animation\PieceAnimator.cpp

view\ImageVIew.obj: view\ImageVIew.cpp
	$(CXX) $(CXXFLAGS) /I$(OPENCV_INC) /c /Fo$@ view\ImageVIew.cpp

view\Renderer.obj: view\Renderer.cpp
	$(CXX) $(CXXFLAGS) /I$(OPENCV_INC) /c /Fo$@ view\Renderer.cpp

gui_main.obj: gui_main.cpp
	$(CXX) $(CXXFLAGS) /I$(OPENCV_INC) /c /Fo$@ gui_main.cpp

test: $(TEST_TARGETS)
	@tests\test_piece_rules.exe
	@tests\test_board_mapper.exe
	@tests\test_board_parser.exe
	@tests\test_script_runner.exe
	@tests\test_game_engine.exe
	@tests\test_board.exe
	@tests\test_rule_engine.exe
	@tests\test_piece_animator.exe

compile-db: compile_commands.json

compile_commands.json: Makefile scripts\gen_compile_db.py
	@$(PYTHON) scripts\gen_compile_db.py

clean:
	-del /Q /F $(KUNGFU) $(KUNGFU_GUI) $(TEST_TARGETS) 2>nul
	-del /Q /F $(ALL_OBJS) 2>nul
	-for %%D in (model rules realtime engine input io texttests view tests) do @del /Q /F %%D\*.obj 2>nul
	-del /Q /F view\animation\*.obj 2>nul
