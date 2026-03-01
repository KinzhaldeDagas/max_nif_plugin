//**************************************************************************/
// Copyright (c) 1998-2022 Autodesk, Inc.
// All rights reserved.
// 
//  Use of this software is subject to the terms of the Autodesk license 
//  agreement provided at the time of installation or download, or which 
//  otherwise accompanies this software in either electronic or hard copy form.
//**************************************************************************/

#pragma once

#include <QtWidgets/QDialog>

class QColorSwatchWidget : public QDialog
{
    Q_OBJECT

public:
    QColorSwatchWidget(QWidget* parent = NULL);

public Q_SLOTS:
    void updatePosition();

protected:
    void paintEvent(QPaintEvent*) override;

private:
    int fNumericHeight;
};
