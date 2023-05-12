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

#define CAFFA_HEADER_INIT( ClassName, ParentClassName )                                                                         \
public:                                                                                                                         \
    static bool Error_You_forgot_to_add_the_macro_CAFFA_HEADER_INIT_and_or_CAFFA_SOURCE_INIT_to_your_cpp_file_for_this_class(); \
    static constexpr std::string_view classKeywordStatic()                                                                      \
    {                                                                                                                           \
        constexpr auto classKeyword       = std::string_view{ #ClassName };                                                     \
        constexpr auto parentClassKeyword = std::string_view{ #ParentClassName };                                               \
        static_assert( isValidKeyword( parentClassKeyword ), "The provided parent class name is not valid" );                   \
        static_assert( isValidKeyword( classKeyword ), "The provided class name is not valid" );                                \
        return classKeyword;                                                                                                    \
    }                                                                                                                           \
    constexpr std::string_view classKeyword() const override                                                                    \
    {                                                                                                                           \
        return classKeywordStatic();                                                                                            \
    }                                                                                                                           \
    constexpr InheritanceStackType classInheritanceStack() const override                                                       \
    {                                                                                                                           \
        InheritanceStackType stack;                                                                                             \
        std::fill( stack.begin(), stack.end(), "" );                                                                            \
        stack[0]                         = classKeywordStatic();                                                                \
        auto parentClassInheritanceStack = ParentClassName::classInheritanceStack();                                            \
        std::copy_n( parentClassInheritanceStack.begin(), parentClassInheritanceStack.size() - 1, stack.begin() + 1 );          \
        return stack;                                                                                                           \
    }

/// Alternative macro that allows you to define class documentation as well
#define CAFFA_HEADER_INIT_WITH_DOC( DOCUMENTATION, ... ) \
    CAFFA_HEADER_INIT( __VA_ARGS__ );                    \
    std::string classDocumentation() const override      \
    {                                                    \
        return std::string( DOCUMENTATION );             \
    }

/// CAFFA_SOURCE_INIT associates the file keyword used for storage with the class and
//  initializes the factory
/// Place this in the cpp file, preferably above the constructor
#define CAFFA_SOURCE_INIT( ClassName )                                       \
    static bool CAFFA_OBJECT_STRING_CONCATENATE( my##ClassName, __LINE__ ) = \
        caffa::DefaultObjectFactory::instance()->registerCreator<ClassName>();

#ifdef __clang__
#pragma clang diagnostic pop
#endif
