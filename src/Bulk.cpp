#include "Bulk.hpp"

#include <memory>
#include <bsoncxx/builder/basic/document.hpp>

using namespace GarrysMod::Lua;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;

#define GET_BULK() \
    auto bulk = LUA->GetUserType<BulkState>(1, BulkMetaTableId); \
    if (bulk == nullptr) return 0;

LUA_FUNCTION(destroy_bulk) {
    GET_BULK()

    delete bulk;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(bulk_insert) {
    GET_BULK()

    auto document = LuaTableToBSON(LUA, 2);

    MONGO_TRY
        bulk->storage.push_back(std::move(document));
        bulk->operations.emplace_back(mongocxx::model::insert_one(bulk->storage.back().view()));
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(bulk_remove) {
    GET_BULK()

    auto filter = LuaTableToBSON(LUA, 2);

    MONGO_TRY
        bulk->storage.push_back(std::move(filter));
        bulk->operations.emplace_back(mongocxx::model::delete_one(bulk->storage.back().view()));
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(bulk_update) {
    GET_BULK()

    LUA->CheckType(2, Type::Table);
    LUA->CheckType(3, Type::Table);

    auto filter = LuaTableToBSON(LUA, 2);
    auto update = LuaTableToBSON(LUA, 3);

    MONGO_TRY
        bulk->storage.push_back(std::move(filter));
        const auto filter_view = bulk->storage.back().view();

        bulk->storage.push_back(std::move(update));
        const auto update_view = bulk->storage.back().view();

        bulk->operations.emplace_back(mongocxx::model::update_one(filter_view, update_view));
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(bulk_replace) {
    GET_BULK()

    LUA->CheckType(2, Type::Table);
    LUA->CheckType(3, Type::Table);

    auto filter = LuaTableToBSON(LUA, 2);
    auto replacement = LuaTableToBSON(LUA, 3);

    MONGO_TRY
        bulk->storage.push_back(std::move(filter));
        const auto filter_view = bulk->storage.back().view();

        bulk->storage.push_back(std::move(replacement));
        const auto replacement_view = bulk->storage.back().view();

        bulk->operations.emplace_back(mongocxx::model::replace_one(filter_view, replacement_view));
    MONGO_CATCH

    return 0;
}

LUA_FUNCTION(bulk_execute) {
    GET_BULK()

    const int callback = GetCallback(LUA, 2);

    const std::string db = bulk->database, coll = bulk->collection;

    auto operations = std::make_shared<std::vector<mongocxx::model::write>>(std::move(bulk->operations));
    auto storage = std::make_shared<std::vector<bsoncxx::document::value>>(std::move(bulk->storage));

    bulk->connection->enqueue(
        [db, coll, operations, storage](mongocxx::client& client) -> ResultValue {
            (void) storage;

            auto result = client[db][coll].bulk_write(*operations);
            if (!result) return std::monostate{};

            return make_document(
                kvp("inserted", result->inserted_count()),
                kvp("matched", result->matched_count()),
                kvp("modified", result->modified_count()),
                kvp("deleted", result->deleted_count()),
                kvp("upserted", result->upserted_count())
            );
        }, callback);

    return 0;
}
