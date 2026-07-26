#include "ObjectID.hpp"

#include <functional>
#include <string_view>

using namespace GarrysMod::Lua;

#define GET_OID() \
    auto oid = LUA->GetUserType<bsoncxx::oid>(1, ObjectIDMetaTableId); \
    if (oid == nullptr) return 0;

LUA_FUNCTION(new_objectid) {
    MONGO_TRY
        if (LUA->IsType(1, Type::String)) {
            unsigned int len;
            const auto str = LUA->GetString(1, &len);

            const auto oid = new bsoncxx::oid(std::string_view(str, len));
            LUA->PushUserType(oid, ObjectIDMetaTableId);
        } else if (LUA->IsType(1, Type::Nil) || LUA->IsType(1, Type::None)) {
            const auto oid = new bsoncxx::oid();
            LUA->PushUserType(oid, ObjectIDMetaTableId);
        } else {
            LUA->ThrowError("Invalid type passed to ObjectID");
            return 0;
        }
    MONGO_CATCH

    return 1;
}

LUA_FUNCTION(destroy_objectid) {
    GET_OID()

    delete oid;
    LUA->SetUserType(1, nullptr);

    return 0;
}

LUA_FUNCTION(objectid_tostring) {
    GET_OID()

    const std::string str = oid->to_string();
    LUA->PushString(str.c_str());

    return 1;
}

LUA_FUNCTION(objectid_eq) {
    const auto a = LUA->GetUserType<bsoncxx::oid>(1, ObjectIDMetaTableId);
    const auto b = LUA->GetUserType<bsoncxx::oid>(2, ObjectIDMetaTableId);

    LUA->PushBool(a != nullptr && b != nullptr && *a == *b);

    return 1;
}

LUA_FUNCTION(objectid_data) {
    GET_OID()

    LUA->PushString(oid->bytes(), static_cast<unsigned int>(oid->size()));

    return 1;
}

LUA_FUNCTION(objectid_hash) {
    GET_OID()

    LUA->PushNumber(static_cast<double>(std::hash<std::string>{}(oid->to_string())));

    return 1;
}
