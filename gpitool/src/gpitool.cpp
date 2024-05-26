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

#include <gtkmm/filechooserdialog.h>
#include <omp.h>
#include "gpitool.h"

static const char* licenseString =
"Permission is hereby granted, free of charge, to any person obtaining a copy\n"
"of this software and associated documentation files (the \"Software\"), to deal\n"
"in the Software without restriction, including without limitation the rights\n"
"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell\n"
"copies of the Software, and to permit persons to whom the Software is\n"
"furnished to do so, subject to the following conditions:\n\n"
"The above copyright notice and this permission notice shall be included in all\n"
"copies or substantial portions of the Software.\n\n"
"THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n"
"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n"
"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n"
"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n"
"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n"
"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE\n"
"SOFTWARE.\n";

static const char* uiXML =
"<interface>"
    "<menu id='menubar'>"
        "<submenu>"
            "<attribute name='label' translatable='yes'>_File</attribute>"
            "<section>"
                "<item>"
                    "<attribute name='label' translatable='yes'>_Open...</attribute>"
                    "<attribute name='action'>app.file.open</attribute>"
                "</item>"
                "<item>"
                    "<attribute name='label' translatable='yes'>_Export...</attribute>"
                    "<attribute name='action'>app.file.export</attribute>"
                "</item>"
            "</section>"
            "<section>"
                "<item>"
                    "<attribute name='label' translatable='yes'>_Quit</attribute>"
                    "<attribute name='action'>app.file.quit</attribute>"
                "</item>"
            "</section>"
        "</submenu>"
        "<submenu>"
            "<attribute name='label' translatable='yes'>_Edit</attribute>"
            "<section>"
                "<item>"
                    "<attribute name='label' translatable='yes'>_Palette...</attribute>"
                    "<attribute name='action'>app.edit.palette</attribute>"
                "</item>"
                "<item>"
                    "<attribute name='label' translatable='yes'>_Dithering...</attribute>"
                    "<attribute name='action'>app.edit.dither</attribute>"
                "</item>"
            "</section>"
        "</submenu>"
        "<submenu>"
            "<attribute name='label' translatable='yes'>_Help</attribute>"
            "<section>"
                "<item>"
                    "<attribute name='label' translatable='yes'>_About...</attribute>"
                    "<attribute name='action'>app.help.about</attribute>"
                "</item>"
            "</section>"
        "</submenu>"
    "</menu>"
"</interface>";

GPITool::GPITool() : Gtk::Application()
{
    Glib::set_application_name("GPITool");
    ihand = new ImageHandler();
    icomp = new ImageCompressor();
    icomp->SetImageHandler(ihand);
}

Glib::RefPtr<GPITool> GPITool::create()
{
    return Glib::RefPtr<GPITool>(new GPITool());
}

void GPITool::on_startup()
{
    Gtk::Application::on_startup();

    aboutDialog.set_program_name("GPITool");
    aboutDialog.set_version("0.0.1");
    aboutDialog.set_copyright("Copyright (C) Maxim Hoxha 2024");
    aboutDialog.set_comments("Converts images into .GPI format.");
    aboutDialog.set_license(licenseString);
    aboutDialog.set_website("https://maxotaku11niku.github.io/");
    aboutDialog.set_website_label("My website");
    aboutDialog.signal_response().connect(sigc::mem_fun(*this, &GPITool::OnAboutDialogResponse));

    add_action("file.open", sigc::mem_fun(*this, &GPITool::OnMenuFileOpen));
    add_action("file.export", sigc::mem_fun(*this, &GPITool::OnMenuFileExport));
    add_action("file.quit", sigc::mem_fun(*this, &GPITool::OnMenuFileQuit));
    add_action("edit.palette", sigc::mem_fun(*this, &GPITool::OnMenuEditPalette));
    add_action("edit.dither", sigc::mem_fun(*this, &GPITool::OnMenuEditDither));
    add_action("help.about", sigc::mem_fun(*this, &GPITool::OnMenuHelpAbout));

    builderRef = Gtk::Builder::create();
    builderRef->add_from_string(uiXML);

    Glib::RefPtr<Glib::Object> uiobj = builderRef->get_object("menubar");
    Glib::RefPtr<Gio::Menu> menubar = Glib::RefPtr<Gio::Menu>::cast_dynamic(uiobj);
    set_menubar(menubar);
}

void GPITool::on_activate()
{
    CreateWindow();
}

void GPITool::CreateWindow()
{
    mwin = new MainWindow();
    add_window(*mwin);
    mwin->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &GPITool::OnHideWindow), mwin));
    mwin->show_all();
}

void GPITool::OnHideWindow(Gtk::Window* window)
{
    delete window;
}

void GPITool::OnMenuFileOpen()
{
    Gtk::FileChooserDialog dialog = Gtk::FileChooserDialog("Open File", Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Open", Gtk::RESPONSE_OK);

    auto typeFilter = Gtk::FileFilter::create();
    typeFilter->set_name("Image files");
    typeFilter->add_pattern("*.png");
    typeFilter->add_pattern("*.jpg");
    typeFilter->add_pattern("*.jpeg");
    typeFilter->add_pattern("*.jfif");
    dialog.add_filter(typeFilter);

    int result = dialog.run();

    switch (result)
    {
        case(Gtk::RESPONSE_OK):
            ihand->CloseImageFile();
            if (!ihand->OpenImageFile((char*)dialog.get_filename().c_str()))
            {
                if (!ihand->IsPalettePerfect()) ihand->DitherImage();
                else ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
                mwin->SetNewImageThumbnail(ihand);
            }
            break;
        case(Gtk::RESPONSE_CANCEL):
            break;
    }
}

void GPITool::OnMenuFileExport()
{
    Gtk::FileChooserDialog dialog = Gtk::FileChooserDialog("Export To", Gtk::FILE_CHOOSER_ACTION_SAVE);
    dialog.add_button("_Cancel", Gtk::RESPONSE_CANCEL);
    dialog.add_button("_Export", Gtk::RESPONSE_OK);

    auto typeFilter = Gtk::FileFilter::create();
    typeFilter->set_name("GPI files");
    typeFilter->add_pattern("*.gpi");
    typeFilter->add_pattern("*.GPI");
    dialog.add_filter(typeFilter);

    int result = dialog.run();

    switch (result)
    {
        case(Gtk::RESPONSE_OK):
            puts("saving");
            icomp->CompressAndSaveImage((char*)dialog.get_filename().c_str());
            break;
        case(Gtk::RESPONSE_CANCEL):
            break;
    }
}

void GPITool::OnMenuFileQuit()
{
    quit();
    std::vector<Gtk::Window*> wins = get_windows();
    for (int i = 0; i < wins.size(); i++)
    {
        wins[i]->hide();
    }
}

void GPITool::OnMenuEditPalette()
{
    colPickWin = new ColourPickerWindow(ihand, mwin);
    add_window(*colPickWin);
    colPickWin->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &GPITool::OnHideWindow), colPickWin));
    colPickWin->show_all();
}

void GPITool::OnMenuEditDither()
{
    dithWin = new DitherWindow(ihand, mwin);
    add_window(*dithWin);
    dithWin->signal_hide().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &GPITool::OnHideWindow), dithWin));
    dithWin->show_all();
}

void GPITool::OnMenuHelpAbout()
{
    aboutDialog.show();
    aboutDialog.present();
}

void GPITool::OnAboutDialogResponse(int responseID)
{
    switch (responseID)
    {
        case Gtk::RESPONSE_CLOSE:
        case Gtk::RESPONSE_CANCEL:
        case Gtk::RESPONSE_DELETE_EVENT:
            aboutDialog.hide();
            break;
    }
}

int main(int argc, char* argv[])
{
    omp_set_num_threads(omp_get_max_threads());
    Glib::RefPtr<GPITool> gpitool = GPITool::create();
    return gpitool->run(argc, argv);
}
