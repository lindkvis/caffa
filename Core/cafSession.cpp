#include "cafSession.h"

#include "cafUuidGenerator.h"

#include <chrono>
#include <random>

using namespace caffa;

Session::Session()
{
    m_uuid = caffa::UuidGenerator::generate();
}

const std::string& Session::uuid() const
{
    return m_uuid;
}