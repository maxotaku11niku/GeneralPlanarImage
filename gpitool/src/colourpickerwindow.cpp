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
 * Colour picking window
 */

#include <QFileDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QLabel>
#include "colourpickerwindow.h"

ColourPickerWindow::ColourPickerWindow(ImageHandler* handler, GPITool* parent) : QDockWidget((QWidget*)parent, Qt::Window)
{
    ihand = handler;
    mwin = parent;
    setWindowTitle("Palette Settings");
    resize(100, 100);

    QVBoxLayout* mainLayout = new QVBoxLayout();
    QLabel* includePlaneLabel = new QLabel("Included planes:");
    QHBoxLayout* planeLayout0 = new QHBoxLayout();
    QHBoxLayout* planeLayout1 = new QHBoxLayout();
    planeChecks[0] = new QCheckBox("Plane 0");
    planeChecks[1] = new QCheckBox("Plane 1");
    planeChecks[2] = new QCheckBox("Plane 2");
    planeChecks[3] = new QCheckBox("Plane 3");
    planeChecks[4] = new QCheckBox("Plane 4");
    planeChecks[5] = new QCheckBox("Plane 5");
    planeChecks[6] = new QCheckBox("Plane 6");
    planeChecks[7] = new QCheckBox("Plane 7");
    planeChecks[8] = new QCheckBox("Mask plane (for transparency)");
    QLabel* colourDepthLabel = new QLabel("Colour depth:");
    bpc4Radio = new QRadioButton("4 bits per channel");
    bpc8Radio = new QRadioButton("8 bits per channel");
    QLabel* transThresholdLabel = new QLabel("Transparency threshold:");
    transThresholdControl = new SliderAndSpinBox();
    QLabel* colourGridLabel = new QLabel("Colours:");
    colourGrid = new QGridLayout();
    for (int i = 0; i < 256; i++)
    {
        char cbuf[4];
        sprintf(cbuf, "%i", i);
        colourButtons[i] = new QPushButton(cbuf);
        colourButtons[i]->setMinimumSize(32, 20);
        colourGrid->addWidget(colourButtons[i], i/16, i % 16);
    }
    findBestPaletteButton = new QPushButton("&Find best palette...");
    loadPaletteButton = new QPushButton("&Load palette from file...");
    savePaletteButton = new QPushButton("&Save palette to file...");

    mainLayout->addWidget(includePlaneLabel);
    planeLayout0->addWidget(planeChecks[0]);
    planeLayout0->addWidget(planeChecks[1]);
    planeLayout0->addWidget(planeChecks[2]);
    planeLayout0->addWidget(planeChecks[3]);
    mainLayout->addLayout(planeLayout0);
    planeLayout1->addWidget(planeChecks[4]);
    planeLayout1->addWidget(planeChecks[5]);
    planeLayout1->addWidget(planeChecks[6]);
    planeLayout1->addWidget(planeChecks[7]);
    mainLayout->addLayout(planeLayout1);
    mainLayout->addWidget(planeChecks[8]);
    mainLayout->addWidget(colourDepthLabel);
    mainLayout->addWidget(bpc4Radio);
    mainLayout->addWidget(bpc8Radio);
    mainLayout->addWidget(transThresholdLabel);
    mainLayout->addWidget(transThresholdControl);
    mainLayout->addWidget(colourGridLabel);
    mainLayout->addLayout(colourGrid);
    mainLayout->addWidget(findBestPaletteButton);
    mainLayout->addWidget(loadPaletteButton);
    mainLayout->addWidget(savePaletteButton);
    mainLayout->setAlignment(Qt::AlignTop);

    QWidget* mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);

    int pMask = ihand->GetPlaneMask();
    int checkBit = 1;
    for (int i = 0; i < 9; i++)
    {
        if (pMask & checkBit) planeChecks[i]->setChecked(true);
        else planeChecks[i]->setChecked(false);
        checkBit <<= 1;
    }
    bpc4Radio->setChecked(!ihand->is8BitColour);
    bpc8Radio->setChecked(ihand->is8BitColour);
    transThresholdControl->SetRange(0, 255);
    transThresholdControl->SetValue(ihand->transparencyThreshold);

    for (int i = 0; i < 9; i++)
    {
        connect(planeChecks[i], &QCheckBox::stateChanged, this, &ColourPickerWindow::OnTogglePlane);
    }
    connect(bpc4Radio, &QRadioButton::toggled, this, &ColourPickerWindow::OnToggleBitDepth);
    connect(bpc8Radio, &QRadioButton::toggled, this, &ColourPickerWindow::OnToggleBitDepth);
    connect(transThresholdControl, &SliderAndSpinBox::ValueChanged, this, &ColourPickerWindow::OnSetTransparencyThreshold);
    connect(findBestPaletteButton, &QPushButton::clicked, this, &ColourPickerWindow::OnRequestBestPalette);
    connect(loadPaletteButton, &QPushButton::clicked, this, &ColourPickerWindow::OnLoadPaletteFromFile);
    connect(savePaletteButton, &QPushButton::clicked, this, &ColourPickerWindow::OnSavePaletteToFile);

    for (int i = 0; i < 256; i++)
    {
        ColourRGBA8 col = ihand->GetCurrentPalette()[i];
        colourButtons[i]->setPalette(QPalette(QColor(col.R, col.G, col.B, col.A)));
        connect(colourButtons[i], &QPushButton::clicked, this, &ColourPickerWindow::OnSetColour);
    }

    ReorganisePaletteGrid(ihand->GetNumColourPlanes());
}

void ColourPickerWindow::UpdateAfterOpenFile()
{
    int pMask = ihand->GetPlaneMask();
    int checkBit = 1;
    for (int i = 0; i < 9; i++)
    {
        if (pMask & checkBit) planeChecks[i]->setChecked(true);
        else planeChecks[i]->setChecked(false);
        checkBit <<= 1;
    }
    bpc4Radio->setChecked(!ihand->is8BitColour);
    bpc8Radio->setChecked(ihand->is8BitColour);
    transThresholdControl->SetValue(ihand->transparencyThreshold);
    ReorganisePaletteGrid(ihand->GetNumColourPlanes());
    SetPaletteGridColours();
}


void ColourPickerWindow::ReorganisePaletteGrid(int numColourPlanes)
{
    for (int i = 0; i < 256; i++)
    {
        colourButtons[i]->hide();
    }
    switch (numColourPlanes)
    {
        case 1:
            colourGrid->addWidget(colourButtons[0], 0, 0);
            colourButtons[0]->show();
            colourGrid->addWidget(colourButtons[1], 0, 1);
            colourButtons[1]->show();
            break;
        case 2:
            colourGrid->addWidget(colourButtons[0], 0, 0);
            colourButtons[0]->show();
            colourGrid->addWidget(colourButtons[1], 0, 1);
            colourButtons[1]->show();
            colourGrid->addWidget(colourButtons[2], 1, 0);
            colourButtons[2]->show();
            colourGrid->addWidget(colourButtons[3], 1, 1);
            colourButtons[3]->show();
            break;
        case 3:
            for (int i = 0; i < 8; i++)
            {
                colourGrid->addWidget(colourButtons[i], i / 4, i % 4);
                colourButtons[i]->show();
            }
            break;
        case 4:
            for (int i = 0; i < 16; i++)
            {
                colourGrid->addWidget(colourButtons[i], i / 4, i % 4);
                colourButtons[i]->show();
            }
            break;
        case 5:
            for (int i = 0; i < 32; i++)
            {
                colourGrid->addWidget(colourButtons[i], i / 8, i % 8);
                colourButtons[i]->show();
            }
            break;
        case 6:
            for (int i = 0; i < 64; i++)
            {
                colourGrid->addWidget(colourButtons[i], i / 8, i % 8);
                colourButtons[i]->show();
            }
            break;
        case 7:
            for (int i = 0; i < 128; i++)
            {
                colourGrid->addWidget(colourButtons[i], i / 16, i % 16);
                colourButtons[i]->show();
            }
            break;
        case 8:
            for (int i = 0; i < 256; i++)
            {
                colourGrid->addWidget(colourButtons[i], i / 16, i % 16);
                colourButtons[i]->show();
            }
            break;
    }
}

void ColourPickerWindow::SetPaletteGridColours()
{
    ColourRGBA8* pal = ihand->GetCurrentPalette();
    for (int i = 0; i < ihand->GetNumColours(); i++)
    {
        ColourRGBA8 palcol = pal[i];
        colourButtons[i]->setPalette(QPalette(QColor(palcol.R, palcol.G, palcol.B, palcol.A)));
    }
}


void ColourPickerWindow::OnTogglePlane(int state)
{
    int planeNum = -1;
    for (int i = 0; i < 9; i++)
    {
        if (sender() == planeChecks[i]) //hehe
        {
            planeNum = i;
            break;
        }
    }
    switch (state)
    {
        case Qt::Unchecked:
            ihand->RemovePlane(planeNum);
            break;
        case Qt::PartiallyChecked:
            ihand->AddPlane(planeNum);
            break;
        case Qt::Checked:
            ihand->AddPlane(planeNum);
            break;
    }
    ReorganisePaletteGrid(ihand->GetNumColourPlanes());
    mwin->UpdateImageThumbnailAfterDither();
}

void ColourPickerWindow::OnToggleBitDepth(bool checked)
{
    if (checked)
    {
        if (bpc4Radio->isChecked())
        {
            ihand->is8BitColour = false;
        }
        else if (bpc8Radio->isChecked())
        {
            ihand->is8BitColour = true;
        }
    }
    mwin->UpdateImageThumbnailAfterDither();
}

void ColourPickerWindow::OnSetColour()
{
    int index = -1;
    for (int i = 0; i < 256; i++)
    {
        if (sender() == colourButtons[i])
        {
            index = i;
            break;
        }
    }
    if (index < 0) return;
    ColourRGBA8* pal = ihand->GetCurrentPalette();
    ColourRGBA8 palcol = pal[index];
    QColor retCol = QColorDialog::getColor(QColor(palcol.R, palcol.G, palcol.B, palcol.A), this, "Select Palette Colour");
    if (!retCol.isValid()) return;
    int oR, oG, oB;
    if (ihand->is8BitColour)
    {
        oR = (int)((retCol.redF() * 255.0) + 0.5);
        oG = (int)((retCol.greenF() * 255.0) + 0.5);
        oB = (int)((retCol.blueF() * 255.0) + 0.5);
        if (oR > 0xFF) oR = 0xFF; else if (oR < 0) oR = 0;
        if (oG > 0xFF) oG = 0xFF; else if (oG < 0) oG = 0;
        if (oB > 0xFF) oB = 0xFF; else if (oB < 0) oB = 0;
    }
    else
    {
        oR = (int)((retCol.redF() * 15.0) + 0.5);
        oG = (int)((retCol.greenF() * 15.0) + 0.5);
        oB = (int)((retCol.blueF() * 15.0) + 0.5);
        if (oR > 0xF) oR = 0xF; else if (oR < 0) oR = 0;
        if (oG > 0xF) oG = 0xF; else if (oG < 0) oG = 0;
        if (oB > 0xF) oB = 0xF; else if (oB < 0) oB = 0;
        oR *= 0x11; oG *= 0x11; oB *= 0x11;
    }
    ColourRGBA8 outcol = { (unsigned char)oR, (unsigned char)oG, (unsigned char)oB, 0xFF };
    ihand->SetPaletteColour(index, outcol);
    colourButtons[index]->setPalette(QPalette(QColor(outcol.R, outcol.G, outcol.B, outcol.A)));
    mwin->UpdateImageThumbnailAfterDither();
}

void ColourPickerWindow::OnSetTransparencyThreshold(int val)
{
    ihand->transparencyThreshold = val;
    mwin->UpdateImageThumbnailAfterDither();
}


void ColourPickerWindow::OnRequestBestPalette()
{
    mwin->UpdateImageThumbnailAfterFindColours();
    SetPaletteGridColours();
}

void ColourPickerWindow::OnLoadPaletteFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open Palette File", nullptr, "Text files (*.txt)");

    if (!fileName.isNull())
    {
        if (ihand->LoadPaletteFile(fileName.toUtf8().constData()))
        {
            mwin->UpdateImageThumbnailAfterDither();
            SetPaletteGridColours();
        }
    }
}

void ColourPickerWindow::OnSavePaletteToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save to Palette File", nullptr, "Text files (*.txt)");

    if (!fileName.isNull())
    {
        ihand->SavePaletteFile(fileName.toUtf8().constData());
    }
}
