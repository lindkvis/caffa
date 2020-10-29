#include "cafInternalPdmIoFieldReaderWriter.h"

#include "cafInternalFieldIoHelper.h"

namespace caf
{
//--------------------------------------------------------------------------------------------------
/// Specialized read function for QStrings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template <>
void FieldReader<QString>::readFieldData( QString& field, QXmlStreamReader& xmlStream, ObjectFactory* )
{
    FieldIOHelper::skipComments( xmlStream );
    if ( !xmlStream.isCharacters() ) return;

    field = xmlStream.text().toString();

    // Make stream point to end of element
    QXmlStreamReader::TokenType type;
    type = xmlStream.readNext();
    FieldIOHelper::skipCharactersAndComments( xmlStream );
}

//--------------------------------------------------------------------------------------------------
/// Specialized read function for QStrings, because the >> operator only can read word by word
//--------------------------------------------------------------------------------------------------
template <>
void FieldReader<QString>::readFieldData( QString& field, const QJsonValue& jsonValue, ObjectFactory* )
{
    field = jsonValue.toString();
}
} // End of namespace caf
