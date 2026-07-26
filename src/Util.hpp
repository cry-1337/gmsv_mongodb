#ifndef GMSV_MONGODB_UTIL_HPP
#define GMSV_MONGODB_UTIL_HPP

#include "MongoDB.hpp"

bsoncxx::document::value LuaTableToBSON(GarrysMod::Lua::ILuaBase* LUA, int index);

bsoncxx::document::value LuaTableToBSONOptional(GarrysMod::Lua::ILuaBase* LUA, int index);

void BSONToLua(GarrysMod::Lua::ILuaBase* LUA, const bsoncxx::document::view& view);
void BSONArrayToLua(GarrysMod::Lua::ILuaBase* LUA, const bsoncxx::array::view& view);

#endif //GMSV_MONGODB_UTIL_HPP
