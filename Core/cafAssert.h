
#pragma once

#include "cafLogger.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

#define CAFFA_ASSERT( expr )                                                                \
    do                                                                                      \
    {                                                                                       \
        if ( !( expr ) )                                                                    \
        {                                                                                   \
            std::stringstream str;                                                          \
            str << __FILE__ << ":" << __LINE__ << ": CAFFA_ASSERT(" << #expr << ") failed"; \
            CAFFA_CRITICAL( str.str() );                                                    \
            std::abort();                                                                   \
        }                                                                                   \
    } while ( false )
