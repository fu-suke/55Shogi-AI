cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(shogi-engine)

find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")

# ソースファイルの文字エンコーディングをUTF-8として指定
if (MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /source-charset:utf-8")
endif()

# ディレクトリパスを変数として定義
set(DIR_SHOGI "./shogi")
set(DIR_COMMON "../common")

# ディレクトリABとCOMMONからソースファイルを一括で取得
file(GLOB_RECURSE SHOGI_SOURCES
     "${DIR_SHOGI}/*.cpp"
     "${DIR_COMMON}/*.cpp"
)

# ヘッダーファイルのディレクトリをインクルード
include_directories("${DIR_SHOGI}" "${DIR_COMMON}")

add_executable(shogi-engine ${SHOGI_SOURCES})
target_link_libraries(shogi-engine "${TORCH_LIBRARIES}")
set_property(TARGET shogi-engine PROPERTY CXX_STANDARD 17)

# WindowsのDLL関連の設定
if (MSVC)
  file(GLOB TORCH_DLLS "${TORCH_INSTALL_PREFIX}/lib/*.dll")
  add_custom_command(TARGET shogi-engine
                     POST_BUILD
                     COMMAND ${CMAKE_COMMAND} -E copy_if_different
                     ${TORCH_DLLS}
                     $<TARGET_FILE_DIR:shogi-engine>)
endif (MSVC)
