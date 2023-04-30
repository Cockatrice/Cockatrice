/***************************************************************************
 *   Copyright (C) 2019 by Fabio Bas   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "main.h"

#include "mocks.h"
#include "version_string.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QtGlobal>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setOrganizationName("Cockatrice");
    app.setApplicationName("Dbconverter");
    app.setApplicationVersion(VERSION_STRING);

    QCommandLineParser parser;
    parser.addPositionalArgument("olddb", "Read existing card database from <file>");
    parser.addPositionalArgument("newdb", "Write new card database to <file>");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    QString oldDbPath;
    QString newDbPath;
    QStringList args = parser.positionalArguments();
    if (args.count() == 2) {
        oldDbPath = QFileInfo(args.at(0)).absoluteFilePath();
        newDbPath = QFileInfo(args.at(1)).absoluteFilePath();
    } else {
        qCritical() << "Usage: dbconverter <olddb> <newdb>";
        parser.showHelp(1);
        exit(0);
    }

    settingsCache = new SettingsCache;
    CardDatabaseConverter *db = new CardDatabaseConverter;

    qInfo() << "---------------------------------------------";
    qInfo() << "Loading cards from" << oldDbPath;
    db->loadCardDatabase(oldDbPath);
    qInfo() << "---------------------------------------------";
    qInfo() << "Saving cards to" << newDbPath;
    db->saveCardDatabase(newDbPath);
    qInfo() << "---------------------------------------------";
}
