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

PLOT_MENU_SOURCE = plot_menu.cpp

# Exécutable
TARGET = $(BIN_DIR)/avion_simulation
PLOT_MENU_TARGET = $(BIN_DIR)/plot_menu
VERSION = 1.0

# ============================================
# Cibles .PHONY
# ============================================
.PHONY: all clean rebuild run help release debug info clean_output clean_all

# ============================================
# Règles principales
# ============================================

all: $(TARGET) $(PLOT_MENU_TARGET)
	@echo [OK] Build complete: $(TARGET)

release: CXXFLAGS += $(CXXFLAGS_RELEASE)
release: clean $(TARGET)
	@echo [OK] Release build complete

debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: clean $(TARGET)
	@echo [OK] Debug build complete
$(PLOT_MENU_TARGET): $(PLOT_MENU_SOURCE) $(SOURCES_H) | $(BIN_DIR)
	@echo [LINK] Building plot menu...
	@$(CXX) $(CXXFLAGS) -I$(HEADER_DIR) -o $@ $(PLOT_MENU_SOURCE)
	@echo [OK] Plot menu executable created: $@
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

.PHONY: plot

plot: $(PLOT_MENU_TARGET)
	@echo [RUN] Executing plot menu...
ifeq ($(OS),Windows_NT)
	@.\$(PLOT_MENU_TARGET)
else
	@./$(PLOT_MENU_TARGET)
endif
# ============================================
# Nettoyage
# ============================================

clean:
	@echo [CLEAN] Removing compiled files...
	@$(RM) $(OBJ_DIR)
	@$(RM) $(TARGET)
	@echo [OK] Build artifacts removed
# ============================================
# Recompilation
# ============================================

rebuild: clean all

# ============================================
# Informations
# ============================================

info:
	@echo ""
	@echo "==================== Project Info ===================="
	@echo "Project: Avion Crash Simulation"
	@echo "Version: $(VERSION)"
	@echo ""
	@echo "Compilation:"
	@echo "  Compiler: $(CXX)"
	@echo "  C++ Standard: c++17"
	@echo "  Flags: $(CXXFLAGS)"
	@echo ""
	@echo "Directories:"
	@echo "  Sources: $(SRC_DIR)/"
	@echo "  Headers: $(HEADER_DIR)/"
	@echo "  Objects: $(OBJ_DIR)/"
	@echo "  Output:  $(OUTPUT_DIR)/"
	@echo ""
	@echo "Files:"
	@echo "  Source files (.cpp): $(words $(SOURCES_CPP))"
	@echo "  Header files (.h): $(words $(SOURCES_H))"
	@echo "  Object files (.o): $(words $(OBJECTS))"
	@echo ""
	@echo "Target: $(TARGET)"
	@echo "======================================================"
	@echo ""
help:
	@echo ""
	@echo "========================================="
	@echo "Available targets:"
	@echo "========================================="
	@echo "  make                Build the project (default)"
	@echo "  make release        Build optimized version (-O2)"
	@echo "  make debug          Build debug version (-g3 -O0)"
	@echo "  make run            Build and execute"
	@echo "  make plot           Build and execute the plot code"
	@echo "  make clean          Remove object files and executable"
	@echo "  make clean_output   Remove output CSV files"
	@echo "  make clean_all      Remove all generated files"
	@echo "  make rebuild        Full rebuild (clean + build)"
	@echo "  make info           Display project information"
	@echo "  make help           Show this help message"
	@echo "  make optimiseur     Build rescue parameter optimizer"
	@echo "  make run_optimiseur Build and run optimizer"
	@echo "========================================="
	@echo ""

