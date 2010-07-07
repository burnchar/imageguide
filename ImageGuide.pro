TARGET = ImageGuide
TEMPLATE = app
SOURCES += main.cpp \
	Window.cpp \
    colorreducer.cpp
HEADERS += Window.h \
	ScrollArea.h \
    colorreducer.h
#CONFIG += stl # To enable STL in the compiler

### Uncomment for static build
#CONFIG += static


# QTPLUGIN += qjpeg qgif qtiff
# win32:QMAKE_LFLAGS += /LIBPATH:"$$[QT_INSTALL_PLUGINS]/imageformats"
# !win32:QMAKE_LFLAGS += -L$$[QT_INSTALL_PLUGINS]/imageformats
# http://osdir.com/ml/lib.qt.general/2008-03/msg00388.html
static {
	QTPLUGIN += qjpeg \
		qgif \
		qtiff
	DEFINES += STATIC
	message("Static build.")
}
