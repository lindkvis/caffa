#include "cafFilePath.h"

#include <QTextStream>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FilePath::FilePath() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::FilePath::FilePath(const QString& filePath) : m_filePath(filePath) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString caf::FilePath::path() const
{
    return m_filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::FilePath::setPath(const QString& filePath)
{
    m_filePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void caf::FilePath::operator=(const FilePath& other)
{
    m_filePath = other.m_filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool caf::FilePath::operator==(const FilePath& other) const
{
    return m_filePath == other.m_filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator>>(QTextStream& str, caf::FilePath& filePath)
{
    QString text;

    while (str.status() == QTextStream::Ok)
    {
        // Read QChar to avoid white space trimming when reading QString
        QChar singleChar;
        str >> singleChar;

        if (!singleChar.isNull())
        {
            text += singleChar;
        }
    }

    filePath.setPath(text);

    return str;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QTextStream& operator<<(QTextStream& str, const caf::FilePath& filePath)
{
    str << filePath.path();

    return str;
}
