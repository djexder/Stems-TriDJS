cmake_minimum_required(VERSION 3.18)
project(TriDJs_Separador_Stems VERSION 1.1.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure LibTorch prefix path
# Default: relative to source tree; override with -DTORCH_PATH=... or env TORCH_PATH
if(DEFINED TORCH_PATH)
  set(CMAKE_PREFIX_PATH "${TORCH_PATH}")
elseif(DEFINED ENV{TORCH_PATH})
  set(CMAKE_PREFIX_PATH "$ENV{TORCH_PATH}")
else()
  set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libtorch")
endif()
find_package(Torch REQUIRED)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/7.0.9.zip
)
FetchContent_MakeAvailable(juce)

juce_add_gui_app(TriDJs_Separador_Stems
    PRODUCT_NAME "TriDJs Stems"
    VERSION 1.1.3
    ICON_BIG "${CMAKE_CURRENT_SOURCE_DIR}/logo.png"
    ICON_SMALL "${CMAKE_CURRENT_SOURCE_DIR}/logo.png"
)

juce_generate_juce_header(TriDJs_Separador_Stems)

target_sources(TriDJs_Separador_Stems PRIVATE
    Source/AudioAnalyzer.cpp
    Source/AudioAnalyzer.h
    Source/Main.cpp
    Source/MainComponent.cpp
    Source/MainComponent.h
    Source/StemEngine.cpp
    Source/StemEngine.h
    Source/UpdateChecker.cpp
    Source/UpdateChecker.h
)

target_compile_options(TriDJs_Separador_Stems PRIVATE "${TORCH_CXX_FLAGS}" "/utf-8")

target_compile_definitions(TriDJs_Separador_Stems PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(TriDJs_Separador_Stems PRIVATE
    "${TORCH_LIBRARIES}"
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# Copy LibTorch runtime DLLs to output directory
add_custom_command(TARGET TriDJs_Separador_Stems POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_PREFIX_PATH}/lib/c10.dll"
        "${CMAKE_PREFIX_PATH}/lib/torch.dll"
        "${CMAKE_PREFIX_PATH}/lib/torch_cpu.dll"
        "${CMAKE_PREFIX_PATH}/lib/torch_global_deps.dll"
        "${CMAKE_PREFIX_PATH}/lib/asmjit.dll"
        "${CMAKE_PREFIX_PATH}/lib/fbgemm.dll"
        "${CMAKE_PREFIX_PATH}/lib/fbjni.dll"
        "${CMAKE_PREFIX_PATH}/lib/pytorch_jni.dll"
        "${CMAKE_PREFIX_PATH}/lib/uv.dll"
        "${CMAKE_PREFIX_PATH}/lib/libiomp5md.dll"
        "${CMAKE_PREFIX_PATH}/lib/libiompstubs5md.dll"
    $<TARGET_FILE_DIR:TriDJs_Separador_Stems>
)

# Copy Intel MKL runtime DLLs (skip missing files for portability)
set(MKL_DLL_NAMES
    mkl_avx.1.dll mkl_avx2.1.dll mkl_avx512.1.dll
    mkl_core.1.dll mkl_def.1.dll mkl_intel_thread.1.dll
    mkl_mc.1.dll mkl_mc3.1.dll mkl_msg.dll
    mkl_pgi_thread.1.dll mkl_rt.1.dll mkl_sequential.1.dll
    mkl_tbb_thread.1.dll
    mkl_vml_avx.1.dll mkl_vml_avx2.1.dll mkl_vml_avx512.1.dll
    mkl_vml_cmpt.1.dll mkl_vml_def.1.dll mkl_vml_mc.1.dll
    mkl_vml_mc2.1.dll mkl_vml_mc3.1.dll
)
set(MKL_DLLS "")
foreach(dll ${MKL_DLL_NAMES})
  set(dll_path "${CMAKE_SOURCE_DIR}/redist/${dll}")
  if(EXISTS "${dll_path}")
    list(APPEND MKL_DLLS "${dll_path}")
  else()
    message(STATUS "MKL DLL not found (optional): ${dll}")
  endif()
endforeach()
if(MKL_DLLS)
  add_custom_command(TARGET TriDJs_Separador_Stems POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_if_different ${MKL_DLLS} $<TARGET_FILE_DIR:TriDJs_Separador_Stems>
  )
endif()

# Copy AI model to resources output directory
add_custom_command(TARGET TriDJs_Separador_Stems POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:TriDJs_Separador_Stems>/resources"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/modelo/htdemucs_compilado.pt"
        "$<TARGET_FILE_DIR:TriDJs_Separador_Stems>/resources/htdemucs_compilado.pt"
)
