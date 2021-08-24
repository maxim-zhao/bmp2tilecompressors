#include <cstdint>
#include <algorithm>
#include <cstdio>

// Pucrunch's main function
int main(int argc, char *argv[]);

int compress(const uint8_t* pSource, const size_t sourceLength, uint8_t* pDestination, const size_t destinationLength)
{
    // Pucrunch only likes to work on files so we have to write them to disk...
    char inFilename[L_tmpnam_s];
    if (tmpnam_s(inFilename) != 0)
    {
        return -1;
    }
    FILE* f;
    if (fopen_s(&f, inFilename, "wb") != 0)
    {
        return -1;
    }
    fwrite(pSource, 1, sourceLength, f);
    fclose(f);

    char outFilename[L_tmpnam_s];
    if (tmpnam_s(outFilename) != 0)
    {
        remove(inFilename);
        return -1;
    }

    // We invoke the Pucrunch main function directly...
    const char* argv[] = { "", "-d", "-c0", inFilename, outFilename };
    const auto status = main(5, const_cast<char**>(argv));  // NOLINT(clang-diagnostic-main)

    remove(inFilename);

    if (status != 0)
    {
        remove(outFilename);
        return -1;
    }

    // Read the file back in again
    if (fopen_s(&f, outFilename, "rb") != 0 || 
        fseek(f, 0, SEEK_END) != 0)
    {
        fclose(f);
        remove(outFilename);
        return -1;
    }

    const size_t compressedSize = ftell(f);

    if (compressedSize > destinationLength)
    {
        fclose(f);
        remove(outFilename);
        return 0; // Buffer too small
    }

    if (fseek(f, 0, SEEK_SET) != 0 ||
        fread_s(pDestination, destinationLength, 1, compressedSize, f) != compressedSize)
    {
        fclose(f);
        remove(outFilename);
        return -1;
    }

    fclose(f);
    remove(outFilename);
    return static_cast<int>(compressedSize);
}

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    // Generally, the name of the game it was REd from
    return "pucrunch";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "pucrunch";
}

extern "C" __declspec(dllexport) int compressTiles(const uint8_t* pSource, const uint32_t numTiles, uint8_t* pDestination, const uint32_t destinationLength)
{
    // Compress tiles
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int compressTilemap(const uint8_t* pSource, const uint32_t width, uint32_t height, uint8_t* pDestination, const uint32_t destinationLength)
{
    // Compress tilemap
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
