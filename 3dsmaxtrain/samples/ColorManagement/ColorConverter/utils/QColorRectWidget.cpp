//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "QColorRectWidget.h"

#include <QEvent.h>
#include <QPainter.h>


QColorRectWidget::QColorRectWidget(QWidget* parent)
	: QFrame(parent)
	, mColor(Qt::black)
{
	setFrameStyle(QFrame::Box);
}

void QColorRectWidget::SetDrawColor(bool enable)
{
	mDrawColor = enable;
	repaint();
}

void QColorRectWidget::UpdateColor(const QColor& color)
{
	mColor = color;
	repaint();
}

void QColorRectWidget::paintEvent(QPaintEvent* event)
{
	// Draw the border
	__super::paintEvent(event);

	// Fill color
	if (mDrawColor)
	{
		QPainter painter(this);

		QRect rect = event->rect();
		static const qreal sAdjust = 1.5;
		rect = rect.adjusted(sAdjust, sAdjust, -sAdjust, -sAdjust);
		painter.fillRect(rect, mColor);

		// Finish painting
		event->accept();
	}
	else
	{

		QPainter painter(this);
		
		QRect rect = event->rect();
		painter.setPen(QPen(Qt::black));
		painter.setRenderHint(QPainter::Antialiasing, true);
		painter.drawLine(rect.topLeft(), rect.bottomRight());
		painter.drawLine(rect.topRight(), rect.bottomLeft());
	}
}
