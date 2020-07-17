#include <cstdint>
#include <algorithm>
#include <cstdio>

// Pucrunch's main function
int main(int argc, char *argv[]);

int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
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
    fwrite(source, 1, sourceLen, f);
    fclose(f);

    char outFilename[L_tmpnam_s];
    if (tmpnam_s(outFilename) != 0)
    {
        remove(inFilename);
        return -1;
    }

    // We invoke the Pucrunch main function directly...
    const char* args[] = { "-d", "-c0", inFilename, outFilename };
    const auto status = main(sizeof(args), const_cast<char**>(args));

    remove(inFilename);

    if (status != 0)
    {
        remove(outFilename);
        return -1;
    }

    // Read the file back in again
    if (fopen_s(&f, outFilename, "rb") != 0 && 
        fseek(f, 0, SEEK_END) != 0)
    {
        remove(outFilename);
        return -1;
    }

    const size_t compressedSize = ftell(f);
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        remove(outFilename);
        return -1;
    }

    if (compressedSize > destLen)
    {
        remove(outFilename);
        return 0; // Buffer too small
    }

    if (fread_s(dest, destLen, 1, compressedSize, f) != compressedSize)
    {
        fclose(f);
        remove(outFilename);
        return -1;
    }

    fclose(f);
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

extern "C" __declspec(dllexport) uint32_t compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest,
                                                        uint32_t destinationLength)
{
    // Compress tiles
    return compress(source, numTiles * 32, dest, destinationLength);
}

extern "C" __declspec(dllexport) uint32_t compressTilemap(uint8_t* source, uint32_t width, uint32_t height,
                                                          uint8_t* dest, uint32_t destLen)
{
    // Compress tilemap
    return compress(source, width * height * 2, dest, destLen);
}
