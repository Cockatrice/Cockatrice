#include "visual_database_filter_display_widget.h"

#include "../../../client/settings/cache_settings.h"

#include <QChar>
#include <QDir>
#include <QFile>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QSize>
#include <QSizePolicy>
#include <libcockatrice/settings/paths_settings.h>
#include <qnamespace.h>

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
    // Capture the target before opening the dialog. Refreshing/closing the pane
    // destroys this receiver and disconnects the pending confirmation safely.
    const QString filename = filterFilename;
    const QString filePath = SettingsCache::instance().paths().getFiltersPath() + QDir::separator() + filename;
    auto *dialog = new QMessageBox(QMessageBox::Question, tr("Confirm Delete"),
                                  tr("Are you sure you want to delete the filter '%1'?").arg(filename),
                                  QMessageBox::Yes | QMessageBox::No, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    connect(dialog, &QMessageBox::finished, this, [this, filename, filePath](int result) {
        if (result != QMessageBox::Yes) {
            return;
        }
        QFile file(filePath);
        if (file.remove()) {
            hide();
            deleteLater();
            // A synchronous listener may rebuild or destroy the entire pane.
            // Do not access this widget after emitting the notification.
            emit filterDeleted(filename);
        } else {
            auto *error = new QMessageBox(QMessageBox::Warning, tr("Delete Failed"),
                                         tr("Failed to delete filter '%1': %2").arg(filename, file.errorString()),
                                         QMessageBox::Ok, this);
            error->setAttribute(Qt::WA_DeleteOnClose);
            error->open();
        }
    });
    dialog->open();
}
