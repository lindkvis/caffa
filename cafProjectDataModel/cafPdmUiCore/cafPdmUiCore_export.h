#pragma once

#include <QtCore/QtGlobal>

#ifdef cafPdmUiCore_LIBRARY
#define cafPdmUiCore_EXPORT Q_DECL_EXPORT
#else
#define cafPdmUiCore_EXPORT Q_DECL_IMPORT
#endif

