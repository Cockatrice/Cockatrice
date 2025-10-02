/**
 * @file dlg_load_deck_from_website.h
 * @ingroup RemoteDeckStorageDialogs
 * @ingroup Lobby
 * @brief TODO: Document this.
 */

#ifndef DLG_LOAD_DECK_FROM_WEBSITE_H
#define DLG_LOAD_DECK_FROM_WEBSITE_H

#include "../client/network/parsers/deck_link_to_api_transformer.h"
#include "../client/network/parsers/interface_json_deck_parser.h"

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QNetworkAccessManager>
#include <QVBoxLayout>

inline Q_LOGGING_CATEGORY(DlgLoadDeckFromWebsiteLog, "dlg_load_deck_from_website");

class DlgLoadDeckFromWebsite : public QDialog
{
    Q_OBJECT
public:
    explicit DlgLoadDeckFromWebsite(QWidget *parent);
    void retranslateUi();
    bool testValidUrl();
    DeckLoader *deck;

    DeckLoader *getDeck()
    {
        return deck;
    }

private:
    QNetworkAccessManager *nam;
    QVBoxLayout *layout;
    QLabel *instructionLabel;
    QLineEdit *urlEdit;

public slots:
    void accept() override;
    QSharedPointer<IJsonDeckParser> createParserForProvider(DeckProvider provider);
};

#endif // DLG_LOAD_DECK_FROM_WEBSITE_H
