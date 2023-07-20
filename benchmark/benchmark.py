import os
import sys
import subprocess
import re
import glob
import json
import pickle
import matplotlib.pyplot
from matplotlib.patches import Ellipse
import itertools
import statistics
import numpy

WLA_PATH = "C:\\Users\\maxim\\Documents\\Code\\C\\wla-dx\\binaries"
BMP2TILE_PATH = "C:\\Users\\maxim\\Documents\\Code\\C#\\bmp2tile"
Z80BENCH_PATH = "C:\\Users\\maxim\\Documents\\Code\\C#\\z80bench\\bin\\Release"
SEVENZIP_PATH = "C:\\Program Files\\7-Zip\\7z.exe"

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
        self.ratio = (uncompressed - compressed) / uncompressed
        frames = cycles / 59736
        self.bytes_per_frame = uncompressed / frames
        self.filename = filename
        

def benchmark(technology, extension, rename_extension, asm_file, image_file):
    try:
        data_file = f"data.{extension}"

        # Run BMP2Tile
        # We extract commandline args from the image filename (!)
        extra_args = image_file.split(".")[1:-1]
        is_test = "test" in image_file
        subprocess.run([
            os.path.join(BMP2TILE_PATH, "bmp2tile.exe"), 
            image_file] 
            + extra_args + [
            image_file,
            "-savetiles",
            data_file,
            "-savetiles",
            "expected.bin"], check=True, capture_output=True, text=True)

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

        if is_test:
            print(f"Test passed: {image_file} for {technology}. {os.stat(data_file).st_size}->{os.stat('expected.bin').st_size} in {cycles} cycles")
            return None

        return Result(
            technology,
            os.stat('expected.bin').st_size,
            os.stat(data_file).st_size,
            cycles,
            image_file
        )
    except subprocess.CalledProcessError as e:
        print(f"Failed while processing {image_file} for {technology}:")
        print(e)
        print(e.stdout)
        print(e.stderr)
        
        
def compute():
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
                    
    results.extend(sevenzip())
                    
    # Save results to file
    with open("benchmark-results.pickle", "wb") as f:
        pickle.dump(results, f)

    return results
    
    
def sevenzip():
    # Now try 7-zip on the lot
    for ext in ["zip", "7z"]:
        uncompressed = []
        compressed = []
        for image_file in itertools.chain(glob.iglob("corpus/*.png"), glob.iglob("corpus/*.bin")):
            if "test" in image_file:
                # Skip test files
                continue
            # Make uncompressed version
            subprocess.run([
                os.path.join(BMP2TILE_PATH, "bmp2tile.exe"), 
                image_file,
                "-savetiles",
                "expected.bin"], check=True, capture_output=True, text=True)
            # Make filename
            filename = f".temp.{ext}"
            # Delete file if there
            if os.path.exists(filename):
                os.remove(filename)
            # Compress it
            subprocess.run([
                SEVENZIP_PATH,
                "a",
                "-mx9",
                filename,
                "expected.bin"],
                check=True, capture_output=True, text=True)
            # Get 7z to tell us the compressed size
            proc = subprocess.run([
                SEVENZIP_PATH,
                'l',
                filename],
                check=True, capture_output=True, text=True)

            for line in iter(proc.stdout.splitlines()):
                match = re.search(r" +(\d+) +(\d+) +1 files", line)
                if match is not None:
                    uncompressed.append(int(match.group(1)))
                    compressed.append(int(match.group(2)))
                    break
        # Print a result
        # The overall average compression is not the same as the average of the items' compressions.
        # As this is what we show elsewhere, we compute that instead.
        total = sum(uncompressed)
        total_compressed = sum(compressed)
        total_compression = (total - total_compressed) / total
        average_compression = statistics.mean([(u-c)/u for u, c in zip(uncompressed, compressed)])
        print(f"{ext}: {total} => {total_compressed} = {total_compression * 100}% overall, {average_compression * 100}% average")
        # Then we need to manipulate a number to recompute that :)
        # If average_compression = (u-c)/u, if u = 1 then we want to compute c = 1-a
        yield Result(
            ext,
            1,
            1 - average_compression,
            -1,
            "line")


        
def plot(results):
    # Now plot the results
    
    # First we extract the lines
    lines = [x for x in results if x.cycles < 0]
    
    NUM_COLORS = len([x for x in itertools.groupby(results, lambda r: r.technology)])

    cm = matplotlib.pyplot.get_cmap('tab20')
    colors = [cm(1.*i/NUM_COLORS) for i in range(NUM_COLORS)]
    index = 0
    
    for line in lines:
        color = colors[index]
        index += 1
        matplotlib.pyplot.axhline(y=line.ratio, color=color, linestyle='--', label=line.technology)

    results = [x for x in results if x.cycles > 0]
    for technology, data in itertools.groupby(results, lambda r: r.technology):
        # Get data series and stats
        group_results = list(data)
        x = [r.bytes_per_frame for r in group_results]
        y = [r.ratio for r in group_results]
        mean_x = statistics.mean(x)
        mean_y = statistics.mean(y)
        if len(x) > 1:
            stdev_x = statistics.stdev(x)
            stdev_y = statistics.stdev(y)

        # Pick the next colour
        color = colors[index]
        index += 1

        # Draw the mean and stdev ellipse (if non-zero stdev)
        if len(x) > 1 and stdev_x > 0 and stdev_y > 0:
            matplotlib.pyplot.gca().add_artist(Ellipse(
                xy=[mean_x, mean_y],
                width=stdev_x,
                height=stdev_y,
                fill=False,
                edgecolor=color,
                linestyle='-',
                zorder = index-200 # SDs at the bottom
            ))
            matplotlib.pyplot.plot(
                mean_x,
                mean_y,
                marker='+',
                color=color,
                label='_', # hide from legend
                zorder=index+200 # means above dots
            )

        # Draw the dots
        matplotlib.pyplot.plot(
            x,
            y,
            marker='.',
            markersize=2,
            linestyle='none',
            label=technology,
            color=color,
            zorder=index+100 # dots from 100
        )

    matplotlib.pyplot.xlabel("⬅ worse ️         Bytes per frame          better ➡️")
    #matplotlib.pyplot.xscale("log")
    #matplotlib.pyplot.minorticks_on
    matplotlib.pyplot.gca().xaxis.set_major_formatter(matplotlib.ticker.ScalarFormatter())
    matplotlib.pyplot.gca().xaxis.set_minor_formatter(matplotlib.ticker.ScalarFormatter())
    matplotlib.pyplot.ylabel("⬅️ worse          Compression percentage          better ➡️")
    matplotlib.pyplot.gca().yaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
    matplotlib.pyplot.legend(markerscale=10)
    matplotlib.pyplot.grid(axis='both', which='major', ls='dashed', alpha=0.4)

    # Overall size
    matplotlib.pyplot.gcf().set_figwidth(10)
    matplotlib.pyplot.gcf().set_figheight(6)
    
    matplotlib.pyplot.savefig("../benchmark.png", bbox_inches="tight")
    matplotlib.pyplot.savefig("../benchmark.svg", bbox_inches="tight")

    matplotlib.pyplot.show()



def main():
    verb = sys.argv[1] if len(sys.argv) > 1 else "compute"
    
    if verb == "compute":
        plot(compute())
    elif verb == "plot":
        with open("benchmark-results.pickle", "rb") as f:
            plot(pickle.load(f))
    elif verb == "sevenzip":
        [x for x in sevenzip()]

main()