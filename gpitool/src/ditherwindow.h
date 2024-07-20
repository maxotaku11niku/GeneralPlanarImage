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

#pragma once

#include <QDockWidget>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include "sliderandspinbox.h"
#include "imagehandler.h"
#include "gpitool.h"

class GPITool;

class DitherWindow : public QDockWidget
{
    Q_OBJECT

public:
    explicit DitherWindow(ImageHandler* handler, GPITool* parent);

protected:
    QComboBox* ditherMethodBox;
    SliderAndDoubleSpinBox* lumDitherControl;
    SliderAndDoubleSpinBox* satDitherControl;
    SliderAndDoubleSpinBox* hueDitherControl;
    SliderAndDoubleSpinBox* lumDiffusionControl;
    SliderAndDoubleSpinBox* chromDiffusionControl;
    SliderAndDoubleSpinBox* lumRandomControl;
    SliderAndDoubleSpinBox* chromRandomControl;
    SliderAndDoubleSpinBox* chromBiasControl;
    SliderAndDoubleSpinBox* preBrightControl;
    SliderAndDoubleSpinBox* preContrastControl;
    SliderAndDoubleSpinBox* postBrightControl;
    SliderAndDoubleSpinBox* postContrastControl;
    QCheckBox* boustroCheck;

private slots:
    void OnSetDitherMethod(int index);
    void OnSetLuminosityDither(double val);
    void OnSetSaturationDither(double val);
    void OnSetHueDither(double val);
    void OnSetLuminosityDiffusion(double val);
    void OnSetChromaDiffusion(double val);
    void OnSetLuminosityRandomisation(double val);
    void OnSetChromaRandomisation(double val);
    void OnSetChromaBias(double val);
    void OnSetPreBrightness(double val);
    void OnSetPreContrast(double val);
    void OnSetPostBrightness(double val);
    void OnSetPostContrast(double val);
    void OnToggleBoustrophedon(int state);

private:
    ImageHandler* ihand;
    GPITool* mwin;
};
