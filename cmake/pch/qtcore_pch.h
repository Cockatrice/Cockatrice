/** @file qtcore_pch.h
 *  @brief Precompiled header for all Qt targets (Qt Core only).
 *
 * Safe for every target that links Qt Core, including the headless
 * Servatrice binary. Keep this header free of any widget/gui types.
 *
 * Candidates are chosen from IWYU include statistics: a header earns a
 * spot when it is not already reachable from the rest of this list and
 * enough translation units re-parse it. Qt headers are stable across
 * builds, so the precompiled header rarely invalidates.
 */

#include <QBasicTimer>
#include <QByteArray>
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QItemSelectionModel>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QLoggingCategory>
#include <QMap>
#include <QMetaEnum>
#include <QMetaObject>
#include <QMutex>
#include <QMutexLocker>
#include <QObject>
#include <QPointer>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QReadWriteLock>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QSharedPointer>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QThread>
#include <QTimer>
#include <QUuid>
#include <QVariant>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
