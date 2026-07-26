#include "Bulk.hpp"

using namespace GarrysMod::Lua;

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

    MONGO_TRY
        auto result = bulk->collection.bulk_write(bulk->operations);

        LUA->CreateTable();

        if (result) {
            LUA->PushNumber(result->inserted_count());
            LUA->SetField(-2, "inserted");
            LUA->PushNumber(result->matched_count());
            LUA->SetField(-2, "matched");
            LUA->PushNumber(result->modified_count());
            LUA->SetField(-2, "modified");
            LUA->PushNumber(result->deleted_count());
            LUA->SetField(-2, "deleted");
            LUA->PushNumber(result->upserted_count());
            LUA->SetField(-2, "upserted");
        }
    MONGO_CATCH

    return 1;
}
