#ifndef GMSV_MONGODB_MACROS_HPP
#define GMSV_MONGODB_MACROS_HPP

// fetch the userdata for `self` (Lua argument 1). Returns nil (0) when absent/wrong type.
#define GET_SELF(VAR, TYPE, META) \
    auto VAR = LUA->GetUserType<TYPE>(1, META); \
    if (VAR == nullptr) return 0;

// turn any std::exception thrown by the driver into a catchable Lua error instead of a crash.
#define MONGO_TRY try {
#define MONGO_CATCH } catch (const std::exception& e) { LUA->ThrowError(e.what()); return 0; }

#endif //GMSV_MONGODB_MACROS_HPP
