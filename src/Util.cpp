#include "Util.hpp"

using namespace GarrysMod::Lua;

static std::string LuaTableToJSON(ILuaBase* LUA, int index) {
    LUA->PushSpecial(SPECIAL_GLOB);
    LUA->GetField(-1, "util");
    LUA->GetField(-1, "TableToJSON");
    LUA->Push(index);

    if (LUA->PCall(1, 1, 0) != 0) {
        std::string err = LUA->GetString(-1);
        LUA->Pop(3);
        throw std::runtime_error(err);
    }

    if (!LUA->IsType(-1, Type::String)) {
        LUA->Pop(3);
        throw std::runtime_error("Invalid table passed to MongoDB!");
    }

    std::string json = LUA->GetString(-1);
    LUA->Pop(3);

    return json;
}

bsoncxx::document::value LuaTableToBSON(ILuaBase* LUA, int index) {
    LUA->CheckType(index, Type::Table);

    std::string json = LuaTableToJSON(LUA, index);

    try {
        return bsoncxx::from_json(json);
    } catch (const std::exception& e) {
        throw std::runtime_error(e.what());
    }
}

bsoncxx::document::value LuaTableToBSONOptional(ILuaBase* LUA, int index) {
    if (!LUA->IsType(index, Type::Table)) {
        return bsoncxx::from_json("{}");
    }

    return LuaTableToBSON(LUA, index);
}

static void PushOID(ILuaBase* LUA, const bsoncxx::oid& oid) {
    auto copy = new bsoncxx::oid(oid);
    LUA->PushUserType(copy, ObjectIDMetaTableId);
}

static void PushValue(ILuaBase* LUA, const bsoncxx::types::bson_value::view& value) {
    switch (value.type()) {
        case bsoncxx::type::k_double:
            LUA->PushNumber(value.get_double().value);
            break;
        case bsoncxx::type::k_int32:
            LUA->PushNumber(static_cast<double>(value.get_int32().value));
            break;
        case bsoncxx::type::k_int64:
            LUA->PushNumber(static_cast<double>(value.get_int64().value));
            break;
        case bsoncxx::type::k_bool:
            LUA->PushBool(value.get_bool().value);
            break;
        case bsoncxx::type::k_string: {
            auto str = value.get_string().value;
            LUA->PushString(str.data(), static_cast<unsigned int>(str.size()));
            break;
        }
        case bsoncxx::type::k_oid:
            PushOID(LUA, value.get_oid().value);
            break;
        case bsoncxx::type::k_date:
            LUA->PushNumber(static_cast<double>(value.get_date().to_int64()));
            break;
        case bsoncxx::type::k_document:
            BSONToLua(LUA, value.get_document().value);
            break;
        case bsoncxx::type::k_array:
            BSONArrayToLua(LUA, value.get_array().value);
            break;
        default:
            LUA->PushNil();
            break;
    }
}

void BSONToLua(ILuaBase* LUA, const bsoncxx::document::view& view) {
    LUA->CreateTable();

    for (auto&& element : view) {
        PushValue(LUA, element.get_value());

        std::string key(element.key().data(), element.key().size());
        LUA->SetField(-2, key.c_str());
    }
}

void BSONArrayToLua(ILuaBase* LUA, const bsoncxx::array::view& view) {
    LUA->CreateTable();

    int i = 0;
    for (auto&& element : view) {
        LUA->PushNumber(++i);
        PushValue(LUA, element.get_value());
        LUA->SetTable(-3);
    }
}
