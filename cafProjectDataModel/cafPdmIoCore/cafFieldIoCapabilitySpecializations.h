#pragma once

#include "cafInternalPdmIoFieldReaderWriter.h"
#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafFieldIoCapability.h"
#include "cafPdmPtrArrayField.h"
#include "cafPdmPtrField.h"

#include <typeinfo>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace caf
{
template <typename FieldType>
class FieldIoCap : public FieldIoCapability
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

    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = QString( "%1" ).arg( typeid( typename FieldType::FieldDataType ).name() );
    }

public:
    // Xml Serializing
    void readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;

    // Json Serializing
    void readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( QJsonValue& jsonValue ) const override;

    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class PdmPtrField;

template <typename DataType>
class FieldIoCap<PdmPtrField<DataType*>> : public FieldIoCapability
{
    typedef PdmPtrField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_dataTypeName    = DataType::classKeywordStatic();
        m_isResolved      = false;
        m_referenceString = "";
    }

    // Xml Serializing
public:
    // Xml Serializing
    void readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;

    // Json Serializing
    void readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( QJsonValue& jsonValue ) const override;

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
class FieldIoCap<PdmPtrArrayField<DataType*>> : public FieldIoCapability
{
    typedef PdmPtrArrayField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field           = field;
        m_dataTypeName    = DataType::classKeywordStatic();
        m_isResolved      = false;
        m_referenceString = "";
    }

public:
    // Xml Serializing
    void readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;

    // Json Serializing
    void readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( QJsonValue& jsonValue ) const override;

    bool    resolveReferences() override;
    QString referenceString() const override;
    bool    isVectorField() const override;

private:
    FieldType* m_field;

    // Resolving
    QString m_referenceString;
    bool    m_isResolved;
};

template <typename DataType>
class ChildField;

template <typename DataType>
class FieldIoCap<ChildField<DataType*>> : public FieldIoCapability
{
    typedef ChildField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = DataType::classKeywordStatic();
    }

public:
    // Xml Serializing
    void readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;

    // Json Serializing
    void readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( QJsonValue& jsonValue ) const override;

    bool resolveReferences() override;

private:
    FieldType* m_field;
};

template <typename DataType>
class ChildArrayField;

template <typename DataType>
class FieldIoCap<ChildArrayField<DataType*>> : public FieldIoCapability
{
    typedef ChildArrayField<DataType*> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field        = field;
        m_dataTypeName = DataType::classKeywordStatic();
    }

public:
    // Xml Serializing
    void readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;

    // Json Serializing
    void readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( QJsonValue& jsonValue ) const override;

    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename DataType>
class Field;

template <typename DataType>
class FieldIoCap<Field<std::vector<DataType>>> : public FieldIoCapability
{
    typedef Field<std::vector<DataType>> FieldType;

public:
    FieldIoCap( FieldType* field, bool giveOwnership )
        : FieldIoCapability( field, giveOwnership )
    {
        m_field = field;

        m_dataTypeName = QString( "%1" ).arg( typeid( DataType ).name() );
    }

public:
    // Xml Serializing
    void readFieldData( QXmlStreamReader& xmlStream, ObjectFactory* objectFactory ) override;
    void writeFieldData( QXmlStreamWriter& xmlStream ) const override;

    // Json Serializing
    void readFieldData( const QJsonValue& jsonValue, ObjectFactory* objectFactory ) override;
    void writeFieldData( QJsonValue& jsonValue ) const override;

    bool resolveReferences() override;
    bool isVectorField() const override;

private:
    FieldType* m_field;
};

template <typename FieldType>
void AddIoCapabilityToField( FieldType* field )
{
    if ( !field->template capability<FieldIoCapability>() )
    {
        new FieldIoCap<FieldType>( field, true );
    }
}

template <typename FieldType>
void RegisterClassWithField( const QString& classKeyword, FieldType* field )
{
    field->setOwnerClass( classKeyword );
}

} // End of namespace caf

#include "cafFieldIoCapabilitySpecializations.inl"
