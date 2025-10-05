# ===============================
# Makefile do projeto Twindisseia (com deps de headers)
# ===============================

CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -Iinclude -MMD -MP
LIBS = -lncursesw   # use a variante wide

SRC_DIR = src
OBJ_DIR = obj
TARGET = twindisseia

SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@ $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

run: $(TARGET)
	./$(TARGET)

# inclui dependências geradas (-MMD)
-include $(DEPS)
