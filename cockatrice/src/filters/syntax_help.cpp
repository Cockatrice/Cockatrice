#include "syntax_help.h"

#include <QFile>
#include <QRegularExpression>
#include <QTextStream>

/**
 * Creates the card search syntax help window
 *
 * @return the QTextBrowser
 */
static QTextBrowser *createBrowser(const QString &helpFile)
{
    QFile file(helpFile);

    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        qCWarning(SyntaxHelpLog) << "Could not open syntax help file: " << helpFile;
        return nullptr;
    }

    QTextStream in(&file);
    QString text = in.readAll();
    file.close();

    // Poor Markdown Converter
    auto opts = QRegularExpression::MultilineOption;
    text = text.replace(QRegularExpression("^(###)(.*)", opts), "<h3>\\2</h3>")
               .replace(QRegularExpression("^(##)(.*)", opts), "<h2>\\2</h2>")
               .replace(QRegularExpression("^(#)(.*)", opts), "<h1>\\2</h1>")
               .replace(QRegularExpression("^------*", opts), "<hr />")
               .replace(QRegularExpression(R"(\[([^[]+)\]\(([^\)]+)\))", opts), R"(<a href='\2'>\1</a>)");

    auto browser = new QTextBrowser();
    browser->setParent(nullptr, Qt::Window | Qt::WindowTitleHint | Qt::WindowSystemMenuHint |
                                    Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint |
                                    Qt::WindowFullscreenButtonHint);
    browser->setWindowTitle("Search Help");
    browser->setReadOnly(true);
    browser->setMinimumSize({500, 600});

    QString sheet = QString("a { text-decoration: underline; color: rgb(71,158,252) };");
    browser->document()->setDefaultStyleSheet(sheet);
    browser->setHtml(text);

    browser->show();

    return browser;
}

/**
 * Creates the card search syntax help window and connects its anchorClicked signal to the given QLineEdit.
 * The window will automatically close when the QLineEdit is destroyed.
 *
 * @return the QTextBrowser
 */
QTextBrowser *createSearchSyntaxHelpWindow(QLineEdit *lineEdit)
{
    auto browser = createBrowser("theme:help/search.md");
    QObject::connect(browser, &QTextBrowser::anchorClicked,
                     [lineEdit](const QUrl &link) { lineEdit->setText(link.fragment()); });
    QObject::connect(lineEdit, &QObject::destroyed, browser, &QTextBrowser::close);
    return browser;
}

/**
 * Creates the deck search syntax help window and connects its anchorClicked signal to the given QLineEdit.
 * The window will automatically close when the QLineEdit is destroyed.
 *
 * @return the QTextBrowser
 */
QTextBrowser *createDeckSearchSyntaxHelpWindow(QLineEdit *lineEdit)
{
    auto browser = createBrowser("theme:help/deck_search.md");
    QObject::connect(browser, &QTextBrowser::anchorClicked, [lineEdit](const QUrl &link) {
        if (link.fragment() == "cardSearchSyntaxHelp") {
            createSearchSyntaxHelpWindow(lineEdit);
        } else {
            lineEdit->setText(link.fragment());
        }
    });
    QObject::connect(lineEdit, &QObject::destroyed, browser, &QTextBrowser::close);
    return browser;
}