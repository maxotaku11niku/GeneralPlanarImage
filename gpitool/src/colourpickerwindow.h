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

#pragma once

#include <QDockWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QGridLayout>
#include <QPushButton>
#include "sliderandspinbox.h"
#include "imagehandler.h"
#include "gpitool.h"

class GPITool;

class ColourPickerWindow : public QDockWidget
{
    Q_OBJECT

public:
    explicit ColourPickerWindow(ImageHandler* handler, GPITool* parent);

    void UpdateAfterOpenFile();

protected:
    QCheckBox* planeChecks[9];
    QRadioButton* bpc4Radio;
    QRadioButton* bpc8Radio;
    SliderAndSpinBox* transThresholdControl;
    QGridLayout* colourGrid;
    QPushButton* colourButtons[256];
    SliderAndDoubleSpinBox* adaptivePreBrightControl;
    SliderAndDoubleSpinBox* adaptivePreContrastControl;
    SliderAndDoubleSpinBox* adaptiveChromaBiasControl;
    QPushButton* findBestPaletteButton;
    QPushButton* loadPaletteButton;
    QPushButton* savePaletteButton;

protected slots:
    void ReorganisePaletteGrid(int numColourPlanes);
    void SetPaletteGridColours();
    void OnTogglePlane(int state);
    void OnToggleBitDepth(bool checked);
    void OnSetTransparencyThreshold(int val);
    void OnSetColour();
    void OnSetAdaptivePreBrightness(double val);
    void OnSetAdaptivePreContrast(double val);
    void OnSetAdaptiveChromaBias(double val);
    void OnRequestBestPalette();
    void OnLoadPaletteFromFile();
    void OnSavePaletteToFile();

private:
    ImageHandler* ihand;
    GPITool* mwin;
};
