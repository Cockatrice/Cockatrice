#ifndef COCKATRICE_TAB_ARCHIDEKT_H
#define COCKATRICE_TAB_ARCHIDEKT_H

#include "../../interface/widgets/cards/card_size_widget.h"
#include "../../interface/widgets/quick_settings/settings_button_widget.h"
#include "../../tab.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QPushButton>
#include <libcockatrice/card/database/card_database.h>

#include <QString>

inline QString archidektApiLink = "https://archidekt.com/api/decks/v3/?name=";

class TabArchidekt : public Tab
{
    Q_OBJECT
public:
    explicit TabArchidekt(TabSupervisor *_tabSupervisor);

    void retranslateUi() override;
    void doSearch();
    QString getTabText() const override
    {
        auto cardName = cardToQuery.isNull() ? QString() : cardToQuery->getName();
        return tr("Archidekt: ") + cardName;
    }

    CardSizeWidget *getCardSizeSlider()
    {
        return cardSizeSlider;
    }

    QNetworkAccessManager *networkManager;

public slots:
    void processApiJson(QNetworkReply *reply);
    void processTopDecksResponse(QJsonObject reply);
    void processDeckResponse(QJsonObject reply);
    void prettyPrintJson(const QJsonValue &value, int indentLevel);
    void actNavigatePage(QString url);
    void getTopDecks();

private:
    QWidget *container;
    QWidget *navigationContainer;
    QWidget *currentPageDisplay;
    QVBoxLayout *mainLayout;
    QHBoxLayout *navigationLayout;
    QVBoxLayout *currentPageLayout;
    QPushButton *decksPushButton;
    QLineEdit *searchBar;
    QPushButton *searchPushButton;
    SettingsButtonWidget *settingsButton;
    CardSizeWidget *cardSizeSlider;
    CardInfoPtr cardToQuery;
};


#endif // COCKATRICE_TAB_ARCHIDEKT_H
