#ifndef SCROLLAREA_H
#define SCROLLAREA_H

#include <QScrollArea>
#include <QScrollBar>

class ScrollArea : public QScrollArea
{
protected:
	void mousePressEvent(QMouseEvent *e) {
		mousePos = e->pos();
	}
	void mouseMoveEvent(QMouseEvent *e) {
		QPoint diff = e->pos() - mousePos;
		mousePos = e->pos();
		verticalScrollBar()->setValue(verticalScrollBar()->value() - diff.y());
		horizontalScrollBar()->setValue(horizontalScrollBar()->value() - diff.x());
	}
private:
	QPoint mousePos;
};

#endif // SCROLLAREA_H
