//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#include "QColorEyeDropper.h"
#include "QColorSwatchWidget.h"

#include <QTimer.h>
#include <QScreen.h>
#include <QGuiApplication.h>
#include <QApplication.h>
#include <QEvent.h>

// MaxSDK
#include <MaxIcon.h>

static const std::chrono::milliseconds updateTimeout(16); // Update the swatch position and color at roughly 60 fps

namespace
{
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

QColorEyeDropper::QColorEyeDropper(QWidget* parent)
	: QPushButton(parent)
	, fColorSwatchWidget(new QColorSwatchWidget())
	, fQTimer(new QTimer(this))
	, fPickedColor(false)
	, fPopupOpacity(1.0)
{
	setIcon(MaxSDK::LoadMaxMultiResIcon("Common/EyeDropper"));

	setGeometry(QRect(0, 0, 30, 30));

	connect(fQTimer.data(), SIGNAL(timeout()), fColorSwatchWidget.data(), SLOT(updatePosition()), Qt::DirectConnection);
}

void QColorEyeDropper::mouseReleaseEvent(QMouseEvent*)
{
	if (isDown())
	{
		startPickLoop();
	}
	else
	{
		releaseMouse();
	}
}

void QColorEyeDropper::startPickLoop()
{
	// Forbids other widgets to receive mouse events
	//   till another mouse release event.
	grabMouse();

	takeControl();
}

void QColorEyeDropper::pickColor()
{
	fPickedColor = true;

	QColor color = ScreenGrabColorAtPoint(QCursor::pos());

	Q_EMIT colorChanged(color);
}

void QColorEyeDropper::takeControl()
{
	fPopupWindow = QApplication::activePopupWidget();
	if (fPopupWindow)
	{
		fPopupOpacity = fPopupWindow->windowOpacity();
		fPopupWindow->setWindowOpacity(0.0);

		QPoint topLeft(0, 0);

		if (QScreen* screen = fPopupWindow->screen()) {
			topLeft = screen->virtualGeometry().topLeft();
		}

		fPopupWindow->move(topLeft.x() - 500, topLeft.y() - 500);
	}

	QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));

	// Enable the popup widget for getting all the events and previewing color.
	fColorSwatchWidget->updatePosition();
	fColorSwatchWidget->show();
	fColorSwatchWidget->raise();

	qApp->installEventFilter(this);

	fQTimer->start(updateTimeout);

	fPickedColor = false;
}

void QColorEyeDropper::releaseControl()
{
	QApplication::restoreOverrideCursor();

	// Disable the popup widget again.
	fColorSwatchWidget->hide();

	qApp->removeEventFilter(this);

	setDown(false);

	fQTimer->stop();

	if (!fPickedColor) {
		cancelled();
	}

	if (fPopupWindow) {
		fPopupWindow->hide();
		fPopupWindow->setWindowOpacity(fPopupOpacity);
	}
}

bool QColorEyeDropper::eventFilter(QObject* object, QEvent* event)
{
	if (event->type() == QEvent::MouseMove)
	{
		return true;
	}
	else if (event->type() == QEvent::MouseButtonPress)
	{
		// Only handle the event when the receiver is the swatch widget.
		QColorSwatchWidget* swatchWidget = dynamic_cast<QColorSwatchWidget*>(object);
		if (swatchWidget == nullptr)
		{
			return false;
		}

		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
		if (mouseEvent && mouseEvent->button() == Qt::LeftButton)
		{
			pickColor();
		}

		releaseControl();

		// Do not call releaseMouse() to avoid the selection

		return true;
	}
	else if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent && keyEvent->key() == Qt::Key_Escape)
		{
			releaseControl();
			releaseMouse();
		}
		return true;
	}
	else if (event->type() == QEvent::ApplicationDeactivated
		|| event->type() == QEvent::Wheel
		|| event->type() == QEvent::MouseButtonRelease)
	{
		releaseControl();
		releaseMouse();

		return true;
	}
	return false;
}
