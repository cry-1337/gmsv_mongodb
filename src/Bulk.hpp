#ifndef GMSV_MONGODB_BULK_HPP
#define GMSV_MONGODB_BULK_HPP

#include "MongoDB.hpp"

#include <vector>
#include <mongocxx/model/write.hpp>

struct BulkState {
    Connection* connection;
    std::string database;
    std::string collection;
    std::vector<mongocxx::model::write> operations;
    std::vector<bsoncxx::document::value> storage;
};

int destroy_bulk(lua_State* L);

int bulk_execute(lua_State* L);

int bulk_insert(lua_State* L);

int bulk_remove(lua_State* L);

int bulk_update(lua_State* L);

int bulk_replace(lua_State* L);

extern int BulkMetaTableId;

#endif //GMSV_MONGODB_BULK_HPP
