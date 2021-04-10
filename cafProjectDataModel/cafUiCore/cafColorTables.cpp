#include "cafColorTables.h"

#include "cafAssert.h"
#include "cafColor.h"
#include "cafColorTools.h"

using namespace caf;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable ColorTables::kellyColors()
{
    static std::vector<Color> hexColors{
        Color( "#FFB300" ), // Vivid Yellow
        Color( "#803E75" ), // Strong Purple
        Color( "#FF6800" ), // Vivid Orange
        Color( "#A6BDD7" ), // Very Light Blue
        Color( "#C10020" ), // Vivid Red
        Color( "#CEA262" ), // Grayish Yellow
        Color( "#817066" ), // Medium Gray

        // The following will not be good for people with defective color vision
        Color( "#007D34" ), // Vivid Green
        Color( "#F6768E" ), // Strong Purplish Pink
        Color( "#00538A" ), // Strong Blue
        Color( "#FF7A5C" ), // Strong Yellowish Pink
        Color( "#53377A" ), // Strong Violet
        Color( "#FF8E00" ), // Vivid Orange Yellow
        Color( "#B32851" ), // Strong Purplish Red
        Color( "#F4C800" ), // Vivid Greenish Yellow
        Color( "#7F180D" ), // Strong Reddish Brown
        Color( "#93AA00" ), // Vivid Yellowish Green
        Color( "#593315" ), // Deep Yellowish Brown
        Color( "#F13A13" ), // Vivid Reddish Orange
        Color( "#232C16" ), // Dark Olive Green
    };

    return ColorTable( hexColors );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ColorTable ColorTables::salmonContrast()
{
    static std::vector<Color> hexColors{ Color( "#adbca5" ),
                                         Color( "#e8b9ab" ),
                                         Color( "#e09891" ),
                                         Color( "#cb769e" ),
                                         Color( "#8c5f66" ),
                                         Color( "#484a47" ),
                                         Color( "#5c6d70" ),
                                         Color( "#a37774" ),
                                         Color( "#52796f" ),
                                         Color( "#354f52ff" ) };
    return ColorTable( hexColors );
}
