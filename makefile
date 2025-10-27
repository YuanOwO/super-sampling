CC = gcc
CFLAGS = -Iinclude
CXX = g++
CXXFLAGS = -std=c++17 -Iinclude -O2 -O3 -Wall -Wextra -Wshadow -fsanitize=address
UNAME_S := $(shell uname -s)

TARGET = convert super

SRCS_convert = convert.c
SRCS_display = display.c
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:.cpp=.o)

ifneq ($(UNAME_S), Darwin) # macOS 不支援 OpenGL
	TARGET += display
endif

all: $(TARGET)

convert: $(SRCS_convert)
	$(CC) $(CFLAGS) $^ -o $@

display: $(SRCS_display)
	$(CC) $(CFLAGS) -lglut -lGLU -lGL $^ -o $@

# 編譯成執行檔
super: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS)
	@echo "編譯成功: $(TARGET)"

# 將 .cpp 編譯成 .o 檔案
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -rf $(OBJS) $(TARGET) convert display image/output_*
