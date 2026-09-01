/** @file qtwidgets_pch.h
 *  @brief Precompiled header for GUI targets (Cockatrice client, Oracle).
 *
 * Includes the Qt Core precompiled header plus the heavy Gui, Widgets and
 * Network layers that virtually every client translation unit re-parses.
 * Do not use on Servatrice (headless, QT_DONT_USE_QTGUI).
 */

#include "qtcore_pch.h"

#include <QAction>
#include <QApplication>
#include <QFrame>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QMainWindow>
#include <QMenu>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QToolBar>
#include <QTreeWidget>
#include <QWidget>

#include <QNetworkAccessManager>
#include <QNetworkReply>