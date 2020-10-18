#pragma once

#include <QtCore/QtGlobal>

#ifdef cafCommand_LIBRARY
#define cafCommand_EXPORT Q_DECL_EXPORT
#else
#define cafCommand_EXPORT Q_DECL_IMPORT
#endif



