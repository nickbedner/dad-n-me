cmake_minimum_required(VERSION 3.12)
project(DadnMe LANGUAGES C VERSION 0.1.0)

set(CMAKE_C_STANDARD 11)

#add_subdirectory(lib/Mana/examples buildmana)

set(ignoreMe "${CMAKE_CXX_COMPILER}")

file(GLOB_RECURSE dadnme_SRC
        include/*.h
        src/*.c)

add_executable(DadnMe ${dadnme_SRC})

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
if(CMAKE_BUILD_TYPE MATCHES Debug)
        add_definitions(-DCHECK_MEMORY_LEAKS=1)
        message("Debug build.")
else()
        add_definitions(-DCUSTOM_ALLOCATOR=1)
endif()

add_subdirectory(lib/Mana buildmana)

if (WIN32)
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Xclang -fopenmp")
endif (WIN32)

if(APPLE)
        add_subdirectory(lib/MoltenVK buildmoltenvk)
        find_package(OpenMP)
        if (OPENMP_FOUND)
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        endif()
endif()

if(UNIX AND NOT APPLE)
        # for Linux, BSD, Solaris, Minix
        find_package(OpenMP)
        if (OPENMP_FOUND)
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        endif()
endif()

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-visibility -Wno-pragma-pack -Wno-deprecated-declarations -mavx -mavx2")
# Compile with up to sse2
add_definitions(-DSIMD_SSE2=1)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-visibility -Wno-pragma-pack -Wno-deprecated-declarations -msse2")

#if(CMAKE_BUILD_TYPE MATCHES Debug)
#        message("Debug build.")
#elseif(CMAKE_BUILD_TYPE MATCHES Release)
#        message("Release build.")
#else()
#        message("Some other build type.")
#endif()

target_link_libraries(DadnMe mana)

set(includeList
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/cstorage/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/cnoise/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/cthreads/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/uber-math/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/mimalloc/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/glfw/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/Vulkan-Headers/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/chaos/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/libsndfile/include
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/lib/libsoundio/soundio
        ${OpenMP_CXX_LIBRARIES})

target_include_directories(DadnMe PUBLIC ${includeList})

if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "AMD64")
        set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin/glslangValidator.exe")
else()
        set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/Bin32/glslangValidator.exe")
endif()

file(GLOB_RECURSE GLSL_SOURCE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/opengl/*.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/opengl/*.vert)

foreach(GLSL ${GLSL_SOURCE_FILES})
        get_filename_component(FILE_NAME ${GLSL} NAME)
        set(SPIRV ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/shaders/spirv/${FILE_NAME}.spv)
        add_custom_command(
                OUTPUT ${SPIRV}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/Mana/assets/shaders/spirv
                COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
                DEPENDS ${GLSL})
        list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(Shaders DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(DadnMe Shaders)

file(GLOB_RECURSE ALL_ASSETS
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets/*)

add_custom_target(Assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/lib/mana/assets ${PROJECT_BINARY_DIR}/assets
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/assets)

add_dependencies(DadnMe Assets)
