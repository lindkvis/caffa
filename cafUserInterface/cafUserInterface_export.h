#pragma once

#include <QtCore/QtGlobal>

#ifdef cafUserInterface_LIBRARY
#define cafUserInterface_EXPORT Q_DECL_EXPORT
#else
#define cafUserInterface_EXPORT Q_DECL_IMPORT
#endif



