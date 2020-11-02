#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafObject.h"
#include "cafPdmPointer.h"

#if 0
class PdmPointerTarget
{
public:
   PdmPointerTarget() {}
   PdmPointerTarget(const PdmPointerTarget& ) {}
   PdmPointerTarget& operator=(const PdmPointerTarget& ) {}

   virtual ~PdmPointerTarget()
   {
      // Set all guarded pointers pointing to this to NULL

      std::set<Object**>::iterator it;
      for (it = m_pointersReferencingMe.begin(); it != m_pointersReferencingMe.end() ; ++it)
      {
          (**it) = NULL;
      }
   }

private:

   // Support system for PdmPointer

   friend class PdmPointerImpl;
   std::set<PdmPointerTarget**>         m_pointersReferencingMe;
};

#endif

class Child;

class Parent : public caf::Object
{
    CAF_HEADER_INIT;

public:
    Parent();
    ~Parent();

    void doSome();

    caf::ChildArrayField<Child*> m_simpleObjectsField;
    caf::ChildField<Child*>      m_simpleObjectF;
};
