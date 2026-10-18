# Имя исполняемого файла
TARGET = relays

# Исходные каталоги проекта
SRC_DIR         = src
HEAD_DIR        = $(SRC_DIR)/headers
BUILD_BASE_DIR  = build

# Режим сборки по умолчанию (1 - Debug, 0 - Release)
DEBUG ?= 1

# Компилятор и стандарт C++
CXX             = g++
CXXSTD          = -std=c++11

# Базовые флаги компиляции и линковки
CXXFLAGS        = -Wall -Wextra -Wpedantic -pthread -I$(HEAD_DIR) -MMD -MP
LIBS            = -lwiringPi -lm
LDFLAGS         = -pthread

# Debug и Release
ifeq ($(DEBUG), 1)
    CXXFLAGS    += -g -O0 -DDEBUG
    BUILD_DIR   = $(BUILD_BASE_DIR)/debug
else
    CXXFLAGS    += -O3 -flto -DNDEBUG
    LDFLAGS     += -flto -s
    BUILD_DIR   = $(BUILD_BASE_DIR)/release
endif

OBJ_DIR         = $(BUILD_DIR)/obj
EXECUTE_DIR     = $(BUILD_DIR)/bin

# Поиск исходников проекта (.cpp)
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

# Автоматическая генерация зависимостей (.d)
DEPS = $(OBJS:.o=.d)

# Финальный путь к исполняемому файлу
EXEC_TARGET = $(EXECUTE_DIR)/$(TARGET)

all: $(EXEC_TARGET)

# Общее правило компиляции объектных файлов
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) -c $< -o $@

# Линковка
$(EXEC_TARGET): $(OBJS) | $(EXECUTE_DIR)
	$(CXX) $(CXXSTD) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(EXEC_TARGET) $(LIBS)
	cp $(EXEC_TARGET) ./$(TARGET)

# Создание директорий сборки
$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(EXECUTE_DIR):
	mkdir -p $(EXECUTE_DIR)

# Включение файлов зависимостей
-include $(DEPS)

clean:
	rm -rf $(BUILD_BASE_DIR)
	rm -f $(TARGET)

.PHONY: all clean
