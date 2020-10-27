#pragma once

#include "cafInternalPdmXmlFieldReaderWriter.h"
#include "cafPdmFieldIoCapability.h"

#include <typeinfo>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace caf
{
template <typename FieldType>
class PdmFieldIoCap : public PdmFieldIoCapability
{
public:
    // Type traits magic to check if a template argument is a vector
    template <typename T>
    struct is_vector : public std::false_type
    {
    };
    template <typename T, typename A>
    struct is_vector<std::vector<T, A>> : public std::true_type
    {
    };

    PdmFieldIoCap( FieldType* field, bool giveOwnership )
        : PdmFieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = QString( "%1" ).arg( typeid( typename FieldType::FieldDataType ).name() );
    }

    // Xml Serializing
public:
    void readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;
    bool resolveReferences() override;

    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class PdmPtrField;

template <typename DataType>
class PdmFieldIoCap<PdmPtrField<DataType*>> : public PdmFieldIoCapability
{
    typedef PdmPtrField<DataType*> FieldType;

public:
    PdmFieldIoCap( FieldType* field, bool giveOwnership )
        : PdmFieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_dataTypeName    = DataType::classKeywordStatic();
        m_isResolved      = false;
        m_referenceString = "";
    }

    // Xml Serializing
public:
    void    readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory ) override;
    void    writeFieldData( QXmlStreamWriter& xmlStream ) const override;
    bool    resolveReferences() override;
    QString referenceString() const override;

private:
    FieldType* m_field;

    // Resolving
    QString m_referenceString;
    bool    m_isResolved;
};

template <typename DataType>
class PdmPtrArrayField;

template <typename DataType>
class PdmFieldIoCap<PdmPtrArrayField<DataType*>> : public PdmFieldIoCapability
{
    typedef PdmPtrArrayField<DataType*> FieldType;

public:
    PdmFieldIoCap( FieldType* field, bool giveOwnership )
        : PdmFieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_dataTypeName    = DataType::classKeywordStatic();
        m_isResolved      = false;
        m_referenceString = "";
    }

    // Xml Serializing
public:
    void readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;
    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;

    // Resolving
    QString m_referenceString;
    bool    m_isResolved;
};

template <typename DataType>
class PdmChildField;

template <typename DataType>
class PdmFieldIoCap<PdmChildField<DataType*>> : public PdmFieldIoCapability
{
    typedef PdmChildField<DataType*> FieldType;

public:
    PdmFieldIoCap( FieldType* field, bool giveOwnership )
        : PdmFieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = DataType::classKeywordStatic();
    }

    // Xml Serializing
public:
    void readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;
    bool resolveReferences() override;

private:
    FieldType* m_field;
};

template <typename DataType>
class PdmChildArrayField;

template <typename DataType>
class PdmFieldIoCap<PdmChildArrayField<DataType*>> : public PdmFieldIoCapability
{
    typedef PdmChildArrayField<DataType*> FieldType;

public:
    PdmFieldIoCap( FieldType* field, bool giveOwnership )
        : PdmFieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = DataType::classKeywordStatic();
    }

    // Xml Serializing
public:
    void readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;
    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class PdmField;

template <typename DataType>
class PdmFieldIoCap<PdmField<std::vector<DataType>>> : public PdmFieldIoCapability
{
    typedef PdmField<std::vector<DataType>> FieldType;

public:
    PdmFieldIoCap( FieldType* field, bool giveOwnership )
        : PdmFieldIoCapability( field, giveOwnership )
    {
        m_field = field;

        m_dataTypeName = QString( "%1" ).arg( typeid( DataType ).name() );
    }

    // Xml Serializing
public:
    void readFieldData( QXmlStreamReader& xmlStream, PdmObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;
    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<PdmFieldIoCapability>() )
    {
        new PdmFieldIoCap<FieldType>( field, true );
    }
}

template <typename FieldType>
void RegisterClassWithField( const QString& classKeyword, FieldType* field )
{
    field->setOwnerClass( classKeyword );
}

} // End of namespace caf

#include "cafPdmFieldIoCapabilitySpecializations.inl"
