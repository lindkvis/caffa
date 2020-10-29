#pragma once

#include <QJsonValue>
#include <QTextStream>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "cafInternalFieldIoHelper.h"
#include "cafInternalPdmFilePathStreamOperators.h"
#include "cafInternalPdmStreamOperators.h"
#include "cafPdmReferenceHelper.h"

namespace caf
{
class ObjectFactory;
template <typename T>
class PdmPointer;

//--------------------------------------------------------------------------------------------------
/// Generic write method for fields. Will work as long as DataType supports the stream operator
/// towards a QTextStream. Some special datatype should not specialize this method unless it is
/// impossible/awkward to implement the stream operator
/// Implemented in a proxy class to allow  partial specialization
//--------------------------------------------------------------------------------------------------
template <typename DataType>
struct FieldWriter
{
    static void writeFieldData( const DataType& fieldValue, QXmlStreamWriter& xmlStream )
    {
        QString     dataString;
        QTextStream data( &dataString, QIODevice::WriteOnly );

        // Use precision of 15 to cover most value ranges for double values
        // Default Qt behavior is precision of 6
        data.setRealNumberPrecision( 15 );

        data << fieldValue;
        xmlStream.writeCharacters( dataString );
    }
    static void writeFieldData( const DataType& fieldValue, QJsonValue& jsonValue )
    {
        QString     dataString;
        QTextStream data( &dataString, QIODevice::WriteOnly );

        // Use precision of 15 to cover most value ranges for double values
        // Default Qt behavior is precision of 6
        data.setRealNumberPrecision( 15 );

        data << fieldValue;
        jsonValue = dataString;
    }
};

template <typename DataType>
struct FieldReader
{
    static void readFieldData( DataType& fieldValue, QXmlStreamReader& xmlStream, ObjectFactory* objectFactory );
    static void readFieldData( DataType& fieldValue, const QJsonValue& jsonValue, ObjectFactory* objectFactory );
};

//--------------------------------------------------------------------------------------------------
/// Generic read method for fields. Will work as long as DataType supports the stream operator
/// towards a QTextStream. Some special datatype should not specialize this method unless it is
/// impossible/awkward to implement the stream operator
//--------------------------------------------------------------------------------------------------
template <typename DataType>
void FieldReader<DataType>::readFieldData( DataType& fieldValue, QXmlStreamReader& xmlStream, ObjectFactory* objectFactory )
{
    FieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    QString     dataString = xmlStream.text().toString();
    QTextStream data( &dataString, QIODevice::ReadOnly );
    data >> fieldValue;

    // Make stream point to end of element
    QXmlStreamReader::TokenType type = xmlStream.readNext();
    Q_UNUSED( type );
    FieldIOHelper::skipCharactersAndComments( xmlStream );
}
template <typename DataType>
void FieldReader<DataType>::readFieldData( DataType& fieldValue, const QJsonValue& jsonValue, ObjectFactory* objectFactory )
{
    QString     dataString = jsonValue.toString();
    QTextStream data( &dataString, QIODevice::ReadOnly );
    data >> fieldValue;
}

//--------------------------------------------------------------------------------------------------
/// Specialized read function for QStrings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template <>
void FieldReader<QString>::readFieldData( QString& field, QXmlStreamReader& xmlStream, ObjectFactory* objectFactory );

template <>
void FieldReader<QString>::readFieldData( QString& field, const QJsonValue& jsonValue, ObjectFactory* objectFactory );

} // End of namespace caf
