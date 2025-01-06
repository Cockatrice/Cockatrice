#include "deck_analytics_widget.h"

DeckAnalyticsWidget::DeckAnalyticsWidget(QWidget *parent, DeckListModel *deck_list_model)
    : QWidget(parent), deck_list_model(deck_list_model)
{

    this->setMinimumSize(0, 0);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setStyleSheet("border: 2px solid red;");
    main_layout = new QHBoxLayout();
    this->setLayout(main_layout);

    flow_widget = new FlowWidget(this, Qt::ScrollBarAlwaysOff, Qt::ScrollBarAlwaysOff);
    this->main_layout->addWidget(flow_widget);

    mana_curve_widget = new ManaCurveWidget(flow_widget);
    mana_curve_widget->setStyleSheet("border: 2px solid yellow;");

    flow_widget->addWidget(mana_curve_widget);

    mana_devotion_widget = new ManaDevotionWidget(flow_widget, this->deck_list_model);
    flow_widget->addWidget(mana_devotion_widget);

    mana_base_widget = new ManaBaseWidget(flow_widget, this->deck_list_model);
    flow_widget->addWidget(mana_base_widget);
}
