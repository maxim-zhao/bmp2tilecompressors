bmp2tile-compressors
====================

Compression DLLs for [BMP2Tile](https://github.com/maxim-zhao/bmp2tile)

[![Build status](https://ci.appveyor.com/api/projects/status/hbooun6oc0ux6ujh?svg=true)](https://ci.appveyor.com/project/maxim-zhao/bmp2tilecompressors)

Overview
----

BMP2Tile supports plug-in compressors, somewhat inspired by Winamp. This repo holds all the ones I've written, plus some that were contributed by others. As far as is possible, I try to also include decompressors for use in homebrew and ROM hacks.

Releases
----

Automated builds are available on the [Releases page](https://github.com/maxim-zhao/bmp2tilecompressors/releases).

Compressors
----

| DLL name |  Short name | Longer name | Description | Tiles supported | Tilemap supported |
|:---------|:------------|:------------|:------------|:-----:|:-------:|
| 1bppraw  | 1bpp        | 1bpp raw (uncompressed) binary | One bit per pixel tiles - discards upper bits   | ✅ |   |
| 2bppraw  | 2bpp        | 2bpp raw (uncompressed) binary | Two bits per pixel tiles - discards upper bits  | ✅ |   |
| 3bppraw  | 3bpp        | 3bpp raw (uncompressed) binary | Three bits per pixel tiles - discards upper bit | ✅ |   |
| aPLib    | aPLib       | aPLib | [aPLib](http://ibsensoftware.com/products_aPLib.html) compression library | ✅ | ✅ |
| apultra  | apultra     | aPLib (apultra) | [apultra](https://github.com/emmanuel-marty/apultra) aPLib compressor - better compression for the same format | ✅ | ✅ |
| berlinwall | berlinwallcompr | Berlin Wall LZ | Compression from the game [The Berlin Wall](http://www.smspower.org/Games/BerlinWall-GG) | ✅ | ✅ |
| exe      | (configurable) | (configurable) | Wraps arbitrary external programs, passing data via files. This is useful if you do not want to implement your algorithm in the form of a DLL. | ✅ | ✅ |
| exomizerv2 | exomizer  | Exomizer v2 | [Exomizer](https://bitbucket.org/magli143/exomizer/wiki/Home) v2 compression ⚠ Seems to crash on some inputs | ✅ | ✅ |
| highschoolkimengumi | hskcompr | High School Kimengumi RLE | Compression from the game [High School! Kimengumi](http://www.smspower.org/Games/HighSchoolKimengumi-SMS) | ✅ | ✅ |
| lemmings | lemmingscompr | Lemmings RLE | Compression from the game [Lemmings](http://www.smspower.org/Games/Lemmings-SMS) | ✅ | ✅ |
| lsb      | lsbtilemap  | LSB-only tilemap | Least significant byte of tilemap data |   | ✅ |
| lz4      | lz4         | LZ4 (raw) | [LZ4](http://www.lz4.org/) compression library, using [smallz4](https://create.stephan-brumme.com/smallz4/). [lz4ultra](https://github.com/emmanuel-marty/lz4ultra) has negligibly different results. | ✅ | ✅ |
| lzsa1    | lzsa1       | LZSA1 | [LZSA](https://github.com/emmanuel-marty/lzsa) compression library | ✅ | ✅ |
| lzsa2    | lzsa2       | LZSA2 | [LZSA](https://github.com/emmanuel-marty/lzsa) compression library | ✅ | ✅ |
| magicknight | mkre2compr | Magic Knight Rayearth 2 RLE or LZ | Compressor from the game [魔法騎士レイアース２ ～making of magic knight～](https://www.smspower.org/Games/MagicKnightRayearth2-GG) | ✅ |  |
| oapack   | oapack      | aPLib (oapack) | [oapack](https://gitlab.com/eugene77/oapack) aPLib compressor - better compression for the same format | ✅ | ✅ |
| phantasystar | pscompr | Phantasy Star RLE | Compression from the game [Phantasy Star](http://www.smspower.org/Games/PhantasyStar-SMS) | ✅ | ✅ |
| psgaiden | psgcompr    | PS Gaiden | Compression from the game [Phantasy Star Gaiden](http://www.smspower.org/Games/PhantasyStarGaiden-GG) | ✅ |   |
| pucrunch | pucrunch    | Pucrunch | [Pucrunch](http://a1bert.kapsi.fi/Dev/pucrunch/) algorithm | ✅ | ✅ |
| raw      | bin         | Raw (uncompressed) binary | Does no compression at all | ✅ | ✅ |
| shrinkler | shrinkler | Shrinkler | [Shrinkler](https://github.com/askeksa/Shrinkler) Amiga executable compressor | ✅ | ✅ |
| sonic1   | soniccompr  | Sonic 1 | Tile compression from the game [Sonic the Hedgehog](http://www.smspower.org/Games/SonicTheHedgehog-SMS) | ✅ |   |
| sonic2   | sonic2compr | Sonic 2 | Tile compression from the game [Sonic the Hedgehog 2](http://www.smspower.org/Games/SonicTheHedgehog2-SMS) | ✅ |   |
| stc0     | stc0compr   | Simple Tile Compression 0 | @sverx's [stc0](https://github.com/sverx/stc0) | ✅ |   |
| zx0      | zx0         | ZX0 | [ZX0](https://github.com/einar-saukas/ZX0) compression library | ✅ | ✅ |
| zx7      | zx7         | ZX7 (8-bit limited) | Variant of [ZX7](http://www.worldofspectrum.org/infoseekid.cgi?id=0027996) compression library tweaked for performance | ✅ | ✅ |

Decompressors
----

All size stats are for emitting data direct to VRAM on Master System, using Z80 decompressors in non-interrupt-safe mode if available. Decompression to RAM will generally use less ROM.

| Description              | ROM (bytes) | RAM (bytes, not including stack) | Interrupt-safe | Non-VRAM support |
|:-------------------------|------------:|---------------------------------:|:--------------:|:----------------:|
| aPLib                    |         303 |                                5 |       ❌       |       ✅        |
| aPLib (fast)             |         333 |                                0 |       ❌       |       ✅        |
| Berlin Wall              |             |                              265 |       ❌       |       ❌        |
| Exomizer v2 (⚠ Broken)  |         208 |                              156 |       ❌       |       ✅        |
| High School Kimengumi (unoptimised) | 119 |                             4 |       ❌       |       ❌        |
| Lemmings (unoptimised)   |         143 |                              512 |       ❌       |       ❌        |
| LZ4                      |         136 |                                0 |       ❌       |       ❌        |
| LZSA1                    |         207 |                                0 |       ❌       |       ✅        |
| LZSA2                    |         332 |                                0 |       ❌       |       ✅        |
| Magic Knight Rayearth 2  |         139 |                                0 |       ❌       |       ❌       |
| Phantasy Star RLE        |         188 |                                0 |       ✅       |       ❌       |
| PS Gaiden                |         223 |                               34 |       ❌       |       ❌       |
| PS Gaiden (fast)         |        1028 |                               32 |       ❌       |       ❌       |
| Pucrunch (⚠ Broken)     |         412 |                               44 |       ❌       |       ❌       |
| Shrinkler                |         259 |                             2048 |       ❌       |       ✅     |
| Sonic                    |         162 |                                8 |       ❌       |       ❌       |
| Sonic 2                  |         289 |                               39 |       ❌       |       ❌       |
| Simple tile Compression 0 |         57 |                                0 |       ❌       |       ❌       |
| ZX0                      |         157 |                                0 |       ❌       |       ✅       |
| ZX0 (fast)               |         274 |                                0 |       ❌       |       ✅       |
| ZX7                      |         117 |                                0 |       ✅       |       ✅       |

Note that the technologies marked with ⚠ above fail the automated benchmark tests, with crashes in the compressor or incorrect decompressed output. 
They could be fixed but as they are rather old, they are probably not competitive with newer compressors.

Interrupt-safe decompressors can run safely while interrupts are happening and interfering with the VDP state. 
This will introduce some overhead - if nothing else, they need to disable interrupts - and they will therefore also change the interrupts enabled state.

Some compressors have assembly-time definitions to switch between decompressing directly to VRAM on SMS and decompressing to RAM.

Benchmark
----

Based on a benchmark corpus made of tilesets and title screens, here's a scatter of the results.

![](/benchmark.svg?raw=true)

How to read:

- Further to the right is faster
- Further up is better compression
- 60% compression means that the compressed data is 40% of the size of the uncompressed data (for example)
- The scatter for a particular colour group is across a corpus of realistic Master System tile data
- Each colour group has an ellipse showing the standard deviation, with a + in the middle showing the mean, across the test corpus.

There can be wide variation in performance between compressors depending on your data; you may want to try a few options for your specific use case.

Other compressors
----

Here's some other compressors people have made.

| Name | Link | Description | Tiles supported | Tilemap supported |
|:-----|:-----|:------------|:---------------:|:-----------------:|
| ShrunkTileMap | https://github.com/sverx/STMcomp | Compresses tilemaps with specific support for sequential tile indices |   | ✅ |
