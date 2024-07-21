# General Planar Image

## A Planar Image Format

**General Planar Image** (GPI) is a planar image format I created to use in my retrocomputing projects. It is intended to be easy to use with systems that use planar VRAM, such as the VGA or the PC-98's GDC. It is *not* intended for systems that use packed-pixel VRAM (such as modern systems).

The format uses LZ4 compression with a pre-filter, as a compromise between compression ratio and decompression speed (it is vital that decompression speed is really fast). It can support up to 8 colour planes and an optional mask plane to be used for drawing transparent pixels (partial transparency is not supported and never will be). Images can use up to 256 distinct colours, no more since at that point you may as well use packed-pixel formats, like PNG.

This repository contains **gpitool** which is a GUI program (with a planned terminal interface) that converts images in modern formats (currently only PNG and JPEG) into GPI files, and **GPIVIEW** which is an MS-DOS-based 8086-compatible test program that allows you to view GPI files in their intended environment. Both are currently alpha software, so expect bugs. Please report any you find, of course.

## gpitool

gpitool is used to make GPI images from other image files. Currently only PNG and JPEG are supported for now. Other input image formats may be supported in the future, but don't bet on it. You can choose the palette freely and choose which planes to store, then the image will be compressed as well as possible. It's also possible to dither and make tilemap images. More options to come soon!

### Dependencies

Windows build: all dependencies included in the package.

Linux build:

- libpng (16)
- libjpeg (8)
- liblz4
- Qt5Widgets
- and all of their respective dependencies (your package manager should handle these automatically, most of these libraries will probably already be installed)

## GPIVIEW

GPIVIEW can be used to view GPI images in MS-DOS. Invoke it by simply typing:

```
GPIVIEW <filename>
```

There are no special options at the moment. Once you see an image you can use the following controls:

- Esc: Quit
- WASD: Move the image around if it's too big to fit on the screen
