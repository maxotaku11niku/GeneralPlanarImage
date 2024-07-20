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

#include <QHBoxLayout>
#include "sliderandspinbox.h"



SliderAndSpinBox::SliderAndSpinBox(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* mainLayout = new QHBoxLayout();
    slider = new QSlider(Qt::Horizontal);
    spinBox = new QSpinBox();
    mainLayout->addWidget(slider);
    mainLayout->addWidget(spinBox);
    setLayout(mainLayout);
    connect(slider, &QSlider::valueChanged, this, &SliderAndSpinBox::OnSliderValueChanged);
    connect(spinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &SliderAndSpinBox::OnSpinBoxValueChanged);
}

void SliderAndSpinBox::SetMinimum(int min)
{
    slider->setMinimum(min);
    spinBox->setMinimum(min);
}

void SliderAndSpinBox::SetMaximum(int max)
{
    slider->setMaximum(max);
    spinBox->setMaximum(max);
}

void SliderAndSpinBox::SetRange(int min, int max)
{
    slider->setRange(min, max);
    spinBox->setRange(min, max);
}

void SliderAndSpinBox::SetValue(int val)
{
    slider->setValue(val);
    spinBox->setValue(val);
}

void SliderAndSpinBox::OnSliderValueChanged(int newValue)
{
    spinBox->setValue(newValue);
    emit ValueChanged(newValue);
}

void SliderAndSpinBox::OnSpinBoxValueChanged(int newValue)
{
    slider->setValue(newValue);
    emit ValueChanged(newValue);
}



SliderAndDoubleSpinBox::SliderAndDoubleSpinBox(QWidget* parent) : QWidget(parent)
{
    QHBoxLayout* mainLayout = new QHBoxLayout();
    slider = new QSlider(Qt::Horizontal);
    slider->setRange(0, 10000);
    spinBox = new QDoubleSpinBox();
    mainLayout->addWidget(slider);
    mainLayout->addWidget(spinBox);
    setLayout(mainLayout);
    connect(slider, &QSlider::valueChanged, this, &SliderAndDoubleSpinBox::OnSliderValueChanged);
    connect(spinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &SliderAndDoubleSpinBox::OnSpinBoxValueChanged);
}

void SliderAndDoubleSpinBox::SetMinimum(double min)
{
    spinBox->setMinimum(min);
}

void SliderAndDoubleSpinBox::SetMaximum(double max)
{
    spinBox->setMaximum(max);
}

void SliderAndDoubleSpinBox::SetRange(double min, double max)
{
    spinBox->setRange(min, max);
}

void SliderAndDoubleSpinBox::SetSingleStep(double step)
{
    spinBox->setSingleStep(step);
}

void SliderAndDoubleSpinBox::SetValue(double val)
{
    spinBox->setValue(val);
    double dmin = spinBox->minimum();
    double dmax = spinBox->maximum();
    double nval = (val - dmin) / (dmax - dmin);
    nval *= 10000.0;
    slider->setValue((int)nval);
}

void SliderAndDoubleSpinBox::OnSliderValueChanged(int newValue)
{
    double sval = (double)newValue;
    sval /= 10000.0;
    double dmin = spinBox->minimum();
    double dmax = spinBox->maximum();
    double actualVal = dmin + ((dmax - dmin) * sval);
    spinBox->setValue(actualVal);
    emit ValueChanged(actualVal);
}

void SliderAndDoubleSpinBox::OnSpinBoxValueChanged(double newValue)
{
    double dmin = spinBox->minimum();
    double dmax = spinBox->maximum();
    double nval = (newValue - dmin) / (dmax - dmin);
    nval *= 10000.0;
    slider->setValue((int)nval);
    emit ValueChanged(newValue);
}
