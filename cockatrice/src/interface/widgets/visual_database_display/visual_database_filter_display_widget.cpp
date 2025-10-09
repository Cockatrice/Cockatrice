#include "visual_database_filter_display_widget.h"

#include <QDir>
#include <QFile>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <libcockatrice/settings/cache_settings.h>

FilterDisplayWidget::FilterDisplayWidget(QWidget *parent, const QString &filename, FilterTreeModel *_filterModel)
    : QWidget(parent), filterFilename(filename), filterModel(_filterModel)
{
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    // Create layout
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(5);

    // Filter button (displays filename)
    filterButton = new QPushButton(filterFilename, this);
    layout->addWidget(filterButton);

    // Create close button (inside the filter button)
    closeButton = new QPushButton("x", this);
    closeButton->setFixedSize(20, 20); // Small square close button
    closeButton->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(closeButton);

    // Connect the filter button for loading the filter
    connect(filterButton, &QPushButton::clicked, this, &FilterDisplayWidget::loadFilter);

    // Connect the close button for deleting the filter
    connect(closeButton, &QPushButton::clicked, this, &FilterDisplayWidget::deleteFilter);
}

QSize FilterDisplayWidget::sizeHint() const
{
    // Calculate the size based on the filter name and the close button
    QFontMetrics fm(font());
    int textWidth = fm.horizontalAdvance(filterFilename);
    int width = textWidth + 30 + 16; // Space for the filename and close button
    int height = fm.height() + 10;   // Height based on font size + padding

    return QSize(width, height);
}

void FilterDisplayWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit filterClicked();
    }
    QWidget::mousePressEvent(event);
}

void FilterDisplayWidget::loadFilter()
{
    // Trigger the loading of the filter
    emit filterLoadRequested(filterFilename);
}

void FilterDisplayWidget::deleteFilter()
{
    // Show confirmation dialog before deleting the filter
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Confirm Delete"), tr("Are you sure you want to delete the filter '%1'?").arg(filterFilename),
        QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        // If confirmed, delete the filter
        QString filePath = SettingsCache::instance().getFiltersPath() + QDir::separator() + filterFilename;
        QFile file(filePath);
        if (file.remove()) {
            emit filterDeleted(filterFilename); // Emit signal for deletion
            delete this;                        // Delete this widget
        } else {
            QMessageBox::warning(this, tr("Delete Failed"), tr("Failed to delete filter '%1'.").arg(filterFilename));
        }
    }
}