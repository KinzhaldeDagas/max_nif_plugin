//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <QtWidgets/QPushButton>
#include <QtCore/QPointer>
#include <QtCore/QTimer>

#include "QColorSwatchWidget.h"

class QColorSwatchWidget;

class QColorEyeDropper : public QPushButton
{
    Q_OBJECT

public:
    QColorEyeDropper(QWidget* parent = nullptr);

    bool eventFilter(QObject* object, QEvent* event) override;

    void startPickLoop();

Q_SIGNALS:
    void colorChanged(const QColor&);
    void cancelled();

protected:
    void mouseReleaseEvent(QMouseEvent*) override;
    void pickColor();
    void takeControl();
    void releaseControl();

private:
    bool fPickedColor;
    QScopedPointer<QColorSwatchWidget> fColorSwatchWidget;
    QScopedPointer<QTimer> fQTimer;
    qreal fPopupOpacity;
    QPointer<QWidget> fPopupWindow;
};
