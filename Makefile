# Compilateur et options
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Ientetes

# Répertoires
SRC_DIR = codecpp
OBJ_DIR = obj
BIN_DIR = .

# Fichiers sources et objets
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) main.cpp
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SOURCES))) \
          $(OBJ_DIR)/main.o

# Exécutable
TARGET = $(BIN_DIR)/avion_simulation

# Règle par défaut
all: $(TARGET)

# Création de l'exécutable
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Compilation des fichiers objets
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/main.o: main.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Création des répertoires
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Nettoyage
clean:
	rm -rf $(OBJ_DIR) $(TARGET) *.csv

# Recompilation complète
rebuild: clean all

.PHONY: all clean rebuild
