// Needed for static-linking
#ifdef STATIC
#include <QtPlugin>
	Q_IMPORT_PLUGIN(qjpeg)
	Q_IMPORT_PLUGIN(qgif)
	Q_IMPORT_PLUGIN(qtiff)
#endif

#include "Window.h"

int main(int argc, char **argv)
{
	QApplication a(argc, argv);
	QString param = "";

	if(argc > 1) {
		// Pass first command-line parameter using current system locale
		param = QString("%L1").arg(argv[1]);
	}
	Window w(param);
	w.show();
	return a.exec();
}
