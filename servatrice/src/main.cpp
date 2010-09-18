/***************************************************************************
 *   Copyright (C) 2008 by Max-Wilhelm Bruker   *
 *   brukie@laptop   *
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

#include <QCoreApplication>
#include <QTextCodec>
#include <iostream>
#include "servatrice.h"
#include "rng_abstract.h"

void myMessageOutput(QtMsgType /*type*/, const char *msg)
{
	static FILE *f = NULL;
	if (!f)
		f = fopen("qdebug.txt", "w");
	fprintf(f, "%s\n", msg);
	fflush(f);
}

void testRNG()
{
	const int n = 500000;
	std::cerr << "Testing random number generator (n = " << n << " * bins)..." << std::endl;
	
	const int min = 1;
	const int minMax = 2;
	const int maxMax = 10;
	
	QVector<QVector<int> > numbers(maxMax - minMax + 1);
	QVector<double> chisq(maxMax - minMax + 1);
	for (int max = minMax; max <= maxMax; ++max) {
		numbers[max - minMax] = rng->makeNumbersVector(n * (max - min + 1), min, max);
		chisq[max - minMax] = rng->testRandom(numbers[max - minMax]);
	}
	for (int i = 0; i <= maxMax - min; ++i) {
		std::cerr << (min + i);
		for (int j = 0; j < numbers.size(); ++j) {
			if (i < numbers[j].size())
				std::cerr << "\t" << numbers[j][i];
			else
				std::cerr << "\t";
		}
		std::cerr << std::endl;
	}
	std::cerr << std::endl << "Chi^2 =";
	for (int j = 0; j < chisq.size(); ++j)
		std::cerr << "\t" << QString::number(chisq[j], 'f', 3).toStdString();
	std::cerr << std::endl << "k =";
	for (int j = 0; j < chisq.size(); ++j)
		std::cerr << "\t" << (j - min + minMax);
	std::cerr << std::endl << std::endl;
}

int main(int argc, char *argv[])
{
	qInstallMsgHandler(myMessageOutput);

	QCoreApplication app(argc, argv);
	app.setOrganizationName("Cockatrice");
	app.setApplicationName("Servatrice");
	
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	
	std::cerr << "Servatrice " << Servatrice::versionString.toStdString() << " starting." << std::endl;
	std::cerr << "-------------------------" << std::endl;

	testRNG();
	
	Servatrice server;
	
	std::cerr << "-------------------------" << std::endl;
	std::cerr << "Server initialized." << std::endl;
	
	return app.exec();
}
