#include "email_parser.h"

#include <QRegularExpression>
#include <QString>

QPair<QString, QString> EmailParser::parseEmailAddress(const QString &dirtyEmailAddress)
{
    // https://www.regular-expressions.info/email.html
    static const QRegularExpression emailRegex(R"(^([A-Z0-9._%+-]+)@([A-Z0-9.-]+\.[A-Z]{2,})$)",
                                               QRegularExpression::CaseInsensitiveOption);
    const auto match = emailRegex.match(dirtyEmailAddress);

    if (dirtyEmailAddress.isEmpty() || !match.hasMatch()) {
        return {};
    }

    QString capturedEmailUser = match.captured(1);
    QString capturedEmailAddressDomain = match.captured(2);

    // Replace googlemail.com with gmail.com, as is standard nowadays
    // https://www.gmass.co/blog/domains-gmail-com-googlemail-com-and-google-com/
    if (capturedEmailAddressDomain.toLower() == "googlemail.com") {
        capturedEmailAddressDomain = "gmail.com";
    }

    // Trim out dots and pluses from Google/Gmail domains
    if (capturedEmailAddressDomain.toLower() == "gmail.com") {
        // Remove all content after the first plus sign (as unnecessary with gmail)
        // https://gmail.googleblog.com/2008/03/2-hidden-ways-to-get-more-from-your.html
        const auto firstPlusSign = capturedEmailUser.indexOf("+");
        if (firstPlusSign != -1) {
            capturedEmailUser = capturedEmailUser.left(firstPlusSign);
        }

        // Remove all periods (as unnecessary with gmail)
        // https://gmail.googleblog.com/2008/03/2-hidden-ways-to-get-more-from-your.html
        capturedEmailUser.replace(".", "");
    }
    // Trim out minuses from Yahoo domains
    else if (capturedEmailAddressDomain.toLower() == "yahoo.com") {
        const auto firstMinusSign = capturedEmailUser.indexOf("-");
        if (firstMinusSign != -1) {
            capturedEmailUser = capturedEmailUser.left(firstMinusSign);
        }
    }

    return {capturedEmailUser, capturedEmailAddressDomain};
}

QString EmailParser::getParsedEmailAddress(const QString &dirtyEmailAddress)
{
    const auto parsedEmailAddress = EmailParser::parseEmailAddress(dirtyEmailAddress);
    return EmailParser::getParsedEmailAddress(parsedEmailAddress);
}

QString EmailParser::getParsedEmailAddress(const QPair<QString, QString> &emailAddressIntermediate)
{
    const auto emailUser = emailAddressIntermediate.first;
    const auto emailDomain = emailAddressIntermediate.second;
    return emailUser + "@" + emailDomain;
}