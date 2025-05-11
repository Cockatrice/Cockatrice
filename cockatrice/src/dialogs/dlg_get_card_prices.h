#ifndef DLG_GET_CARD_PRICES_H
#define DLG_GET_CARD_PRICES_H

#include "../client/ui/widgets/general/layout_containers/flow_widget.h"
#include "../deck/deck_list_model.h"

#include <QCheckBox> // Include for the checkbox
#include <QComboBox>
#include <QDialog>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressBar>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>

class DeckListModel;
class QLabel;
class QScrollArea;

class CardPriceTableWidgetItem : public QTableWidgetItem
{
public:
    // Constructor
    CardPriceTableWidgetItem(const QString &text) : QTableWidgetItem(text)
    {
    }

    bool operator<(const QTableWidgetItem &other) const override
    {
        bool ok1, ok2;
        double val1 = text().remove('%').toDouble(&ok1);       // Remove '%' and convert to double
        double val2 = other.text().remove('%').toDouble(&ok2); // Remove '%' and convert to double

        // Handle non-numeric cases gracefully
        if (ok1 && ok2) {
            return val1 < val2; // Compare as double values
        }

        // If conversion to double failed, fallback to string comparison
        return text() < other.text();
    }
};

class DlgGetCardPrices : public QDialog
{
    Q_OBJECT

public:
    explicit DlgGetCardPrices(QWidget *parent, DeckListModel *_model);

signals:
    void allRequestsFinished();

private slots:
    void onCardPriceReply();
    void onAllRequestsFinished();
    void actOK();
    void onSelectionChanged();
    void onExcludeLandsChanged(bool checked);
    void onAllPrintingsChanged(bool checked);
    void onCurrencyChanged(int index);

private:
    void retranslateUi();
    void startCardPriceRequests();
    void sendCardPriceRequest(const QString &cardName);

    DeckListModel *model;
    QLabel *instructionLabel;
    QLabel *minTotalLabel;
    QLabel *maxTotalLabel;
    QTableWidget *tableWidget;
    QScrollArea *scrollArea;
    QNetworkAccessManager networkManager;
    int pendingRequests = 0;
    int totalRequests = 0;
    int finishedRequests = 0;
    QMap<QString, QStringList> setCards;
    double totalPrice = 0.0;
    double totalMinPrice = 0.0;
    double totalMaxPrice = 0.0;
    QCheckBox *allPrintingsCheckBox;
    QCheckBox *excludeLandsCheckBox;
    QComboBox *currencyComboBox;
    QMap<QString, QList<QString>> printingsByCardName;
    QMap<QString, double> uuidPrices;
    QMap<QString, QString> uuidToCardName;
    QMap<QString, int> pendingPerCard;
    QMap<QString, QString> actualUuidsByCardName;
    QPushButton *fetchButton;
    QProgressBar *progressBar;
    QProgressBar *uuidProgressBar;
    QLabel *progressLabel;
    QLabel *uuidProgressLabel;

    int totalCardsToProcess = 0;
    int finishedCardCount = 0;
};

#endif // DLG_GET_CARD_PRICES_H
