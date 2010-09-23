/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@gmx.net   *
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

#include <QApplication>
#include <QTextCodec>
#include <QtPlugin>
#include <QTranslator>
#include <QLibraryInfo>
#include <QDateTime>
#include <QSettings>
#include <QIcon>
#include <QDir>
#include <stdio.h>

#include "main.h"
#include "window_main.h"
#include "dlg_settings.h"
#include "carddatabase.h"
#include "settingscache.h"
#include "pixmapgenerator.h"
#include "rng_sfmt.h"

//Q_IMPORT_PLUGIN(qjpeg)

CardDatabase *db;
QTranslator *translator, *qtTranslator;
SettingsCache *settingsCache;
RNG_Abstract *rng;

void myMessageOutput(QtMsgType /*type*/, const char *msg)
{
	static FILE *f = NULL;
	if (!f)
		f = fopen("qdebug.txt", "w");
	fprintf(f, "%s\n", msg);
	fflush(f);
}

void installNewTranslator()
{
	QString lang = settingsCache->getLang();

	qtTranslator->load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
	qApp->installTranslator(qtTranslator);
	
	translator->load(translationPrefix + "_" + lang, ":/translations");
	qApp->installTranslator(translator);
}

int main(int argc, char *argv[])
{
//	qInstallMsgHandler(myMessageOutput);
	QApplication app(argc, argv);
#ifdef Q_OS_MAC
	QDir baseDir(app.applicationDirPath());
	baseDir.cdUp();
	QDir pluginsDir = baseDir;
	pluginsDir.cd("PlugIns");
	app.addLibraryPath(pluginsDir.absolutePath());
	baseDir.cdUp();
	baseDir.cdUp();
#endif
#ifdef Q_OS_WIN
	app.addLibraryPath(app.applicationDirPath() + "/plugins");
#endif
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

	QCoreApplication::setOrganizationName("Cockatrice");
	QCoreApplication::setOrganizationDomain("cockatrice.de");
	QCoreApplication::setApplicationName("Cockatrice");
	
	rng = new RNG_SFMT;
	settingsCache = new SettingsCache;
	db = new CardDatabase;

	qtTranslator = new QTranslator;
	translator = new QTranslator;
	installNewTranslator();
	
	qsrand(QDateTime::currentDateTime().toTime_t());
	
	bool startMainProgram = true;
#ifdef Q_OS_MAC
	if (!db->getLoadSuccess())
		if (db->loadCardDatabase(baseDir.absolutePath() + "/cards.xml"))
			settingsCache->setCardDatabasePath(baseDir.absolutePath() + "/cards.xml");
	if (!QDir(settingsCache->getDeckPath()).exists())
		settingsCache->setDeckPath(baseDir.absolutePath() + "/decks");
	if (!QDir(settingsCache->getPicsPath()).exists())
		settingsCache->setPicsPath(baseDir.absolutePath() + "/pics");
#endif
	if (!db->getLoadSuccess() || !QDir(settingsCache->getDeckPath()).exists() || !QDir(settingsCache->getPicsPath()).exists()) {
		DlgSettings dlgSettings;
		dlgSettings.show();
		app.exec();
		startMainProgram = (db->getLoadSuccess() && QDir(settingsCache->getDeckPath()).exists() && QDir(settingsCache->getPicsPath()).exists());
	}
	
	if (startMainProgram) {
		MainWindow ui;
		qDebug("main(): MainWindow constructor finished");
		
		QIcon icon(":/resources/appicon.svg");
		ui.setWindowIcon(icon);
		
		ui.show();
		qDebug("main(): ui.show() finished");
		
		app.exec();
	}

	delete db;
	delete settingsCache;
	delete rng;
	PingPixmapGenerator::clear();
	CountryPixmapGenerator::clear();
	UserLevelPixmapGenerator::clear();
	
	return 0;
}
