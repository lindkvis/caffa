#include "cafRestServiceInterface.h"

using namespace caffa::rpc;
using namespace std::chrono_literals;

constexpr std::chrono::seconds RATE_LIMITER_TIME_PERIOD  = 1s;
constexpr size_t               RATE_LIMITER_MAX_REQUESTS = 200;

std::mutex                                       RestServiceInterface::s_requestMutex;
std::list<std::chrono::steady_clock::time_point> RestServiceInterface::s_requestTimes;

const std::string RestServiceInterface::HTTP_OK = std::to_string( static_cast<unsigned>( http::status::ok ) );
const std::string RestServiceInterface::HTTP_ACCEPTED = std::to_string( static_cast<unsigned>( http::status::accepted ) );
const std::string RestServiceInterface::HTTP_FORBIDDEN = std::to_string( static_cast<unsigned>( http::status::forbidden ) );
const std::string RestServiceInterface::HTTP_TOO_MANY_REQUESTS =
    std::to_string( static_cast<unsigned>( http::status::too_many_requests ) );
const std::string RestServiceInterface::HTTP_NOT_FOUND = std::to_string( static_cast<unsigned>( http::status::not_found ) );

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

nlohmann::ordered_json RestServiceInterface::plainErrorResponse()
{
    auto errorContent          = nlohmann::ordered_json::object();
    errorContent["text/plain"] = { { "schema", { { "$ref", "#/components/error_schemas/PlainError" } } } };
    auto errorResponse = nlohmann::ordered_json{ { "description", "Error message" }, { "content", errorContent } };
    return errorResponse;
}

std::map<std::string, nlohmann::ordered_json> RestServiceInterface::basicServiceSchemas()
{
    auto plainError = nlohmann::ordered_json{ { "type", "string" }, { "example", "An example error" } };

    return { { "error_schemas", { { "PlainError", plainError } } } };
}
