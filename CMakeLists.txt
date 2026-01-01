cmake_minimum_required(VERSION 3.15)
project(StylizedInstaller)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui source files
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
)

# Platform-specific backend files
if(WIN32)
    find_package(OpenGL REQUIRED)

    set(IMGUI_BACKEND_SOURCES
        ${IMGUI_DIR}/backends/imgui_impl_win32.cpp
        ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
    )

    # Enable resource compilation
    enable_language(RC)

    add_executable(${PROJECT_NAME} WIN32
        src/main.cpp
        ${IMGUI_SOURCES}
        ${IMGUI_BACKEND_SOURCES}
        resources.rc
    )

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${IMGUI_DIR}
        ${IMGUI_DIR}/backends
        ${OPENGL_INCLUDE_DIR}
        ${CMAKE_SOURCE_DIR}
    )

    target_link_libraries(${PROJECT_NAME}
        ${OPENGL_LIBRARIES}
        dwmapi
        imm32
    )

    # Copy OpenGL DLL if needed
    if(EXISTS "${CMAKE_SOURCE_DIR}/external/opengl32.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_SOURCE_DIR}/external/opengl32.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        )
    endif()
endif()

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
