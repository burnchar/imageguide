#include "ScrollArea.h"

// Thanks to Matthias Ettrich for providing an example of mouse panning.
// http://lists.trolltech.com/qt-interest/2005-12/thread00993-0.html
class ScrollArea : public QScrollArea
{
protected:
	void mousePressEvent(QMouseEvent *e) {
		mousePos = e->pos();
	}
	void mouseMoveEvent(QMouseEvent *e) {
		QPoint diff = e->pos() - mousePos;
		mousePos = e->pos();
		verticalScrollBar()->setValue(verticalScrollBar()->value() + diff.y());
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() + diff.x());
	}
private:
	QPoint mousePos;
};
