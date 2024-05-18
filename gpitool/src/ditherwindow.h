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

#include <glibmm.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/window.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/builder.h>
#include "imagehandler.h"
#include "mainwindow.h"

class DitherWindow : public Gtk::Window
{
public:
    DitherWindow(ImageHandler* handler, MainWindow* mainwind);
    virtual ~DitherWindow();

protected:
    Glib::RefPtr<Gtk::Builder> builderRef;
    Glib::RefPtr<Gtk::Adjustment> lumDither;
    Glib::RefPtr<Gtk::Adjustment> satDither;
    Glib::RefPtr<Gtk::Adjustment> hueDither;
    Glib::RefPtr<Gtk::Adjustment> lumDiffusion;
    Glib::RefPtr<Gtk::Adjustment> chromDiffusion;
    Glib::RefPtr<Gtk::Adjustment> lumRandom;
    Glib::RefPtr<Gtk::Adjustment> chromRandom;
    Glib::RefPtr<Gtk::Adjustment> chromBias;
    Glib::RefPtr<Gtk::Adjustment> preBright;
    Glib::RefPtr<Gtk::Adjustment> preContrast;
    Glib::RefPtr<Gtk::Adjustment> postBright;
    Glib::RefPtr<Gtk::Adjustment> postContrast;
    Gtk::ComboBoxText* ditherMethodBox;
    Gtk::CheckButton* boustroCheck;

    void OnSetDitherMethod();
    void OnSetLuminosityDither();
    void OnSetSaturationDither();
    void OnSetHueDither();
    void OnSetLuminosityDiffusion();
    void OnSetChromaDiffusion();
    void OnSetLuminosityRandomisation();
    void OnSetChromaRandomisation();
    void OnSetChromaBias();
    void OnSetPreBrightness();
    void OnSetPreContrast();
    void OnSetPostBrightness();
    void OnSetPostContrast();
    void OnToggleBoustrophedon();

private:
    MainWindow* mwin;
    ImageHandler* ihand;
};
