#ifndef GMSV_MONGODB_ASYNC_HPP
#define GMSV_MONGODB_ASYNC_HPP

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include <mongocxx/client.hpp>
#include <mongocxx/uri.hpp>
#include <bsoncxx/document/value.hpp>

namespace GarrysMod::Lua { class ILuaBase; }
struct lua_State;

inline constexpr int NO_REF = -1;

using ResultValue = std::variant<
    std::monostate,
    bool,
    int64_t,
    bsoncxx::document::value,
    std::vector<bsoncxx::document::value>>;

using Work = std::function<ResultValue(mongocxx::client&)>;

struct Task {
    Work work;
    int callback = NO_REF;
};

struct Completed {
    int callback = NO_REF;
    bool ok = false;
    ResultValue value;
    std::string error;
};

class Connection {
public:
    Connection(std::string uri, std::size_t workers);
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    void enqueue(Work work, int callback);
    std::vector<Completed> drain();
    void shutdown();

    [[nodiscard]] const std::string& uri() const { return uri_; }
    [[nodiscard]] const std::string& default_database() const { return default_database_; }

private:
    void worker_loop();

    std::string uri_;
    std::string default_database_;
    std::vector<std::thread> workers_;

    std::mutex task_mutex_;
    std::condition_variable task_cv_;
    std::deque<Task> tasks_;
    std::atomic<bool> running_;

    std::mutex done_mutex_;
    std::vector<Completed> done_;
};

struct DatabaseHandle {
    Connection* connection;
    std::string database;
};

struct CollectionHandle {
    Connection* connection;
    std::string database;
    std::string collection;
};

extern std::vector<Connection*> g_connections;

int GetCallback(GarrysMod::Lua::ILuaBase* LUA, int index);
int PushResult(GarrysMod::Lua::ILuaBase* LUA, ResultValue& value);
int poll(lua_State* state);

#endif //GMSV_MONGODB_ASYNC_HPP
