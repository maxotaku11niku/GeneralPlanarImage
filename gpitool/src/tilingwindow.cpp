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

#include <gtkmm/box.h>
#include "tilingwindow.h"

static const char* uiXML =
"<interface>"
    "<object class='GtkAdjustment' id='tileSizeX'>"
        "<property name='upper'>1024</property>"
        "<property name='lower'>2</property>"
        "<property name='step-increment'>1</property>"
        "<property name='page-increment'>8</property>"
    "</object>"
    "<object class='GtkAdjustment' id='tileSizeY'>"
        "<property name='upper'>1024</property>"
        "<property name='lower'>2</property>"
        "<property name='step-increment'>1</property>"
        "<property name='page-increment'>8</property>"
    "</object>"
    "<object class='GtkBox' id='mainView'>"
        "<property name='visible'>True</property>"
        "<property name='can-focus'>False</property>"
        "<property name='orientation'>vertical</property>"
        "<child>"
            "<object class='GtkCheckButton' id='tilingCheck'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>True</property>"
                "<property name='label' translatable='yes'>Enable tiling</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>0</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkLabel'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='label' translatable='yes'>Tile size:</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>1</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkBox'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='orientation'>horizontal</property>"
                "<child>"
                    "<object class='GtkBox'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>False</property>"
                        "<property name='orientation'>vertical</property>"
                        "<child>"
                            "<object class='GtkLabel'>"
                                "<property name='visible'>True</property>"
                                "<property name='can-focus'>False</property>"
                                "<property name='label' translatable='yes'>X</property>"
                            "</object>"
                            "<packing>"
                                "<property name='expand'>True</property>"
                                "<property name='fill'>True</property>"
                                "<property name='position'>0</property>"
                            "</packing>"
                        "</child>"
                        "<child>"
                            "<object class='GtkSpinButton'>"
                                "<property name='visible'>True</property>"
                                "<property name='can-focus'>True</property>"
                                "<property name='input-purpose'>number</property>"
                                "<property name='adjustment'>tileSizeX</property>"
                                "<property name='climb-rate'>1</property>"
                                "<property name='numeric'>True</property>"
                            "</object>"
                            "<packing>"
                                "<property name='expand'>False</property>"
                                "<property name='fill'>True</property>"
                                "<property name='position'>1</property>"
                            "</packing>"
                        "</child>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>0</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkBox'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>False</property>"
                        "<property name='orientation'>vertical</property>"
                        "<child>"
                            "<object class='GtkLabel'>"
                                "<property name='visible'>True</property>"
                                "<property name='can-focus'>False</property>"
                                "<property name='label' translatable='yes'>Y</property>"
                            "</object>"
                            "<packing>"
                                "<property name='expand'>True</property>"
                                "<property name='fill'>True</property>"
                                "<property name='position'>0</property>"
                            "</packing>"
                        "</child>"
                        "<child>"
                            "<object class='GtkSpinButton'>"
                                "<property name='visible'>True</property>"
                                "<property name='can-focus'>True</property>"
                                "<property name='input-purpose'>number</property>"
                                "<property name='adjustment'>tileSizeY</property>"
                                "<property name='climb-rate'>1</property>"
                                "<property name='numeric'>True</property>"
                            "</object>"
                            "<packing>"
                                "<property name='expand'>False</property>"
                                "<property name='fill'>True</property>"
                                "<property name='position'>1</property>"
                            "</packing>"
                        "</child>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>1</property>"
                    "</packing>"
                "</child>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>2</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkLabel'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='label' translatable='yes'>Tile order:</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>3</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkBox'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='orientation'>horizontal</property>"
                "<child>"
                    "<object class='GtkRadioButton' id='rowmajradio'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Left-to-right, then top-to-bottom</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>0</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkRadioButton' id='colmajradio'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='group'>rowmajradio</property>"
                        "<property name='label' translatable='yes'>Top-to-bottom, then left-to-right</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>1</property>"
                    "</packing>"
                "</child>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>4</property>"
            "</packing>"
        "</child>"
    "</object>"
"</interface>";

TilingWindow::TilingWindow(ImageHandler* handler, MainWindow* mainwind)
{
    set_title("Tiling Settings");
    set_default_size(360, 2);
    builderRef = Gtk::Builder::create();
    builderRef->add_from_string(uiXML);

    Gtk::Box* mainView;
    builderRef->get_widget<Gtk::Box>("mainView", mainView);
    add(*mainView);

    Glib::RefPtr<Glib::Object> uiobj = builderRef->get_object("tileSizeX");
    tileSizeX = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(uiobj);
    uiobj = builderRef->get_object("tileSizeY");
    tileSizeY = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(uiobj);

    builderRef->get_widget<Gtk::CheckButton>("tilingCheck", enableCheck);
    builderRef->get_widget<Gtk::RadioButton>("rowmajradio", rowmajRadio);
    builderRef->get_widget<Gtk::RadioButton>("colmajradio", colmajRadio);

    ihand = handler;
    mwin = mainwind;

    enableCheck->signal_toggled().connect(sigc::mem_fun(*this, &TilingWindow::OnToggleTiling));
    tileSizeX->signal_value_changed().connect(sigc::mem_fun(*this, &TilingWindow::OnSetTileSizeX));
    tileSizeY->signal_value_changed().connect(sigc::mem_fun(*this, &TilingWindow::OnSetTileSizeY));
    rowmajRadio->signal_toggled().connect(sigc::mem_fun(*this, &TilingWindow::OnToggleOrder));
    colmajRadio->signal_toggled().connect(sigc::mem_fun(*this, &TilingWindow::OnToggleOrder));

    enableCheck->set_active(ihand->isTiled);
    tileSizeX->set_upper(ihand->GetEncodedImage()->width);
    tileSizeY->set_upper(ihand->GetEncodedImage()->height);
    tileSizeX->set_value(ihand->tileSizeX);
    tileSizeY->set_value(ihand->tileSizeY);
    switch (ihand->tileOrdering)
    {
        case ROWMAJOR:
            rowmajRadio->set_active(true);
            colmajRadio->set_active(false);
            break;
        case COLUMNMAJOR:
            rowmajRadio->set_active(false);
            colmajRadio->set_active(true);
            break;
    }
}

TilingWindow::~TilingWindow()
{

}

void TilingWindow::OnToggleTiling()
{
    ihand->isTiled = enableCheck->get_active();
}

void TilingWindow::OnSetTileSizeX()
{
    ihand->tileSizeX = (int)tileSizeX->get_value();
}

void TilingWindow::OnSetTileSizeY()
{
    ihand->tileSizeY = (int)tileSizeY->get_value();
}

void TilingWindow::OnToggleOrder()
{
    if (rowmajRadio->get_active())
    {
        ihand->tileOrdering = ROWMAJOR;
    }
    else if (colmajRadio->get_active())
    {
        ihand->tileOrdering = COLUMNMAJOR;
    }
}
