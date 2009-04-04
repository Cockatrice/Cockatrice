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
#include "window_main.h"
#include <stdio.h>

//Q_IMPORT_PLUGIN(qjpeg)

void myMessageOutput(QtMsgType type, const char *msg)
{
	static FILE *f = NULL;
	if (!f)
		f = fopen("qdebug.txt", "w");
	fprintf(f, "%s\n", msg);
	fflush(f);
}

int main(int argc, char *argv[])
{
//	qInstallMsgHandler(myMessageOutput);
      QApplication app(argc, argv);
      app.addLibraryPath("plugins");
      QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));

      MainWindow *ui = new MainWindow;
      ui->show();

      return app.exec();
}

