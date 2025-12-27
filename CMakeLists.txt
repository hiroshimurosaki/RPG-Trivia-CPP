cmake_minimum_required(VERSION 3.10)

# Nome do projeto
project(MeuJogoSFML)

# Configura C++ 17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- PARTE 1: SFML do Sistema ---
find_package(SFML 2.5 COMPONENTS graphics window system audio REQUIRED)

# --- PARTE 2: Baixar ImGui e ImGui-SFML ---
include(FetchContent)

# 2.1. Baixar o ImGui Original (Necessário para o adaptador funcionar)
FetchContent_Declare(
  ImGui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.89 # Versão estável e muito compatível
)
FetchContent_MakeAvailable(ImGui)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(json)

# 2.2. Configurar o ImGui-SFML
# [CORREÇÃO 1] Diz onde o ImGui original foi baixado
set(IMGUI_DIR ${imgui_SOURCE_DIR})
# [CORREÇÃO 2] Diz para não procurar o SFML de novo (evita erro static/shared)
set(IMGUI_SFML_FIND_SFML OFF)

# 2.3. Baixar o Adaptador ImGui-SFML
FetchContent_Declare(
  ImGui-SFML
  GIT_REPOSITORY https://github.com/SFML/imgui-sfml
  GIT_TAG v2.6
)
FetchContent_MakeAvailable(ImGui-SFML)

# --- PARTE 3: Seu Executável ---
add_executable(MeuJogo src/main.cpp)

# --- PARTE 4: Linkagem ---
target_link_libraries(MeuJogo 
    PRIVATE 
    sfml-graphics sfml-window sfml-system sfml-audio # SFML
    ImGui-SFML::ImGui-SFML                           # ImGui Integrado
    nlohmann_json::nlohmann_json
)