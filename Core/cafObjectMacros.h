#pragma once

#include "cafDefaultObjectFactory.h"
#include "cafFieldJsonCapabilitySpecializations.h"

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wgnu-zero-variadic-macro-arguments"
#endif

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

/// CAFFA_HEADER_INIT assists the factory used when reading objects from file
/// Place this in the header file inside the class definition of your Object

#define CAFFA_HEADER_INIT                                                                              \
public:                                                                                                \
    virtual std::string              classKeyword() const override;                                    \
    static std::string               classKeywordStatic();                                             \
    static std::vector<std::string>  classInheritanceStackStatic();                                    \
    virtual std::vector<std::string> classInheritanceStack() const override;                           \
    virtual bool                     matchesClassKeyword( const std::string& keyword ) const override; \
                                                                                                       \
    static bool Error_You_forgot_to_add_the_macro_CAFFA_HEADER_INIT_and_or_CAFFA_SOURCE_INIT_to_your_cpp_file_for_this_class()

#define CAFFA_ABSTRACT_SOURCE_INIT( ClassName, keyword, ... )                                                                      \
    bool ClassName::Error_You_forgot_to_add_the_macro_CAFFA_HEADER_INIT_and_or_CAFFA_SOURCE_INIT_to_your_cpp_file_for_this_class() \
    {                                                                                                                              \
        return false;                                                                                                              \
    }                                                                                                                              \
                                                                                                                                   \
    std::string              ClassName::classKeyword() const { return classKeywordStatic(); }                                      \
    std::string              ClassName::classKeywordStatic() { return classInheritanceStackStatic().front(); }                     \
    std::vector<std::string> ClassName::classInheritanceStackStatic() { return { keyword, ##__VA_ARGS__ }; }                       \
    std::vector<std::string> ClassName::classInheritanceStack() const { return classInheritanceStackStatic(); }                    \
    bool                     ClassName::matchesClassKeyword( const std::string& matchKeyword ) const                               \
    {                                                                                                                              \
        auto aliases = classInheritanceStackStatic();                                                                              \
        for ( auto alias : aliases )                                                                                               \
        {                                                                                                                          \
            if ( alias == matchKeyword ) return true;                                                                              \
        }                                                                                                                          \
        return false;                                                                                                              \
    }

/// CAFFA_SOURCE_INIT associates the file keyword used for storage with the class and
//  initializes the factory
/// Place this in the cpp file, preferably above the constructor
#define CAFFA_SOURCE_INIT( ClassName, keyword, ... )                         \
    CAFFA_ABSTRACT_SOURCE_INIT( ClassName, keyword, ##__VA_ARGS__ )          \
    static bool CAFFA_OBJECT_STRING_CONCATENATE( my##ClassName, __LINE__ ) = \
        caffa::DefaultObjectFactory::instance()->registerCreator<ClassName>();

#ifdef __clang__
#pragma clang diagnostic pop
#endif
