import os
import sys
import subprocess
import re
import glob
import json
import pickle
import matplotlib.pyplot
from brokenaxes import brokenaxes
import itertools
import statistics
import numpy

class Result:
    technology: str
    uncompressed: int
    compressed: int
    cycles: int
    ratio: float
    tiles_per_frame: float
    filename: str

    def __init__(self, technology, uncompressed, compressed, cycles, filename):
        self.technology = technology
        self.uncompressed = uncompressed
        self.compressed = compressed
        self.cycles = cycles
        self.ratio = (uncompressed - compressed) / uncompressed
        frames = cycles / (228 * 262) # One frame is 262 lines of 228 cycles each
        self.tiles_per_frame = uncompressed / frames / 32 # 32 bytes per tile
        self.filename = filename


def benchmark(technology, extension, rename_extension, asm_file, image_file):
    try:
        data_file = f"data.{extension}"

        # Run BMP2Tile
        # We extract commandline args from the image filename (!)
        extra_args = image_file.split(".")[1:-1]
        is_test = "test" in image_file
        subprocess.run([
            "bmp2tile.exe",
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
            "wla-z80.exe",
            "-o",
            "benchmark.o",
            asm_file],
            check=True, capture_output=True, text=True)

        # Link the file
        subprocess.run([
            "wlalink.exe",
            "-d", "-r", "-v", "-S", "-A",
            "linkfile",
            "benchmark.sms"],
            check=True, capture_output=True, text=True)

        # Benchmark it
        proc = subprocess.run([
            "z80bench.exe",
            'benchmark.sms',
            '--vram-compare=expected.bin'],
            check=True, capture_output=True, text=True)

        cycles = 0
        for line in iter(proc.stdout.splitlines()):
            match = re.search('Executed (?P<cycles>[0-9]+) cycles', line)
            if match is None:
                continue
            else:
                cycles = int(match.groups('cycles')[0])

        print(f"Test passed: {image_file} for {technology}. {os.stat('expected.bin').st_size}->{os.stat(data_file).st_size} in {cycles} cycles")

        if is_test:
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
        print(" ".join(e.cmd))
        print(e.stdout)
        print(e.stderr)
        if is_test:
            # Some compressors fail if they can't compress the extreme test images, we ignore these
            return None
        return e


def compute():
    results = []
    errors = []
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
                if isinstance(result, Result):
                    results.append(result)
                elif isinstance(result, Exception):
                    errors.append(result)

    results.extend(sevenzip())

    # Save results to file
    with open("benchmark-results.pickle", "wb") as f:
        pickle.dump(results, f)
    print("Saved data to benchmark-results.pickle")

    return results, errors


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
                "bmp2tile.exe",
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
                "7z.exe",
                "a",
                "-mx9",
                filename,
                "expected.bin"],
                check=True, capture_output=True, text=True)
            # Get 7z to tell us the compressed size
            proc = subprocess.run([
                "7z.exe",
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

    # First we extract the lines, uncompressed and the rest
    lines = [x for x in results if x.cycles < 0]
    uncompressed = [x for x in results if "Uncompressed" in x.technology]
    compressed = [x for x in results if x not in uncompressed and x not in lines]

    NUM_COLORS = len([x for x in itertools.groupby(compressed, lambda r: r.technology)])

    cm = matplotlib.colors.ListedColormap(matplotlib.cm.tab20b.colors + matplotlib.cm.tab20c.colors)
    colors = [cm(i) for i in range(NUM_COLORS)]
    index = 0

    compressed_xs = [x.tiles_per_frame for x in compressed]
    uncompressed_xs = [x.tiles_per_frame for x in uncompressed]
    minx = 0 # min(compressed_xs)
    maxx = max(compressed_xs)
    minx2 = min(uncompressed_xs) if len(uncompressed_xs) > 0 else maxx
    maxx2 = max(uncompressed_xs) if len(uncompressed_xs) > 0 else maxx+1
    bax = brokenaxes(xlims=((minx, maxx), (minx2, maxx2)), wspace=0.02, d=0.005)

    for line in lines:
        color = 'gray'
        bax.axhline(
            y=line.ratio,
            color=color,
            linestyle='--',
            linewidth = 0.5)
        bax.annotate(line.technology, (minx2, line.ratio), va='bottom', color=color, size='x-small')
        
    mean_points = []

    for technology, data in itertools.groupby(compressed, lambda r: r.technology):
        # Get data series and stats
        group_results = list(data)
        x = [r.tiles_per_frame for r in group_results]
        y = [r.ratio for r in group_results]
        mean_x = statistics.mean(x)
        mean_y = statistics.mean(y)
        if len(x) > 1:
            stdev_x = statistics.stdev(x)
            stdev_y = statistics.stdev(y)
        else:
            stdev_x = 0
            stdev_y = 0

        # Pick the next colour
        color = colors[index]
        index += 1

        # Draw the mean and stdev ellipse (if non-zero stdev)
        if stdev_x > 0 and stdev_y > 0:
            bax.errorbar(
                mean_x,
                mean_y,
                marker='.',
                color=color,
                xerr=stdev_x,
                yerr=stdev_y,
                elinewidth=0.5,
                capsize=4,
                capthick=0.5
            )

        # Draw the dots
        bax.plot(
            x,
            y,
            marker='.',
            markersize=2,
            linestyle='none',
            label=technology,
            color=color,
            zorder=index+100 # dots from 100
        )
        
        # Remember the point
        if not "Gaiden (fast)" in technology:
            mean_points.append((mean_x, mean_y))
        
    # Draw the "pareto front"
    # We want to select only points where there's no other point in its upper-right quadrant
    pareto_points = []
    for p1 in mean_points:
        is_good_point = True
        for p2 in mean_points:
            if p2[0] > p1[0] and p2[1] > p1[1]:
                # p1 should be ignored
                is_good_point = False
                break
        if is_good_point:
            pareto_points.append(p1)
    pareto_points.sort(key=lambda p: p[0])
    bax.plot([p[0] for p in pareto_points], [p[1] for p in pareto_points], linestyle="dotted", linewidth=0.5, color="black")

    for technology, data in itertools.groupby(uncompressed, lambda r: r.technology):
        group_results = list(data)
        x = [r.tiles_per_frame for r in group_results]
        y = [r.ratio for r in group_results]
        # Draw the dots and some text labels
        bax.plot(
            x,
            y,
            marker='.',
            markersize=2,
            linestyle='none',
            color='black',
            zorder=index+100 # dots from 100
        )
        bax.annotate(technology, (x[0], y[0] + 0.01), color='gray', fontsize='small', horizontalalignment='right' if 'fast' in technology else 'left')

    bax.set_xlabel("⬅ worse ️         Tiles per frame          better ➡️")
    bax.set_ylabel("⬅️ worse          Compression percentage          better ➡️")
    bax.axs[0].yaxis.set_major_formatter(matplotlib.ticker.PercentFormatter(1.0))
    bax.legend(markerscale=10, ncol=1, prop={'size': 9})
    bax.grid(axis='both', ls='dashed', alpha=0.4)
    bax.standardize_ticks(xbase=5)

    # Overall size
    matplotlib.pyplot.gcf().set_figwidth(11)
    matplotlib.pyplot.gcf().set_figheight(7)

    matplotlib.pyplot.savefig("../benchmark.svg", bbox_inches="tight", metadata={'Date': None})
    print("Saved chart to benchmark.svg")

    return matplotlib.pyplot


def main():
    # Change to file's dir as we glob in here
    os.chdir(os.path.dirname(__file__))
    
    # We make the SVG less randomised by setting these seeds
    matplotlib.rcParams['svg.hashsalt'] = '42'
    numpy.random.seed(42)

    args = ["compute", "plot", "show"] if len(sys.argv) == 1 else sys.argv[1:]

    if "compute" in args:
        data, errors = compute()
    else:
        with open("benchmark-results.pickle", "rb") as f:
            data = pickle.load(f)
            errors = []

    if len(errors) > 0:
        print(f"There were {len(errors)} errors")
        return 1;

    if "plot" in args:
        chart = plot(data)

    if "show" in args:
        chart.show()

    if "print" in args:
        zip_ratio = [x for x in data if x.technology == "zip"][0].ratio
        name_len = max([len(r.technology) for r in data])
        reference_tiles_per_frame = 88.6
        for technology, data in itertools.groupby(data, lambda r: r.technology):
            data = [x for x in data]
            rating = statistics.mean([r.ratio for r in data])/zip_ratio*100
            tiles_per_frame = statistics.mean([r.tiles_per_frame for r in data])
            relative_speed = tiles_per_frame / reference_tiles_per_frame * 100
            print(f"{technology:{name_len + 2}}{rating:.1f}\t{relative_speed:.0f}%")

main()