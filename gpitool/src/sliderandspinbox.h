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
 * Slider and spin box combo classes, for convenience
 */

#pragma once

#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>

class SliderAndSpinBox : public QWidget
{
    Q_OBJECT

public:
    explicit SliderAndSpinBox(QWidget* parent = nullptr);

    void SetMinimum(int min);
    void SetMaximum(int max);
    void SetRange(int min, int max);
    void SetValue(int val);

protected slots:
    void OnSliderValueChanged(int newValue);
    void OnSpinBoxValueChanged(int newValue);

signals:
    void ValueChanged(int newValue);

protected:
    QSlider* slider;
    QSpinBox* spinBox;
};

class SliderAndDoubleSpinBox : public QWidget
{
    Q_OBJECT

public:
    explicit SliderAndDoubleSpinBox(QWidget* parent = nullptr);

    void SetMinimum(double min);
    void SetMaximum(double max);
    void SetRange(double min, double max);
    void SetSingleStep(double step);
    void SetDecimals(int prec);
    void SetValue(double val);

protected slots:
    void OnSliderValueChanged(int newValue);
    void OnSpinBoxValueChanged(double newValue);

signals:
    void ValueChanged(double newValue);

protected:
    QSlider* slider;
    QDoubleSpinBox* spinBox;
};
