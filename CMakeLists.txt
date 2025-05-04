cmake_minimum_required(VERSION 3.10)
project(MyAsteroidsGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Find SFML ---
# Ensure this path points to the correct location of SFMLConfig.cmake
set(SFML_DIR "D:/Downloads/SFML-Sources/SFML-2.6.2-custom/lib/cmake/SFML" CACHE PATH "Path to SFML cmake config")

find_package(SFML 2.6 REQUIRED COMPONENTS system window graphics audio) # Added audio

# --- Add Source Files ---
# Use GLOB to find all .cpp files in the src directory
# Note: GLOB is convenient but can sometimes cause issues if files are added/removed
# without re-running CMake. For larger projects, listing files explicitly is safer.
file(GLOB SOURCE_FILES "src/*.cpp")

# --- Create Executable ---
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

# --- Include Directories ---
# Add the src directory so headers can be found easily
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE sfml-system sfml-window sfml-graphics sfml-audio)

# --- Copy Assets Post-Build (Improved) ---
set(ASSET_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}) # Root of your source project
set(ASSET_DEST_DIR $<TARGET_FILE_DIR:${PROJECT_NAME}>) # Directory where the .exe is built

# Function to copy assets if source exists
function(copy_assets source_folder)
    set(SOURCE_PATH "${ASSET_SOURCE_DIR}/${source_folder}")
    set(DEST_PATH "${ASSET_DEST_DIR}/${source_folder}")
    if(EXISTS "${SOURCE_PATH}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${SOURCE_PATH}" "${DEST_PATH}"
            COMMENT "Copying ${source_folder} assets to build directory")
    else()
        message(WARNING "Asset source directory not found: ${SOURCE_PATH}. Assets might be missing.")
    endif()
endfunction()

# Copy images and sounds folders
copy_assets(images)
copy_assets(sounds)

# --- Copy SFML DLLs (If on Windows) ---
if(SFML_FOUND AND CMAKE_HOST_WIN32)
    # Simplified DLL copying - relies on SFML_DIR being set correctly
    get_filename_component(SFML_CMAKE_DIR ${SFML_DIR} DIRECTORY)
    get_filename_component(SFML_LIB_DIR ${SFML_CMAKE_DIR} DIRECTORY)
    get_filename_component(SFML_ROOT_DIR ${SFML_LIB_DIR} DIRECTORY)
    set(SFML_BIN_DIR "${SFML_ROOT_DIR}/bin")

    if(EXISTS "${SFML_BIN_DIR}")
        # Find all DLLs in the SFML bin directory
        file(GLOB SFML_DLLS "${SFML_BIN_DIR}/*.dll")
        if(SFML_DLLS)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy ${SFML_DLLS} "${ASSET_DEST_DIR}"
                COMMENT "Copying SFML DLLs to build directory")
        else()
            message(WARNING "No DLLs found in ${SFML_BIN_DIR}. SFML might not run correctly.")
        endif()
    else()
        message(WARNING "SFML bin directory not found at '${SFML_BIN_DIR}'. DLLs may need to be copied manually.")
    endif()
endif()

# Optional: Set output directory for executable (useful for organization)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)