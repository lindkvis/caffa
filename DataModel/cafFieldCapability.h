#pragma once

namespace caffa
{
class FieldHandle;

class FieldCapability
{
public:
    FieldCapability()          = default;
    virtual ~FieldCapability() = default;

protected:
    friend class FieldHandle;
    FieldHandle* owner() const { return m_field; }
    void         setOwner( FieldHandle* field ) { m_field = field; }

private:
    FieldHandle* m_field = nullptr;
};

} // End of namespace caffa
