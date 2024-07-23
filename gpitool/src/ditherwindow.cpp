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
 * Dither settings window
 */

#include <QFormLayout>
#include "ditherwindow.h"

DitherWindow::DitherWindow(ImageHandler* handler, GPITool* parent) : QDockWidget((QWidget*)parent, Qt::Window)
{
    ihand = handler;
    mwin = parent;
    setWindowTitle("Dither Settings");
    resize(400, 100);

    QFormLayout* mainLayout = new QFormLayout();
    ditherMethodBox = new QComboBox();
    lumDitherControl = new SliderAndDoubleSpinBox();
    satDitherControl = new SliderAndDoubleSpinBox();
    hueDitherControl = new SliderAndDoubleSpinBox();
    lumDiffusionControl = new SliderAndDoubleSpinBox();
    chromDiffusionControl = new SliderAndDoubleSpinBox();
    lumRandomControl = new SliderAndDoubleSpinBox();
    chromRandomControl = new SliderAndDoubleSpinBox();
    chromBiasControl = new SliderAndDoubleSpinBox();
    preBrightControl = new SliderAndDoubleSpinBox();
    preContrastControl = new SliderAndDoubleSpinBox();
    postBrightControl = new SliderAndDoubleSpinBox();
    postContrastControl = new SliderAndDoubleSpinBox();
    boustroCheck = new QCheckBox();

    mainLayout->addRow("Dither Method", ditherMethodBox);
    mainLayout->addRow("Luminosity Dither", lumDitherControl);
    mainLayout->addRow("Saturation Dither", satDitherControl);
    mainLayout->addRow("Hue Dither", hueDitherControl);
    mainLayout->addRow("Luminosity Error Diffusion", lumDiffusionControl);
    mainLayout->addRow("Chroma Error Diffusion", chromDiffusionControl);
    mainLayout->addRow("Luminosity Diffusion Randomisation", lumRandomControl);
    mainLayout->addRow("Chroma Diffusion Randomisation", chromRandomControl);
    mainLayout->addRow("Chroma Bias", chromBiasControl);
    mainLayout->addRow("Pre-brightness", preBrightControl);
    mainLayout->addRow("Pre-contrast", preContrastControl);
    mainLayout->addRow("Post-brightness", postBrightControl);
    mainLayout->addRow("Post-contrast", postContrastControl);
    mainLayout->addRow("Boustrophedon Scanning", boustroCheck);
    mainLayout->setAlignment(Qt::AlignTop);

    QWidget* mainWidget = new QWidget();
    mainWidget->setLayout(mainLayout);
    setWidget(mainWidget);

    ditherMethodBox->addItems({ "None", "Bayer 2x2", "Bayer 4x4", "Bayer 8x8", "Bayer 16x16", "Void and cluster 16x16", "Floyd-Steinberg", "False Floyd-Steinberg", "Jarvis-Judice-Ninke", "Stucki", "Burkes", "Sierra", "Sierra 2-Row", "Filter Lite", "Atkinson" });
    ditherMethodBox->setCurrentIndex(ihand->ditherMethod);
    lumDitherControl->SetRange(0.0, 1.0);
    lumDitherControl->SetSingleStep(0.001);
    lumDitherControl->SetDecimals(3);
    lumDitherControl->SetValue(ihand->luminosityDither);
    satDitherControl->SetRange(0.0, 1.0);
    satDitherControl->SetSingleStep(0.001);
    satDitherControl->SetDecimals(3);
    satDitherControl->SetValue(ihand->saturationDither);
    hueDitherControl->SetRange(0.0, 10.0);
    hueDitherControl->SetSingleStep(0.01);
    hueDitherControl->SetDecimals(2);
    hueDitherControl->SetValue(ihand->hueDither);
    lumDiffusionControl->SetRange(0.0, 1.0);
    lumDiffusionControl->SetSingleStep(0.001);
    lumDiffusionControl->SetDecimals(3);
    lumDiffusionControl->SetValue(ihand->luminosityDiffusion);
    chromDiffusionControl->SetRange(0.0, 1.0);
    chromDiffusionControl->SetSingleStep(0.001);
    chromDiffusionControl->SetDecimals(3);
    chromDiffusionControl->SetValue(ihand->chromaDiffusion);
    lumRandomControl->SetRange(0.0, 0.2);
    lumRandomControl->SetSingleStep(0.0002);
    lumRandomControl->SetDecimals(4);
    lumRandomControl->SetValue(ihand->luminosityRandomisation);
    chromRandomControl->SetRange(0.0, 0.2);
    chromRandomControl->SetSingleStep(0.0002);
    chromRandomControl->SetDecimals(4);
    chromRandomControl->SetValue(ihand->chromaRandomisation);
    chromBiasControl->SetRange(0.5, 2.0);
    chromBiasControl->SetSingleStep(0.001);
    chromBiasControl->SetDecimals(3);
    chromBiasControl->SetValue(ihand->chromaBias);
    preBrightControl->SetRange(-1.0, 1.0);
    preBrightControl->SetSingleStep(0.001);
    preBrightControl->SetDecimals(3);
    preBrightControl->SetValue(ihand->preBrightness);
    preContrastControl->SetRange(-1.0, 1.0);
    preContrastControl->SetSingleStep(0.001);
    preContrastControl->SetDecimals(3);
    preContrastControl->SetValue(ihand->preContrast);
    postBrightControl->SetRange(-1.0, 1.0);
    postBrightControl->SetSingleStep(0.001);
    postBrightControl->SetDecimals(3);
    postBrightControl->SetValue(ihand->postBrightness);
    postContrastControl->SetRange(-1.0, 1.0);
    postContrastControl->SetSingleStep(0.001);
    postContrastControl->SetDecimals(3);
    postContrastControl->SetValue(ihand->postContrast);
    boustroCheck->setChecked(ihand->boustrophedon);

    connect(ditherMethodBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DitherWindow::OnSetDitherMethod);
    connect(lumDitherControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetLuminosityDither);
    connect(satDitherControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetSaturationDither);
    connect(hueDitherControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetHueDither);
    connect(lumDiffusionControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetLuminosityDiffusion);
    connect(chromDiffusionControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetChromaDiffusion);
    connect(lumRandomControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetLuminosityRandomisation);
    connect(chromRandomControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetChromaRandomisation);
    connect(chromBiasControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetChromaBias);
    connect(preBrightControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetPreBrightness);
    connect(preContrastControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetPreContrast);
    connect(postBrightControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetPostBrightness);
    connect(postContrastControl, &SliderAndDoubleSpinBox::ValueChanged, this, &DitherWindow::OnSetPostContrast);
    connect(boustroCheck, &QCheckBox::stateChanged, this, &DitherWindow::OnToggleBoustrophedon);
}

void DitherWindow::OnSetDitherMethod(int index)
{
    if (index >= 0) ihand->ditherMethod = index;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetLuminosityDither(double val)
{
    ihand->luminosityDither = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetSaturationDither(double val)
{
    ihand->saturationDither = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetHueDither(double val)
{
    ihand->hueDither = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetLuminosityDiffusion(double val)
{
    ihand->luminosityDiffusion = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetChromaDiffusion(double val)
{
    ihand->chromaDiffusion = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetLuminosityRandomisation(double val)
{
    ihand->luminosityRandomisation = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetChromaRandomisation(double val)
{
    ihand->chromaRandomisation = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetChromaBias(double val)
{
    ihand->chromaBias = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetPreBrightness(double val)
{
    ihand->preBrightness = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetPreContrast(double val)
{
    ihand->preContrast = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetPostBrightness(double val)
{
    ihand->postBrightness = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnSetPostContrast(double val)
{
    ihand->postContrast = val;
    mwin->UpdateImageThumbnailAfterDither();
}

void DitherWindow::OnToggleBoustrophedon(int state)
{
    switch (state)
    {
        case Qt::Unchecked:
            ihand->boustrophedon = false;
            break;
        case Qt::PartiallyChecked:
            ihand->boustrophedon = true;
            break;
        case Qt::Checked:
            ihand->boustrophedon = true;
            break;
    }
    mwin->UpdateImageThumbnailAfterDither();
}
