#include "Database.hpp"

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>

using namespace GarrysMod::Lua;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

LUA_FUNCTION(destroy_database) {
    const auto database = LUA->GetUserType<mongocxx::database>(1, DatabaseMetaTableId);
    if (database == nullptr) return 0;

    delete database;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(database_name) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    MONGO_TRY
        const auto view = database->name();
        const std::string name(view.data(), view.size());
        LUA->PushString(name.c_str());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_copy) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    MONGO_TRY
        const auto copy = new mongocxx::database(*database);
        LUA->PushUserType(copy, DatabaseMetaTableId);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_drop) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    MONGO_TRY
        database->drop();
        LUA->PushBool(true);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_command) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    const auto command = LuaTableToBSON(LUA, 2);

    MONGO_TRY
        const auto reply = database->run_command(command.view());
        BSONToLua(LUA, reply.view());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_user_add) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    auto username = LUA->CheckString(2);
    auto password = LUA->CheckString(3);

    MONGO_TRY
        const auto command = make_document(
            kvp("createUser", username),
            kvp("pwd", password),
            kvp("roles", make_array())
        );

        database->run_command(command.view());
        LUA->PushBool(true);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_user_remove) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    auto username = LUA->CheckString(2);

    MONGO_TRY
        const auto command = make_document(kvp("dropUser", username));

        database->run_command(command.view());
        LUA->PushBool(true);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_collection_exists) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    const auto name = LUA->CheckString(2);

    MONGO_TRY
        LUA->PushBool(database->has_collection(name));
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_collection_get) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    const auto name = LUA->CheckString(2);

    MONGO_TRY
        const auto collection = new mongocxx::collection((*database)[name]);
        LUA->PushUserType(collection, CollectionMetaTableId);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(database_collection_create) {
    GET_SELF(database, mongocxx::database, DatabaseMetaTableId)

    const auto name = LUA->CheckString(2);

    MONGO_TRY
        const auto collection = new mongocxx::collection(database->create_collection(name));
        LUA->PushUserType(collection, CollectionMetaTableId);
    MONGO_CATCH

    return 1;
}
