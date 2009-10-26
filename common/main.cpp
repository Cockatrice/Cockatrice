#include <QApplication>
#include <QTextCodec>
#include "widget.h"

int main(int argc, char *argv[])
{
//	qInstallMsgHandler(myMessageOutput);
	QApplication app(argc, argv);
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	
	Widget *widget = new Widget;
	widget->show();
	
	return app.exec();
}

 
