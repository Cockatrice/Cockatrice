#ifndef TAB_H
#define TAB_H

#include <QWidget>

class QMenu;
class TabSupervisor;
class CardInfoWidget;

class Tab : public QWidget {
    Q_OBJECT
signals:
    void userEvent(bool globalEvent = true);
    void tabTextChanged(Tab *tab, const QString &newTabText);
protected:
    TabSupervisor *tabSupervisor;
    void addTabMenu(QMenu *menu) { tabMenus.append(menu); }
protected slots:
    void showCardInfoPopup(const QPoint &pos, const QString &cardName);
    void deleteCardInfoPopup(const QString &cardName);
private:
    bool contentsChanged;
    CardInfoWidget *infoPopup;
    QList<QMenu *> tabMenus;
public:
    Tab(TabSupervisor *_tabSupervisor, QWidget *parent = 0);
    const QList<QMenu *> &getTabMenus() const { return tabMenus; }
    TabSupervisor *getTabSupervisor() const { return tabSupervisor; }
    bool getContentsChanged() const { return contentsChanged; }
    void setContentsChanged(bool _contentsChanged) { contentsChanged = _contentsChanged; }
    virtual QString getTabText() const = 0;
    virtual void retranslateUi() = 0;
    virtual void closeRequest() { }
};

#endif
