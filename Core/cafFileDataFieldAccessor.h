//
// Copyright (c) 2024- Kontur. All Rights Reserved.
//
#pragma once

#include "cafDataFieldAccessor.h"
#include "cafFieldHandle.h"
#include "cafFieldIoCapability.h"

#include <fstream>

namespace caffa
{
template <class DataType>
class FileDataFieldAccessor final : public DataFieldAccessor<DataType>
{
public:
    explicit FileDataFieldAccessor( FieldHandle* fieldHandle )
        : m_fieldHandle( fieldHandle )
    {
    }

    std::unique_ptr<FileDataFieldAccessor> clone() const override
    {
        return std::make_unique<FileDataFieldAccessor>( m_fieldHandle );
    }

    DataType value() override
    {
        DataType value = {};
        if ( const auto io = m_fieldHandle->capability<FieldIoCapability>(); !( io && io->isReadable() ) )
        {
            throw std::runtime_error( "Field is not readable!" );
        }
        std::ifstream stream( m_filePath.generic_string() );
        stream >> value;
        return value;
    }

    void setValue( const DataType& value ) override
    {
        if ( const auto io = m_fieldHandle->capability<FieldIoCapability>(); !( io && io->isWritable() ) )
        {
            throw std::runtime_error( "Field is not writable!" );
        }
        std::ofstream stream( m_filePath.generic_string() );
        stream << value;
    }

    [[nodiscard]] bool hasGetter() const override
    {
        if ( const auto io = m_fieldHandle->capability<FieldIoCapability>(); io )
        {
            return io->isReadable();
        }
        return false;
    }

    [[nodiscard]] bool hasSetter() const override
    {
        if ( const auto io = m_fieldHandle->capability<FieldIoCapability>(); io )
        {
            return io->isWritable();
        }
        return false;
    }

private:
    FieldHandle*                m_fieldHandle;
    const std::filesystem::path m_filePath;
};

} // namespace caffa
