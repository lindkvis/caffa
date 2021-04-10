#pragma once

#define CAF_IS_DEFINING_CAF_FIELD
#define DataValueField Field
#include "cafDataValueField.h"
#undef DataValueField
#undef CAF_IS_DEFINING_CAF_FIELD

#ifndef __clang__
namespace caf
{
// Specialization to create compiler errors to help finding the Field's to rename

#ifdef WIN32

template <typename DataType>
class Field<DataType*> : public Rename_Field_of_pointer_to_ChildField // You must rename Field<T*> to
                                                                      // ChildField<T*>
{
};

#endif // WIN32

} // namespace caf
#endif // __clang__
