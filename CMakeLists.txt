cmake_minimum_required(VERSION 3.14)

project(RPG_Trivia_Online LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ---------------------------------------------------------
# 1. SFML (Baixa primeiro)
# ---------------------------------------------------------
FetchContent_Declare(
  SFML
  GIT_REPOSITORY https://github.com/SFML/SFML.git
  GIT_TAG        2.6.1
)
FetchContent_MakeAvailable(SFML)

# ---------------------------------------------------------
# 2. ImGui (Baixa em seguida)
# ---------------------------------------------------------
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.4-docking
)
FetchContent_MakeAvailable(imgui) # <--- Isso preenche a variável ${imgui_SOURCE_DIR}

# ---------------------------------------------------------
# 3. ImGui-SFML (Configura e baixa por último)
# ---------------------------------------------------------
# CORREÇÃO 1: Avisa onde o ImGui está (Resolve o erro atual "ImGui directory not found")
set(IMGUI_DIR ${imgui_SOURCE_DIR})

# CORREÇÃO 2: Avisa para usar o SFML que já baixamos (Resolve o erro anterior do conflito)
set(IMGUI_SFML_FIND_SFML OFF)

FetchContent_Declare(
  imgui-sfml
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
  GIT_TAG        v2.6
)
FetchContent_MakeAvailable(imgui-sfml)

# ---------------------------------------------------------
# 4. Seu Jogo
# ---------------------------------------------------------
# Procura todos os .cpp na pasta src/
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Se não tiver a pasta src ainda, usa o rpg.cpp da raiz (fallback para seu teste atual)
if(NOT SOURCES)
    set(SOURCES rpg.cpp)
endif()

add_executable(RPG ${SOURCES})

target_link_libraries(RPG PRIVATE 
    sfml-graphics 
    sfml-window 
    sfml-system 
    sfml-network
    ImGui-SFML::ImGui-SFML
)

# Copia a pasta assets se ela existir
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET RPG POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:RPG>/assets
    )
endif()