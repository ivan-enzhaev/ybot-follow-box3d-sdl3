cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

project(ybot-follow-box3d-sdl3 CXX C)

set(CMAKE_OBJECT_PATH_MAX 512)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Locate System OpenGL & Threads Packages (Desktop only)
# -----------------------------------------------------------------------------
if(NOT ANDROID AND NOT EMSCRIPTEN)
    find_package(OpenGL REQUIRED)
    find_package(Threads REQUIRED)
endif()

# -----------------------------------------------------------------------------
# Platform Directory Configurations (SDL3, Physics, AI, Animation)
# -----------------------------------------------------------------------------
if(EMSCRIPTEN)
    set(SDL3_DIR "C:/libs/sdl3-3.4.16-wasm/lib/cmake/SDL3")
    set(SDL3_image_DIR "C:/libs/sdl3_image-3.4.6-wasm/lib/cmake/SDL3_image")
    set(SDL3_ttf_DIR "C:/libs/sdl3_ttf-3.2.2-wasm/lib/cmake/SDL3_ttf")

    # ZLIB 1.3.2 for WebAssembly
    set(ZLIB_DIR "C:/libs/zlib-1.3.2-wasm")
    set(ZLIB_ROOT "C:/libs/zlib-1.3.2-wasm")
    set(ENV{ZLIB_ROOT} "C:/libs/zlib-1.3.2-wasm")
    list(APPEND CMAKE_PREFIX_PATH "C:/libs/zlib-1.3.2-wasm")
    list(APPEND CMAKE_FIND_ROOT_PATH "C:/libs/zlib-1.3.2-wasm")

    # libpng 1.6.58 for WebAssembly
    set(PNG_DIR "C:/libs/libpng-1.6.58-wasm")
    set(PNG_ROOT "C:/libs/libpng-1.6.58-wasm")
    set(ENV{PNG_ROOT} "C:/libs/libpng-1.6.58-wasm")
    list(APPEND CMAKE_PREFIX_PATH "C:/libs/libpng-1.6.58-wasm")
    list(APPEND CMAKE_FIND_ROOT_PATH "C:/libs/libpng-1.6.58-wasm")

    # WebP 1.6.0 for WebAssembly
    set(WEBP_DIR "C:/libs/webp-1.6.0-wasm")
    set(ENV{WEBP_DIR} "C:/libs/webp-1.6.0-wasm")
    list(APPEND CMAKE_PREFIX_PATH "C:/libs/webp-1.6.0-wasm")
    list(APPEND CMAKE_FIND_ROOT_PATH "C:/libs/webp-1.6.0-wasm")

    # FreeType 2.14.3 for WebAssembly
    set(FREETYPE_DIR "C:/libs/freetype-2.14.3-wasm")
    set(ENV{FREETYPE_DIR} "C:/libs/freetype-2.14.3-wasm")
    list(APPEND CMAKE_PREFIX_PATH "C:/libs/freetype-2.14.3-wasm")
    list(APPEND CMAKE_FIND_ROOT_PATH "C:/libs/freetype-2.14.3-wasm")

    set(BOX3D_DIR "C:/libs/box3d-0.1.0-wasm")
    set(RECAST_DIR "C:/libs/recastnavigation-wasm")
    set(BTCPP_PATH "C:/libs/behaviortree-4.9.0-wasm")
    set(TINYXML2_PATH "C:/libs/tinyxml2-11.0.0-wasm")
    set(OZZ_DIR "C:/libs/ozz-animation-0.17.0-wasm")

    set(IMGUI_DIR "C:/libs/imgui-1.92.9b")
    set(CGLM_INCLUDE_DIR "C:/libs/cglm-0.9.6/include")
    set(CGLTF_INCLUDE_DIR "C:/libs/cgltf-1.15")
    set(GLM_INCLUDE_DIR "C:/libs/glm-1.0.3")

elseif(ANDROID)
    set(BOX3D_DIR "C:/libs/box3d-0.1.0-android/arm64-v8a")
    set(RECAST_DIR "C:/libs/recastnavigation-android/arm64-v8a")
    set(BTCPP_PATH "C:/libs/behaviortree-4.9.0-android/arm64-v8a")
    set(TINYXML2_PATH "C:/libs/tinyxml2-11.0.0-android/arm64-v8a")
    set(OZZ_DIR "C:/libs/ozz-animation-0.17.0-android/arm64-v8a")

    set(IMGUI_DIR "C:/libs/imgui-1.92.9b")
    set(CGLM_INCLUDE_DIR "C:/libs/cglm-0.9.6/include")
    set(CGLTF_INCLUDE_DIR "C:/libs/cgltf-1.15")
    set(GLM_INCLUDE_DIR "C:/libs/glm-1.0.3")

elseif(WIN32)
    set(SDL3_DIR "C:/libs/sdl3-3.4.16-mingw/lib/cmake/SDL3")
    set(SDL3_image_DIR "C:/libs/sdl3_image-3.4.6-webp-mingw/lib/cmake/SDL3_image")
    set(SDL3_ttf_DIR "C:/libs/sdl3_ttf-3.2.2-mingw/lib/cmake/SDL3_ttf")

    set(BOX3D_DIR "C:/libs/box3d-0.1.0-mingw")
    set(RECAST_DIR "C:/libs/recastnavigation-mingw")
    set(BTCPP_PATH "C:/libs/behaviortree-4.9.0-mingw")
    set(TINYXML2_PATH "C:/libs/tinyxml2-11.0.0-mingw")
    set(OZZ_DIR "C:/libs/ozz-animation-0.17.0-mingw")

    set(FREETYPE_DIR "C:/libs/freetype-2.14.3-mingw")
    set(ENV{FREETYPE_DIR} "C:/libs/freetype-2.14.3-mingw")
    list(APPEND CMAKE_PREFIX_PATH "C:/libs/freetype-2.14.3-mingw")

    set(WEBP_DIR "C:/libs/webp-1.6.0-mingw")
    set(ENV{WEBP_DIR} "C:/libs/webp-1.6.0-mingw")
    list(APPEND CMAKE_PREFIX_PATH "C:/libs/webp-1.6.0-mingw")

    set(IMGUI_DIR "C:/libs/imgui-1.92.9b")
    set(CGLM_INCLUDE_DIR "C:/libs/cglm-0.9.6/include")
    set(CGLTF_INCLUDE_DIR "C:/libs/cgltf-1.15")
    set(GLAD_INCLUDE_DIR "C:/libs/glad-0.1.36/opengl-3.3/include")
    set(GLAD_SOURCE "C:/libs/glad-0.1.36/opengl-3.3/src/glad.c")
    set(GLM_INCLUDE_DIR "C:/libs/glm-1.0.3")

else()
    # Ubuntu / Linux Desktop
    set(SDL3_DIR "$ENV{HOME}/libs/sdl3-3.4.16-ubuntu/lib/cmake/SDL3")
    set(SDL3_image_DIR "$ENV{HOME}/libs/sdl3_image-3.4.6-webp-ubuntu/lib/cmake/SDL3_image")
    set(SDL3_ttf_DIR "$ENV{HOME}/libs/sdl3_ttf-3.2.2-ubuntu/lib/cmake/SDL3_ttf")

    set(BOX3D_DIR "$ENV{HOME}/libs/box3d-0.1.0-ubuntu")
    set(RECAST_DIR "$ENV{HOME}/libs/recastnavigation-ubuntu")
    set(BTCPP_PATH "$ENV{HOME}/libs/behaviortree-4.9.0-ubuntu")
    set(TINYXML2_PATH "$ENV{HOME}/libs/tinyxml2-11.0.0-ubuntu")
    set(OZZ_DIR "$ENV{HOME}/libs/ozz-animation-0.17.0-ubuntu")

    set(FREETYPE_DIR "$ENV{HOME}/libs/freetype-2.14.3-ubuntu")
    set(ENV{FREETYPE_DIR} "$ENV{HOME}/libs/freetype-2.14.3-ubuntu")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/libs/freetype-2.14.3-ubuntu")

    set(WEBP_DIR "$ENV{HOME}/libs/webp-1.6.0-ubuntu")
    set(ENV{WEBP_DIR} "$ENV{HOME}/libs/webp-1.6.0-ubuntu")
    list(APPEND CMAKE_PREFIX_PATH "$ENV{HOME}/libs/webp-1.6.0-ubuntu")

    set(IMGUI_DIR "$ENV{HOME}/libs/imgui-1.92.9b")
    set(CGLM_INCLUDE_DIR "$ENV{HOME}/libs/cglm-0.9.6/include")
    set(CGLTF_INCLUDE_DIR "$ENV{HOME}/libs/cgltf-1.15")
    set(GLAD_INCLUDE_DIR "$ENV{HOME}/libs/glad-0.1.36/opengl-3.3/include")
    set(GLAD_SOURCE "$ENV{HOME}/libs/glad-0.1.36/opengl-3.3/src/glad.c")
    set(GLM_INCLUDE_DIR "$ENV{HOME}/libs/glm-1.0.3")
endif()

# Pre-find ZLIB and PNG for Wasm so find_dependency in SDL3_imageConfig succeeds
if(EMSCRIPTEN)
    find_package(ZLIB REQUIRED)
    find_package(PNG REQUIRED)
endif()

find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_image REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

# -----------------------------------------------------------------------------
# Static Imported Targets for AI, Nav, and Physics
# -----------------------------------------------------------------------------
add_library(Box3D_Static STATIC IMPORTED)
set_target_properties(Box3D_Static PROPERTIES IMPORTED_LOCATION "${BOX3D_DIR}/lib/libbox3d.a")

# Only declare static targets for Desktop / Wasm (Android uses .so directly)
if(NOT ANDROID)
    add_library(Recast_Static STATIC IMPORTED)
    set_target_properties(Recast_Static PROPERTIES IMPORTED_LOCATION "${RECAST_DIR}/lib/libRecast.a")

    add_library(Detour_Static STATIC IMPORTED)
    set_target_properties(Detour_Static PROPERTIES IMPORTED_LOCATION "${RECAST_DIR}/lib/libDetour.a")

    add_library(DetourCrowd_Static STATIC IMPORTED)
    set_target_properties(DetourCrowd_Static PROPERTIES IMPORTED_LOCATION "${RECAST_DIR}/lib/libDetourCrowd.a")

    add_library(tinyxml2 STATIC IMPORTED)
    set_target_properties(tinyxml2 PROPERTIES
        IMPORTED_LOCATION "${TINYXML2_PATH}/lib/libtinyxml2.a"
        INTERFACE_INCLUDE_DIRECTORIES "${TINYXML2_PATH}/include"
    )

    add_library(behaviortree_cpp STATIC IMPORTED)
    set_target_properties(behaviortree_cpp PROPERTIES
        IMPORTED_LOCATION "${BTCPP_PATH}/lib/libbehaviortree_cpp.a"
        INTERFACE_INCLUDE_DIRECTORIES "${BTCPP_PATH}/include"
        INTERFACE_LINK_LIBRARIES "tinyxml2"
    )
endif()

# -----------------------------------------------------------------------------
# Target Creation (Shared library for Android .so, Executable for Desktop/Wasm)
# -----------------------------------------------------------------------------
if(ANDROID)
    add_library(app SHARED)
else()
    add_executable(app)
endif()

target_sources(app
PRIVATE
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    src/anim_player.h
    src/app.h
    src/enemy_ai.h
    src/enemy_ai.cpp
    src/file_utils.h
    src/file_utils.c
    src/main.cpp
    src/model_loader.h
    src/msaa_fbo.h
    src/ozz_sdl_stream.h
    src/physics_debug.h
    src/physics_debug.cpp
    src/physics_setup.h
    src/projection_utils.h
    src/scene_renderer.h
    src/shader_program.h
    src/shader_program.c
    src/texture_utils.h
    src/ui_overlay.h
)

target_include_directories(app PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${CGLM_INCLUDE_DIR}
    ${GLM_INCLUDE_DIR}
    ${CGLTF_INCLUDE_DIR}
    ${OZZ_DIR}/include
    ${BOX3D_DIR}/include
    ${RECAST_DIR}/include
    ${RECAST_DIR}/include/recastnavigation
    ${BTCPP_PATH}/include
)

# Desktop OpenGL loader (GLAD only on Windows/Linux, mobile/wasm use <GLES3/gl3.h>)
if(NOT ANDROID AND NOT EMSCRIPTEN)
    target_include_directories(app PRIVATE ${GLAD_INCLUDE_DIR})
    target_sources(app PRIVATE ${GLAD_SOURCE})
endif()

# -----------------------------------------------------------------------------
# Platform-Specific Compilation & Link Flags
# -----------------------------------------------------------------------------
if(EMSCRIPTEN)
    target_link_options(app PRIVATE 
        "-Oz"
        "-sMALLOC=emmalloc"
        "-sUSE_SDL=0"
        "-sMAX_WEBGL_VERSION=2"
        "-sMIN_WEBGL_VERSION=2"
        "-sFULL_ES3=1"
        "-sALLOW_MEMORY_GROWTH=1"
        "SHELL:--embed-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@/assets"
    )
elseif(WIN32)
    target_compile_options(app PRIVATE -O2)
    target_link_options(app PRIVATE -static -static-libgcc -static-libstdc++)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(app PRIVATE -mconsole)
    else()
        target_link_options(app PRIVATE -mwindows -s)
    endif()

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
        add_custom_command(TARGET app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:app>/assets"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                    "$<TARGET_FILE_DIR:app>/assets"
            COMMENT "Copying assets directory"
        )
    endif()
else()
    # Ubuntu / Linux Desktop
    target_compile_options(app PRIVATE -O2)
endif()

# -----------------------------------------------------------------------------
# Asset Copying (Windows & Linux Desktop)
# -----------------------------------------------------------------------------
if(NOT ANDROID AND NOT EMSCRIPTEN)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
        add_custom_command(TARGET app POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:app>/assets"
            COMMAND ${CMAKE_COMMAND} -E copy_directory
                    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
                    "$<TARGET_FILE_DIR:app>/assets"
            COMMENT "Copying assets directory"
        )
    endif()
endif()

# -----------------------------------------------------------------------------
# Link Libraries
# -----------------------------------------------------------------------------
if(EMSCRIPTEN)
    target_link_libraries(app PRIVATE
        SDL3_image::SDL3_image
        ${WEBP_DIR}/lib/libwebpmux.a
        ${WEBP_DIR}/lib/libwebpdemux.a
        ${WEBP_DIR}/lib/libwebp.a
        ${WEBP_DIR}/lib/libwebpdecoder.a
        ${WEBP_DIR}/lib/libsharpyuv.a
        SDL3_ttf::SDL3_ttf
        SDL3::SDL3

        # Ozz static libraries compiled for WebAssembly
        ${OZZ_DIR}/lib/libozz_animation_offline_r.a
        ${OZZ_DIR}/lib/libozz_animation_r.a
        ${OZZ_DIR}/lib/libozz_geometry_r.a
        ${OZZ_DIR}/lib/libozz_options_r.a
        ${OZZ_DIR}/lib/libozz_base_r.a

        Box3D_Static
        DetourCrowd_Static
        Detour_Static
        Recast_Static
        behaviortree_cpp
        tinyxml2
        PNG::PNG
        ZLIB::ZLIB
    )
elseif(ANDROID)
    target_link_libraries(app PRIVATE
        EGL
        GLESv3
        SDL3_image::SDL3_image
        SDL3_ttf::SDL3_ttf
        SDL3::SDL3

        # Ozz-animation shared libraries (.so)
        ${OZZ_DIR}/lib/libozz_animation.so
        ${OZZ_DIR}/lib/libozz_animation_offline.so
        ${OZZ_DIR}/lib/libozz_geometry.so
        ${OZZ_DIR}/lib/libozz_options.so
        ${OZZ_DIR}/lib/libozz_base.so

        # Recast Navigation shared libraries (.so)
        ${RECAST_DIR}/lib/libRecast.so
        ${RECAST_DIR}/lib/libDetour.so
        ${RECAST_DIR}/lib/libDetourCrowd.so

        # BehaviorTree.CPP shared library (.so)
        ${BTCPP_PATH}/lib/libbehaviortree_cpp.so

        # Box3D static library
        Box3D_Static

        log
    )
elseif(WIN32)
    target_link_libraries(app PRIVATE
        SDL3_image::SDL3_image-static
        ${WEBP_DIR}/lib/libwebpmux.a
        ${WEBP_DIR}/lib/libwebpdemux.a
        ${WEBP_DIR}/lib/libwebp.a
        ${WEBP_DIR}/lib/libwebpdecoder.a
        ${WEBP_DIR}/lib/libsharpyuv.a
        SDL3_ttf::SDL3_ttf-static
        SDL3::SDL3-static

        ${OZZ_DIR}/lib/libozz_animation_offline_r.a
        ${OZZ_DIR}/lib/libozz_animation_r.a
        ${OZZ_DIR}/lib/libozz_geometry_r.a
        ${OZZ_DIR}/lib/libozz_options_r.a
        ${OZZ_DIR}/lib/libozz_base_r.a

        Box3D_Static
        DetourCrowd_Static
        Detour_Static
        Recast_Static
        behaviortree_cpp
        tinyxml2

        OpenGL::GL
        Threads::Threads
    )
else()
    # Ubuntu / Linux Desktop
    target_link_libraries(app PRIVATE
        SDL3_image::SDL3_image-static
        ${WEBP_DIR}/lib/libwebpmux.a
        ${WEBP_DIR}/lib/libwebpdemux.a
        ${WEBP_DIR}/lib/libwebp.a
        ${WEBP_DIR}/lib/libwebpdecoder.a
        ${WEBP_DIR}/lib/libsharpyuv.a
        SDL3_ttf::SDL3_ttf-static
        SDL3::SDL3-static

        ${OZZ_DIR}/lib/libozz_animation_offline_r.a
        ${OZZ_DIR}/lib/libozz_animation_r.a
        ${OZZ_DIR}/lib/libozz_geometry_r.a
        ${OZZ_DIR}/lib/libozz_options_r.a
        ${OZZ_DIR}/lib/libozz_base_r.a

        Box3D_Static
        DetourCrowd_Static
        Detour_Static
        Recast_Static
        behaviortree_cpp
        tinyxml2

        OpenGL::GL
        Threads::Threads
    )
endif()
