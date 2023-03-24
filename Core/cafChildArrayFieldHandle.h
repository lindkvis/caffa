#pragma once

#include "cafFieldHandle.h"

#include <memory>

namespace caffa
{
class ChildArrayFieldAccessor;
class ObjectHandle;

/**
 * @brief A non-templated base interface for ChildArrayField<DataType*>
 * Used so we can have pointers to any ChildArrayField.
 *
 */
class ChildArrayFieldHandle : public FieldHandle
{
public:
    ChildArrayFieldHandle() {}
    ~ChildArrayFieldHandle() override {}

    /**
     * @brief Get the number of child objects.
     *
     * @return size_t
     */
    virtual size_t size() const = 0;
    /**
     * @brief Check if the child array field is empty
     *
     * @return true if empty
     * @return false if there are child objects.
     */
    bool empty() const { return this->size() == 0u; }

    /**
     * @brief Remove all child objects and return a vector of unique pointers to the destroyed objects.
     * This way the called can take over ownership or just disregard the return value to destroy them.
     *
     * @return std::vector<std::unique_ptr<ObjectHandle>>
     */
    virtual std::vector<std::unique_ptr<ObjectHandle>> clear() = 0;

    /**
     * @brief Erase a particular child object by index
     *
     * @param index the index to the object
     */
    virtual void erase( size_t index ) = 0;

    /**
     * @brief Get a raw pointer to the object at a particular index
     *
     * @param index The index to look up
     * @return A raw pointer to the Caffa object.
     */
    virtual ObjectHandle* at( size_t index ) = 0;

    /**
     * @brief Insert an object at a particular index. Ownership will be taken.
     *
     * @param index the index to insert at
     * @param obj A unique pointer to the object.
     */
    virtual void insertAt( size_t index, std::unique_ptr<ObjectHandle> obj ) = 0;

    /**
     * @brief push back and object taking over ownership.
     *
     * @param obj object to take.
     */
    virtual void push_back_obj( std::unique_ptr<ObjectHandle> obj ) = 0;

    /**
     * @brief Set a new accessor
     *
     * @param accessor
     */
    virtual void setAccessor( std::unique_ptr<ChildArrayFieldAccessor> accessor ) = 0;

    /**
     * @brief Get the class keyword of the contained children
     *
     */
    virtual constexpr std::string_view childClassKeyword() const = 0;
};

} // namespace caffa
