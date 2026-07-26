#include "Database.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>

using namespace GarrysMod::Lua;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

LUA_FUNCTION(destroy_database) {
    const auto handle = LUA->GetUserType<DatabaseHandle>(1, DatabaseMetaTableId);
    if (handle == nullptr) return 0;

    delete handle;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(database_name) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    LUA->PushString(handle->database.c_str());

    return 1;
}

LUA_FUNCTION(database_copy) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    auto* copy = new DatabaseHandle{handle->connection, handle->database};
    LUA->PushUserType(copy, DatabaseMetaTableId);

    return 1;
}

LUA_FUNCTION(database_drop) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    const int callback = GetCallback(LUA, 2);

    const std::string db = handle->database;
    handle->connection->enqueue([db](mongocxx::client& client) -> ResultValue {
        client[db].drop();
        return true;
    }, callback);

    return 0;
}

LUA_FUNCTION(database_command) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    MONGO_TRY
        auto command = LuaTableToBSON(LUA, 2);
        const int callback = GetCallback(LUA, 3);

        const std::string db = handle->database;
        handle->connection->enqueue([db, command = std::move(command)](mongocxx::client& client) -> ResultValue {
            return client[db].run_command(command.view());
        }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(database_user_add) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    const std::string username = LUA->CheckString(2);
    const std::string password = LUA->CheckString(3);
    const int callback = GetCallback(LUA, 4);

    const std::string db = handle->database;
    handle->connection->enqueue([db, username, password](mongocxx::client& client) -> ResultValue {
        const auto command = make_document(
            kvp("createUser", username),
            kvp("pwd", password),
            kvp("roles", make_array())
        );

        client[db].run_command(command.view());
        return true;
    }, callback);

    return 0;
}

LUA_FUNCTION(database_user_remove) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    const std::string username = LUA->CheckString(2);
    const int callback = GetCallback(LUA, 3);

    const std::string db = handle->database;
    handle->connection->enqueue([db, username](mongocxx::client& client) -> ResultValue {
        const auto command = make_document(kvp("dropUser", username));

        client[db].run_command(command.view());
        return true;
    }, callback);

    return 0;
}

LUA_FUNCTION(database_collection_exists) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    const std::string name = LUA->CheckString(2);
    const int callback = GetCallback(LUA, 3);

    const std::string db = handle->database;
    handle->connection->enqueue([db, name](mongocxx::client& client) -> ResultValue {
        return client[db].has_collection(name);
    }, callback);

    return 0;
}

LUA_FUNCTION(database_collection_get) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    const std::string name = LUA->CheckString(2);

    auto* collection = new CollectionHandle{handle->connection, handle->database, name};
    LUA->PushUserType(collection, CollectionMetaTableId);

    return 1;
}

LUA_FUNCTION(database_collection_create) {
    GET_SELF(handle, DatabaseHandle, DatabaseMetaTableId)

    const std::string name = LUA->CheckString(2);
    const int callback = GetCallback(LUA, 3);

    const std::string db = handle->database;
    handle->connection->enqueue([db, name](mongocxx::client& client) -> ResultValue {
        client[db].create_collection(name);
        return true;
    }, callback);

    return 0;
}
