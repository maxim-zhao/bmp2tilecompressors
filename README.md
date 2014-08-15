bmp2tile-compressors
====================

Compression DLLs for [BMP2Tile](https://github.com/maxim-zhao/bmp2tile)

Overview
----

BMP2Tile supports plug-in compressors, somewhat inspired by Winamp. This repo holds all the ones I've written, plus some that were contributed by others. As far as is possible, I try to also include decompressors for use in homebrew and ROM hacks.

Benchmark
----

Based on a crude benchmark (the large tile chunk used by [SMS Competition Cart](https://github.com/maxim-zhao/sms-competition-cart)), here's some compression ratios and decompression times:

| Compression | Bytes | Ratio | Load time (cycles) | Ratio |
|-------------|-------|-------|--------------------|-------|
| None        |  9888 |  100% |             494435 |  100% |
| PScompr     |  8472 |   86% |            1354926 |  274% |
| Sonic 1     |  5616 |   57% |            1026969 |  207% |
| PSGcompr    |  5116 |   52% |            1600820 |  324% |
| PuCrunch    |  4109 |   42% |            3475587 |  703% |
| aPLib       |  4058 |   41% |            3640882 |  736% |

This was mostly done manually, using [Meka](http://www.smspower.org/meka/)'s CLOCK debugger feature for cycle counting. It would be better to have an automated benchmark for it, which could then cover a better corpus of source data. There is scope for optimising the "None" case (for speed), but that would just scale the other results up - which might be a useful thing to do.
