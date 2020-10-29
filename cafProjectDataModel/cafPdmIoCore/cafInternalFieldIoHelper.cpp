#include "cafInternalFieldIoHelper.h"

#include <QXmlStreamReader>

namespace caf
{
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldIOHelper::skipCharactersAndComments( QXmlStreamReader& xmlStream )
{
    QXmlStreamReader::TokenType type;
    while ( !xmlStream.atEnd() && xmlStream.isCharacters() || xmlStream.isComment() )
    {
        type = xmlStream.readNext();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldIOHelper::skipComments( QXmlStreamReader& xmlStream )
{
    QXmlStreamReader::TokenType type;
    while ( !xmlStream.atEnd() && xmlStream.isComment() )
    {
        type = xmlStream.readNext();
    }
}

} // End of namespace caf
