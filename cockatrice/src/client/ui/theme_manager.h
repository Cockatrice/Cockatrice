#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QBrush>
#include <QDir>
#include <QLoggingCategory>
#include <QMap>
#include <QObject>
#include <QPixmap>
#include <QString>

inline Q_LOGGING_CATEGORY(ThemeManagerLog, "theme_manager");

typedef QMap<QString, QString> QStringMap;
typedef QMap<int, QBrush> QBrushMap;

class QApplication;

class ThemeManager : public QObject
{
    Q_OBJECT
public:
    ThemeManager(QObject *parent = nullptr);

private:
    QBrush handBgBrush, stackBgBrush, tableBgBrush, playerBgBrush;
    QStringMap availableThemes;
    /*
      Internal cache for multiple backgrounds
    */
    QBrushMap tableBgBrushesCache, stackBgBrushesCache, playerBgBrushesCache, handBgBrushesCache;

protected:
    void ensureThemeDirectoryExists();
    QBrush loadBrush(QString fileName, QColor fallbackColor);
    QBrush loadExtraBrush(QString fileName, QBrush &fallbackBrush);

public:
    QBrush &getHandBgBrush()
    {
        return handBgBrush;
    }
    QBrush &getStackBgBrush()
    {
        return stackBgBrush;
    }
    QBrush &getTableBgBrush()
    {
        return tableBgBrush;
    }
    QBrush &getPlayerBgBrush()
    {
        return playerBgBrush;
    }
    QStringMap &getAvailableThemes();
    QBrush getExtraTableBgBrush(QString extraNumber, QBrush &fallbackBrush);
    QBrush getExtraStackBgBrush(QString extraNumber, QBrush &fallbackBrush);
    QBrush getExtraPlayerBgBrush(QString extraNumber, QBrush &fallbackBrush);
    QBrush getExtraHandBgBrush(QString extraNumber, QBrush &fallbackBrush);
protected slots:
    void themeChangedSlot();
signals:
    void themeChanged();
};

extern ThemeManager *themeManager;

#endif
