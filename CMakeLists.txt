cmake_minimum_required(VERSION 3.20)

project(ImGuiTemplate)
add_executable(${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# --- CONFIGURATION DES RÉPERTOIRES ---

# Définit où l'exécutable est généré
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF)

# --- GESTION DES FICHIERS ---

# Cette ligne scanne récursivement tout ce qui est dans src/ (.cpp)
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS src/*)
target_sources(${PROJECT_NAME} PRIVATE ${MY_SOURCES})

# TRÈS IMPORTANT : On ajoute "include" ET "src" aux chemins de recherche
# Cela permet de faire #include "Piece.hpp" sans souci
target_include_directories(${PROJECT_NAME} PRIVATE include src)

# --- BIBLIOTHÈQUES ---

add_subdirectory(lib/quick_imgui)
target_link_libraries(${PROJECT_NAME} PRIVATE quick_imgui::quick_imgui)