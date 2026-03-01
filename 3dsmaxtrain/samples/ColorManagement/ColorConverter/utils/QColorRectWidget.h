//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <QtWidgets/QFrame>

class QColorRectWidget : public QFrame
{
    Q_OBJECT

public:
    QColorRectWidget(QWidget* parent = nullptr);

    void UpdateColor(const QColor& color);
    void SetDrawColor(bool enable);

protected:
    void paintEvent(QPaintEvent*) override;

private:
    QColor mColor;
    bool mDrawColor = true;
};
