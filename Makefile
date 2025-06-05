# コンパイラとフラグ
CXX = g++
CXXFLAGS = -Wall -Iinclude -std=c++17 -I/usr/include/eigen3
LDFLAGS = -L/usr/local/lib -lpagmo

# フォルダ構成
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# サブディレクトリを含むソースファイルとオブジェクトファイル
SRC_FILES = $(wildcard $(SRC_DIR)/*/*.cpp)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.o, $(SRC_FILES))

# 実行ファイルの名前
TARGET = optTWPA

# デフォルトのターゲット
all: $(TARGET)

# 実行ファイルの作成（リンク）
$(TARGET): $(OBJ_FILES)
	$(CXX) $(OBJ_FILES) -o $(TARGET) $(LDFLAGS)

# 各オブジェクトファイルのビルド
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# クリーンアップ
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
