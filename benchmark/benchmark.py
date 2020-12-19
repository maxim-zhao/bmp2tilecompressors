import os
import subprocess
import re
import glob
import json
import matplotlib.pyplot
import itertools

WLA_PATH = "C:\\Users\\maxim\\Documents\\Code\\C\\wla-dx\\binaries"
BMP2TILE_PATH = "C:\\Users\\maxim\\Documents\\Code\\C#\\bmp2tile"
Z80BENCH_PATH = "C:\\Users\\maxim\\Documents\\Code\\C#\\z80bench\\bin\\Release"


class Result:
    technology: str
    uncompressed: int
    compressed: int
    cycles: int
    ratio: float
    bytes_per_frame: float
    filename: str

    def __init__(self, technology, uncompressed, compressed, cycles, filename):
        self.technology = technology
        self.uncompressed = uncompressed
        self.compressed = compressed
        self.cycles = cycles
        self.ratio = (1.0 * uncompressed - compressed) / uncompressed
        frames = cycles / 59736.0
        self.bytes_per_frame = uncompressed / frames
        self.filename = filename


def benchmark(technology, extension, rename_extension, asm_file, image_file):
    try:
        data_file = f"data.{extension}"
        # Run BMP2Tile
        subprocess.run([
            os.path.join(BMP2TILE_PATH, "bmp2tile.exe"),
            image_file,
            "-savetiles",
            data_file,
            "-savetiles",
            "expected.bin"],
            check=True, capture_output=True, text=True)

        # Rename output file to expected name
        if rename_extension is not None and rename_extension != extension:
            dest = f"data.{rename_extension}"
            if os.path.exists(dest):
                os.remove(dest)
            os.rename(data_file, dest)
            data_file = dest

        # Assemble the file
        subprocess.run([
            os.path.join(WLA_PATH, "wla-z80.exe"),
            "-o",
            "benchmark.o",
            asm_file],
            check=True, capture_output=True, text=True)

        # Link the file
        subprocess.run([
            os.path.join(WLA_PATH, "wlalink.exe"),
            "-d", "-r", "-v", "-S", "-A",
            "linkfile",
            "benchmark.sms"],
            check=True, capture_output=True, text=True)

        # Benchmark it
        proc = subprocess.run([
            os.path.join(Z80BENCH_PATH, "z80bench.exe"),
            'benchmark.sms',
            '--vram-compare',
            'expected.bin'],
            check=True, capture_output=True, text=True)

        cycles = 0
        for line in iter(proc.stdout.splitlines()):
            match = re.search('Executed (?P<cycles>[0-9]+) cycles', line)
            if match is None:
                continue
            else:
                cycles = int(match.groups('cycles')[0])

        return Result(
            technology,
            os.stat('expected.bin').st_size,
            os.stat(data_file).st_size,
            cycles,
            image_file
        )
    except subprocess.CalledProcessError as e:
        print(e)
        print(e.stdout)
        print(e.stderr)


def main():
    results = []
    for benchmark_file in glob.glob("benchmark-*.asm"):
        # Open the file and check the formats we want to use
        with open(benchmark_file) as file:
            first_line = file.readline()
        json_position = first_line.find("{")
        if json_position < 0:
            print(f"Ignoring {benchmark_file} as it has no metadata")
            continue
        json_data = json.loads(first_line[json_position:])
        extension = json_data["extension"]
        extensions = [extension]
        if "extra-extensions" in json_data:
            extensions.extend(json_data["extra-extensions"])
        for test_extension in extensions:
            for image in itertools.chain(glob.iglob("corpus/*.png"), glob.iglob("corpus/*.bin")):
                result = benchmark(
                    json_data["technology"],
                    test_extension,
                    extension,
                    benchmark_file,
                    image)
                if result is not None:
                    print(f'{result.technology}\t{test_extension}+{benchmark_file}+{image.replace(" ", "_")}\t{result.cycles}\t{result.uncompressed}\t{result.compressed}\t{result.ratio}\t{result.bytes_per_frame}')
                    results.append(result)

    # Now plot the results
    for technology, data in itertools.groupby(results, lambda r: r.technology):
        group_results = list(data)
        matplotlib.pyplot.plot(
            [r.bytes_per_frame for r in group_results],
            [r.ratio for r in group_results],
            marker='.',
            linestyle='none',
            label=technology
        )
    matplotlib.pyplot.xlabel("Bytes per frame")
    matplotlib.pyplot.xscale("log")
    matplotlib.pyplot.minorticks_on
    matplotlib.pyplot.gca().xaxis.set_major_formatter(matplotlib.ticker.ScalarFormatter())
    matplotlib.pyplot.gca().xaxis.set_minor_formatter(matplotlib.ticker.ScalarFormatter())
    matplotlib.pyplot.ylabel("Compression level")
    matplotlib.pyplot.legend()
    matplotlib.pyplot.show()
    
    

main()