#ifndef Window_H
#define Window_H

#include <QWidget>
#include <QLabel>
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QByteArray>
#include <QBuffer>
#include <QPixmap>
#include <QPushButton>
#include <QImage>
#include <QDragEnterEvent>
#include <QFileInfo>
#include <QUrl>
#include <QTime>
#include <QApplication>
#include <QFileDialog>
#include <QDesktopServices>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtGui/QStatusBar>

#include <QDebug>

#include "mediancut.h"
#include "ColorQuantizer.h"


#include "ScrollArea.h"
#include <QPainter>
#include <QtDebug>

class Window : public QWidget
{
	Q_OBJECT
	static const quint8 numImages = 3;
	void createImageGroupBox(QString &imageFileName);
	void createControlsGroupBox();
	void createStatusBar();
	void createMainLayout();
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	void loadImageFile(QString &fileURI);
	bool fileTestLossy(QString &imageURI);
	quint32 imageSaveIndexed(QImage &in, QImage &out);
	quint32 imageSaveLossy(QImage &in, QImage &out, quint8 quality);
	quint32 imageMeanSquaredError(QImage &original, QImage &lossy);
	char imageAdviseSaveFormat(quint32 sizeJPG, quint32 sizePNG);

	QString *fileOpenPath, *defaultStatusMessage;
	QVBoxLayout *mainLayout;
	QHBoxLayout *loadLayout;
	QGridLayout *imageLayout, *controlsLayout;
	ScrollArea *scrollArea[numImages];
	QImage *image[numImages];
	QLabel *labelImage[numImages], *labelText[numImages], *statusBarLabel;
	QGroupBox *loadGroupBox, *imageGroupBox, *controlsGroupBox;
	QPushButton *buttonJpg, *buttonPNG, *buttonCantTell, *buttonLoad;
	QStatusBar *statusBar;

	quint32 sizeJPG, sizePNG;
	quint8 jpgQuality;

private slots:
	void openFileDialog();
	void saveAsPNG();
	void saveAsJPG();
	void saveAsAdvised();
	void syncHorizontalScrollbars(int newVal);
	void syncVerticalScrollbars(int newVal);
	void openSystemWebBrowser(QString url);

public:
	Window(QString &imageURI);
	~Window(){}
	qint8 getJpegQualityForSize(QImage &image, qint32 fileSize);
};

#endif // Window_H

