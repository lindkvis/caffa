
#include "gtest.h"

#include "cafUiTreeSelectionQModel.h"

std::deque<caf::OptionItemInfo> createOptions()
{
    std::deque<caf::OptionItemInfo> options;

    {
        std::string text;

        text = "First";
        options.push_back( caf::OptionItemInfo( text, text ) );

        text = "Second";
        options.push_back( caf::OptionItemInfo( text, text ) );

        {
            text                         = "Second_a";
            caf::OptionItemInfo itemInfo = caf::OptionItemInfo( text, text );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        {
            text                         = "Second_b";
            caf::OptionItemInfo itemInfo = caf::OptionItemInfo( text, text );
            itemInfo.setLevel( 1 );
            options.push_back( itemInfo );
        }

        text = "Third";
        options.push_back( caf::OptionItemInfo( text, text ) );

        text = "Fourth";
        options.push_back( caf::OptionItemInfo( text, text ) );
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeSelectionQModelTest, BasicUsage )
{
    auto options = createOptions();

    caf::UiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    EXPECT_EQ( options.size(), myModel.optionItemCount() );

    EXPECT_EQ( 4, myModel.rowCount( myModel.index( -1, -1 ) ) );

    EXPECT_EQ( 0, myModel.rowCount( myModel.index( 0, 0 ) ) );
    EXPECT_EQ( 2, myModel.rowCount( myModel.index( 1, 0 ) ) );
    EXPECT_EQ( 0, myModel.rowCount( myModel.index( 2, 0 ) ) );
    EXPECT_EQ( 0, myModel.rowCount( myModel.index( 3, 0 ) ) );

    // Test for row out of bounds
    EXPECT_FALSE( myModel.index( 4, 0 ).isValid() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeSelectionQModelTest, ParentBehaviour )
{
    auto options = createOptions();

    caf::UiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    QModelIndex parentIndex = myModel.index( 1, 0 );
    EXPECT_EQ( 2, myModel.rowCount( parentIndex ) );

    {
        QModelIndex firstChildIndex = myModel.index( 0, 0, parentIndex );
        EXPECT_STREQ( "Second_a", myModel.data( firstChildIndex ).toString().toLatin1() );
        EXPECT_TRUE( parentIndex == myModel.parent( firstChildIndex ) );
    }

    {
        QModelIndex secondChildIndex = myModel.index( 1, 0, parentIndex );
        EXPECT_STREQ( "Second_b", myModel.data( secondChildIndex ).toString().toLatin1() );
        EXPECT_TRUE( parentIndex == myModel.parent( secondChildIndex ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeSelectionQModelTest, SetDataAndSignal )
{
    auto options = createOptions();

    caf::UiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    QModelIndex parentIndex = myModel.index( 0, 0 );

    myModel.setData( parentIndex, QVariant( true ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
TEST( UiTreeSelectionQModelTest, SetCheckedStateForItems )
{
    auto options = createOptions();

    caf::UiTreeSelectionQModel myModel;
    myModel.setOptions( nullptr, options );

    QModelIndex parentIndex     = myModel.index( 1, 0 );
    QModelIndex firstChildIndex = myModel.index( 0, 0, parentIndex );

    QModelIndexList indices;
    indices << firstChildIndex;

    myModel.setCheckedStateForItems( indices, false );

    // No test code for this behaviour, only making sure this runs without any errors
}
