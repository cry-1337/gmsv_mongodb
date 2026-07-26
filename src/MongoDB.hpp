#ifndef GMSV_MONGODB_MONGODB_HPP
#define GMSV_MONGODB_MONGODB_HPP

#include <stdexcept>
#include <string>
#include <cstdint>

#include <mongocxx/instance.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/database.hpp>
#include <mongocxx/collection.hpp>
#include <mongocxx/bulk_write.hpp>
#include <mongocxx/uri.hpp>
#include <mongocxx/exception/exception.hpp>

#include <bsoncxx/oid.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <bsoncxx/document/value.hpp>
#include <bsoncxx/document/view.hpp>
#include <bsoncxx/array/view.hpp>

#include <GarrysMod/Lua/Interface.h>
#include <GarrysMod/Lua/Types.h>

#include "Macros.hpp"
#include "Async.hpp"
#include "Util.hpp"
#include "ObjectID.hpp"
#include "Client.hpp"
#include "Database.hpp"
#include "Collection.hpp"
#include "Bulk.hpp"

#endif //GMSV_MONGODB_MONGODB_HPP
