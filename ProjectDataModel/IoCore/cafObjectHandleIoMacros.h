#pragma once

#include "cafDefaultObjectFactory.h"
#include "cafFieldIoCapabilitySpecializations.h"
#include "cafXmlStringValidation.h"

// Taken from gtest.h
//
// Due to C++ preprocessor weirdness, we need double indirection to
// concatenate two tokens when one of them is __LINE__.  Writing
//
//   foo ## __LINE__
//
// will result in the token foo__LINE__, instead of foo followed by
// the current line number.  For more details, see
// http://www.parashift.com/c++-faq-lite/misc-technical-issues.html#faq-39.6
#define CAFFA_OBJECT_STRING_CONCATENATE( foo, bar ) CAFFA_OBJECT_STRING_CONCATENATE_IMPL_( foo, bar )
#define CAFFA_OBJECT_STRING_CONCATENATE_IMPL_( foo, bar ) foo##bar

// In order for a keyword to be valid, it has to be valid in all the IO implementations provided.
#define CAFFA_VERIFY_IO_KEYWORD( keyword )                                                                  \
    static_assert( isFirstCharacterValidInXmlKeyword( keyword ), "First character in keyword is invalid" ); \
    static_assert( !isFirstThreeCharactersXml( keyword ), "Keyword starts with invalid sequence xml" );     \
    static_assert( isValidXmlKeyword( keyword ), "Detected invalid character in keyword" );

/// CAFFA_HEADER_INIT assists the factory used when reading objects from file
/// Place this in the header file inside the class definition of your Object

#define CAFFA_IO_HEADER_INIT                                                                           \
public:                                                                                                \
    virtual std::string              classKeyword() const override;                                    \
    static std::string               classKeywordStatic();                                             \
    static std::vector<std::string>  classInheritanceStackStatic();                                    \
    virtual std::vector<std::string> classInheritanceStack() const override;                           \
    virtual bool                     matchesClassKeyword( const std::string& keyword ) const override; \
                                                                                                       \
    static bool Error_You_forgot_to_add_the_macro_CAFFA_IO_HEADER_INIT_and_or_CAFFA_IO_SOURCE_INIT_to_your_cpp_file_for_this_class()

#define CAFFA_IO_ABSTRACT_SOURCE_INIT( ClassName, keyword, ... )                                                                         \
    bool ClassName::Error_You_forgot_to_add_the_macro_CAFFA_IO_HEADER_INIT_and_or_CAFFA_IO_SOURCE_INIT_to_your_cpp_file_for_this_class() \
    {                                                                                                                                    \
        return false;                                                                                                                    \
    }                                                                                                                                    \
                                                                                                                                         \
    std::string              ClassName::classKeyword() const { return classKeywordStatic(); }                                            \
    std::string              ClassName::classKeywordStatic() { return classInheritanceStackStatic().front(); }                           \
    std::vector<std::string> ClassName::classInheritanceStackStatic()                                                                    \
    {                                                                                                                                    \
        CAFFA_VERIFY_IO_KEYWORD( keyword )                                                                                               \
        return { keyword, ##__VA_ARGS__ };                                                                                               \
    }                                                                                                                                    \
    std::vector<std::string> ClassName::classInheritanceStack() const { return classInheritanceStackStatic(); }                          \
    bool                     ClassName::matchesClassKeyword( const std::string& matchKeyword ) const                                     \
    {                                                                                                                                    \
        auto aliases = classInheritanceStackStatic();                                                                                    \
        for ( auto alias : aliases )                                                                                                     \
        {                                                                                                                                \
            if ( alias == matchKeyword ) return true;                                                                                    \
        }                                                                                                                                \
        return false;                                                                                                                    \
    }

/// CAFFA_IO_SOURCE_INIT associates the file keyword used for storage with the class and
//  initializes the factory
/// Place this in the cpp file, preferably above the constructor
#define CAFFA_IO_SOURCE_INIT( ClassName, keyword, ... )                      \
    CAFFA_IO_ABSTRACT_SOURCE_INIT( ClassName, keyword, ##__VA_ARGS__ )       \
    static bool CAFFA_OBJECT_STRING_CONCATENATE( my##ClassName, __LINE__ ) = \
        caffa::DefaultObjectFactory::instance()->registerCreator<ClassName>()

#define CAFFA_IO_InitField( field, keyword )                                                                                      \
    {                                                                                                                             \
        CAFFA_VERIFY_IO_KEYWORD( keyword )                                                                                        \
        static bool checkingThePresenceOfHeaderAndSourceInitMacros =                                                              \
            Error_You_forgot_to_add_the_macro_CAFFA_IO_HEADER_INIT_and_or_CAFFA_IO_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
        this->isInheritedFromSerializable();                                                                                      \
                                                                                                                                  \
        AddIoCapabilityToField( ( field ) );                                                                                      \
        addField( ( field ), ( keyword ) );                                                                                       \
    }
