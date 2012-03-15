#pragma once
#include <QString>
/// @brief Interface for signature mappers: translate from a
/// QObject method signature to a Lua function name
struct ILuaSignatureMapper {
    /// Map method signature to Lua function name
    virtual QString map( const QString& ) const = 0;
    /// Required virtual destructor to allow derived classes to
    /// invoke proper finalization code
    virtual ~ILuaSignatureMapper() {}
};


