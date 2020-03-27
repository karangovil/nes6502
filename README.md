# NES Emulator

[![Build Status](https://travis-ci.org/karangovil/nes6502.svg?branch=master)](https://travis-ci.org/karangovil/nes6502)
[![codecov](https://codecov.io/gh/karangovil/nes6502/branch/master/graph/badge.svg)](https://codecov.io/gh/karangovil/nes6502)
[![MIT License][license-badge]](LICENSE.md)

A C++ project to build a NES (6502) emulator on linux following the [NES Emulator From Scratch](https://www.youtube.com/playlist?list=PLrOv9FMX8xJHqMvSGB_9G9nZZ_4IgteYf) by [javidx9](https://www.youtube.com/channel/UC-yuWVUplUJZvieEligKBkA) and [Nesdev Wiki](http://wiki.nesdev.com/w/index.php/Nesdev_Wiki).

The project also uses [olcPixelGameEngine](https://github.com/OneLoneCoder/olcPixelGameEngine) library (also developed by javidx9) for testing etc. olcPixelGameEngine is licensed by javidx9 under [OLC-3 license](https://github.com/OneLoneCoder/olcPixelGameEngine/blob/master/LICENCE.md).

## PPU Notes

The screen resolution in NES is 256x240 pixels and the PPU allows for displaying graphics by dividing the screen up into tiles made up of 8x8 pixels. These tiles are also referred to as sprites. 

### CHR Memory and Sprites

The sprites are stored in the 8KB "Pattern Memory" or "CHR" memory in the ROM. This memory is split into two 4KB chunks and PPU can select between either side.  Typically one side contains character sprites and the other contains background tiles e.g. scenery.

Each 4KB chunk has a 16x16 grid of sprites giving a total of 256 sprites on each side. Note that each sprite is 16 bytes (256 * 16 bytes = 4096 bytes = 4KB). Note that since this memory is on the ROM, mapper provides access to this memory and can selectively swap out sprites to provide animation.

Once we know the offset of a tile in the CHR memory, we can just read the next 16 bytes to get the appropriate tile/sprite data.

Each tile is a bitmap but each pixel is only 2 bits (2 * 8 * 8 = 128 bits = 16 bytes for a tile) giving a total of 4 colors per pixel. The information for the tiles is stored in two bit planes, one for least significant bit (LSB) and the other for most significant bit (MSB). The value of a specific pixel is obtained by concatenating the LSB and MSB from the respective locations in the bit planes to get a 2 bit number (pixel values of 0 represent transparency)

### Color Palettes

Now that we know the value in a pixel, we need to get the corresponding color from a palette. This is stored in Palette memory. It starts at 0x3F00 and that address contains the background color as an 8-bit value. Next entries in this memory map contain 4 bytes (but the 4th byte isn't used and typically is same as the background color to implement transparency). A "palette" in this map is 3 one byte colors and the NES contains 8 such palettes (4 for sprites and 4 for background)

|Address|             Purpose|
|------------|:----------------------:|
|$3F00	          |      Universal background color|
|$3F01-$3F03|	    Background palette 0|
|$3F05-$3F07|	    Background palette 1|
|$3F09-$3F0B|	    Background palette 2|
|$3F0D-$3F0F|	    Background palette 3|
|$3F11-$3F13 |    Sprite palette 0|
|$3F15-$3F17|	    Sprite palette 1|
|$3F19-$3F1B|	    Sprite palette 2|
|$3F1D-$3F1F|	    Sprite palette 3|

[license-badge]:   https://img.shields.io/badge/license-MIT-007EC7.svg
