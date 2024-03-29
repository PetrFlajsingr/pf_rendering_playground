cmake_minimum_required(VERSION 3.20)
project(pf_rendering_playground)

if (MSVC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.29.30129 AND CMAKE_VERSION VERSION_GREATER 3.20.3)
    set(CMAKE_CXX_STANDARD 23) # /std:c++latest - unlocks the non stable cpp20 features. For new 16.11 versions
else ()
    set(CMAKE_CXX_STANDARD 20) # /std:c++latest for msvc and -std=c++20 for everyone else.
endif ()

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    list(APPEND flags "/std:c++latest" "/W4" "/wd4201")
else ()
    list(APPEND flags "-fconcepts" "-fconcepts-diagnostics-depth=10" "-Werror=return-type" "-fcoroutines"
            "-Wall" "-Wextra" "-Werror" "-Wpedantic" "-Wno-unknown-pragmas" "-Wno-unused-function"
            "-Wpointer-arith" "-Wno-cast-qual" "-Wno-type-limits" "-fno-strict-aliasing" "-Wno-format-security")
endif ()

if (CMAKE_BUILD_TYPE MATCHES Debug)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        list(APPEND flags "/WX")
    endif ()
elseif (CMAKE_BUILD_TYPE MATCHES ASAN)
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=address")
elseif (CMAKE_BUILD_TYPE MATCHES LSAN)
    list(APPEND flags "-fno-omit-frame-pointer" "-fsanitize=leak")
elseif (CMAKE_BUILD_TYPE MATCHES UBSAN)
    list(APPEND flags "-fsanitize=undefined")
elseif (CMAKE_BUILD_TYPE MATCHES TSAN)
    list(APPEND flags "-fsanitize=thread")
endif ()


add_compile_definitions(GLFW_INCLUDE_NONE)
add_compile_definitions(GLM_FORCE_SWIZZLE)
add_compile_definitions(ASSERT_USE_MAGIC_ENUM)
#add_compile_definitions(FMT_HEADER_ONLY )
#add_compile_definitions(SPDLOG_FMT_EXTERNAL  )

find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(src)
include_directories(include/stb)
include_directories(include/libassert)

find_package(GLFW3 REQUIRED)

add_compile_definitions(PF_GLFW_OPENGL)

find_package(pf_common CONFIG REQUIRED)
find_package(magic_enum CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)
find_package(pf_glfw CONFIG REQUIRED)
find_package(pf_mainloop CONFIG REQUIRED)
find_package(pf_imgui CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)


find_package(glslang CONFIG REQUIRED)
add_library(glad src/glad.c)

set(UTILS_SRC
        src/utils/files.h
        src/utils/files.cpp
        src/utils/FPSCounter.cpp
        src/utils/glsl/glsl_typenames.h
        src/utils/glsl/GlslToSpirv.cpp
        src/utils/glsl/GlslToSpirv.h
        src/utils/algorithms.h
        src/utils/opengl_utils.h
        src/utils/stb_image_impl.cpp
        src/utils/profiling.h
        src/utils/ImageLoader.cpp
        src/utils/ImageLoader.h
        src/utils/glsl/DirStackFileIncluder.cpp
        src/utils/glsl/DirStackFileIncluder.h
        src/utils/glsl/DefaultBuiltinResource.h
        src/utils/logging.h
        )

set(GPU_INTERFACE_SRC
        src/gpu/Texture.cpp
        src/gpu/Texture.h
        src/gpu/GpuObject.h
        src/gpu/TextureTypes.h
        src/gpu/Shader.cpp
        src/gpu/Shader.h
        src/gpu/Program.cpp
        src/gpu/Program.h
        src/gpu/Types.h
        src/gpu/utils.h
        src/gpu/RenderThread.cpp
        src/gpu/RenderThread.h
        )

set(OPENGL_INTERFACE_SRC
        src/gpu/opengl/OpenGl.h
        src/gpu/opengl/Texture.cpp
        src/gpu/opengl/Texture.h
        src/gpu/opengl/Shader.cpp
        src/gpu/opengl/Shader.h
        src/gpu/opengl/Program.cpp
        src/gpu/opengl/Program.h
        src/gpu/opengl/RenderThread.cpp
        src/gpu/opengl/RenderThread.h
        )

set(SHADERTOY_SRC
        src/shader_toy/ShaderToyMode.cpp
        src/shader_toy/ShaderToyMode.h
        src/shader_toy/ShaderBuilder.cpp
        src/shader_toy/ShaderBuilder.h
        src/shader_toy/utils.h
        src/shader_toy/dialogs/GlslLVariableInputDialog.cpp
        src/shader_toy/dialogs/GlslLVariableInputDialog.h
        src/shader_toy/models/ImageAssetModel.cpp
        src/shader_toy/models/ImageAssetModel.h
        src/shader_toy/models/ShaderVariableModel.cpp
        src/shader_toy/models/ShaderVariableModel.h
        src/shader_toy/views/ImageAssetsView.cpp
        src/shader_toy/views/ImageAssetsView.h
        src/shader_toy/views/ShaderVariablesWindowView.cpp
        src/shader_toy/views/ShaderVariablesWindowView.h
        src/shader_toy/controllers/ShaderVariablesController.cpp
        src/shader_toy/controllers/ShaderVariablesController.h
        src/shader_toy/controllers/ImageAssetsController.cpp
        src/shader_toy/controllers/ImageAssetsController.h
        src/shader_toy/views/GlslEditorView.cpp
        src/shader_toy/views/GlslEditorView.h
        src/shader_toy/models/GlslEditorModel.cpp
        src/shader_toy/models/GlslEditorModel.h
        src/shader_toy/controllers/GlslEditorController.cpp
        src/shader_toy/controllers/GlslEditorController.h
        src/shader_toy/views/OutputView.cpp
        src/shader_toy/views/OutputView.h
        src/shader_toy/models/OutputModel.cpp
        src/shader_toy/models/OutputModel.h
        src/shader_toy/controllers/OutputController.cpp
        src/shader_toy/controllers/OutputController.h
        src/shader_toy/controllers/MainController.cpp
        src/shader_toy/controllers/MainController.h
        src/shader_toy/models/MainModel.h
        src/shader_toy/views/MainView.cpp
        src/shader_toy/views/MainView.h
        )


add_executable(${PROJECT_NAME}
        include/libassert/assert.cpp
        src/main.cpp
        src/imgui/ImGuiGlfwOpenGLInterface.cpp
        src/imgui/impl/imgui_impl_glfw.cpp
        src/imgui/impl/imgui_impl_opengl3.cpp
        src/modes/Mode.h
        src/modes/ModeManager.cpp
        src/modes/ModeManager.h
        src/log/UISink.h
        src/modes/Mode.cpp
        src/modes/DummyMode.h
        src/mvc/Controller.h
        src/mvc/View.h
        src/mvc/reactive.h
        src/common_ui/views/LogWindowView.cpp
        src/common_ui/views/LogWindowView.h
        src/common_ui/controllers/LogWindowController.cpp
        src/common_ui/controllers/LogWindowController.h
        src/common_ui/models/LogModel.h
        src/mvc/Model.h
        ${UTILS_SRC}
        ${GPU_INTERFACE_SRC}
        ${OPENGL_INTERFACE_SRC}
        ${SHADERTOY_SRC}
        )

target_compile_definitions(${PROJECT_NAME} PUBLIC NOGDI)
target_link_libraries(${PROJECT_NAME} glad
        ${OPENGL_LIBRARIES}
        pf_common::pf_common pf_imgui::pf_imgui pf_mainloop::pf_mainloop pf_glfw::pf_glfw
        spdlog::spdlog_header_only
        HLSL SPIRV glslang OGLCompiler
        $<$<CXX_COMPILER_ID:MSVC>:dbghelp>
        )

target_compile_options(${PROJECT_NAME} PRIVATE ${flags})
