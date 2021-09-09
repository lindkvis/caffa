//##################################################################################################
//
//   CAFFA
//   Copyright (C) 2021- 3d-Radar AS
//
//   GNU Lesser General Public License Usage
//   This library is free software; you can redistribute it and/or modify
//   it under the terms of the GNU Lesser General Public License as published by
//   the Free Software Foundation; either version 2.1 of the License, or
//   (at your option) any later version.
//
//   This library is distributed in the hope that it will be useful, but WITHOUT ANY
//   WARRANTY; without even the implied warranty of MERCHANTABILITY or
//   FITNESS FOR A PARTICULAR PURPOSE.
//
//   See the GNU Lesser General Public License at <<http://www.gnu.org/licenses/lgpl-2.1.html>>
//   for more details.
//
//##################################################################################################
#pragma once

#include "cafFieldHandle.h"
#include "cafLogger.h"
#include "cafPortableDataType.h"

namespace caffa
{
class RegisterFieldAccessorInterface
{
public:
    virtual uint32_t read( uint32_t addressOffset ) const            = 0;
    virtual void     write( uint32_t addressOffset, uint32_t value ) = 0;
};

class RegisterField : public FieldHandle
{
public:
    RegisterField() {}
    ~RegisterField() override {}

    // Basic access
    uint32_t read( uint32_t addressOffset ) const { return m_fieldDataAccessor->read( addressOffset ); }
    void     write( uint32_t addressOffset, uint32_t value ) { m_fieldDataAccessor->write( addressOffset, value ); }

    template <typename MaskEnum>
    uint32_t readMaskedValue( uint32_t addressOffset,
                              uint32_t ( *unmaskingMethod )( MaskEnum, uint32_t ),
                              MaskEnum maskEnumValue ) const
    {
        uint32_t maskedValue = this->read( addressOffset );
        return unmaskingMethod( maskEnumValue, maskedValue );
    }

    template <typename MaskEnum>
    void writeMaskedValue( uint32_t addressOffset,
                           uint32_t ( *maskingMethod )( MaskEnum, uint32_t ),
                           MaskEnum maskEnumValue,
                           uint32_t value )
    {
        uint32_t maskedNewValue = maskingMethod( maskEnumValue, value );

        uint32_t existingValue = this->read( addressOffset );
        uint32_t allMask       = maskingMethod( maskEnumValue, (uint32_t)-1 );

        uint32_t writeValue = ( existingValue & ~allMask ) | maskedNewValue;

        this->write( addressOffset, writeValue );
    }

    // Replace accessor
    void setAccessor( std::unique_ptr<RegisterFieldAccessorInterface> accessor )
    {
        m_fieldDataAccessor = std::move( accessor );
    }

    std::string dataType() const override { return PortableDataType<uint32_t>::name(); }

protected:
    std::unique_ptr<RegisterFieldAccessorInterface> m_fieldDataAccessor;

private:
    RegisterField( const RegisterField& other ) = delete;
    RegisterField& operator=( const RegisterField& other ) = delete;
};

} // End of namespace caffa
