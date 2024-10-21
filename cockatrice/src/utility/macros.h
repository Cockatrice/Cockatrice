#ifndef COCKATRICE_MACROS_H
#define COCKATRICE_MACROS_H

#include <QtGlobal>

// Qt6.7 changed how stateChanged functionality
// of QCheckBoxes work.
// See https://doc.qt.io/qt-6/qcheckbox.html#checkStateChanged
#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)
#define QT_STATE_CHANGED checkStateChanged
#define QT_STATE_CHANGED_T Qt::CheckState
#else
#define QT_STATE_CHANGED stateChanged
#define QT_STATE_CHANGED_T int
#endif

#endif // COCKATRICE_MACROS_H
