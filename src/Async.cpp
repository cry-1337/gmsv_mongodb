#include "MongoDB.hpp"

#include <cstdio>
#include <memory>
#include <utility>

using namespace GarrysMod::Lua;

std::vector<Connection*> g_connections;

Connection::Connection(std::string uri, std::size_t workers) : uri_(std::move(uri)), running_(true) {
    try {
        default_database_ = mongocxx::uri{uri_}.database();
    } catch (const std::exception&) {
        default_database_.clear();
    }

    if (workers == 0) workers = 1;

    for (std::size_t i = 0; i < workers; ++i) {
        workers_.emplace_back([this] { worker_loop(); });
    }
}

Connection::~Connection() {
    shutdown();
}

void Connection::shutdown() {
    if (!running_.exchange(false)) return;

    task_cv_.notify_all();

    for (auto& worker : workers_) {
        if (worker.joinable()) worker.join();
    }

    workers_.clear();
}

void Connection::enqueue(Work work, int callback) {
    {
        std::lock_guard lock(task_mutex_);
        tasks_.push_back(Task{std::move(work), callback});
    }

    task_cv_.notify_one();
}

std::vector<Completed> Connection::drain() {
    std::vector<Completed> out;

    std::lock_guard lock(done_mutex_);
    out.swap(done_);

    return out;
}

void Connection::worker_loop() {
    std::unique_ptr<mongocxx::client> client;
    std::string init_error;

    try {
        client = std::make_unique<mongocxx::client>(mongocxx::uri{uri_});
    } catch (const std::exception& e) {
        init_error = e.what();
    }

    while (true) {
        Task task;

        {
            std::unique_lock lock(task_mutex_);
            task_cv_.wait(lock, [this] { return !running_ || !tasks_.empty(); });

            if (!running_) break;

            task = std::move(tasks_.front());
            tasks_.pop_front();
        }

        Completed done;
        done.callback = task.callback;

        if (!client) {
            done.ok = false;
            done.error = init_error.empty() ? "mongodb client unavailable" : init_error;
        } else {
            try {
                done.value = task.work(*client);
                done.ok = true;
            } catch (const std::exception& e) {
                done.ok = false;
                done.error = e.what();
            }
        }

        {
            std::lock_guard lock(done_mutex_);
            done_.push_back(std::move(done));
        }
    }
}

template<class... Ts>
struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

int GetCallback(ILuaBase* LUA, int index) {
    if (!LUA->IsType(index, Type::Function)) return NO_REF;

    LUA->Push(index);
    return LUA->ReferenceCreate();
}

int PushResult(ILuaBase* LUA, ResultValue& value) {
    std::visit(overloaded{
        [&](std::monostate) { LUA->PushNil(); },
        [&](bool result) { LUA->PushBool(result); },
        [&](int64_t number) { LUA->PushNumber(static_cast<double>(number)); },
        [&](bsoncxx::document::value& document) { BSONToLua(LUA, document.view()); },
        [&](std::vector<bsoncxx::document::value>& documents) {
            LUA->CreateTable();

            int i = 0;
            for (auto& document : documents) {
                LUA->PushNumber(++i);
                BSONToLua(LUA, document.view());
                LUA->SetTable(-3);
            }
        },
    }, value);

    return 1;
}

static void deliver(ILuaBase* LUA, Completed& completed) {
    if (completed.callback == NO_REF) return;

    LUA->ReferencePush(completed.callback);

    if (!LUA->IsType(-1, Type::Function)) {
        LUA->Pop();
        LUA->ReferenceFree(completed.callback);
        return;
    }

    if (completed.ok) {
        LUA->PushNil();
        PushResult(LUA, completed.value);
    } else {
        LUA->PushString(completed.error.c_str());
        LUA->PushNil();
    }

    if (LUA->PCall(2, 0, 0) != 0) {
        std::fprintf(stderr, "[mongodb] callback error: %s\n", LUA->GetString(-1));
        LUA->Pop();
    }

    LUA->ReferenceFree(completed.callback);
}

LUA_FUNCTION(poll) {
    std::vector<Completed> ready;

    for (auto* connection : g_connections) {
        auto done = connection->drain();
        for (auto& completed : done) {
            ready.push_back(std::move(completed));
        }
    }

    for (auto& completed : ready) {
        deliver(LUA, completed);
    }

    return 0;
}
