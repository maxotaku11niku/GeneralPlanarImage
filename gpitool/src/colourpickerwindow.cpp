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

#include <gtkmm/box.h>
#include "colourpickerwindow.h"

static const char* uiXML =
"<interface>"
    "<object class='GtkAdjustment' id='transThres'>"
        "<property name='upper'>255</property>"
        "<property name='step-increment'>1</property>"
        "<property name='page-increment'>10</property>"
    "</object>"
    "<object class='GtkBox' id='mainView'>"
        "<property name='visible'>True</property>"
        "<property name='can-focus'>False</property>"
        "<property name='orientation'>vertical</property>"
        "<child>"
            "<object class='GtkLabel'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='label' translatable='yes'>Included Planes:</property>"
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
                "<property name='orientation'>horizontal</property>"
                "<child>"
                    "<object class='GtkCheckButton' id='plane0check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 0</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>0</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkCheckButton' id='plane1check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 1</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>1</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkCheckButton' id='plane2check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 2</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>2</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkCheckButton' id='plane3check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 3</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>3</property>"
                    "</packing>"
                "</child>"
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
                    "<object class='GtkCheckButton' id='plane4check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 4</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>0</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkCheckButton' id='plane5check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 5</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>1</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkCheckButton' id='plane6check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 6</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>2</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkCheckButton' id='plane7check'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>Plane 7</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>3</property>"
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
            "<object class='GtkCheckButton' id='planeMcheck'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>True</property>"
                "<property name='label' translatable='yes'>Mask plane (for transparency)</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>3</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkLabel'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='label' translatable='yes'>Colour depth:</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>4</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkBox'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='orientation'>horizontal</property>"
                "<child>"
                    "<object class='GtkRadioButton' id='4bpcradio'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='label' translatable='yes'>4 bits per channel</property>"
                    "</object>"
                    "<packing>"
                        "<property name='expand'>True</property>"
                        "<property name='fill'>True</property>"
                        "<property name='position'>0</property>"
                    "</packing>"
                "</child>"
                "<child>"
                    "<object class='GtkRadioButton' id='8bpcradio'>"
                        "<property name='visible'>True</property>"
                        "<property name='can-focus'>True</property>"
                        "<property name='group'>4bpcradio</property>"
                        "<property name='label' translatable='yes'>8 bits per channel</property>"
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
                "<property name='position'>5</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkLabel'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='label' translatable='yes'>Transparency threshold:</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>6</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkSpinButton' id='transThresSelect'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>True</property>"
                "<property name='input-purpose'>number</property>"
                "<property name='adjustment'>transThres</property>"
                "<property name='climb-rate'>1</property>"
                "<property name='numeric'>True</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>7</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkLabel'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
                "<property name='label' translatable='yes'>Colours:</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>8</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkGrid' id='colourGrid'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>False</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>9</property>"
            "</packing>"
        "</child>"
        "<child>"
            "<object class='GtkButton' id='findBestPaletteButton'>"
                "<property name='visible'>True</property>"
                "<property name='can-focus'>True</property>"
                "<property name='label' translatable='yes'>Find best palette...</property>"
            "</object>"
            "<packing>"
                "<property name='expand'>True</property>"
                "<property name='fill'>True</property>"
                "<property name='position'>10</property>"
            "</packing>"
        "</child>"
    "</object>"
"</interface>";

ColourPickerWindow::ColourPickerWindow(ImageHandler* handler, MainWindow* mainwind)
{
    set_title("Palette Settings");
    set_default_size(480, 640);
    builderRef = Gtk::Builder::create();
    builderRef->add_from_string(uiXML);

    Gtk::Box* mainView;
    builderRef->get_widget<Gtk::Box>("mainView", mainView);
    add(*mainView);

    Glib::RefPtr<Glib::Object> uiobj = builderRef->get_object("transThres");
    transThreshold = Glib::RefPtr<Gtk::Adjustment>::cast_dynamic(uiobj);

    builderRef->get_widget<Gtk::CheckButton>("plane0check", planeChecks[0]);
    builderRef->get_widget<Gtk::CheckButton>("plane1check", planeChecks[1]);
    builderRef->get_widget<Gtk::CheckButton>("plane2check", planeChecks[2]);
    builderRef->get_widget<Gtk::CheckButton>("plane3check", planeChecks[3]);
    builderRef->get_widget<Gtk::CheckButton>("plane4check", planeChecks[4]);
    builderRef->get_widget<Gtk::CheckButton>("plane5check", planeChecks[5]);
    builderRef->get_widget<Gtk::CheckButton>("plane6check", planeChecks[6]);
    builderRef->get_widget<Gtk::CheckButton>("plane7check", planeChecks[7]);
    builderRef->get_widget<Gtk::CheckButton>("planeMcheck", planeChecks[8]);

    builderRef->get_widget<Gtk::RadioButton>("4bpcradio", bpc4Radio);
    builderRef->get_widget<Gtk::RadioButton>("8bpcradio", bpc8Radio);
    builderRef->get_widget<Gtk::SpinButton>("transThresSelect", transThresholdSpin);
    builderRef->get_widget<Gtk::Grid>("colourGrid", colourGrid);
    builderRef->get_widget<Gtk::Button>("findBestPaletteButton", findBestPaletteButton);

    ihand = handler;
    mwin = mainwind;

    int pMask = ihand->GetPlaneMask();
    int checkBit = 1;
    for (int i = 0; i < 9; i++)
    {
        if (pMask & checkBit) planeChecks[i]->set_active(true);
        else planeChecks[i]->set_active(false);
        checkBit <<= 1;
    }

    bpc4Radio->set_active(!ihand->is8BitColour);
    bpc8Radio->set_active(ihand->is8BitColour);
    transThreshold->set_value(ihand->transparencyThreshold);

    for (int i = 0; i < 9; i++)
    {
        planeChecks[i]->signal_toggled().connect(sigc::bind<int>(sigc::mem_fun(*this, &ColourPickerWindow::OnTogglePlane), i));
    }
    bpc4Radio->signal_toggled().connect(sigc::mem_fun(*this, &ColourPickerWindow::OnToggleBitDepth));
    bpc8Radio->signal_toggled().connect(sigc::mem_fun(*this, &ColourPickerWindow::OnToggleBitDepth));
    transThreshold->signal_value_changed().connect(sigc::mem_fun(*this, &ColourPickerWindow::OnSetTransparencyThreshold));
    findBestPaletteButton->signal_clicked().connect(sigc::mem_fun(*this, &ColourPickerWindow::OnRequestBestPalette));

    ColourRGBA8* pal = ihand->GetCurrentPalette();
    for (int i = 0; i < 256; i++)
    {
        ColourRGBA8 palcol = pal[i];
        Gdk::RGBA col(((float)palcol.R)/255.0f, ((float)palcol.G)/255.0f, ((float)palcol.B)/255.0f, ((float)palcol.A)/255.0f);
        colourButtons[i] = new Gtk::ColorButton(col);
        Glib::PropertyProxy<bool> cButtonShowEditor = colourButtons[i]->property_show_editor();
        cButtonShowEditor.set_value(true);
        colourButtons[i]->signal_color_set().connect(sigc::bind<int>(sigc::mem_fun(*this, &ColourPickerWindow::OnSetColour), i));
    }

    for (int i = 0; i < 16; i++)
    {
        colourGrid->insert_row(0);
        colourGrid->insert_column(0);
    }

    ReorganisePaletteGrid(ihand->GetNumColourPlanes());
}

ColourPickerWindow::~ColourPickerWindow()
{
    for (int i = 0; i < 256; i++)
    {
        delete colourButtons[i];
    }
}

void ColourPickerWindow::ReorganisePaletteGrid(int numColourPlanes)
{
    for (int i = 0; i < 256; i++)
    {
        Gtk::CheckButton* cbut = (Gtk::CheckButton*)colourGrid->get_child_at(i % 16, i / 16);
        if (cbut != nullptr) colourGrid->remove(*cbut);
    }
    switch (numColourPlanes)
    {
        case 1:
            colourGrid->attach(*colourButtons[0],0,0);
            colourGrid->attach(*colourButtons[1],1,0);
            break;
        case 2:
            colourGrid->attach(*colourButtons[0],0,0);
            colourGrid->attach(*colourButtons[1],1,0);
            colourGrid->attach(*colourButtons[2],0,1);
            colourGrid->attach(*colourButtons[3],1,1);
            break;
        case 3:
            for (int i = 0; i < 8; i++)
            {
                colourGrid->attach(*colourButtons[i], i % 4, i / 4);
            }
            break;
        case 4:
            for (int i = 0; i < 16; i++)
            {
                colourGrid->attach(*colourButtons[i], i % 4, i / 4);
            }
            break;
        case 5:
            for (int i = 0; i < 32; i++)
            {
                colourGrid->attach(*colourButtons[i], i % 8, i / 8);
            }
            break;
        case 6:
            for (int i = 0; i < 64; i++)
            {
                colourGrid->attach(*colourButtons[i], i % 8, i / 8);
            }
            break;
        case 7:
            for (int i = 0; i < 128; i++)
            {
                colourGrid->attach(*colourButtons[i], i % 16, i / 16);
            }
            break;
        case 8:
            for (int i = 0; i < 256; i++)
            {
                colourGrid->attach(*colourButtons[i], i % 16, i / 16);
            }
            break;
    }
}

void ColourPickerWindow::SetPaletteGridColours()
{
    ColourRGBA8* pal = ihand->GetCurrentPalette();
    for (int i = 0; i < ihand->GetNumColours(); i++)
    {
        ColourRGBA8 palcol = pal[i];
        Gdk::RGBA col(((float)palcol.R)/255.0f, ((float)palcol.G)/255.0f, ((float)palcol.B)/255.0f, ((float)palcol.A)/255.0f);
        colourButtons[i]->set_rgba(col);
    }
}


void ColourPickerWindow::OnTogglePlane(int planeNum)
{
    if (planeChecks[planeNum]->get_active()) ihand->AddPlane(planeNum);
    else ihand->RemovePlane(planeNum);
    ReorganisePaletteGrid(ihand->GetNumColourPlanes());
    if(!ihand->IsPalettePerfect()) ihand->DitherImage(STUCKI, 0.0, 0.0, 0.0, 1.0, 0.9, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.2, true);
    else ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
    mwin->SetNewImageThumbnail(ihand);
}

void ColourPickerWindow::OnToggleBitDepth()
{
    ihand->is8BitColour = bpc8Radio->get_active();
    if(!ihand->IsPalettePerfect()) ihand->DitherImage(STUCKI, 0.0, 0.0, 0.0, 1.0, 0.9, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.2, true);
    else ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
    mwin->SetNewImageThumbnail(ihand);
}

void ColourPickerWindow::OnSetColour(int index)
{
    ColourRGBA8* pal = ihand->GetCurrentPalette();
    Gdk::RGBA col = colourButtons[index]->get_rgba();
    int oR, oG, oB;
    if (ihand->is8BitColour)
    {
        oR = (int)((col.get_red() * 255.0f) + 0.5f);
        oG = (int)((col.get_green() * 255.0f) + 0.5f);
        oB = (int)((col.get_blue() * 255.0f) + 0.5f);
        if (oR > 0xFF) oR = 0xFF; else if (oR < 0) oR = 0;
        if (oG > 0xFF) oG = 0xFF; else if (oG < 0) oG = 0;
        if (oB > 0xFF) oB = 0xFF; else if (oB < 0) oB = 0;
    }
    else
    {
        oR = (int)((col.get_red() * 15.0f) + 0.5f);
        oG = (int)((col.get_green() * 15.0f) + 0.5f);
        oB = (int)((col.get_blue() * 15.0f) + 0.5f);
        if (oR > 0xF) oR = 0xF; else if (oR < 0) oR = 0;
        if (oG > 0xF) oG = 0xF; else if (oG < 0) oG = 0;
        if (oB > 0xF) oB = 0xF; else if (oB < 0) oB = 0;
        oR *= 0x11; oG *= 0x11; oB *= 0x11;
    }
    ColourRGBA8 outcol = { (unsigned char)oR, (unsigned char)oG, (unsigned char)oB, 0xFF };
    ihand->SetPaletteColour(index, outcol);
    Gdk::RGBA newcol(((float)outcol.R)/255.0f, ((float)outcol.G)/255.0f, ((float)outcol.B)/255.0f, ((float)outcol.A)/255.0f);
    colourButtons[index]->set_rgba(newcol);
    if(!ihand->IsPalettePerfect()) ihand->DitherImage(STUCKI, 0.0, 0.0, 0.0, 1.0, 0.9, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.2, true);
    else ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
    mwin->SetNewImageThumbnail(ihand);
}

void ColourPickerWindow::OnSetTransparencyThreshold()
{
    ihand->transparencyThreshold = (unsigned char)transThreshold->get_value();
    if(!ihand->IsPalettePerfect()) ihand->DitherImage(STUCKI, 0.0, 0.0, 0.0, 1.0, 0.9, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.2, true);
    else ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
    mwin->SetNewImageThumbnail(ihand);
}


void ColourPickerWindow::OnRequestBestPalette()
{
    if (!ihand->GetBestPalette(1.0, 0.0, 0.0))
    {
        ihand->DitherImage(STUCKI, 0.0, 0.0, 0.0, 1.0, 0.9, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.2, true);
    }
    else
    {
        ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
    }
    ihand->ShufflePaletteBasedOnOccurrence();
    SetPaletteGridColours();
    mwin->SetNewImageThumbnail(ihand);
}
