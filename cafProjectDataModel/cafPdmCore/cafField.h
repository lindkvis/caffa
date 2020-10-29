#pragma once

#define CAF_IS_DEFINING_PDM_FIELD
#define PdmDataValueField Field
#include "cafPdmDataValueField.h"
#undef PdmDataValueField
#undef CAF_IS_DEFINING_PDM_FIELD

#ifndef __clang__
namespace caf
{
// Specialization to create compiler errors to help finding the Field's to rename

#ifdef WIN32

template <typename DataType>
class Field<DataType*> : public Rename_Field_of_pointer_to_PdmChildField // You must rename Field<T*> to
                                                                               // PdmChildField<T*>
{
};

#endif // WIN32

} // namespace caf
#endif // __clang__
