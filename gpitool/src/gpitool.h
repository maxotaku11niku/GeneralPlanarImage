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
 * Application root
 */

#pragma once

#include <glibmm.h>
#include <gtkmm/application.h>
#include <gtkmm/builder.h>
#include <gtkmm/aboutdialog.h>
#include "mainwindow.h"
#include "colourpickerwindow.h"
#include "ditherwindow.h"
#include "tilingwindow.h"
#include "imagehandler.h"
#include "imagecompressor.h"

class GPITool : public Gtk::Application
{
public:
    static Glib::RefPtr<GPITool> create();

protected:
    GPITool();

    void on_startup() override;
    void on_activate() override;

private:
    void CreateWindow();

    void OnHideMainWindow(MainWindow* mainwindow);
    void OnHideWindow(Gtk::Window** window);
    void OnMenuFileOpen();
    void OnMenuFileExport();
    void OnMenuFileQuit();
    void OnMenuEditPalette();
    void OnMenuEditDither();
    void OnMenuEditTiling();
    void OnMenuHelpAbout();
    void OnAboutDialogResponse(int responseID);

    Glib::RefPtr<Gtk::Builder> builderRef;
    Gtk::AboutDialog aboutDialog;
    MainWindow* mwin;
    ColourPickerWindow* colPickWin;
    DitherWindow* dithWin;
    TilingWindow* tileWin;
    ImageHandler* ihand;
    ImageCompressor* icomp;
};
