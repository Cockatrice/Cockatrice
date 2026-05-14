#ifndef COCKATRICE_COMPACT_PUSH_BUTTON_H
#define COCKATRICE_COMPACT_PUSH_BUTTON_H

#include <QPushButton>

class CompactPushButton : public QPushButton
{
    Q_OBJECT

public:
    explicit CompactPushButton(QWidget *parent = nullptr);

    void setButtonText(const QString &text);

    void setButtonIcon(const QIcon &icon);

    void setCompact(bool enabled);

    int expandedWidth() const;

    int compactWidth() const;

private:
    void updateGeometryState();

private:
    QString fullText;
    bool compact = false;
};

#endif // COCKATRICE_COMPACT_PUSH_BUTTON_H
