bmp2tile-compressors
====================

Compression DLLs for [BMP2Tile](https://github.com/maxim-zhao/bmp2tile)

Overview
----

BMP2Tile supports plug-in compressors, somewhat inspired by Winamp. This repo holds all the ones I've written, plus some that were contributed by others. As far as is possible, I try to also include decompressors for use in homebrew and ROM hacks.

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

| Name | Link |
|------|------|
| ShrunkTileMap | https://github.com/sverx/STMcomp |  
