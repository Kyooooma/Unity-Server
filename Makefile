# 编译器和选项
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2

# 目标文件夹
BUILD_DIR = build

# 目标文件
OBJECTS = $(BUILD_DIR)/proto/msg.pb.o \
          			$(BUILD_DIR)/message/messageInfo.o \
          			$(BUILD_DIR)/message/messageUtils.o \
          			$(BUILD_DIR)/client/client.o \
          			$(BUILD_DIR)/connect/connect.o \

OBJECT_GAME = $(BUILD_DIR)/connect/gameConnect.o \
              $(BUILD_DIR)/server/gameServer.o

OBJECT_GATE = $(BUILD_DIR)/connect/gateConnect.o \
              $(BUILD_DIR)/server/gateServer.o

OBJECT_DB = $(BUILD_DIR)/connect/dbConnect.o \
              $(BUILD_DIR)/server/dbServer.o

# 链接选项
LDFLAGS = -lprotobuf

all: game gate db

# 目标：gameServer
game: $(OBJECTS) $(OBJECT_GAME)
	$(CXX) $(OBJECTS) $(OBJECT_GAME) -o $(BUILD_DIR)/gameServer $(LDFLAGS)

# 目标：gateServer
gate: $(OBJECTS) $(OBJECT_GATE)
	$(CXX) $(OBJECTS) $(OBJECT_GATE) -o $(BUILD_DIR)/gateServer $(LDFLAGS)

# 目标：dbServer
db: $(OBJECTS) $(OBJECT_DB)
	$(CXX) $(OBJECTS) $(OBJECT_DB) -o $(BUILD_DIR)/dbServer $(LDFLAGS)

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
	find $(BUILD_DIR)/ -name "*.o" -type f -delete
	find $(BUILD_DIR)/ -name "dbServer" -type f -delete
	find $(BUILD_DIR)/ -name "gateServer" -type f -delete
	find $(BUILD_DIR)/ -name "gameServer" -type f -delete


