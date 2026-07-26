#include "Collection.hpp"

using namespace GarrysMod::Lua;

LUA_FUNCTION(destroy_collection) {
    const auto collection = LUA->GetUserType<mongocxx::collection>(1, CollectionMetaTableId);
    if (collection == nullptr) return 0;

    delete collection;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(collection_name) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    MONGO_TRY
        const auto view = collection->name();
        const std::string name(view.data(), view.size());
        LUA->PushString(name.c_str());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(collection_count) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    const auto filter = LuaTableToBSONOptional(LUA, 2);

    MONGO_TRY
        const int64_t count = collection->count_documents(filter.view());
        LUA->PushNumber(static_cast<double>(count));
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(collection_find) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    const auto filter = LuaTableToBSONOptional(LUA, 2);

    MONGO_TRY
        auto cursor = collection->find(filter.view());

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

LUA_FUNCTION(collection_find_one) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    const auto filter = LuaTableToBSONOptional(LUA, 2);

    MONGO_TRY
        if (auto result = collection->find_one(filter.view())) {
            BSONToLua(LUA, result->view());
        } else {
            LUA->PushNil();
        }
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(collection_insert) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    const auto document = LuaTableToBSON(LUA, 2);

    MONGO_TRY
        const auto result = collection->insert_one(document.view());
        LUA->PushBool(result.has_value());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(collection_update) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    const auto filter = LuaTableToBSON(LUA, 2);
    const auto update = LuaTableToBSON(LUA, 3);

    MONGO_TRY
        const auto result = collection->update_many(filter.view(), update.view());
        LUA->PushBool(result.has_value());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(collection_remove) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    const auto filter = LuaTableToBSON(LUA, 2);

    MONGO_TRY
        const auto result = collection->delete_many(filter.view());
        LUA->PushBool(result.has_value());
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(collection_bulk) {
    GET_SELF(collection, mongocxx::collection, CollectionMetaTableId)

    MONGO_TRY
        const auto bulk = new BulkState(*collection);
        LUA->PushUserType(bulk, BulkMetaTableId);
    MONGO_CATCH

    return 1;
}
