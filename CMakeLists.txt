cmake_minimum_required(VERSION 3.6 FATAL_ERROR)
project(vulkan_engine CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (Clang only)." TRUE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics -fansi-escape-codes)
    endif()
endif()

set(CMAKE_CXX_CLANG_TIDY
	clang-tidy;
	-checks=-*,cppcoreguidelines-*,bugprone-*,performance-*,modernize-*,analyze-*,-modernize-use-trailing-return-type*;
	-p=std=c++20;
	--use-color)

# add log lib
add_subdirectory(external/fmt)
add_subdirectory(external/spdlog)

# Set directory paths
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/source)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(sources 
	${SOURCE_DIR}/vendor/vk_mem_alloc.cpp

	${SOURCE_DIR}/main.cpp

	${SOURCE_DIR}/window.cpp
	${SOURCE_DIR}/Application.cpp

	# renderer
	${SOURCE_DIR}/renderer/Instance.cpp
	${SOURCE_DIR}/renderer/Device.cpp
	${SOURCE_DIR}/renderer/Swapchain.cpp

	# renderer/graphics
	${SOURCE_DIR}/renderer/graphics/Shader.cpp
	${SOURCE_DIR}/renderer/graphics/GraphicsPipeline.cpp
	${SOURCE_DIR}/renderer/graphics/RenderPass.cpp
	${SOURCE_DIR}/renderer/graphics/Framebuffer.cpp
	${SOURCE_DIR}/renderer/graphics/Renderer.cpp

	# renderer/sync
	${SOURCE_DIR}/renderer/sync/CommandPool.cpp
	${SOURCE_DIR}/renderer/sync/CommandBuffer.cpp
	${SOURCE_DIR}/renderer/sync/Semaphore.cpp
	${SOURCE_DIR}/renderer/sync/Fence.cpp
)
add_executable(${PROJECT_NAME} ${sources})
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt spdlog::spdlog)
target_include_directories(${PROJECT_NAME} PUBLIC ${SOURCE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -pedantic -Wno-nullability-extension -Wno-nullability-completeness -Wno-gnu-zero-variadic-macro-arguments -Wno-missing-field-initializers -Wno-unused-parameter>
)

# Vulkan
find_package(Vulkan REQUIRED)
if (Vulkan_FOUND)
	message(STATUS "Vulkan found!")
	target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)
else()
	message(FATAL_ERROR "Vulkan NOT FOUND!")
endif()

# Perform dependency linkage
include(${CMAKE_DIR}/LinkGLFW.cmake)
LinkGLFW(${PROJECT_NAME} PRIVATE)

# Enable C++20
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
