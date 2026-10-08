/** @file qtwidgets_pch.h
 *  @brief Precompiled header for GUI targets (Cockatrice client, Oracle).
 *
 * Includes the Qt Core precompiled header plus the heavy Gui, Widgets and
 * Network layers that virtually every client translation unit re-parses.
 * Do not use on Servatrice (headless, QT_DONT_USE_QTGUI).
 *
 * Candidates are chosen from IWYU include statistics: a header earns a
 * spot when it is not already reachable from the rest of this list and
 * enough translation units re-parse it. Qt headers are stable across
 * builds, so the precompiled header rarely invalidates.
 */

#include "qtcore_pch.h"

#include <QAction>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCompleter>
#include <QDialogButtonBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QFormLayout>
#include <QFrame>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QImage>
#include <QImageReader>
#include <QInputDialog>
#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QMainWindow>
#include <QMenu>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QStandardItemModel>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QToolBar>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QWidget>
