#include <cstdint> // uint8_t, etc
#include <cstdlib> // free
#include <memory>

#include "utils.h"

extern unsigned char *d;
extern struct metadata {
  unsigned reeks;
  unsigned cpos[7],clen[7];
} *m;
void initvarcost();
void createmetadata();
extern struct pakdata {
  unsigned cost,mode,mlen;
} *p[7];
int getlen(pakdata *p, unsigned q);

extern "C" __declspec(dllexport) const char* getName()
{
    // A pretty name for this compression type
    return "Pletter";
}

extern "C" __declspec(dllexport) const char* getExt()
{
    // A string suitable for use as a file extension
    return "pletter";
}

int pletter_main(const unsigned char* source, int len, unsigned char* dest, int destLen);

// The actual compressor function, calling into the pletter code
int32_t compress(
    const uint8_t* pSource,
    const size_t sourceLength,
    uint8_t* pDestination,
    const size_t destinationLength)
{
    return pletter_main(pSource, sourceLength, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int32_t compressTiles(
    const uint8_t* pSource,
    const uint32_t numTiles,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    return compress(pSource, numTiles * 32, pDestination, destinationLength);
}

extern "C" __declspec(dllexport) int32_t compressTilemap(
    const uint8_t* pSource,
    const uint32_t width,
    const uint32_t height,
    uint8_t* pDestination,
    const uint32_t destinationLength)
{
    return compress(pSource, width * height * 2, pDestination, destinationLength);
}
