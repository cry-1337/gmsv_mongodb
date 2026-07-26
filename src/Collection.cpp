#include "Collection.hpp"

#include <mongocxx/options/update.hpp>

using namespace GarrysMod::Lua;

LUA_FUNCTION(destroy_collection) {
    const auto handle = LUA->GetUserType<CollectionHandle>(1, CollectionMetaTableId);
    if (handle == nullptr) return 0;

    delete handle;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(collection_name) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    LUA->PushString(handle->collection.c_str());

    return 1;
}

LUA_FUNCTION(collection_count) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    MONGO_TRY
        auto filter = LuaTableToBSONOptional(LUA, 2);
        const int callback = GetCallback(LUA, 3);

        const std::string db = handle->database, coll = handle->collection;
        handle->connection->enqueue([db, coll, filter = std::move(filter)](mongocxx::client& client) -> ResultValue {
            return static_cast<int64_t>(client[db][coll].count_documents(filter.view()));
        }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(collection_find) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    MONGO_TRY
        auto filter = LuaTableToBSONOptional(LUA, 2);
        const int callback = GetCallback(LUA, 3);

        const std::string db = handle->database, coll = handle->collection;
        handle->connection->enqueue([db, coll, filter = std::move(filter)](mongocxx::client& client) -> ResultValue {
            auto cursor = client[db][coll].find(filter.view());

            std::vector<bsoncxx::document::value> out;
            for (auto&& document : cursor) {
                out.emplace_back(document);
            }

            return out;
        }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(collection_find_one) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    MONGO_TRY
        auto filter = LuaTableToBSONOptional(LUA, 2);
        const int callback = GetCallback(LUA, 3);

        const std::string db = handle->database, coll = handle->collection;
        handle->connection->enqueue([db, coll, filter = std::move(filter)](mongocxx::client& client) -> ResultValue {
            if (auto result = client[db][coll].find_one(filter.view())) {
                return std::move(*result);
            }

            return std::monostate{};
        }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(collection_insert) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    MONGO_TRY
        auto document = LuaTableToBSON(LUA, 2);
        const int callback = GetCallback(LUA, 3);

        const std::string db = handle->database, coll = handle->collection;
        handle->connection->enqueue([db, coll, document = std::move(document)](mongocxx::client& client) -> ResultValue {
            return client[db][coll].insert_one(document.view()).has_value();
        }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(collection_update) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    MONGO_TRY
        auto filter = LuaTableToBSON(LUA, 2);
        auto update = LuaTableToBSON(LUA, 3);
        const int callback = GetCallback(LUA, 4);

        const std::string db = handle->database, coll = handle->collection;
        handle->connection->enqueue(
            [db, coll, filter = std::move(filter), update = std::move(update)](mongocxx::client& client) -> ResultValue {
                return client[db][coll].update_many(filter.view(), update.view()).has_value();
            }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(collection_upsert) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    MONGO_TRY
        auto filter = LuaTableToBSON(LUA, 2);
        auto update = LuaTableToBSON(LUA, 3);
        const int callback = GetCallback(LUA, 4);

        const std::string db = handle->database, coll = handle->collection;
        handle->connection->enqueue(
            [db, coll, filter = std::move(filter), update = std::move(update)](mongocxx::client& client) -> ResultValue {
                mongocxx::options::update options;
                options.upsert(true);

                return client[db][coll].update_one(filter.view(), update.view(), options).has_value();
            }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(collection_remove) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    MONGO_TRY
        auto filter = LuaTableToBSON(LUA, 2);
        const int callback = GetCallback(LUA, 3);

        const std::string db = handle->database, coll = handle->collection;
        handle->connection->enqueue([db, coll, filter = std::move(filter)](mongocxx::client& client) -> ResultValue {
            return client[db][coll].delete_many(filter.view()).has_value();
        }, callback);
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(collection_bulk) {
    GET_SELF(handle, CollectionHandle, CollectionMetaTableId)

    auto* bulk = new BulkState{handle->connection, handle->database, handle->collection, {}, {}};
    LUA->PushUserType(bulk, BulkMetaTableId);

    return 1;
}
