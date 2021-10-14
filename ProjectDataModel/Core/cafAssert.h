
#pragma once

#include "cafLogger.h"

#include <cstdlib>
#include <iostream>

#define CAFFA_ASSERT( expr )                                                                           \
    do                                                                                                 \
    {                                                                                                  \
        if ( !( expr ) )                                                                               \
        {                                                                                              \
            CAFFA_CRITICAL( __FILE__ << ":" << __LINE__ << ": CAFFA_ASSERT(" << #expr << ") failed" ); \
            std::abort();                                                                              \
        }                                                                                              \
    } while ( false )
