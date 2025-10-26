CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -O2 -O3 -Wall -Wextra -Wshadow -fsanitize=address

TARGET = super

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET) show

# 編譯成執行檔
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "編譯成功: $(TARGET)"

# 將 .cpp 編譯成 .o 檔案
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -rf $(OBJS) $(TARGET) convert display image/output_*

# 編譯 convert.c、 display.c
show:
	gcc convert.c -Iinclude -o convert
	gcc display.c -Iinclude -lglut -lGLU -lGL -o display