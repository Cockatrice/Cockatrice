#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QBrush>
#include <QPixmap>
#include <QStringList>

class QApplication;

class ThemeManager : public QObject {
    Q_OBJECT
public:
    ThemeManager(QObject *parent = 0);
private:
    QBrush handBgBrush, stackBgBrush, tableBgBrush, playerBgBrush;
    QPixmap cardBackPixmap;
    QStringList availableThemes;
protected:
    void ensureUserThemeDirectoryExists();
public:
    QBrush &getHandBgBrush() { return handBgBrush; }
    QBrush &getStackBgBrush() { return stackBgBrush; }
    QBrush &getTableBgBrush() { return tableBgBrush; }
    QBrush &getPlayerBgBrush() { return playerBgBrush; }
    QPixmap &getCardBackPixmap() { return cardBackPixmap; }
    QStringList &getAvailableThemes();
protected slots:
    void themeChangedSlot();
signals:
    void themeChanged();
};

extern ThemeManager * themeManager;

#endif
