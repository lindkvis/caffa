#include "cafRestServiceInterface.h"

using namespace caffa::rpc;
using namespace std::chrono_literals;

constexpr std::chrono::seconds RATE_LIMITER_TIME_PERIOD  = 1s;
constexpr size_t               RATE_LIMITER_MAX_REQUESTS = 20;

std::mutex                                       RestServiceInterface::s_requestMutex;
std::list<std::chrono::steady_clock::time_point> RestServiceInterface::s_requestTimes;

bool RestServiceInterface::refuseDueToTimeLimiter()
{
    std::scoped_lock lock( s_requestMutex );

    auto now = std::chrono::steady_clock::now();

    std::list<std::chrono::steady_clock::time_point> recentRequests;
    for ( auto requestTime : s_requestTimes )
    {
        if ( now - requestTime < RATE_LIMITER_TIME_PERIOD )
        {
            recentRequests.push_back( requestTime );
        }
    }

    s_requestTimes.swap( recentRequests );

    if ( s_requestTimes.size() >= RATE_LIMITER_MAX_REQUESTS )
    {
        return true;
    }

    s_requestTimes.push_back( now );
    return false;
}