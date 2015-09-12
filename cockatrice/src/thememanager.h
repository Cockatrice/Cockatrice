#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QBrush>
#include <QPixmap>
#include <QMap>
#include <QDir>
#include <QString>

typedef QMap<QString, QString> QStringMap;

class QApplication;

class ThemeManager : public QObject {
    Q_OBJECT
public:
    ThemeManager(QObject *parent = 0);
private:
    QBrush handBgBrush, stackBgBrush, tableBgBrush, playerBgBrush;
    QStringMap availableThemes;
protected:
    void ensureThemeDirectoryExists();
    QBrush loadBrush(QString fileName, QColor fallbackColor);
public:
    QBrush &getHandBgBrush() { return handBgBrush; }
    QBrush &getStackBgBrush() { return stackBgBrush; }
    QBrush &getTableBgBrush() { return tableBgBrush; }
    QBrush &getPlayerBgBrush() { return playerBgBrush; }
    QStringMap &getAvailableThemes();
protected slots:
    void themeChangedSlot();
signals:
    void themeChanged();
};

extern ThemeManager * themeManager;

#endif
