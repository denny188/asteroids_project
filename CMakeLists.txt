cmake_minimum_required(VERSION 3.10)
project(MyAsteroidsGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Cấu hình SFML 2.6.22 ---
# !!! Đảm bảo đường dẫn này chính xác !!!
set(SFML_DIR "D:/Downloads/SFML-Sources/SFML-2.6.2-custom/lib/cmake/SFML")

# --- Tìm kiếm gói ---
# Chỉ tìm OpenGL và SFML (với các component cần thiết)
find_package(OpenGL REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenAL REQUIRED)
find_package(SFML 2.6 REQUIRED COMPONENTS graphics window system)
# Nếu dòng find_package(SFML...) này báo lỗi "Unsupported component" trở lại,
# thì đó là vấn đề gốc cần giải quyết.

# --- Tạo file thực thi ---
add_executable(${PROJECT_NAME} main.cpp)

# --- Liên kết Thư viện ---
# Chỉ liên kết với SFML và OpenGL
target_link_libraries(${PROJECT_NAME} PRIVATE
    sfml-graphics   # Sử dụng tên target không có namespace SFML::
    sfml-window
    sfml-system
    opengl32
)
# Lưu ý: Thử dùng tên target không có namespace "SFML::" trước.
# Nếu lỗi, thử đổi lại thành "SFML::graphics", "SFML::window", "SFML::system".

# --- Sao chép DLLs (Giữ lại phiên bản đơn giản nhất) ---
if(SFML_FOUND AND CMAKE_HOST_WIN32)
    get_filename_component(SFML_CMAKE_DIR ${SFML_DIR} DIRECTORY)
    get_filename_component(SFML_LIB_DIR ${SFML_CMAKE_DIR} DIRECTORY)
    get_filename_component(SFML_ROOT_DIR ${SFML_LIB_DIR} DIRECTORY)
    set(SFML_BIN_DIR "${SFML_ROOT_DIR}/bin")
    set(OUTPUT_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>")

    if(EXISTS "${SFML_BIN_DIR}")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${SFML_BIN_DIR}" "${OUTPUT_DIR}"
            COMMENT "Copying SFML DLLs from ${SFML_BIN_DIR} to ${OUTPUT_DIR}")
    else()
        message(WARNING "SFML bin directory not found at '${SFML_BIN_DIR}'. DLLs may need to be copied manually.")
    endif()
endif()