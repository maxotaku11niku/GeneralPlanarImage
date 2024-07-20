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

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QPixmap>
#include <QImage>
#include <QBoxLayout>
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
"SOFTWARE.\n\n"
"What follows is the license information for all used libraries\n\n"
"libpng:\n\n"
"PNG Reference Library License version 2\n"
"---------------------------------------\n\n"
" * Copyright (c) 1995-2024 The PNG Reference Library Authors.\n"
" * Copyright (c) 2018-2024 Cosmin Truta.\n"
" * Copyright (c) 2000-2002, 2004, 2006-2018 Glenn Randers-Pehrson.\n"
" * Copyright (c) 1996-1997 Andreas Dilger.\n"
" * Copyright (c) 1995-1996 Guy Eric Schalnat, Group 42, Inc.\n\n"
"The software is supplied \"as is\", without warranty of any kind,\n"
"express or implied, including, without limitation, the warranties\n"
"of merchantability, fitness for a particular purpose, title, and\n"
"non-infringement.  In no event shall the Copyright owners, or\n"
"anyone distributing the software, be liable for any damages or\n"
"other liability, whether in contract, tort or otherwise, arising\n"
"from, out of, or in connection with the software, or the use or\n"
"other dealings in the software, even if advised of the possibility\n"
"of such damage.\n\n"
"Permission is hereby granted to use, copy, modify, and distribute\n"
"this software, or portions hereof, for any purpose, without fee,\n"
"subject to the following restrictions:\n\n"
" 1. The origin of this software must not be misrepresented; you\n"
"    must not claim that you wrote the original software.  If you\n"
"    use this software in a product, an acknowledgment in the product\n"
"    documentation would be appreciated, but is not required.\n\n"
" 2. Altered source versions must be plainly marked as such, and must\n"
"    not be misrepresented as being the original software.\n\n"
" 3. This Copyright notice may not be removed or altered from any\n"
"    source or altered source distribution.\n\n"
"libjpeg(-turbo):\n\n"
"libjpeg-turbo note:  This file has been modified by The libjpeg-turbo Project\n"
"to include only information relevant to libjpeg-turbo, to wordsmith certain\n"
"sections, and to remove impolitic language that existed in the libjpeg v8\n"
"README.  It is included only for reference.  Please see README.md for\n"
"information specific to libjpeg-turbo.\n\n\n"
"The authors make NO WARRANTY or representation, either express or implied,\n"
"with respect to this software, its quality, accuracy, merchantability, or\n"
"fitness for a particular purpose.  This software is provided \"AS IS\", and you,\n"
"its user, assume the entire risk as to its quality and accuracy.\n\n"
"This software is copyright (C) 1991-2020, Thomas G. Lane, Guido Vollbeding.\n"
"All Rights Reserved except as specified below.\n\n"
"Permission is hereby granted to use, copy, modify, and distribute this\n"
"software (or portions thereof) for any purpose, without fee, subject to these\n"
"conditions:\n"
"(1) If any part of the source code for this software is distributed, then this\n"
"README file must be included, with this copyright and no-warranty notice\n"
"unaltered; and any additions, deletions, or changes to the original files\n"
"must be clearly indicated in accompanying documentation.\n"
"(2) If only executable code is distributed, then the accompanying\n"
"documentation must state that \"this software is based in part on the work of\n"
"the Independent JPEG Group\".\n"
"(3) Permission for use of this software is granted only if the user accepts\n"
"full responsibility for any undesirable consequences; the authors accept\n"
"NO LIABILITY for damages of any kind.\n\n"
"These conditions apply to any software derived from or based on the IJG code,\n"
"not just to the unmodified library.  If you use our work, you ought to\n"
"acknowledge us.\n\n"
"Permission is NOT granted for the use of any IJG author's name or company name\n"
"in advertising or publicity relating to this software or products derived from\n"
"it.  This software may be referred to only as \"the Independent JPEG Group's\n"
"software\".\n\n"
"We specifically permit and encourage the use of this software as the basis of\n"
"commercial products, provided that all warranty or liability claims are\n"
"assumed by the product vendor.\n\n"
"Copyright (C)2009-2023 D. R. Commander. All Rights Reserved.\n"
"Copyright (C)2015 Viktor Szathmáry. All Rights Reserved.\n\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions are met:\n\n"
"  - Redistributions of source code must retain the above copyright notice,\n"
"    this list of conditions and the following disclaimer.\n\n"
"  - Redistributions in binary form must reproduce the above copyright notice,\n"
"    this list of conditions and the following disclaimer in the documentation\n"
"    and/or other materials provided with the distribution.\n\n"
"  - Neither the name of the libjpeg-turbo Project nor the names of its\n"
"    contributors may be used to endorse or promote products derived from this\n"
"    software without specific prior written permission.\n\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\",\n"
"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE\n"
"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE\n"
"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n"
"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR\n"
"SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER\n"
"CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,\n"
"OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\n"
"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n"
"liblz4:\n\n"
"LZ4 Library\n"
"Copyright (c) 2011-2020, Yann Collet\n"
"All rights reserved.\n\n"
"Redistribution and use in source and binary forms, with or without modification,\n"
"are permitted provided that the following conditions are met:\n\n"
"* Redistributions of source code must retain the above copyright notice, this\n"
"  list of conditions and the following disclaimer.\n\n"
"* Redistributions in binary form must reproduce the above copyright notice, this\n"
"  list of conditions and the following disclaimer in the documentation and/or\n"
"  other materials provided with the distribution.\n\n"
"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" AND\n"
"ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n"
"WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE\n"
"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR\n"
"ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES\n"
"(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;\n"
"LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON\n"
"ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS\n"
"SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n"
"gtkmm 3.0:\n\n"
"                   GNU LESSER GENERAL PUBLIC LICENSE\n"
"                       Version 3, 29 June 2007\n\n"
" Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>\n"
" Everyone is permitted to copy and distribute verbatim copies\n"
" of this license document, but changing it is not allowed.\n\n\n"
"  This version of the GNU Lesser General Public License incorporates\n"
"the terms and conditions of version 3 of the GNU General Public\n"
"License, supplemented by the additional permissions listed below.\n\n"
"  0. Additional Definitions.\n\n"
"  As used herein, \"this License\" refers to version 3 of the GNU Lesser\n"
"General Public License, and the \"GNU GPL\" refers to version 3 of the GNU\n"
"General Public License.\n\n"
"  \"The Library\" refers to a covered work governed by this License,\n"
"other than an Application or a Combined Work as defined below.\n\n"
"  An \"Application\" is any work that makes use of an interface provided\n"
"by the Library, but which is not otherwise based on the Library.\n"
"Defining a subclass of a class defined by the Library is deemed a mode\n"
"of using an interface provided by the Library.\n\n"
"  A \"Combined Work\" is a work produced by combining or linking an\n"
"Application with the Library.  The particular version of the Library\n"
"with which the Combined Work was made is also called the \"Linked\n"
"Version\".\n\n"
"  The \"Minimal Corresponding Source\" for a Combined Work means the\n"
"Corresponding Source for the Combined Work, excluding any source code\n"
"for portions of the Combined Work that, considered in isolation, are\n"
"based on the Application, and not on the Linked Version.\n\n"
"  The \"Corresponding Application Code\" for a Combined Work means the\n"
"object code and/or source code for the Application, including any data\n"
"and utility programs needed for reproducing the Combined Work from the\n"
"Application, but excluding the System Libraries of the Combined Work.\n\n"
"  1. Exception to Section 3 of the GNU GPL.\n\n"
"  You may convey a covered work under sections 3 and 4 of this License\n"
"without being bound by section 3 of the GNU GPL.\n\n"
"  2. Conveying Modified Versions.\n\n"
"  If you modify a copy of the Library, and, in your modifications, a\n"
"facility refers to a function or data to be supplied by an Application\n"
"that uses the facility (other than as an argument passed when the\n"
"facility is invoked), then you may convey a copy of the modified\n"
"version:\n\n"
"   a) under this License, provided that you make a good faith effort to\n"
"   ensure that, in the event an Application does not supply the\n"
"   function or data, the facility still operates, and performs\n"
"   whatever part of its purpose remains meaningful, or\n\n"
"   b) under the GNU GPL, with none of the additional permissions of\n"
"   this License applicable to that copy.\n\n"
"  3. Object Code Incorporating Material from Library Header Files.\n\n"
"  The object code form of an Application may incorporate material from\n"
"a header file that is part of the Library.  You may convey such object\n"
"code under terms of your choice, provided that, if the incorporated\n"
"material is not limited to numerical parameters, data structure\n"
"layouts and accessors, or small macros, inline functions and templates\n"
"(ten or fewer lines in length), you do both of the following:\n\n"
"   a) Give prominent notice with each copy of the object code that the\n"
"   Library is used in it and that the Library and its use are\n"
"   covered by this License.\n\n"
"   b) Accompany the object code with a copy of the GNU GPL and this license\n"
"   document.\n\n"
"  4. Combined Works.\n\n"
"  You may convey a Combined Work under terms of your choice that,\n"
"taken together, effectively do not restrict modification of the\n"
"portions of the Library contained in the Combined Work and reverse\n"
"engineering for debugging such modifications, if you also do each of\n"
"the following:\n\n"
"   a) Give prominent notice with each copy of the Combined Work that\n"
"   the Library is used in it and that the Library and its use are\n"
"   covered by this License.\n\n"
"   b) Accompany the Combined Work with a copy of the GNU GPL and this license\n"
"   document.\n\n"
"   c) For a Combined Work that displays copyright notices during\n"
"   execution, include the copyright notice for the Library among\n"
"   these notices, as well as a reference directing the user to the\n"
"   copies of the GNU GPL and this license document.\n\n"
"   d) Do one of the following:\n\n"
"       0) Convey the Minimal Corresponding Source under the terms of this\n"
"       License, and the Corresponding Application Code in a form\n"
"       suitable for, and under terms that permit, the user to\n"
"       recombine or relink the Application with a modified version of\n"
"       the Linked Version to produce a modified Combined Work, in the\n"
"       manner specified by section 6 of the GNU GPL for conveying\n"
"       Corresponding Source.\n\n"
"       1) Use a suitable shared library mechanism for linking with the\n"
"       Library.  A suitable mechanism is one that (a) uses at run time\n"
"       a copy of the Library already present on the user's computer\n"
"       system, and (b) will operate properly with a modified version\n"
"       of the Library that is interface-compatible with the Linked\n"
"       Version.\n\n"
"   e) Provide Installation Information, but only if you would otherwise\n"
"   be required to provide such information under section 6 of the\n"
"   GNU GPL, and only to the extent that such information is\n"
"   necessary to install and execute a modified version of the\n"
"   Combined Work produced by recombining or relinking the\n"
"   Application with a modified version of the Linked Version. (If\n"
"   you use option 4d0, the Installation Information must accompany\n"
"   the Minimal Corresponding Source and Corresponding Application\n"
"   Code. If you use option 4d1, you must provide the Installation\n"
"   Information in the manner specified by section 6 of the GNU GPL\n"
"   for conveying Corresponding Source.)\n\n"
"  5. Combined Libraries.\n\n"
"  You may place library facilities that are a work based on the\n"
"Library side by side in a single library together with other library\n"
"facilities that are not Applications and are not covered by this\n"
"License, and convey such a combined library under terms of your\n"
"choice, if you do both of the following:\n\n"
"   a) Accompany the combined library with a copy of the same work based\n"
"   on the Library, uncombined with any other library facilities,\n"
"   conveyed under the terms of this License.\n\n"
"   b) Give prominent notice with the combined library that part of it\n"
"   is a work based on the Library, and explaining where to find the\n"
"   accompanying uncombined form of the same work.\n\n"
"  6. Revised Versions of the GNU Lesser General Public License.\n\n"
"  The Free Software Foundation may publish revised and/or new versions\n"
"of the GNU Lesser General Public License from time to time. Such new\n"
"versions will be similar in spirit to the present version, but may\n"
"differ in detail to address new problems or concerns.\n\n"
"  Each version is given a distinguishing version number. If the\n"
"Library as you received it specifies that a certain numbered version\n"
"of the GNU Lesser General Public License \"or any later version\"\n"
"applies to it, you have the option of following the terms and\n"
"conditions either of that published version or of any later version\n"
"published by the Free Software Foundation. If the Library as you\n"
"received it does not specify a version number of the GNU Lesser\n"
"General Public License, you may choose any version of the GNU Lesser\n"
"General Public License ever published by the Free Software Foundation.\n\n"
"  If the Library as you received it specifies that a proxy can decide\n"
"whether future versions of the GNU Lesser General Public License shall\n"
"apply, that proxy's public statement of acceptance of any version is\n"
"permanent authorization for you to choose that version for the\n"
"Library.\n\n";

GPITool::GPITool(QWidget *parent) : QMainWindow()
{
    ihand = new ImageHandler();
    icomp = new ImageCompressor();
    icomp->SetImageHandler(ihand);

    setWindowTitle("GPITool");
    resize(800, 600);

    QMenuBar* menubar = new QMenuBar(this);
    QMenu* fileMenu = new QMenu("&File");
    fileMenu->addAction("&Open...", this, &GPITool::OnMenuFileOpen);
    fileMenu->addAction("&Export...", this, &GPITool::OnMenuFileExport);
    fileMenu->addSeparator();
    fileMenu->addAction("&Quit", this, &GPITool::OnMenuFileQuit);
    menubar->addMenu(fileMenu);
    QMenu* editMenu = new QMenu("&Edit");
    editMenu->addAction("&Palette...", this, &GPITool::OnMenuEditPalette);
    editMenu->addAction("&Dithering...", this, &GPITool::OnMenuEditDither);
    editMenu->addAction("&Tiling...", this, &GPITool::OnMenuEditTiling);
    menubar->addMenu(editMenu);
    QMenu* helpMenu = new QMenu("&Help");
    helpMenu->addAction("&About...", this, &GPITool::OnMenuHelpAbout);
    menubar->addMenu(helpMenu);
    setMenuBar(menubar);

    scene = new QGraphicsScene(this);
    QPixmap imagePixmap = QPixmap();
    image = scene->addPixmap(imagePixmap);
    image->setVisible(true);
    sceneView = new QGraphicsView(scene, this);
    setCentralWidget(sceneView);

    colPickOpen = false;
    dithOpen = false;
    tileOpen = false;
}

void GPITool::OnMenuFileOpen()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open File", nullptr, "Image files (*.png *.jpg *.jpeg *.jfif)");

    if (!fileName.isNull())
    {
        ihand->CloseImageFile();
        if (!ihand->OpenImageFile(fileName.toUtf8().constData()))
        {
            if (!ihand->IsPalettePerfect()) ihand->DitherImage();
            else ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
            SetNewImageThumbnail();
            if (colPickOpen) colPickWin->UpdateAfterOpenFile();
        }
    }
}

void GPITool::OnMenuFileExport()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export To", nullptr, "GPI files (*.gpi *.GPI)");

    if (!fileName.isNull())
    {
        icomp->CompressAndSaveImage(fileName.toUtf8().constData());
    }
}

void GPITool::OnMenuFileQuit()
{
    close();
}

void GPITool::OnMenuEditPalette()
{
    if (!colPickOpen)
    {
        colPickWin = new ColourPickerWindow(ihand, this);
        colPickWin->show();
        connect(colPickWin, &QDockWidget::visibilityChanged, this, &GPITool::OnPaletteClose);
        colPickOpen = true;
    }

}

void GPITool::OnPaletteClose()
{
    colPickOpen = colPickWin->isVisible();
}

void GPITool::OnMenuEditDither()
{
    if (!dithOpen)
    {
        dithWin = new DitherWindow(ihand, this);
        dithWin->show();
        connect(dithWin, &QDockWidget::visibilityChanged, this, &GPITool::OnDitherClose);
        dithOpen = true;
    }
}

void GPITool::OnDitherClose()
{
    dithOpen = dithWin->isVisible();
}

void GPITool::OnMenuEditTiling()
{
    if (!tileOpen)
    {
        tileWin = new TilingWindow(ihand, this);
        tileWin->show();
        connect(tileWin, &QDockWidget::visibilityChanged, this, &GPITool::OnTilingClose);
        tileOpen = true;
    }
}

void GPITool::OnTilingClose()
{
    tileOpen = tileWin->isVisible();
}

void GPITool::OnMenuHelpAbout()
{
    QMessageBox aboutDialog;
    aboutDialog.setText("GPITool v0.5.0");
    aboutDialog.setInformativeText("Converts images into .GPI format.\nCopyright (C) Maxim Hoxha 2024\nhttps://maxotaku11niku.github.io/");
    aboutDialog.setDetailedText(licenseString);
    aboutDialog.setStandardButtons(QMessageBox::Ok);
    aboutDialog.setDefaultButton(QMessageBox::Ok);
    aboutDialog.exec();
}

void GPITool::SetNewImageThumbnail()
{
    ImageInfo* iinf = ihand->GetEncodedImage();
    QImage imagePixels = QImage((uchar*)iinf->data, iinf->width, iinf->height, QImage::Format_RGBA8888);
    QPixmap imagePixmap = QPixmap::fromImage(imagePixels);
    image->setPixmap(imagePixmap);
    sceneView->setSceneRect(0.0, 0.0, iinf->width, iinf->height);
}

void GPITool::UpdateImageThumbnail()
{
    ImageInfo* iinf = ihand->GetEncodedImage();
    QImage imagePixels = QImage((uchar*)iinf->data, iinf->width, iinf->height, QImage::Format_RGBA8888);
    QPixmap imagePixmap = QPixmap::fromImage(imagePixels);
    image->setPixmap(imagePixmap);
}

void GPITool::UpdateImageThumbnailAfterDither()
{
    if(!ihand->IsPalettePerfect()) ihand->DitherImage();
    else ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
    UpdateImageThumbnail();
}

void GPITool::UpdateImageThumbnailAfterFindColours()
{
    if (!ihand->GetBestPalette(1.0, 0.0, 0.0))
    {
        ihand->DitherImage();
    }
    else
    {
        ihand->DitherImage(NODITHER, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, false);
    }
    ihand->ShufflePaletteBasedOnOccurrence();
    UpdateImageThumbnail();
}

int main(int argc, char* argv[])
{
    omp_set_num_threads(omp_get_max_threads());

    //TODO: put console interface redirect here (avoid Qt related stuff unless we need the GUI)

    QApplication app = QApplication(argc, argv);
    GPITool gpitool;
    gpitool.show();
    return app.exec();
}
