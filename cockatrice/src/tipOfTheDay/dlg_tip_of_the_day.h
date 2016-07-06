#ifndef DLG_TIPOFTHEDAY_H
#define DLG_TIPOFTHEDAY_H

#include <QDialog>
#include "tip_of_the_day.h"
#include "tip_of_the_day_settings.h"

class DlgTipOfTheDay : public QDialog
{
    Q_OBJECT
private:
    int tipIndex;
    QVBoxLayout *tipLayout;
    QList<TipOfTheDay> tips;
    void renderTip(TipOfTheDay tip);
public:
    DlgTipOfTheDay(QWidget *parent = 0, TipOfTheDaySettings *settings = 0);

    void addTips(QList<TipOfTheDay> &_tips);

    void showNextTip();

    void showPreviousTip();

    void clearTipLayout();

    QVBoxLayout* getLayoutForTip(TipOfTheDay &day);

    void markTipAsRead(const QString &tipSettingName);
signals:
    void tipSeen(QString tipSettingName);

    void showFirstUnseenTip();
};

#endif //DLG_TIPOFTHEDAY_H
