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

#include <glibmm.h>
#include <gtkmm/adjustment.h>
#include <gtkmm/window.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/grid.h>
#include <gtkmm/colorbutton.h>
#include <gtkmm/builder.h>
#include "imagehandler.h"
#include "mainwindow.h"

class ColourPickerWindow : public Gtk::Window
{
public:
    ColourPickerWindow(ImageHandler* handler, MainWindow* mainwind);
    virtual ~ColourPickerWindow();

protected:
    Glib::RefPtr<Gtk::Builder> builderRef;
    Glib::RefPtr<Gtk::Adjustment> transThreshold;
    Gtk::CheckButton* planeChecks[9];
    Gtk::RadioButton* bpc4Radio;
    Gtk::RadioButton* bpc8Radio;
    Gtk::Grid* colourGrid;
    Gtk::ColorButton* colourButtons[256];
    Gtk::Button* findBestPaletteButton;
    Gtk::Button* loadPaletteButton;
    Gtk::Button* savePaletteButton;

    void ReorganisePaletteGrid(int numColourPlanes);
    void SetPaletteGridColours();
    void OnTogglePlane(int planeNum);
    void OnToggleBitDepth();
    void OnSetTransparencyThreshold();
    void OnSetColour(int index);
    void OnRequestBestPalette();
    void OnLoadPaletteFromFile();
    void OnSavePaletteToFile();

private:
    MainWindow* mwin;
    ImageHandler* ihand;
};
