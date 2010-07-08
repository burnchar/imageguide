/*
  ImageGuide 1.0

  (C) 2009 Charles N. Burns (charlesnburns@gmail.com)

  Shows an opened image as both a JPG and an indexed PNG file of the same size.
  One of the two will usually look noticeably better than the other. The user
  may then choose the best looking image, or allow the program to recommend a
  format.

  Bugs:
  - Doesn't give a very good recommendation for many large images or for
	images which have at any time been saved in a lossy format.
	- Solution (partial) Informs user if image is CURRENTLY saved as JPEG
  - Uses system palette. Doesn't pick colors very intelligently for PNG images.
	- Solution: Implement a color quantization feature (using Median Cut, etc.)

*/
// TODO: Really need to fix that weird color problem
// TODO: Use "setText" on saved functions to reference this rp[ogram
// TODO: If recommended JPEG quality is really high, like 98+, probably need to reconsider saving as PNG
// TODO: Sometimes the JPEG is much larger or smaller than the PNG, like when it's a flat color. What to do about that?
// TODO: The occasional off color is probably because it uses 16-bit colors.
// TODO: Doesn't always pick obvious colors, like in darwin_doppler_original.png
//        For indexed color images, grab their index and use it for the output.
// TODO: Support alpha, at least a transparent color. Maybe the above will help.
// TODO: Show image sizes, maybe
// TODO: Maybe could use Qt to do the conversion to a 16-bit color image using convertToFormat
// TODO: If dest image is larger than source image, keep old image
// TODO: Right-click: Display menu: Zoom in or out (Maybe just double-click zoom)
// TODO: Include sample file and show what to look for
// TODO: Make command-line support unicode more complete
// TODO: Save getJpegQualityForSize's JPG so it doesn't have to be repeated. Same with PNG
// TODO: Clicking on label to left of Open Image button also opens an image
// TODO: Add //: comments for every tr()
// TODO: Make default botton "Open image"
 // TODO: Test with large image files. The recommendation tool needs tweaking.
// TODO: If image path is too long to display, replace part with elipses
// TODO: Save as PNG should have a slider to allow user to choose quality
// NOTABUG: Does saving the file actually work?
// FIXED: Pictures with alpha have their alpha set to BLACK
//

#include "Window.h"

//! Constructor for Window class.
//! @param imageURI Optional URI of a file to initially open (from command line)
Window::Window(QString &imageURI)
{
	this->setWindowTitle("CNB ImageGuide 0.4 Beta Initial Release");
//	defaultStatusMessage = new QString(tr("© 2009 Charles N. Burns - <a href=\"http://www.synergysoftwaregroup.com\">www.SynergySoftwareGroup.com</a>"));
	defaultStatusMessage = new QString(tr("© 2009 Charles N. Burns - <a href=\"http://www.formortals.com/author/charles\">www.formortals.com</a>"));
	this->fileOpenPath = new QString(".");
	this->sizeJPG = this->sizePNG = this->jpgQuality = 0;

	createStatusBar();
	createImageGroupBox(imageURI);
	createControlsGroupBox();
	createMainLayout();

	this->setAcceptDrops(true);
	buttonCantTell->setEnabled(false);
	buttonJpg->setEnabled(false);
	buttonPNG->setEnabled(false);

	connect(buttonLoad, SIGNAL(clicked()), this, SLOT(openFileDialog()));
	connect(buttonPNG, SIGNAL(clicked()), this, SLOT(saveAsPNG()));
	connect(buttonJpg, SIGNAL(clicked()), this, SLOT(saveAsJPG()));
	connect(buttonCantTell, SIGNAL(clicked()), this, SLOT(saveAsAdvised()));
	connect(statusBarLabel, SIGNAL(linkActivated(QString)), this,
			SLOT(openSystemWebBrowser(QString)));
}


//! Sets up and creates the status bar (the information panel at the bottom)
//! @see Window()
void Window::createStatusBar()
{
	statusBar = new QStatusBar(this);
	statusBarLabel = new QLabel(*this->defaultStatusMessage);
	statusBar->addWidget(statusBarLabel, 100);
}


//! Sets up the group box from which views of the opened image are displayed
//! param imageURI Optional URI of image file to initially open.
//! @see Window()
void Window::createImageGroupBox(QString &imageURI)
{
	imageLayout = new QGridLayout();
	QString imageText[this->numImages] = {tr("Original"), tr("JPG"), tr("PNG")};
	for(quint8 count = 0; count < this->numImages; ++count) {
		image[count] = new QImage();
		labelImage[count] = new QLabel(this);
		scrollArea[count] = new ScrollArea;
		scrollArea[count]->setBackgroundRole(QPalette::Light);
		scrollArea[count]->setWidget(labelImage[count]);
		imageLayout->addWidget(scrollArea[count], 0, count);
		//labelImage[count]->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
		connect(scrollArea[count]->horizontalScrollBar(),
				SIGNAL(valueChanged(int)), this, SLOT(syncHorizontalScrollbars(int)));
		connect(scrollArea[count]->verticalScrollBar(),
				SIGNAL(valueChanged(int)), this, SLOT(syncVerticalScrollbars(int)));
		labelText[count] = new QLabel(imageText[count]);
		imageLayout->addWidget(labelText[count], 1, count, Qt::AlignBottom);
	}
	imageGroupBox = new QGroupBox(tr("ImageGuide 1.0 by Charles Burns - Helps choose the best image type: JPG and PNG"));
	imageGroupBox->setMinimumHeight(130);
	imageGroupBox->setLayout(imageLayout);
	if(!imageURI.isEmpty()) loadImageFile(imageURI);
}


//! Sets up the group box from which the buttons are displayed
//! @see Window()
void Window::createControlsGroupBox()
{
	buttonLoad = new QPushButton(tr("&Open A Picture"));
	buttonJpg = new QPushButton(tr("&JPG"));
	buttonPNG = new QPushButton(tr("&PNG"));
	buttonCantTell = new QPushButton(tr("I &Can't Tell"));
	buttonJpg->setMinimumWidth(80);
	buttonPNG->setMinimumWidth(80);
	buttonCantTell->setFixedWidth(100);
	controlsLayout = new QGridLayout();
	controlsLayout->addWidget(new QLabel(tr(
			"1. Drag and drop a picture here or click to")), 0, 0);
	controlsLayout->addWidget(buttonLoad, 0, 1, 1, 3);
	controlsLayout->addWidget(new QLabel(tr(
			"2. Choose and save <b>best looking</b> image:")), 1, 0);
	controlsLayout->addWidget(buttonJpg, 1, 1);
	controlsLayout->addWidget(buttonPNG, 1, 2);
	controlsLayout->addWidget(buttonCantTell, 1, 3);
	controlsGroupBox = new QGroupBox();
	controlsGroupBox->setLayout(controlsLayout);
}


//! Sets up and creates the main Window layout
//! @see Window()
void Window::createMainLayout()
{
	mainLayout = new QVBoxLayout();
	mainLayout->addWidget(imageGroupBox, 99);
	mainLayout->addWidget(controlsGroupBox, 0, Qt::AlignLeft);
	mainLayout->addWidget(statusBar);
	this->setLayout(mainLayout);
}


//! This is a slot which open's "url" on system's default web browser
//! @param url The URL to open in the web browser
//! @see Window()
//! @see createStatusBar()
void Window::openSystemWebBrowser(QString url)
{
	QDesktopServices::openUrl(url);
}


//! Sets up the default file dialog box for opening supported image files.
//! @see Window()
//! @see loadImageFile()
void Window::openFileDialog()
{
	QString imageURI = QFileDialog::getOpenFileName(
			this, tr("Open Picture"), *this->fileOpenPath,
			tr("Image Files (*.bmp *.png *.gif *.jpg *.jpe *.jpeg *.tif *.tiff *.pbm *.pgm *.ppm *.xbm *.xpm);;All files (*.*)"));
	loadImageFile(imageURI);
	QFileInfo fInfo(imageURI);
	if(! fInfo.canonicalPath().isEmpty())
		*this->fileOpenPath = fInfo.canonicalPath();
}


//! Opens file dialog box to save image as an indexed color PNG
//! @see loadImageFile()
//! @see saveAsAdvised()
void Window::saveAsPNG()
{
	QString fileURI = QFileDialog::getSaveFileName(this, tr("Save Foo"), // TODO: Change foo
		*this->fileOpenPath, tr("PNG file (*.png)"));
	if(image[2]->save(fileURI, "PNG") == true) {
		statusBarLabel->setText(QString(tr("<b>File saved:</b> ")
										+ QDir::toNativeSeparators(fileURI)));
	}
	else { // User cancellation (or possibly write error)
		statusBarLabel->setText(QString(tr("<b>File not saved</b>")));
	}
}


//! Opens file dialog box to save image as a JPEG
//! @see loadImageFile()
//! @see saveAsAdvised()
void Window::saveAsJPG()
{
	QString fileURI = QFileDialog::getSaveFileName(this, tr("Save Foo"),
		*this->fileOpenPath, tr("JPG file (*.jpg)"));

	if(image[0]->save(fileURI, "JPG", this->jpgQuality) == true) {
		statusBarLabel->setText(QString(tr("<b>File saved:</b> ")
										+ QDir::toNativeSeparators(fileURI)));
	}
	else { // User cancellation (or possibly write error)
		statusBarLabel->setText(QString(tr("<b>File not saved</b>")));
	}
}


//! Calls the save function appropriate to save in advised format
//! @see loadImageFile()
//! @see imageAdviseSaveFormat()
void Window::saveAsAdvised()
{
	char myformat = imageAdviseSaveFormat(this->sizeJPG, this->sizePNG);
	switch(myformat) {
		case 'p': saveAsPNG();
		break;
		case 'j': saveAsJPG();
		break;
		default:
			statusBarLabel->setText(QString(tr("Alternate universe detected")));
	}
}

//! Looks at a file name's extension to see if it is likely to have been saved
//! in a lossy format. Currently looks for JPEG extensions.
//! @param imageURI The file name from which to get the extension.
//! @return True if file is saved in a lossy format, false otherwise.
//! @see loadImageFile()
bool Window::fileTestLossy(QString &imageURI)
{
	bool retval = false;
	QFileInfo fileInfo(imageURI);
	QString extension = fileInfo.suffix().toLower();
	if(extension == "jpg" || extension == "jpe" || extension == "jpeg") {
		retval = true;
	}
	return retval;
}


//! Converts input image to indexed color (8-bit), then saves to output image.
//! Uses PNG compression.
//! Note: The color index is from the system palette. It does not generate
//! an adaptive palette through color quantization of the image. Maybe v2.0
//! @param[in] in The lossless input image
//! @param[out] out The image to save to using indexed color
//! @return The size of the saved image
quint32 Window::imageSaveIndexed(QImage &in, QImage &out)
{
	QImage imageRgb = in.convertToFormat(QImage::Format_ARGB32);
	QRgb *imagePtr = reinterpret_cast<QRgb *>(imageRgb.bits());

	const int maxColors = 256;
	std::vector<QRgb> colorsv(maxColors);
	QVector<QRgb> colors(maxColors);

	ColorReducer cr(imagePtr, imageRgb.numBytes() / 4);
	cr.reduceColors(colorsv, maxColors);


	// TODO: Make this a direct conversion rather than a slow loop.
	for(int color = 0; color < maxColors; ++color) {
		colors[color] = colorsv[color];
	}
//! END Test new color reducer class


//! TEST MY MEDIAN CUT
	// TODO: The conversion between newColMap and colors isn't really necessary.
	//       Do it directly.
//	quint8 newColMap[maxColors][4];
//	myMedianCut(newHist, newColMap, maxColors);

//	QVector<QRgb> colors(maxColors);
//	for(int c = 0; c < maxColors; ++c) {
//		colors.append(qRgb(newColMap[c][1], newColMap[c][2], newColMap[c][3]));
//	}
//! END TEST MY MEDIAN CUT


/*******************************************************************************
 ** Code to try Median Cut ends here
\******************************************************************************/

	QImage temp(in.size(), QImage::Format_ARGB32);
	temp.fill(QColor(Qt::white).rgb());
	QPainter painter(&temp);
	painter.drawImage(0, 0, in);

//	temp.save(&buffer, "JPG", quality);

	out = temp.convertToFormat(QImage::Format_Indexed8, colors, Qt::ThresholdDither);

//	out = in.convertToFormat(QImage::Format_Indexed8, colors, Qt::ThresholdDither);

//	QImage temp(out.size(), QImage::Format_Indexed8);
//	temp.fill(QColor(Qt::transparent).rgb());
//	QPainter painter(&temp);
//	painter.drawImage(0,0,in.createAlphaMask());
//	painter.drawImage(0, 0, out);
//	out=temp;


//	temp.save(&buffer, "JPG", quality);


//	if(in.hasAlphaChannel()) // Copies alpha to PNG image. Otherwise transparent color would be black.
//		out.setAlphaChannel(in.alphaChannel()); // This sets image to PNG24
//	out = in.convertToFormat(QImage::Format_Indexed8, colors, Qt::ThresholdDither);
//	out.invertPixels(QImage::InvertRgba);
//	out.invertPixels(QImage::InvertRgb);

//image[2]->save("OUTTEST3.PNG", "PNG");
//			if(image[0]->hasAlphaChannel()) // Copies alpha to PNG image. Otherwise transparent color would be black.
//				image[2]->setAlphaChannel(image[0]->alphaChannel()); // This sets image to PNG24
//				image[2]->setAlphaChannel(image[0]->createAlphaMask()); // This sets image to PNG24
			// TRY CONVERTING TO PIXMAP AND USING SETMASK. May need to set to 256 colors first.
//			this->sizePNG = imageSaveIndexed(*image[0], *image[2]);


	quint32 retval;
	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::WriteOnly);

	//Why am I doing the following?
	out.save(&buffer, "PNG");
	retval = (quint32)ba.size();
// qDebug() << "File size in imageSaveIndexed():" << ba.size();

	ba.clear();
	buffer.close();
	return retval;
}


//! Saves image with JPEG compression in given quality (0 - 100, Qt's scale)
//! @param[in] in The lossless input image
//! @param[out] out The image to save to using JPEG compression
//! @param quality The quality level (0 - 100, 0 the most compressed)
//! @return The size of the saved image
quint32 Window::imageSaveLossy(QImage &in, QImage &out, quint8 quality)
{
	quint32 retval;
	QByteArray ba;
	QBuffer buffer(&ba);
	buffer.open(QIODevice::WriteOnly);

	QImage temp(in.size(), QImage::Format_ARGB32);
	temp.fill(QColor(Qt::white).rgb());
	QPainter painter(&temp);
	painter.drawImage(0, 0, in);

	temp.save(&buffer, "JPG", quality);
//	in.save(&buffer, "JPG", quality);
	out.loadFromData(ba, "JPG");
	retval = (quint32)ba.size();
	buffer.close();
	return retval;
}


//! Returns an approximation of the Mean Squared Error (MSE) between the
//! original and compressed images. A proper MSE will use the square of the
//! absolute value of the distance between pixels. For performance reasons,
//! this code just takes the square of the xor. It works reasonably well.
//! @param original Reference to the original image
//! @param lossy Reference to the compressed image to compare to the original
//! @see mageAdviseSaveFormat()
quint32 Window::imageMeanSquaredError(QImage &original, QImage &lossy)
{
	QImage lossy32 = lossy.convertToFormat(QImage::Format_ARGB32);
	QImage orig32 = original.convertToFormat(QImage::Format_ARGB32);
	quint32 *originalPtr = reinterpret_cast<quint32 *>(orig32.bits());
	quint32 *lossyPtr    = reinterpret_cast<quint32 *>(lossy32.bits());
	quint32 pixelCount = original.height() * original.width() - 1;
	quint32 redError = 0, greenError = 0, blueError = 0;
	QRgb pixelOrig, pixelLossy, delta;

	for(quint32 index = 0; index < pixelCount; ++index) {
		pixelOrig = originalPtr[index];
		pixelLossy = lossyPtr[index];
		delta = pixelOrig ^ pixelLossy;
		redError   += qRed(delta);//   * qRed(delta);
		greenError += qGreen(delta);// * qGreen(delta);
		blueError  += qBlue(delta) ;// * qBlue(delta);
	}
	quint64 sum = redError + greenError + blueError;
	quint32 mse = sum / (pixelCount * 3);
	return mse;
}


//! Recommends a compression format in which to save the opened image.
//! This is meant only for when the user cannot tell a difference between the
//! compressed visible images. It doesn't work especially well with large
//! pictures or with images which have been compressed as JPEG before opening.
//! @param image The image for which to recommend a save format.
//! @param sizePNG The size of the file saved as an 8-bit indexed color PNG
//! @return Recommended format, lower-case. 'p' for PNG, 'j' for JPEG
char Window::imageAdviseSaveFormat(quint32 sizeJPG, quint32 sizePNG)
{
	// Also needs to take into account final file size
	quint32 jpgError = imageMeanSquaredError(*this->image[0], *this->image[1]);
	quint32 pngError = imageMeanSquaredError(*this->image[0], *this->image[2]);
//	qDebug() << "Mean Squared Error: JPG:" << jpgError << ".  PNG:" << pngError;
//	qDebug() << "File size: JPG:" << sizeJPG << ".  PNG:" << sizePNG;
	char retval = 0;
	if     (jpgError > pngError) retval = 'p';
	else if(jpgError < pngError) retval = 'j';
	else if(sizePNG  < sizeJPG ) retval = 'p';
	else retval = 'j';

	return retval;
}


//! Loads an image file and displays the original and saved as JPG & PNG
//! Displays an error message in the status bar if file cannot be loaded.
//! Displays warning if a JPG file is loaded.
//! @param imageURI URI of the image to attempt to load.
void Window::loadImageFile(QString &imageURI)
{
	QTime time;
	time.start();

	if(! imageURI.isEmpty()) {
		if(image[0]->load(imageURI) == false) {
			statusBarLabel->setText(QString(
					tr("<b>I can't open file:</b> ")
					+ QDir::toNativeSeparators(imageURI)));
		}
		else {
			*image[0] = image[0]->convertToFormat(QImage::Format_ARGB32);
//			qDebug() << "Has alpha: " << image[0]->hasAlphaChannel();
			QApplication::setOverrideCursor(Qt::BusyCursor);
			statusBarLabel->setText(*defaultStatusMessage);
			if(fileTestLossy(imageURI) == true) {
				QMessageBox::warning(this, imageURI + tr(" - Original image altered by JPG format"),
									 tr("This file's type (<b>.JPG</b>) makes <b>PNG</b> unable to compress it properly. Saving as .JPG makes changes to an image which cannot be fixed. <ul><li>If the file is a <i>photograph</i>, do nothing: .JPG is a good choice. <li>Otherwise, if you have the original before it was converted to .JPG, use the original.</ul>"), QMessageBox::Ok);
			}
			this->sizePNG = imageSaveIndexed(*image[0], *image[2]);
			this->jpgQuality = getJpegQualityForSize(*image[0], this->sizePNG);

			this->sizeJPG = imageSaveLossy(*image[0], *image[1], this->jpgQuality);

			for(quint8 count = 0; count < this->numImages; ++count) {
				labelImage[count]->setPixmap(QPixmap::fromImage(*image[count]));
				labelImage[count]->adjustSize();
			}
			QApplication::restoreOverrideCursor();

			if(this->isVisible()) {
				buttonCantTell->setEnabled(true);
				buttonJpg->setEnabled(true);
				buttonPNG->setEnabled(true);
				imageLayout->activate();
			}
			imageGroupBox->setTitle(QDir::toNativeSeparators(imageURI));
		}
	}
	qDebug("Time elapsed: %d ms", time.elapsed());
	// Debug build:
	// 3.75 secs for a 1024x1024  Mine:
	// 18 secs for a 2048x2048    Mine: 11.7
	// 75 seconds for 4096x4096   Mine: 47 sec

}


//! Tries to save "image" as a JPEG at a quality level such that its file size
//! is as close as possible to "targetSize".
//! This can take a long time with large images.
//! In certain cases, can save at the same quality level twice
//! Approximate worst-case runtime is O(log100 * width * height)
//! @param image A QImage which is already associated with a file
//! @param targetSize The target size to approximate (in bytes)
//! @return The closest matching JPEG save quality (0-100, Qt's scale)
qint8 Window::getJpegQualityForSize(QImage &image, qint32 targetSize)
{
	QByteArray ba;
	QBuffer buffer(&ba);
	quint8 quality = 50, lo = 0, hi = 100;
//qDebug() << "Looking for JPG quality to get file size close to PNG file size.";

	//TODO: If more than 1 images is being processed at a time,
	//      use one core per image rather than splitting just one image n-ways.

	// 7 is log base 2 of 100, the max possible number of resaves. TODO: Clean up
	QProgressDialog progress("Examining picture...", "Cancel", 0, 7, this);
	progress.setModal(true);

	bool cancelled = false; // TODO: Use this value to display if it was cancelled

	quint8 tryCount = 0;
	do {
		//! qDebug() << "Trying quality: " << quality << " of 100.";

		progress.setValue(tryCount);
		if(progress.wasCanceled()) {
			cancelled = true;
			statusBarLabel->setText(tr("Processing cancelled."));
			break;
		}

		buffer.open(QIODevice::WriteOnly);
		image.save(&buffer, "JPG", quality);

		// Encoded size is too big. Need to reduce quality to reach target size.
		if(ba.size() >= targetSize) {
			hi = quality;
			quality = (quality + lo) / 2;
			if(quality == hi) break;
		}

		// Encoded size is too small. Need to increase quality.
		else {
			lo = quality;
			quality = (quality + hi) / 2;
			if(quality == lo) break;
		}

		ba.clear();
		buffer.close();
		qApp->processEvents();
		++tryCount;
	} while(true);

//qDebug() << "Recommended JPEG quality to be close to PNG file size:" << quality;
	return quality;
}


//! Called when user drags a file over the window.
//! Filters drag and drop MIME data to only include URIs.
//! @param event The event from which to filter all but URI MIME types.
void Window::dragEnterEvent(QDragEnterEvent *event)
{
	if(event->mimeData()->hasUrls()) {
		event->acceptProposedAction();
	}
}


//! Called when a user drops a file or files to the window.
//! If the user attempts to drop more a group of files, only the first is taken.
//! Checks to ensure the drop is a file, but does not check file validity.
//! Once checks are passed, loads the image.
//! @param event The drop event data from which to grab the file URI
//! @see dragEnterEvent()
//! @see loadImageFile()
void Window::dropEvent(QDropEvent *event)
{
	QFileInfo fInfo;
	QList<QUrl> urls = event->mimeData()->urls();
	if(! urls.isEmpty()) {
		QString fileURI = urls.first().toLocalFile();
		if(! fileURI.isEmpty()) {
			fInfo.setFile(fileURI);
			if(fInfo.isFile()) {
				loadImageFile(fileURI);
			}
		}
	}
}


//! Slot called on any horizontal movement of an image, scrollbar or mouse.
//! Ensures that all images are also horizontally scrolled.
//! @param newval The scrollbar position, simply passed to other scrollbars.
//! @see createImageGroupBox()
void Window::syncHorizontalScrollbars(int newVal)
{
	for(quint8 count = 0; count < this->numImages; ++count) {
		scrollArea[count]->horizontalScrollBar()->setValue(newVal);
	}
}

void Window::syncVerticalScrollbars(int newVal)
{
	for(quint8 count = 0; count < this->numImages; ++count) {
		scrollArea[count]->verticalScrollBar()->setValue(newVal);
	}
}

