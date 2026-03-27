# ============================================
# Avion Simulation - Makefile (Windows/MinGW)
# ============================================

# Détection du système d'exploitation
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
    RM = rm -rf
    MKDIR = mkdir -p
else ifeq ($(OS),Windows_NT)
    RM = powershell -Command "Remove-Item -Recurse -Force"
    MKDIR = powershell -Command "New-Item -ItemType Directory -Force -Path"
else
    RM = rm -rf
    MKDIR = mkdir -p
endif

# Compilateur
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g
CXXFLAGS_RELEASE = -O2
CXXFLAGS_DEBUG = -O0 -g3
LDFLAGS = 

# Répertoires
SRC_DIR = codecpp
HEADER_DIR = entetes
OBJ_DIR = obj
BIN_DIR = .
OUTPUT_DIR = output

# Fichiers sources
SOURCES_CPP = $(wildcard $(SRC_DIR)/*.cpp) main.cpp
SOURCES_H = $(wildcard $(HEADER_DIR)/*.h)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SOURCES_CPP))) \
          $(OBJ_DIR)/main.o

# Exécutable
TARGET = $(BIN_DIR)/avion_simulation
VERSION = 1.0

# ============================================
# Cibles .PHONY
# ============================================
.PHONY: all clean rebuild run help release debug info clean_output clean_all

# ============================================
# Règles principales
# ============================================

all: $(TARGET)
	@echo [OK] Build complete: $(TARGET)

release: CXXFLAGS += $(CXXFLAGS_RELEASE)
release: clean $(TARGET)
	@echo [OK] Release build complete

debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: clean $(TARGET)
	@echo [OK] Debug build complete

# ============================================
# Édition des exécutables
# ============================================

$(TARGET): $(OBJECTS) | $(BIN_DIR) $(OUTPUT_DIR)
	@echo [LINK] Building $@ from $(words $(OBJECTS)) objects
	@$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^
	@echo [OK] Executable created: $@

# ============================================
# Compilation des fichiers objets
# ============================================

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(SOURCES_H) | $(OBJ_DIR)
	@echo [CC] Compiling $(notdir $<)
	@$(CXX) $(CXXFLAGS) -I$(HEADER_DIR) -c $< -o $@

$(OBJ_DIR)/main.o: main.cpp $(SOURCES_H) | $(OBJ_DIR)
	@echo [CC] Compiling main.cpp
	@$(CXX) $(CXXFLAGS) -I$(HEADER_DIR) -c $< -o $@

# ============================================
# Création des répertoires
# ============================================

$(OBJ_DIR):
ifeq ($(UNAME_S),Linux)
	@mkdir -p $(OBJ_DIR)  # Commande Linux/Unix
else ifeq ($(UNAME_S),Darwin)
	@mkdir -p $(OBJ_DIR)  # Commande macOS
else
	@powershell -Command "New-Item -ItemType Directory -Force -Path '$(OBJ_DIR)' | Out-Null"  # Commande PowerShell (Windows)
endif
	@echo [MKDIR] Created directory: $@

$(BIN_DIR):
ifeq ($(UNAME_S),Linux)
	@mkdir -p $(BIN_DIR)  # Commande Linux/Unix
else ifeq ($(UNAME_S),Darwin)
	@mkdir -p $(BIN_DIR)  # Commande macOS
else
	@powershell -Command "New-Item -ItemType Directory -Force -Path '$(BIN_DIR)' | Out-Null"  # Commande PowerShell (Windows)
endif

$(OUTPUT_DIR):
ifeq ($(UNAME_S),Linux)
	@mkdir -p $(OUTPUT_DIR)  # Commande Linux/Unix
else ifeq ($(UNAME_S),Darwin)
	@mkdir -p $(OUTPUT_DIR)  # Commande macOS
else
	@powershell -Command "New-Item -ItemType Directory -Force -Path '$(OUTPUT_DIR)' | Out-Null"  # Commande PowerShell (Windows)
endif
	@echo [MKDIR] Created directory: $@
# ============================================
# Exécution
# ============================================

run: $(TARGET)
	@echo [RUN] Executing $(TARGET)...
	@./$(TARGET)

# ============================================
# Nettoyage
# ============================================

clean:
	@echo [CLEAN] Removing compiled files...
	@if exist "$(OBJ_DIR)" powershell -Command "Remove-Item -Recurse -Force '$(OBJ_DIR)' -ErrorAction SilentlyContinue"
	@if exist "$(TARGET).exe" del /q "$(TARGET).exe"
	@echo [OK] Build artifacts removed

clean_output:
	@echo [CLEAN] Removing output CSV files...
	@if exist "$(OUTPUT_DIR)\*.csv" del /q "$(OUTPUT_DIR)\*.csv"
	@echo [OK] Output files cleaned

clean_all: clean clean_output
	@echo [OK] Full clean complete

# ============================================
# Recompilation
# ============================================

rebuild: clean all

# ============================================
# Informations
# ============================================

info:
	@echo.
	@echo ==================== Project Info ====================
	@echo Project: Avion Crash Simulation
	@echo Version: $(VERSION)
	@echo.
	@echo Compilation:
	@echo   Compiler: $(CXX)
	@echo   C++ Standard: c++17
	@echo   Flags: $(CXXFLAGS)
	@echo.
	@echo Directories:
	@echo   Sources: $(SRC_DIR)/
	@echo   Headers: $(HEADER_DIR)/
	@echo   Objects: $(OBJ_DIR)/
	@echo   Output:  $(OUTPUT_DIR)/
	@echo.
	@echo Files:
	@echo   Source files (.cpp): $(words $(SOURCES_CPP))
	@echo   Header files (.h): $(words $(SOURCES_H))
	@echo   Object files (.o): $(words $(OBJECTS))
	@echo.
	@echo Target: $(TARGET)
	@echo ======================================================
	@echo.

help:
	@echo.
	@echo =========================================
	@echo Available targets:
	@echo =========================================
	@echo   mingw32-make              Build the project (default)
	@echo   mingw32-make release      Build optimized version (-O2)
	@echo   mingw32-make debug        Build debug version (-g3 -O0)
	@echo   mingw32-make run          Build and execute
	@echo   mingw32-make clean        Remove object files and executable
	@echo   mingw32-make clean_output Remove output CSV files
	@echo   mingw32-make clean_all    Remove all generated files
	@echo   mingw32-make rebuild      Full rebuild (clean + build)
	@echo   mingw32-make info         Display project information
	@echo   mingw32-make help         Show this help message
	@echo   mingw32-make optimiseur   Build rescue parameter optimizer
	@echo   mingw32-make run_optimiseur Build and run optimizer
	@echo =========================================
	@echo.


# ============================================
# Optimiseur de sauvetage (Machine Learning)
# ============================================

# Définir les objets nécessaires pour l'optimiseur
OPTIMISEUR_SOURCES = test_optimiseur_sauvetage.cpp \
                     $(SRC_DIR)/OptimiseurSauvetage.cpp \
                     $(SRC_DIR)/Avion.cpp \
                     $(SRC_DIR)/Simulateur.cpp \
                     $(SRC_DIR)/SauvetageAvion.cpp \
                     $(SRC_DIR)/Environnement.cpp \
                     $(SRC_DIR)/EtatCinematique.cpp \
                     $(SRC_DIR)/ProprietesInertie.cpp \
                     $(SRC_DIR)/ModeleAerodynamique.cpp \
                     $(SRC_DIR)/ModeleLineaire.cpp \
                     $(SRC_DIR)/ModeleHysteresis.cpp \
                     $(SRC_DIR)/ForcesAerodynamiques.cpp \
                     $(SRC_DIR)/Propulsion.cpp \
                     $(SRC_DIR)/SystemeControle.cpp \
                     $(SRC_DIR)/CalculateurTrim.cpp \
                     $(SRC_DIR)/Integration.cpp \
                     $(SRC_DIR)/Constantes.cpp

OPTIMISEUR_TARGET = $(BIN_DIR)/test_optimiseur

.PHONY: optimiseur run_optimiseur

optimiseur: $(OPTIMISEUR_TARGET)
	@echo [OK] Optimizer build complete: $(OPTIMISEUR_TARGET)

$(OPTIMISEUR_TARGET): $(OPTIMISEUR_SOURCES) $(SOURCES_H) | $(BIN_DIR) $(OUTPUT_DIR)
	@echo [LINK] Building rescue parameter optimizer...
	@$(CXX) $(CXXFLAGS) $(CXXFLAGS_RELEASE) -I$(HEADER_DIR) -o $@ $(OPTIMISEUR_SOURCES)
	@echo [OK] Optimizer executable created: $@

run_optimiseur: $(OPTIMISEUR_TARGET)
	@echo [RUN] Executing optimizer...
	@.\$(OPTIMISEUR_TARGET)
	@echo.
	@echo [INFO] Database generated: parametres_sauvetage_db.csv
	@echo [INFO] To visualize results, run: python visualiser_optimisation.py
