#pragma once

#include "cafChildArrayField.h"
#include "cafChildField.h"
#include "cafObject.h"
#include "cafPointer.h"

#if 0
class PointerTarget
{
public:
   PointerTarget() {}
   PointerTarget(const PointerTarget& ) {}
   PointerTarget& operator=(const PointerTarget& ) {}

   virtual ~PointerTarget()
   {
      // Set all guarded pointers pointing to this to NULL

      std::set<Object**>::iterator it;
      for (it = m_pointersReferencingMe.begin(); it != m_pointersReferencingMe.end() ; ++it)
      {
          (**it) = NULL;
      }
   }

private:

   // Support system for Pointer

   friend class PointerImpl;
   std::set<PointerTarget**>         m_pointersReferencingMe;
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
