#ifndef DLG_LOAD_URL_DECK_H
#define DLG_LOAD_URL_DECK_H

#include <QDialog>
#include <QLineEdit>
#include <QNetworkRequest>

class DeckLoader;
class QLabel;
class QNetworkAccessManager;
class QNetworkReply;
class QPushButton;

class DlgLoadURLDeck : public QDialog
{
    Q_OBJECT
public:
    DlgLoadURLDeck(QWidget *parent = nullptr);
    DeckLoader *getDeckList() const
    {
        return deckList;
    }

private:
    DeckLoader *deckList;
    QLabel *urlLabel;
    QLineEdit *urlEdit;
    QNetworkAccessManager *networkManager;

private slots:
    void actOk();
    void deckDownloadFinished(QNetworkReply *reply);
};

#endif
