//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "QColorSwatchWidget.h"

#include <QTimer.h>
#include <QScreen.h>
#include <QGuiApplication.h>
#include <QApplication.h>
#include <QEvent.h>
#include <QPainter.h>

// MaxSDK
#include <winutil.h>

namespace
{
	const int space = 15; // Giving the space between the swatch widget and the mouse icon.

	int GetSwatchWidth()
	{
		return MaxSDK::UIScaled(80);
	}

	int GetSwatchHeight()
	{
		return MaxSDK::UIScaled(50);
	}

	QColor ScreenGrabColorAtPoint(const QPoint& pt)
	{
		QColor color(QColorConstants::Black);
		QScreen* screen = QGuiApplication::screenAt(pt);
		if (screen)
		{
			QPoint ptScreenRelative = pt - screen->geometry().topLeft();
			QImage image = screen->grabWindow(0, ptScreenRelative.x(), ptScreenRelative.y(), 1, 1).toImage();

			if (image.width() > 0 && image.height() > 0)
			{
				color = QColor(image.pixel(0, 0));
			}
		}
		return color;
	}
}

QColorSwatchWidget::QColorSwatchWidget(QWidget* parent)
	: QDialog(parent, Qt::Popup)
	, fNumericHeight(0)
{
	QFontMetrics tmp = QFontMetrics(QFont());
	fNumericHeight = 3 * (tmp.lineSpacing()) + 3;

	resize(GetSwatchWidth(), GetSwatchHeight() + fNumericHeight);

	// Disable it by default
	hide();
}

void QColorSwatchWidget::updatePosition()
{
	// Make sure that the swatch widget is never moved outside of the screen.
	QPoint clickPt = QCursor::pos();
	int newX = clickPt.x() + space;
	int newY = clickPt.y() + space;

	if (auto screen = QGuiApplication::screenAt(clickPt))
	{
		QRect screenGeometry = screen->availableGeometry();

		if (newX + width() > screenGeometry.right())
		{
			newX = clickPt.x() - space - width();
		}

		if (newY + height() > screenGeometry.bottom())
		{
			newY = clickPt.y() - space - height();
		}
	}

	move(newX, newY);
	update();
}

void QColorSwatchWidget::paintEvent(QPaintEvent* event)
{
	// Create the painter
	QPainter painter(this);

	QColor color = ScreenGrabColorAtPoint(QCursor::pos());

	// Draw the current color swatch
	const QRect swatchRect(0, 0, width() - 1, GetSwatchHeight());
	painter.fillRect(swatchRect, color);

	// Draw background for numeric read-out
	const QColor numericBg(42, 42, 42);
	const QRect numericRect(0, swatchRect.bottom() + 1, swatchRect.width(), fNumericHeight);
	painter.fillRect(numericRect, numericBg);

	// Draw RGB text of render space color (i.e. color that gets picked)
	const int lineSpacing = painter.fontMetrics().lineSpacing();
	const QRect rRect(1, numericRect.top() + 1, width() - 3, numericRect.height());
	const QRect gRect(rRect.x(), rRect.top() + lineSpacing, rRect.width(), rRect.height() - lineSpacing);
	const QRect bRect(gRect.x(), gRect.top() + lineSpacing, gRect.width(), gRect.height() - lineSpacing);

	auto drawText = [&painter](const QColor& color, const QRect& rect, float value, int iValue) {
		painter.setPen(color);
		painter.drawText(rect,
			Qt::AlignRight | Qt::AlignTop,
			QString("%1 (%2)").arg(value, 0, 'g', 5).arg(iValue, 3, 10, QLatin1Char('0')));
	};

	drawText(QColor(255, 65, 65), rRect, color.redF(), color.red());
	drawText(QColor(50, 255, 50), gRect, color.greenF(), color.green());
	drawText(QColor(37, 130, 255), bRect, color.blueF(), color.blue());

	// Draw outline
	const QRect outline(0, 0, width() - 1, height() - 1);
	painter.setPen(Qt::yellow);
	painter.drawRect(outline);

	// Finish painting
	event->accept();
}
