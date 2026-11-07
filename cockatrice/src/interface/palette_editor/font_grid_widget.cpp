#include "font_grid_widget.h"

#include "../theme_manager.h"

#include <QApplication>
#include <QFont>
#include <QFontComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QMetaEnum>
#include <QSpinBox>
#include <QtGlobal>

static constexpr int FALLBACK_FONT_SIZE = 10;

static const QMap<AppFont::Role, const char *> APP_FONT_DESCRIPTIONS = {
    {AppFont::InterfaceDefault, QT_TR_NOOP("Default font for menus, dialogs and widgets")},
    {AppFont::ChatLog, QT_TR_NOOP("Font for chat messages and logs")},
    {AppFont::CardText, QT_TR_NOOP("Font for text rendered on cards")},
    {AppFont::DeckList, QT_TR_NOOP("Font for deck lists and the deck editor")},
};

static QString roleDisplayName(AppFont::Role role)
{
    switch (role) {
        case AppFont::InterfaceDefault:
            return FontGridWidget::tr("Interface default");
        case AppFont::ChatLog:
            return FontGridWidget::tr("Chat log");
        case AppFont::CardText:
            return FontGridWidget::tr("Card text");
        case AppFont::DeckList:
            return FontGridWidget::tr("Deck list");
        default:
            return QString();
    }
}

FontGridWidget::FontGridWidget(QWidget *parent) : QWidget(parent)
{
    auto *grid = new QGridLayout(this);
    grid->setSpacing(3);
    grid->setContentsMargins(12, 8, 12, 8);
    grid->setColumnStretch(0, 0);
    grid->setColumnStretch(1, 1);
    grid->setColumnStretch(2, 0);
    grid->setColumnStretch(3, 0);

    QMetaEnum fontEnum = QMetaEnum::fromType<AppFont::Role>();
    for (int i = 0; i < fontEnum.keyCount(); ++i) {
        auto role = static_cast<AppFont::Role>(fontEnum.value(i));

        auto *label = new QLabel(this);
        grid->addWidget(label, i, 0);

        auto *familyCombo = new QFontComboBox(this);
        grid->addWidget(familyCombo, i, 1);
        connect(familyCombo, &QFontComboBox::currentFontChanged, this, [this] { emit fontsChanged(); });

        if (role == AppFont::CardText) {
            auto *minSpin = new QSpinBox(this);
            minSpin->setRange(FontConfig::CARD_TEXT_MIN_SIZE, FontConfig::MAX_FONT_SIZE);
            grid->addWidget(minSpin, i, 2);
            connect(minSpin, &QSpinBox::valueChanged, this, [this] { emit fontsChanged(); });

            auto *maxSpin = new QSpinBox(this);
            maxSpin->setRange(FontConfig::MIN_FONT_SIZE, FontConfig::MAX_FONT_SIZE);
            grid->addWidget(maxSpin, i, 3);
            connect(maxSpin, &QSpinBox::valueChanged, this, [this] { emit fontsChanged(); });

            // Keep the range consistent: otherwise min 20 / max 12 saves as-is
            // and cardTextMaxSize() silently clamps the max up to the min, so
            // the max spin shows a size that is never used. Moving one spin
            // pulls the other's bound along, but never below the max control's
            // own floor: the renderer resolves the max into the same range, so a
            // smaller value here would be the same mismatch in reverse.
            connect(minSpin, &QSpinBox::valueChanged, this,
                    [maxSpin](int value) { maxSpin->setMinimum(qMax(value, FontConfig::MIN_FONT_SIZE)); });
            connect(maxSpin, &QSpinBox::valueChanged, this, [minSpin](int value) { minSpin->setMaximum(value); });

            label->setBuddy(familyCombo);

            fontRows[role] = {label, familyCombo, maxSpin, minSpin};
            continue;
        }

        auto *sizeSpin = new QSpinBox(this);
        sizeSpin->setRange(FontConfig::MIN_FONT_SIZE, FontConfig::MAX_FONT_SIZE);
        grid->addWidget(sizeSpin, i, 2);
        connect(sizeSpin, &QSpinBox::valueChanged, this, [this] { emit fontsChanged(); });

        label->setBuddy(familyCombo);

        fontRows[role] = {label, familyCombo, sizeSpin, nullptr};
    }

    retranslateUi();
}

void FontGridWidget::loadFonts(const FontConfig &cfg)
{
    // Suppress fontsChanged while materializing fallback rows so the initial
    // load never reads as a user edit (mirrors paletteGrid blocking).
    blockSignals(true);
    QMetaEnum fontEnum = QMetaEnum::fromType<AppFont::Role>();
    for (int i = 0; i < fontEnum.keyCount(); ++i) {
        auto role = static_cast<AppFont::Role>(fontEnum.value(i));
        const FontRow &row = fontRows[role];

        // Roles absent from the file resolve the same way the runtime does: an unset
        // role falls back to the config's InterfaceDefault, and only then to the
        // startup font. appFont() would return the live preview instead, so Reset and
        // a first load must not measure against it.
        const QFont fallback = cfg.fonts.value(AppFont::InterfaceDefault, themeManager->defaultAppFont(role));
        const QFont font = cfg.fonts.value(role, fallback);
        row.familyCombo->setCurrentFont(font);

        if (role == AppFont::CardText && row.minSpin != nullptr) {
            // A previous load may have narrowed the linked bounds (min 20 pulls
            // max up to 20), which would clamp the values written below. Reset
            // both to the full range first; the valueChanged lambdas re-link
            // them as the values are set.
            row.minSpin->setRange(FontConfig::CARD_TEXT_MIN_SIZE, FontConfig::MAX_FONT_SIZE);
            row.sizeSpin->setRange(FontConfig::MIN_FONT_SIZE, FontConfig::MAX_FONT_SIZE);

            // Resolved by FontConfig, which the renderer reads through as well:
            // a stored size the editor cannot represent (for example a migrated
            // appearance/maxFontSize above 72) shows at the bound the board
            // draws, so a save never rewrites a size that was in use.
            // Max first, then min: each value re-applies the linked bound, so
            // the pair ends up consistent instead of clamping the smaller one.
            row.sizeSpin->setValue(cfg.resolvedCardTextMaxSize());
            row.minSpin->setValue(cfg.resolvedCardTextMinSize());
            continue;
        }

        int size = font.pointSize();
        if (size <= 0) {
            size = qApp->font().pointSize();
        }
        if (size <= 0) {
            size = FALLBACK_FONT_SIZE;
        }
        row.sizeSpin->setValue(qBound(FontConfig::MIN_FONT_SIZE, size, FontConfig::MAX_FONT_SIZE));
    }
    blockSignals(false);
}

FontConfig FontGridWidget::currentFontConfig() const
{
    FontConfig cfg;
    QMetaEnum fontEnum = QMetaEnum::fromType<AppFont::Role>();
    for (int i = 0; i < fontEnum.keyCount(); ++i) {
        auto role = static_cast<AppFont::Role>(fontEnum.value(i));
        const FontRow &row = fontRows.value(role);

        // Canonical family + point size only: a fresh QFont carries default
        // (non-bold, non-italic) weight/style, so nothing combo-side leaks in.
        QFont font(row.familyCombo->currentFont().family());
        font.setPointSize(row.sizeSpin->value());
        cfg.fonts[role] = font;

        if (role == AppFont::CardText && row.minSpin != nullptr) {
            cfg.cardTextMinSize = row.minSpin->value();
            cfg.cardTextMaxSize = row.sizeSpin->value();
        }
    }
    return cfg;
}

void FontGridWidget::retranslateUi()
{
    QMetaEnum fontEnum = QMetaEnum::fromType<AppFont::Role>();
    for (int i = 0; i < fontEnum.keyCount(); ++i) {
        auto role = static_cast<AppFont::Role>(fontEnum.value(i));
        const FontRow &row = fontRows.value(role);

        row.label->setText(roleDisplayName(role));
        row.label->setToolTip(tr(APP_FONT_DESCRIPTIONS.value(role, "")));
        row.familyCombo->setToolTip(tr(APP_FONT_DESCRIPTIONS.value(role, "")));
        row.familyCombo->setAccessibleName(roleDisplayName(role));
        if (role == AppFont::CardText && row.minSpin != nullptr) {
            row.minSpin->setToolTip(tr("Minimum card text size"));
            row.minSpin->setAccessibleName(tr("Minimum card text size"));
            row.minSpin->setSuffix(tr(" pt"));
            row.sizeSpin->setToolTip(tr("Maximum card text size"));
            row.sizeSpin->setAccessibleName(tr("Maximum card text size"));
            row.sizeSpin->setSuffix(tr(" pt"));
            continue;
        }
        row.sizeSpin->setToolTip(tr("Point size for %1").arg(roleDisplayName(role)));
        row.sizeSpin->setAccessibleName(tr("Point size for %1").arg(roleDisplayName(role)));
        row.sizeSpin->setSuffix(tr(" pt"));
    }
}
