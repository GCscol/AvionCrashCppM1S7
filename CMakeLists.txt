cmake_minimum_required(VERSION 3.10)
project(AvionSimulation)

# C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dossiers d'inclusion
include_directories(include)

# Fichiers sources
set(SOURCES
    codecpp/Environnement.cpp
    codecpp/ModeleAerodynamique.cpp
    codecpp/ModeleLineaire.cpp
    codecpp/Propulsion.cpp
    codecpp/ProprietesInertie.cpp
    codecpp/SystemeControle.cpp
    codecpp/CalculateurTrim.cpp
    codecpp/Avion.cpp
    codecpp/AnalyseurEnveloppeVol.cpp
    codecpp/Simulateur.cpp
    main.cpp
)

# Exécutable
add_executable(avion_simulation ${SOURCES})

# Options de compilation
target_compile_options(avion_simulation PRIVATE -Wall -Wextra -O2)
