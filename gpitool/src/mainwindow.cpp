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
 * Main window
 */

#include <gtkmm/box.h>
#include <string.h>
#include "mainwindow.h"

static const char* uiXML =
"<interface>"
    "<object class='GtkBox' id='mainView'>"
        "<property name='visible'>True</property>"
        "<property name='can-focus'>False</property>"
        "<property name='orientation'>vertical</property>"
        "<child>"
            "<object class='GtkImage' id='mainImage'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>0</property>"
            "</packing>"
        "</child>"
    "</object>"
"</interface>";

MainWindow::MainWindow()
{
    set_title("GPITool");
    set_default_size(1280, 600);
    builderRef = Gtk::Builder::create();
    builderRef->add_from_string(uiXML);

    Gtk::Box* mainView;
    builderRef->get_widget<Gtk::Box>("mainView", mainView);
    add(*mainView);

    builderRef->get_widget<Gtk::Image>("mainImage", mainImage);

    mainPixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, true, 8, 8, 8);
    mainPixbuf->fill(0x00000000);
    mainImage->set(mainPixbuf);
}

MainWindow::~MainWindow()
{

}

void MainWindow::SetNewImageThumbnail(ImageHandler* ihand)
{
    ImageInfo* iinf = ihand->GetEncodedImage();
    mainPixbuf = Gdk::Pixbuf::create(Gdk::Colorspace::COLORSPACE_RGB, true, 8, iinf->width, iinf->height);
    memcpy(mainPixbuf->get_pixels(), iinf->data, iinf->width * iinf->height * 4);
    mainImage->set(mainPixbuf);
}

void MainWindow::UpdateImageThumbnail(ImageHandler* ihand)
{
    ImageInfo* iinf = ihand->GetEncodedImage();
    memcpy(mainPixbuf->get_pixels(), iinf->data, iinf->width * iinf->height * 4);
    mainImage->set(mainPixbuf);
}


