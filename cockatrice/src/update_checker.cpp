//
// Created by miguel on 28/12/15.
//

#include <algorithm>
#include <QMessageBox>

#include "update_checker.h"
#include "version_string.h"
#include "qt-json/json.h"

#define LATEST_FILES_URL "https://api.bintray.com/packages/cockatrice/Cockatrice/Cockatrice/files"

UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent){
    //Parse the commit date. We'll use this to check for new versions
    //We know the format because it's based on `git log` which is documented here:
    //  https://git-scm.com/docs/git-log#_commit_formatting
    buildDate = QDate::fromString(VERSION_DATE, "yyyy-MM-dd");
    latestFilesUrl = QUrl(LATEST_FILES_URL);
    response = NULL;
    netMan = new QNetworkAccessManager(this);
    build = NULL;
}

UpdateChecker::~UpdateChecker()
{
    delete build;
}

void UpdateChecker::check()
{
    response = netMan->get(QNetworkRequest(latestFilesUrl));
    connect(response, SIGNAL(finished()),
            this, SLOT(fileListFinished()));
}

#if defined(Q_OS_OSX)
bool UpdateChecker::downloadMatchesCurrentOS(QVariant build)
{
   return build
            .toMap()["name"]
            .toString()
            .contains("osx");
}
#elif defined(Q_OS_WIN)
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
bool UpdateChecker::downloadMatchesCurrentOS(QVariant build)
{
       return build
            .toMap()["name"]
            .toString()
            .contains("qt5.exe");
}
#else
bool UpdateChecker::downloadMatchesCurrentOS(QVariant build)
{
       return build
            .toMap()["name"]
            .toString()
            .contains("qt4.exe");
}
#endif
#else

bool UpdateChecker::downloadMatchesCurrentOS(QVariant)
{
    //If the OS doesn't fit one of the above #defines, then it will never match
    return false;
}

#endif

QDate UpdateChecker::dateFromBuild(QVariant build)
{
    QString formatString = "yyyy-MM-dd";
    QString dateString = build.toMap()["date"].toString();
    dateString = dateString.remove(formatString.length(), dateString.length());

    return QDate::fromString(dateString, formatString);
}

QDate UpdateChecker::findOldestBuild(QVariantList allBuilds)
{
    //Map the build array into an array of dates
    std::vector<QDate> dateArray(allBuilds.size());
    std::transform(allBuilds.begin(), allBuilds.end(), dateArray.begin(), dateFromBuild);

    //Return the first date
    return *std::min_element(dateArray.begin(), dateArray.end());
}

QVariantMap *UpdateChecker::findCompatibleBuild(QVariantList allBuilds) {

    QVariantList::iterator result = std::find_if(allBuilds.begin(), allBuilds.end(), downloadMatchesCurrentOS);

    //If there is no compatible version, return NULL
    if (result == allBuilds.end())
        return NULL;
    else
    {
        QVariantMap *ret = new QVariantMap;
        *ret = (*result).toMap();
        return ret;
    }
}

void UpdateChecker::fileListFinished() {
    try {
        QVariantList builds = QtJson::Json::parse(response->readAll()).toList();
        build = findCompatibleBuild(builds);
        QDate bintrayBuildDate = findOldestBuild(builds);

        bool needToUpdate = bintrayBuildDate > buildDate;
        bool compatibleVersion = build != NULL;

        emit finishedCheck(needToUpdate, compatibleVersion, build);
    }
    catch (const std::exception &exc){
        emit error(exc.what());
    }
}
