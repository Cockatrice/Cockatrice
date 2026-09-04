/**
 * @file share_bar_widget.h
 * @ingroup DeckShareWidgets
 */
//! \todo Document this file.

#ifndef SHARE_BAR_WIDGET_H
#define SHARE_BAR_WIDGET_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QPushButton;

/**
 * @brief The activated toolbar used to create a temporary deck share.
 *
 * A single reusable component shared by the local visual deck storage and the
 * remote server deck storage tabs, so the share workflow renders identically in
 * both places. It owns its own widgets, strings, and layout; the owning tab only
 * sets the count/hint text and reacts to the create/cancel signals.
 */
class ShareBarWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit ShareBarWidget(QWidget *parent = nullptr);

    void retranslateUi();

    /** @return The trimmed name entered by the user. */
    [[nodiscard]] QString name() const;

    /** @brief Resets the name field to the given default. */
    void setName(const QString &name);

    /** @brief Sets the selected-count summary label text. */
    void setCountText(const QString &text);

    /** @brief Sets the explainer hint text, showing it when @p visible is true. */
    void setHintText(const QString &text, bool visible);

    /** @brief Enables or disables the create-share-link button (guards double submission). */
    void setCreateEnabled(bool enabled);

    /** @brief Moves keyboard focus to the name field. */
    void focusName();

signals:
    void createRequested();
    void cancelRequested();

private:
    QLabel *hintLabel;
    QLineEdit *nameEdit;
    QLabel *countLabel;
    QPushButton *cancelButton;
    QPushButton *createButton;
};

#endif // SHARE_BAR_WIDGET_H