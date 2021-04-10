#pragma once

#include "cafDefaultObjectFactory.h"
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
#define CAF_OBJECT_STRING_CONCATENATE( foo, bar ) CAF_OBJECT_STRING_CONCATENATE_IMPL_( foo, bar )
#define CAF_OBJECT_STRING_CONCATENATE_IMPL_( foo, bar ) foo##bar

// In order for a keyword to be valid, it has to be valid in all the IO implementations provided.
#define CAF_VERIFY_IO_KEYWORD( keyword )                                                                    \
    static_assert( isFirstCharacterValidInXmlKeyword( keyword ), "First character in keyword is invalid" ); \
    static_assert( !isFirstThreeCharactersXml( keyword ), "Keyword starts with invalid sequence xml" );     \
    static_assert( isValidXmlKeyword( keyword ), "Detected invalid character in keyword" );

/// CAF_HEADER_INIT assists the factory used when reading objects from file
/// Place this in the header file inside the class definition of your Object

#define CAF_IO_HEADER_INIT                                                                            \
public:                                                                                               \
    virtual std::string             classKeyword() const override;                                    \
    static std::string              classKeywordStatic();                                             \
    static std::vector<std::string> classKeywordAliases();                                            \
    virtual bool                    matchesClassKeyword( const std::string& keyword ) const override; \
                                                                                                      \
    static bool Error_You_forgot_to_add_the_macro_CAF_IO_HEADER_INIT_and_or_CAF_IO_SOURCE_INIT_to_your_cpp_file_for_this_class()

#define CAF_IO_ABSTRACT_SOURCE_INIT( ClassName, keyword, ... )                                                                       \
    bool ClassName::Error_You_forgot_to_add_the_macro_CAF_IO_HEADER_INIT_and_or_CAF_IO_SOURCE_INIT_to_your_cpp_file_for_this_class() \
    {                                                                                                                                \
        return false;                                                                                                                \
    }                                                                                                                                \
                                                                                                                                     \
    std::string              ClassName::classKeyword() const { return classKeywordStatic(); }                                        \
    std::string              ClassName::classKeywordStatic() { return classKeywordAliases().front(); }                               \
    std::vector<std::string> ClassName::classKeywordAliases()                                                                        \
    {                                                                                                                                \
        CAF_VERIFY_IO_KEYWORD( keyword )                                                                                             \
        return { keyword, ##__VA_ARGS__ };                                                                                           \
    }                                                                                                                                \
    bool ClassName::matchesClassKeyword( const std::string& matchKeyword ) const                                                     \
    {                                                                                                                                \
        auto aliases = classKeywordAliases();                                                                                        \
        for ( auto alias : aliases )                                                                                                 \
        {                                                                                                                            \
            if ( alias == matchKeyword ) return true;                                                                                \
        }                                                                                                                            \
        return false;                                                                                                                \
    }

/// CAF_IO_SOURCE_INIT associates the file keyword used for storage with the class and
//  initializes the factory
/// Place this in the cpp file, preferably above the constructor
#define CAF_IO_SOURCE_INIT( ClassName, keyword, ... )                      \
    CAF_IO_ABSTRACT_SOURCE_INIT( ClassName, keyword, ##__VA_ARGS__ )       \
    static bool CAF_OBJECT_STRING_CONCATENATE( my##ClassName, __LINE__ ) = \
        caf::DefaultObjectFactory::instance()->registerCreator<ClassName>()

#define CAF_IO_InitField( field, keyword )                                                                                    \
    {                                                                                                                         \
        CAF_VERIFY_IO_KEYWORD( keyword )                                                                                      \
        static bool checkingThePresenceOfHeaderAndSourceInitMacros =                                                          \
            Error_You_forgot_to_add_the_macro_CAF_IO_HEADER_INIT_and_or_CAF_IO_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
        this->isInheritedFromSerializable();                                                                                  \
                                                                                                                              \
        AddIoCapabilityToField( ( field ) );                                                                                  \
        addField( ( field ), ( keyword ) );                                                                                   \
    }
