/* gpitool - Converts images into .GPI format
 * Copyright (c) 2024 Maxim Hoxha
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Tiling settings window
 */

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include "tilingwindow.h"

TilingWindow::TilingWindow(ImageHandler* handler, GPITool* parent) : QDockWidget((QWidget*)parent, Qt::Window)
{
    ihand = handler;
    setWindowTitle("Tiling Settings");
    resize(100, 100);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    enableCheck = new QCheckBox("Enable tiling");
    QLabel* tileSizeLabel = new QLabel("Tile size:");
    QHBoxLayout* xLayout = new QHBoxLayout();
    QHBoxLayout* yLayout = new QHBoxLayout();
    QLabel* xLabel = new QLabel("X");
    tileSizeXBox = new QSpinBox();
    QLabel* yLabel = new QLabel("Y");
    tileSizeYBox = new QSpinBox();
    QLabel* tileOrderLabel = new QLabel("Tile order:");
    rowmajRadio = new QRadioButton("Left-to-right, then top-to-bottom");
    colmajRadio = new QRadioButton("Top-to-bottom, then left-to-right");

    mainLayout->addWidget(enableCheck);
    mainLayout->addWidget(tileSizeLabel);
    xLayout->addWidget(xLabel);
    xLayout->addWidget(tileSizeXBox);
    mainLayout->addLayout(xLayout);
    yLayout->addWidget(yLabel);
    yLayout->addWidget(tileSizeYBox);
    mainLayout->addLayout(yLayout);
    mainLayout->addWidget(tileOrderLabel);
    mainLayout->addWidget(rowmajRadio);
    mainLayout->addWidget(colmajRadio);
    mainLayout->setAlignment(Qt::AlignTop);

    QWidget* mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);

    enableCheck->setChecked(ihand->isTiled);
    tileSizeXBox->setRange(1, ihand->GetEncodedImage()->width);
    tileSizeYBox->setRange(1, ihand->GetEncodedImage()->height);
    tileSizeXBox->setValue(ihand->tileSizeX);
    tileSizeYBox->setValue(ihand->tileSizeY);
    switch (ihand->tileOrdering)
    {
        case ROWMAJOR:
            rowmajRadio->setChecked(true);
            colmajRadio->setChecked(false);
            break;
        case COLUMNMAJOR:
            rowmajRadio->setChecked(false);
            colmajRadio->setChecked(true);
            break;
    }

    connect(enableCheck, &QCheckBox::stateChanged, this, &TilingWindow::OnToggleTiling);
    connect(tileSizeXBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TilingWindow::OnSetTileSizeX);
    connect(tileSizeYBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &TilingWindow::OnSetTileSizeY);
    connect(rowmajRadio, &QRadioButton::toggled, this, &TilingWindow::OnToggleOrder);
    connect(colmajRadio, &QRadioButton::toggled, this, &TilingWindow::OnToggleOrder);
}

void TilingWindow::OnToggleTiling(int state)
{
    switch (state)
    {
        case Qt::Unchecked:
            ihand->isTiled = false;
            break;
        case Qt::PartiallyChecked:
            ihand->isTiled = true;
            break;
        case Qt::Checked:
            ihand->isTiled = true;
            break;
    }
}

void TilingWindow::OnSetTileSizeX(int val)
{
    ihand->tileSizeX = val;
}

void TilingWindow::OnSetTileSizeY(int val)
{
    ihand->tileSizeY = val;
}

void TilingWindow::OnToggleOrder(bool checked)
{
    if (checked)
    {
        if (rowmajRadio->isChecked())
        {
            ihand->tileOrdering = ROWMAJOR;
        }
        else if (colmajRadio->isChecked())
        {
            ihand->tileOrdering = COLUMNMAJOR;
        }
    }
}
