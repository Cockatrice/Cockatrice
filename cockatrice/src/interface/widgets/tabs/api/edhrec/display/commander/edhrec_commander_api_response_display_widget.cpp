#include "edhrec_commander_api_response_display_widget.h"

#include "../../../../../cards/card_info_picture_widget.h"
#include "../../api_response/commander/edhrec_commander_api_response.h"
#include "../cards/edhrec_api_response_card_list_display_widget.h"
#include "edhrec_api_response_commander_details_display_widget.h"

#include <QListView>
#include <QResizeEvent>
#include <QScrollArea>
#include <QSplitter>
#include <QStringListModel>

EdhrecCommanderApiResponseDisplayWidget::EdhrecCommanderApiResponseDisplayWidget(QWidget *parent,
                                                                                 EdhrecCommanderApiResponse response,
                                                                                 QString baseUrl)
    : QWidget(parent)
{
    layout = new QHBoxLayout(this);
    setLayout(layout);

    cardDisplayLayout = new QVBoxLayout(this);

    // Create a QSplitter to hold the ListView and ScrollArea holding CardListdisplayWidgets side by side
    auto splitter = new QSplitter(this);
    splitter->setOrientation(Qt::Horizontal);

    auto listView = new QListView(splitter);
    listView->setMinimumWidth(50);
    listView->setMaximumWidth(150);
    auto listModel = new QStringListModel(this);
    QStringList widgetNames;

    // Add commander details
    auto commanderPicture = new EdhrecCommanderResponseCommanderDetailsDisplayWidget(
        this, response.container.getCommanderDetails(), baseUrl);
    cardDisplayLayout->addWidget(commanderPicture);
    widgetNames.append("Commander Details");

    // Add card list widgets
    auto edhrec_commander_api_response_card_lists = response.container.getCardlists();
    for (const EdhrecApiResponseCardList &card_list : edhrec_commander_api_response_card_lists) {
        auto cardListDisplayWidget = new EdhrecApiResponseCardListDisplayWidget(this, card_list);
        cardDisplayLayout->addWidget(cardListDisplayWidget);
        widgetNames.append(cardListDisplayWidget->getBannerText());
    }

    // Create a QScrollArea to hold the card display widgets
    scrollArea = new QScrollArea(splitter);
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set the cardDisplayLayout inside the scroll area
    auto scrollWidget = new QWidget(scrollArea);
    scrollWidget->setLayout(cardDisplayLayout);
    connect(splitter, &QSplitter::splitterMoved, this, &EdhrecCommanderApiResponseDisplayWidget::onSplitterChange);
    scrollArea->setWidget(scrollWidget);

    // Configure the list view
    listModel->setStringList(widgetNames);
    listView->setModel(listModel);
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Connect the list view to ensure the corresponding widget is visible
    connect(listView, &QListView::clicked, this, [this](const QModelIndex &index) {
        int widgetIndex = index.row();
        qDebug() << "clicked: " << widgetIndex;
        auto targetWidget = cardDisplayLayout->itemAt(widgetIndex)->widget();
        if (targetWidget) {
            qDebug() << "Found targetWidget" << targetWidget;
            // Attempt to cast the parent to QScrollArea
            auto scrollArea = qobject_cast<QScrollArea *>(this->scrollArea); // Use the scroll area instance
            if (scrollArea) {
                qDebug() << "ScrollArea" << scrollArea;
                scrollArea->ensureWidgetVisible(targetWidget);
            }
        }
    });

    // Add splitter to the main layout
    splitter->addWidget(listView);
    splitter->addWidget(scrollArea);

    layout->addWidget(splitter);
}

void EdhrecCommanderApiResponseDisplayWidget::onSplitterChange()
{
    scrollArea->widget()->resize(scrollArea->size());
}

void EdhrecCommanderApiResponseDisplayWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    qDebug() << event->size();
    layout->invalidate();
    layout->activate();
    layout->update();
    if (scrollArea && scrollArea->widget()) {
        scrollArea->widget()->resize(event->size());
    }
}
