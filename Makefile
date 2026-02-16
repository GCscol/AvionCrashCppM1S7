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

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@echo [CC] Compiling $$(notdir $<)
	@$(CXX) $(CXXFLAGS) -I$(HEADER_DIR) -c $< -o $@

$(OBJ_DIR)/main.o: main.cpp | $(OBJ_DIR)
	@echo [CC] Compiling main.cpp
	@$(CXX) $(CXXFLAGS) -I$(HEADER_DIR) -c $< -o $@

# ============================================
# Création des répertoires
# ============================================

$(OBJ_DIR):
	@powershell -Command "New-Item -ItemType Directory -Force -Path '$(OBJ_DIR)' | Out-Null"
	@echo [MKDIR] Created directory: $@

$(BIN_DIR):
	@powershell -Command "New-Item -ItemType Directory -Force -Path '$(BIN_DIR)' | Out-Null"

$(OUTPUT_DIR):
	@powershell -Command "New-Item -ItemType Directory -Force -Path '$(OUTPUT_DIR)' | Out-Null"
	@echo [MKDIR] Created directory: $@

# ============================================
# Exécution
# ============================================

run: $(TARGET)
	@echo [RUN] Executing $(TARGET)...
	@.\$(TARGET)

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
	@echo =========================================
	@echo.

