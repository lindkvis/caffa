#include "cafColorTables.h"
#include "cafColorTools.h"

#include "cafAssert.h"

#include <QColor>

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable ColorTables::kellyColors()
{
    static std::vector<QColor> hexColors{
        QColor( "#FFB300" ), // Vivid Yellow
        QColor( "#803E75" ), // Strong Purple
        QColor( "#FF6800" ), // Vivid Orange
        QColor( "#A6BDD7" ), // Very Light Blue
        QColor( "#C10020" ), // Vivid Red
        QColor( "#CEA262" ), // Grayish Yellow
        QColor( "#817066" ), // Medium Gray

        // The following will not be good for people with defective color vision
        QColor( "#007D34" ), // Vivid Green
        QColor( "#F6768E" ), // Strong Purplish Pink
        QColor( "#00538A" ), // Strong Blue
        QColor( "#FF7A5C" ), // Strong Yellowish Pink
        QColor( "#53377A" ), // Strong Violet
        QColor( "#FF8E00" ), // Vivid Orange Yellow
        QColor( "#B32851" ), // Strong Purplish Red
        QColor( "#F4C800" ), // Vivid Greenish Yellow
        QColor( "#7F180D" ), // Strong Reddish Brown
        QColor( "#93AA00" ), // Vivid Yellowish Green
        QColor( "#593315" ), // Deep Yellowish Brown
        QColor( "#F13A13" ), // Vivid Reddish Orange
        QColor( "#232C16" ), // Dark Olive Green
    };

    return ColorTable( hexColors );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable ColorTables::salmonContrast()
{
    static std::vector<QColor> hexColors{ QColor( "#adbca5" ),
                                          QColor( "#e8b9ab" ),
                                          QColor( "#e09891" ),
                                          QColor( "#cb769e" ),
                                          QColor( "#8c5f66" ),
                                          QColor( "#484a47" ),
                                          QColor( "#5c6d70" ),
                                          QColor( "#a37774" ),
                                          QColor( "#52796f" ),
                                          QColor( "#354f52ff" ) };
    return ColorTable( hexColors );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable ColorTables::svgColors( int maxCount )
{
    QStringList colorNames = QColor::colorNames();

    std::vector<QColor> colors;
    for ( int i = 0; i < std::min( maxCount, colorNames.size() ); ++i )
    {
        colors.push_back( QColor( colorNames[i] ) );
    }
    return ColorTable( colors );
}
