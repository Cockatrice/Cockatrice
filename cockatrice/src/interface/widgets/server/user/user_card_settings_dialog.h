#ifndef COCKATRICE_USER_CARD_ART_SETTINGS_DIALOG_H
#define COCKATRICE_USER_CARD_ART_SETTINGS_DIALOG_H

#include "user_list_painter.h"

#include <QComboBox>
#include <QDialog>
#include <QPixmap>

class QCompleter;
class QFocusEvent;
class QGroupBox;
class QKeyEvent;
class QMouseEvent;
class QLineEdit;
class QDoubleSpinBox;
class QLabel;
class QPushButton;
class QWheelEvent;
class CardDatabaseModel;
class CardDatabaseDisplayModel;
class CardSearchModel;
class CardCompleterProxyModel;

/**
 * @brief Interactive preview of the user list banner art.
 *
 * Renders the banner strip with the given CardArtParams through the same
 * UserListPainter::drawCardArt() the live delegate uses, including the
 * avatar placeholder and fade masks. Dragging pans the art vertically at
 * output scale, the wheel zooms, arrow keys nudge, Backspace or Esc
 * restores the parameters as of focus gain.
 */
class CardArtPreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CardArtPreviewWidget(QWidget *parent = nullptr);

    void setPixmap(const QPixmap &pixmap);
    void setParams(const CardArtParams &params);
    void setAttribution(const QString &attribution);

signals:
    /** @brief Emitted whenever direct manipulation (drag, wheel, keys) changes the parameters. */
    void paramsEdited(const CardArtParams &params);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void focusInEvent(QFocusEvent *event) override;

private:
    qreal bannerTravel() const; ///< vertical travel of the art behind the strip, in output pixels
    void applyCropDelta(qreal dOffset, qreal zoomFactor);
    bool sameCrop(const CardArtParams &a, const CardArtParams &b) const;
    void restoreSnapshot();

    QPixmap sourcePixmap;
    CardArtParams params;
    CardArtParams paramsAtFocusIn; ///< crop as of the latest focus gain, restored by Esc or Backspace
    QString attributionText;
    QPoint lastDragPos;   ///< widget space position of the previous mouse move while panning
    bool dragging{false}; ///< true between an accepted press and its release, guards stale drag positions
};

class UserCardArtSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UserCardArtSettingsDialog(const CardArtParams &initial = {}, QWidget *parent = nullptr);

    CardArtParams params() const;

private slots:
    void onCardNameChanged(const QString &name);
    void reloadPreview();
    void onParamChanged();

private:
    void setupUi();
    void retranslateUi();
    void populateProviderCombo(const QString &cardName);
    void initializeSearchBar();
    QDoubleSpinBox *makeSpinBox(double min, double max, double value, double step);

    QLineEdit *searchBar;
    QCompleter *completer;
    CardDatabaseModel *cardDatabaseModel;
    CardDatabaseDisplayModel *cardDatabaseDisplayModel;
    CardSearchModel *searchModel;
    CardCompleterProxyModel *proxyModel;

    QComboBox *providerComboBox;

    QMetaObject::Connection pixmapUpdatedConnection;

    QLabel *cardNameLabel;
    QLabel *printingLabel;
    QLabel *marginLLabel;
    QLabel *marginRLabel;
    QGroupBox *controlsGroup;
    QLabel *previewCaptionLabel;
    QGroupBox *previewGroup;
    QPushButton *removeBtn;
    QDoubleSpinBox *marginLSpin;
    QDoubleSpinBox *marginRSpin;
    CardArtPreviewWidget *preview;

    QPixmap currentPixmap;
    CardArtParams currentParams;
};

#endif // COCKATRICE_USER_CARD_ART_SETTINGS_DIALOG_H
