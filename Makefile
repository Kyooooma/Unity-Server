# 编译器和选项
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# 目标文件夹
BUILD_DIR = build

# 目标文件
OBJECTS = $(BUILD_DIR)/proto/msg.pb.o \
          $(BUILD_DIR)/message/messageInfo.o \
          $(BUILD_DIR)/client/client.o \
          $(BUILD_DIR)/callback/callback.o \
          $(BUILD_DIR)/message/message.o \
          $(BUILD_DIR)/connect/connect.o \
          $(BUILD_DIR)/handler/handler.o \
          $(BUILD_DIR)/main.o

# 链接选项
LDFLAGS = -lprotobuf

all: server

# 目标：server
server: $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(BUILD_DIR)/server $(LDFLAGS)

# 编译 protobuf 文件
$(BUILD_DIR)/proto/msg.pb.o: proto/msg.pb.cc
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 编译其他源文件
$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理生成的目标文件和可执行文件
clean:
	rm -rf $(BUILD_DIR) server
