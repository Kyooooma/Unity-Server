#include "dbUtils.h"

void databaseManager::init_link() {
    mt = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());
    // 创建 MySQL 连接驱动
    driver = sql::mysql::get_mysql_driver_instance();
    // 建立连接
    con = driver->connect("tcp://127.0.0.1:3306", "Kyooma", "12345678");
    // 选择数据库
    con->setSchema("players");
    stmt = con->createStatement();
    std::cout << "Connected to MySQL server success." << std::endl;

    // 创建 Redis 连接上下文
    context = redisConnect("127.0.0.1", 6379);
    if (context == nullptr || context->err) {
        if (context) {
            std::cout << "Error: " << context->errstr << std::endl;
            redisFree(context);
        } else {
            std::cout << "Unable to allocate redis context" << std::endl;
        }
    }
    std::cout << "Connected to Redis server success." << std::endl;
}

databaseManager::~databaseManager() {
    delete stmt;
    delete con;
    delete driver;
}

bool databaseManager::login(const std::string& username, const std::string& password) {
//    if(loged_users.count(username)){
//        std::cout << "user " << username << " already loged!!\n";
//        return false;
//    }
    std::string pwd;
    if(getKey(username, pwd) == 1){
        //说明在缓存中
        if(pwd != password) return false;
        //更新上线信息
        stmt->execute("insert loginfo(username, info, time) values('" + username + "', 0, '" + getCurrentTimestamp() + "');");
        loged_users.insert(username);
        return true;
    }
    //否则查mysql
    auto res = stmt->executeQuery("select password from accounts where username = '" + username + "'");
    if(res->next()){
        if(res->getString("password") != password) return false;
        //更新上线信息
        stmt->execute("insert loginfo(username, info, time) values('" + username + "', 0, '" + getCurrentTimestamp() + "');");
        loged_users.insert(username);
        //加入缓存
        setKey(username, password);
        return true;
    }else{
        //说明数据库中不存在, 自动注册
        stmt->execute("insert accounts(username, password) values('" + username + "', '" + password + "');");
        stmt->execute("insert loginfo(username, info, time) values('" + username + "', 0, '" + getCurrentTimestamp() + "');");
        loged_users.insert(username);
        //加入缓存
        setKey(username, password);
        return true;
    }
}

// 获取当前时间戳的函数
std::string databaseManager::getCurrentTimestamp() {
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();

    // 转换为 time_t 类型
    std::time_t time = std::chrono::system_clock::to_time_t(now);

    // 将 time_t 转换为 struct tm 结构体
    struct std::tm *tm = std::localtime(&time);

    // 格式化时间戳字符串
    std::stringstream ss;
    ss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}

void databaseManager::logout(const std::string &username) {
    if(!loged_users.count(username)){
        std::cout << "user " << username << " already logout!!\n";
        return;
    }
    stmt->execute("insert loginfo(username, info, time) values('" + username + "', 1, '" + getCurrentTimestamp() + "');");
    loged_users.erase(username);
}

void databaseManager::setKey(const std::string &username, const std::string &password) {
    std::string cmd = "SET " + username + " " + password + " EX " + std::to_string(rng(20, 60));
    auto reply = (redisReply *)redisCommand(context, cmd.c_str());
    if (reply == nullptr) {
        std::cerr << "Error executing SET command" << std::endl;
        redisFree(context);
        return;
    }
    freeReplyObject(reply);
}

int databaseManager::getKey(const std::string &username, std::string &key) const {
    std::string cmd = "GET " + username;
    auto reply = (redisReply *)redisCommand(context, cmd.c_str());
    if (reply == nullptr) {
        std::cerr << "Error executing GET command" << std::endl;
        freeReplyObject(reply);
        redisFree(context);
        return -1;
    }
    // 检查回复对象是否为空
    if (reply->type == REDIS_REPLY_NIL) {
        std::cout << "Key not found" << std::endl;
        freeReplyObject(reply);
        return 0;
    } else {
        std::cout << "Value: " << reply->str << std::endl;
        key = reply->str;
    }
    freeReplyObject(reply);
    return 1;
}

int databaseManager::rng(int l, int r) {
    std::uniform_int_distribution<int> uni(l, r);
    return uni(mt);
}
