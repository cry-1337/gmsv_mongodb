#include "Client.hpp"

using namespace GarrysMod::Lua;

LUA_FUNCTION(new_client) {
    std::string connection = LUA->CheckString(1);

    if (LUA->IsType(2, Type::String)) {
        if (const std::string app = LUA->GetString(2); !app.empty()) {
            connection += (connection.find('?') == std::string::npos ? "?appName=" : "&appName=");
            connection += app;
        }
    }

    std::size_t workers = 1;
    if (LUA->IsType(3, Type::Number)) {
        if (const auto count = static_cast<long long>(LUA->GetNumber(3)); count > 0) {
            workers = static_cast<std::size_t>(count);
        }
    }

    MONGO_TRY
        mongocxx::uri validate{connection};
        (void) validate;

        auto* conn = new Connection(connection, workers);
        g_connections.push_back(conn);

        LUA->PushUserType(conn, ClientMetaTableId);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(destroy_client) {
    const auto conn = LUA->GetUserType<Connection>(1, ClientMetaTableId);
    if (conn == nullptr) return 0;

    std::erase(g_connections, conn);
    conn->shutdown();

    delete conn;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(client_command) {
    GET_SELF(conn, Connection, ClientMetaTableId)

    const std::string database = LUA->CheckString(2);

    MONGO_TRY
        auto command = LuaTableToBSON(LUA, 3);
        const int callback = GetCallback(LUA, 4);

        conn->enqueue([database, command = std::move(command)](mongocxx::client& client) -> ResultValue {
            return client[database].run_command(command.view());
        }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(client_uri) {
    GET_SELF(conn, Connection, ClientMetaTableId)

    LUA->PushString(conn->uri().c_str());

    return 1;
}

LUA_FUNCTION(client_default_database) {
    GET_SELF(conn, Connection, ClientMetaTableId)

    const std::string& database = conn->default_database();
    if (database.empty()) {
        LUA->ThrowError("The connection URI has no default database!");
        return 0;
    }

    auto* handle = new DatabaseHandle{conn, database};
    LUA->PushUserType(handle, DatabaseMetaTableId);

    return 1;
}

LUA_FUNCTION(client_list_databases) {
    GET_SELF(conn, Connection, ClientMetaTableId)

    const int callback = GetCallback(LUA, 2);

    conn->enqueue([](mongocxx::client& client) -> ResultValue {
        auto cursor = client.list_databases();

        std::vector<bsoncxx::document::value> out;
        for (auto&& document : cursor) {
            out.emplace_back(document);
        }

        return out;
    }, callback);

    return 0;
}

LUA_FUNCTION(client_database) {
    GET_SELF(conn, Connection, ClientMetaTableId)

    const std::string name = LUA->CheckString(2);

    auto* handle = new DatabaseHandle{conn, name};
    LUA->PushUserType(handle, DatabaseMetaTableId);

    return 1;
}

LUA_FUNCTION(client_collection) {
    GET_SELF(conn, Connection, ClientMetaTableId)

    const std::string database = LUA->CheckString(2);
    const std::string name = LUA->CheckString(3);

    auto* handle = new CollectionHandle{conn, database, name};
    LUA->PushUserType(handle, CollectionMetaTableId);

    return 1;
}
