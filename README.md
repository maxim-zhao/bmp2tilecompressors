bmp2tile-compressors
====================

Compression DLLs for [BMP2Tile](https://github.com/maxim-zhao/bmp2tile). [![Build status](https://ci.appveyor.com/api/projects/status/hbooun6oc0ux6ujh?svg=true)](https://ci.appveyor.com/project/maxim-zhao/bmp2tilecompressors)


Overview
----

BMP2Tile supports plug-in compressors, somewhat inspired by Winamp. This repo holds all the ones I've written, plus some that were contributed by others. As far as is possible, I try to also include decompressors for use in homebrew and ROM hacks.

Releases
----

Automated builds are available on the [Releases page](https://github.com/maxim-zhao/bmp2tilecompressors/releases).

Compressors
----

| DLL name |  Short name | Longer name | Description | Tiles supported | Tilemap supported |
|:---------|:------------|:------------|:------------|-------|---------|
| 1bppraw  | 1bpp        | 1bpp raw (uncompressed) binary | One bit per pixel tiles - discards upper bits   | ✅ |   |
| 2bppraw  | 2bpp        | 2bpp raw (uncompressed) binary | Two bits per pixel tiles - discards upper bits  | ✅ |   |
| 3bppraw  | 3bpp        | 3bpp raw (uncompressed) binary | Three bits per pixel tiles - discards upper bit | ✅ |   |
| aPLib    | aPLib       | aPLib | [aPLib](http://ibsensoftware.com/products_aPLib.html) compression library | ✅ | ✅ |
| exe      | (configurable) | (configurable) | Wraps arbitrary external programs, passing data via files. This is useful if you do not want to implement your algorithm in the form of a DLL. | ✅ | ✅ |
| exomizerv2 | exomizer  | Exomizer v2 | [Exomizer](https://bitbucket.org/magli143/exomizer/wiki/Home) v2 compression | ✅ | ✅ |
| highschoolkimengumi | hskcompr | High School Kimengumi RLE | Compression from the game [High School! Kimengumi](http://www.smspower.org/Games/HighSchoolKimengumi-SMS) | ✅ | ✅ |
| lsb      | lsbtilemap  | LSB-only tilemap | Least significant byte of tilemap data |   | ✅ |
| lz4      | lz4         | LZ4 (raw) | [LZ4](http://www.lz4.org/) compression library | ✅ | ✅ |
| phantasystar | pscompr | Phantasy Star RLE | Compression from the game [Phantasy Star](http://www.smspower.org/Games/PhantasyStar-SMS) | ✅ | ✅ |
| psgaiden | psgcompr    | PS Gaiden | Compression from the game [Phantasy Star Gaiden](http://www.smspower.org/Games/PhantasyStarGaiden-GG) | ✅ |   |
| raw      | bin         | Raw (uncompressed) binary | Does no compression at all | ✅ | ✅ |
| sonic1   | soniccompr  | Sonic 1 | Tile compression from the game [Sonic the Hedgehog](http://www.smspower.org/Games/SonicTheHedgehog-SMS) | ✅ |   |
| zx7      | zx7         | ZX7 (8-bit limited) | Variant of [ZX7](http://www.worldofspectrum.org/infoseekid.cgi?id=0027996) compression llibrary tweaked for performance | ✅ | ✅ |

Benchmark
----

Based on a crude benchmark (the large tile chunk used by [SMS Competition Cart](https://github.com/maxim-zhao/sms-competition-cart)), here's some (incomplete) compression ratios and decompression times:

| Compression | Data size | Ratio | Decompressor size | Load time (cycles) | Ratio |
|:------------|----------:|------:|------------------:|-------------------:|------:|
| None        |      9728 | 100%  |                24 |             161365 |  100% |
| PScompr     |      8338 |  86%  |                54 |            1335193 |  827% |
| Sonic 1     |      5507 |  57%  |               162 |            1011588 |  627% |
| PSGcompr    |      5029 |  52%  |               223 |            1576965 |  977% |
| PuCrunch    |      4005 |  41%  |               414 |            3394510 | 2104% |
| zx7         |      3995 |  41%  |               117 |            1513653 |  938% |
| aPLib       |      3946 |  41%  |               304 |            3552372 | 2201% |
| aPLib-fast  |      3946 |  41%  |               334 |            1789523 | 1109% |
| exomizer    |      3668 |  38%  |               208 |            3153906 | 1955% |

This was mostly done manually, using [Meka](http://www.smspower.org/meka/)'s CLOCK debugger feature for cycle counting. It would be better to have an automated benchmark for it, which could then cover a better corpus of source data. There is scope for optimising the "None" case (for speed), but that would just scale the other results up - which might be a useful thing to do.

Other compressors
----

Here's some other compressors people have made.

| Name | Link | Description | Tiles supported | Tilemap supported |
|:-----|:-----|:------------|-----------------|-------------------|
| ShrunkTileMap | https://github.com/sverx/STMcomp | Compresses tilemaps with specific support for sequential tile indices |   | ✅ |
