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

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsPixmapItem>
#include "colourpickerwindow.h"
#include "ditherwindow.h"
#include "tilingwindow.h"
#include "imagehandler.h"
#include "imagecompressor.h"

class ColourPickerWindow;
class DitherWindow;
class TilingWindow;

class GPITool : public QMainWindow
{
    Q_OBJECT

public:
    explicit GPITool(QWidget *parent = nullptr);
    void SetNewImageThumbnail();
    void UpdateImageThumbnail();
    void UpdateImageThumbnailAfterDither();
    void UpdateImageThumbnailAfterFindColours();

private slots:
    void OnMenuFileOpen();
    void OnMenuFileExport();
    void OnMenuFileQuit();
    void OnMenuEditPalette();
    void OnMenuEditDither();
    void OnMenuEditTiling();
    void OnMenuHelpAbout();
    void OnMenuHelpAboutQt();
    void OnPaletteClose();
    void OnDitherClose();
    void OnTilingClose();

private:
    QGraphicsView* sceneView;
    QGraphicsScene* scene;
    QGraphicsPixmapItem* image;
    ColourPickerWindow* colPickWin;
    DitherWindow* dithWin;
    TilingWindow* tileWin;
    ImageHandler* ihand;
    ImageCompressor* icomp;

    bool colPickOpen;
    bool dithOpen;
    bool tileOpen;
};
