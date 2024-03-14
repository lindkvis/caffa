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

nlohmann::json RestServiceInterface::plainErrorResponse()
{
    auto errorContent          = nlohmann::json::object();
    errorContent["text/plain"] = { { "schema", { { "$ref", "#/components/error_schemas/PlainError" } } } };
    auto errorResponse         = nlohmann::json{ { "description", "Error message" }, { "content", errorContent } };
    return errorResponse;
}

std::map<std::string, nlohmann::json> RestServiceInterface::basicServiceSchemas()
{
    auto plainError = nlohmann::json{ { "type", "string" }, { "example", "An example error" } };

    return { { "error_schemas", { { "PlainError", plainError } } } };
}

nlohmann::json RestServiceInterface::createOperation( const std::string&    operationId,
                                                      const std::string&    summary,
                                                      const nlohmann::json& parameters,
                                                      const nlohmann::json& responses,
                                                      const nlohmann::json& requestBody,
                                                      const nlohmann::json& tags )
{
    auto schema = nlohmann::json{ { "operationId", operationId }, { "summary", summary }, { "responses", responses } };
    if ( !parameters.is_null() )
    {
        if ( parameters.is_array() )
        {
            schema["parameters"] = parameters;
        }
        else
        {
            schema["parameters"] = nlohmann::json::array( { parameters } );
        }
    }
    if ( !requestBody.is_null() )
    {
        schema["requestBody"] = requestBody;
    }
    if ( !tags.is_null() )
    {
        schema["tags"] = tags;
    }
    return schema;
}