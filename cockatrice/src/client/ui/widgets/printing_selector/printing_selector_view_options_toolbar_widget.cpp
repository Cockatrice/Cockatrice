#include "printing_selector_view_options_toolbar_widget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

PrintingSelectorViewOptionsToolbarWidget::PrintingSelectorViewOptionsToolbarWidget(QWidget *_parent,
                                                                                   PrintingSelector *_printingSelector)
    : QWidget(_parent), printingSelector(_printingSelector)
{
    layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    expandedWidget = new QWidget(this);
    QVBoxLayout *expandedLayout = new QVBoxLayout(expandedWidget);
    expandedLayout->setContentsMargins(0, 0, 0, 0);
    expandedLayout->setSpacing(0);

    collapseButton = new QPushButton("▼", this);
    collapseButton->setFixedSize(20, 20);
    collapseButton->setToolTip("Collapse");
    collapseButton->setStyleSheet("border: none;");
    connect(collapseButton, &QPushButton::clicked, this, &PrintingSelectorViewOptionsToolbarWidget::collapse);
    expandedLayout->addWidget(collapseButton, 0, Qt::AlignLeft);

    viewOptions = new PrintingSelectorViewOptionsWidget(expandedWidget, printingSelector);
    expandedLayout->addWidget(viewOptions);

    expandedWidget->setLayout(expandedLayout);

    collapsedWidget = new QWidget(this);
    QHBoxLayout *collapsedLayout = new QHBoxLayout(collapsedWidget);
    collapsedLayout->setContentsMargins(5, 0, 5, 0);
    collapsedLayout->setSpacing(0);

    expandButton = new QPushButton("▲", this);
    expandButton->setFixedSize(20, 20);
    expandButton->setToolTip("Expand");
    expandButton->setStyleSheet("border: none;");
    connect(expandButton, &QPushButton::clicked, this, &PrintingSelectorViewOptionsToolbarWidget::expand);
    collapsedLayout->addWidget(expandButton);

    QLabel *collapsedLabel = new QLabel(tr("Display Options"), this);
    collapsedLayout->addWidget(collapsedLabel);

    collapsedWidget->setLayout(collapsedLayout);

    stackedWidget = new QStackedWidget(this);
    stackedWidget->addWidget(expandedWidget);
    stackedWidget->addWidget(collapsedWidget);

    layout->addWidget(stackedWidget);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    stackedWidget->setCurrentWidget(expandedWidget);

    connect(stackedWidget, &QStackedWidget::currentChanged, this,
            &PrintingSelectorViewOptionsToolbarWidget::onWidgetChanged);
}

void PrintingSelectorViewOptionsToolbarWidget::collapse()
{
    stackedWidget->setCurrentWidget(collapsedWidget);
    updateGeometry();
}

void PrintingSelectorViewOptionsToolbarWidget::expand()
{
    stackedWidget->setCurrentWidget(expandedWidget);
    updateGeometry();
}

// Handle Geometry Update
void PrintingSelectorViewOptionsToolbarWidget::onWidgetChanged(int)
{
    updateGeometry();
    if (parentWidget() && parentWidget()->layout()) {
        parentWidget()->layout()->invalidate();
    }
}

// Size Hints
QSize PrintingSelectorViewOptionsToolbarWidget::sizeHint() const
{
    return stackedWidget->currentWidget()->sizeHint();
}

QSize PrintingSelectorViewOptionsToolbarWidget::minimumSizeHint() const
{
    return stackedWidget->currentWidget()->minimumSizeHint();
}

PrintingSelectorViewOptionsWidget *PrintingSelectorViewOptionsToolbarWidget::getViewOptionsWidget() const
{
    return viewOptions;
}