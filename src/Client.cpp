#include "Client.hpp"

#include <memory>

using namespace GarrysMod::Lua;

static const auto PING = bsoncxx::from_json(R"({"ping":1})");

LUA_FUNCTION(new_client) {
    std::string connection = LUA->CheckString(1);

    if (LUA->IsType(2, Type::String)) {
        if (const std::string app = LUA->GetString(2); !app.empty()) {
            connection += (connection.find('?') == std::string::npos ? "?appName=" : "&appName=");
            connection += app;
        }
    }

    MONGO_TRY
        mongocxx::uri uri{connection};
        auto client = std::make_unique<mongocxx::client>(uri);

        (*client)["admin"].run_command(PING.view());

        LUA->PushUserType(client.release(), ClientMetaTableId);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(destroy_client) {
    const auto client = LUA->GetUserType<mongocxx::client>(1, ClientMetaTableId);
    if (client == nullptr) return 0;

    delete client;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(client_command) {
    GET_SELF(client, mongocxx::client, ClientMetaTableId)

    const auto database = LUA->CheckString(2);
    const auto command = LuaTableToBSON(LUA, 3);

    MONGO_TRY
        const auto reply = (*client)[database].run_command(command.view());
        BSONToLua(LUA, reply.view());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(client_uri) {
    GET_SELF(client, mongocxx::client, ClientMetaTableId)

    MONGO_TRY
        const std::string uri = client->uri().to_string();
        LUA->PushString(uri.c_str());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(client_default_database) {
    GET_SELF(client, mongocxx::client, ClientMetaTableId)

    MONGO_TRY
        const std::string name = client->uri().database();
        if (name.empty()) {
            LUA->ThrowError("The connection URI has no default database!");
            return 0;
        }

        const auto db = new mongocxx::database(client->database(name));
        LUA->PushUserType(db, DatabaseMetaTableId);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(client_list_databases) {
    GET_SELF(client, mongocxx::client, ClientMetaTableId)

    MONGO_TRY
        auto cursor = client->list_databases();

        LUA->CreateTable();

        int i = 0;
        for (auto&& doc : cursor) {
            LUA->PushNumber(++i);
            BSONToLua(LUA, doc);
            LUA->SetTable(-3);
        }
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(client_database) {
    GET_SELF(client, mongocxx::client, ClientMetaTableId)

    const auto name = LUA->CheckString(2);

    MONGO_TRY
        auto db = std::make_unique<mongocxx::database>(client->database(name));

        db->run_command(PING.view());

        LUA->PushUserType(db.release(), DatabaseMetaTableId);
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(client_collection) {
    GET_SELF(client, mongocxx::client, ClientMetaTableId)

    const auto database = LUA->CheckString(2);
    const auto name = LUA->CheckString(3);

    MONGO_TRY
        const auto collection = new mongocxx::collection((*client)[database][name]);
        LUA->PushUserType(collection, CollectionMetaTableId);
    MONGO_CATCH

    return 1;
}
