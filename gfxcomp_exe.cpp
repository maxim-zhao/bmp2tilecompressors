// This plugin looks at its own filename and uses that to find a config file that tells it how to work.
// The config file looks like this:
//
// [Settings]
// Name=Some name (for display purposes)
// Command=program.exe "%input%" "%output%"
//
// You can put anything in the Command, %input% and %output% will be substituted with filenames
// (possibly with spaces). Non-zero return codes will be interpreted as an error.
// TODO: allow stdin/stdout?

#pragma warning(push,3)
#include <windows.h>
#include <cstdint>
#include <string>
#include <sstream>
#pragma warning(pop)

using namespace ::std;

HINSTANCE g_hInstance;

extern "C" BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD, LPVOID)
{
	g_hInstance = hInst;
	return TRUE;
}

string getConfigFilename()
{
	static string filename;
	if (!filename.empty())
	{
		return filename;
	}

	char* buffer = NULL;
	unsigned int bufferSize = 256;
	bool success = false;
	while (!success && bufferSize < 1024*1024*1024)
	{
		delete [] buffer;
		buffer = new char[bufferSize];

		// Try to get the DLL filename.
		// This returns the number of chars copied on success, 0 otherwise.
		DWORD result = GetModuleFileName(g_hInstance, buffer, bufferSize);
		success = result != bufferSize;
		bufferSize *= 2;
	}
	filename = std::string(buffer);
	delete [] buffer;
	// We append .ini
	filename += ".ini";
	return filename;
}

string getSetting(const string& name)
{
	const std::string& configFilename = getConfigFilename();
	std::string setting;

	char* buffer = NULL;
	unsigned int bufferSize = 256;
	bool success = false;
	while (!success)
	{
		delete [] buffer;
		buffer = new char[bufferSize];

		// Try to get the setting value.
		// This returns the number of chars copied on success, 0 otherwise.
		DWORD result = GetPrivateProfileString("Settings", name.c_str(), NULL, buffer, bufferSize, configFilename.c_str());
		success = result != bufferSize;
	}
	setting = std::string(buffer);
	delete [] buffer;
	return setting;
}

int getSettingInt(const string& name, int default)
{
	const string& configFilename = getConfigFilename();
	return GetPrivateProfileInt("Settings", name.c_str(), default, configFilename.c_str());
}

extern "C" __declspec(dllexport) const char* getName()
{
	static string name = getSetting("Name").c_str();
	return name.c_str();
}

extern "C" __declspec(dllexport) const char* getExt()
{
	static string extension;
	if (!extension.empty())
	{
		return extension.c_str();
	}
	const string& configFilename = getConfigFilename();
	string::size_type pos = configFilename.find("gfxcomp_") + 8;
	extension = configFilename.substr(pos);
	pos = extension.find('.');
	extension = extension.substr(0, pos);
	return extension.c_str();
}

void replace(string& haystack, const string& needle, const string& replacement)
{
	for (string::size_type pos = haystack.find(needle); pos != string::npos; pos = haystack.find(needle))
	{
		// Replace while we find it
		// This can go on forever with badly-chosen needles and replacements...
		haystack.replace(pos, needle.length(), replacement);
	}
}

int compress(uint8_t* source, uint32_t sourceLen, uint8_t* dest, uint32_t destLen)
{
	// We get a couple of temp filenames...
	char tempPathBuf[MAX_PATH];
	DWORD tempPathResult = GetTempPath(MAX_PATH, tempPathBuf);
	if (tempPathResult == 0 || tempPathResult >= MAX_PATH)
	{
		return -1;
	}
	ostringstream ss;
	ss << tempPathBuf << "gfxcomp_exe_temp_in_" << rand() << ".bin";
	string inFilename(ss.str());
	ss.str(string());
	ss << tempPathBuf << "gfxcomp_exe_temp_out_" << rand() << ".bin";
	string outFilename(ss.str());

	// We write the data to the first filename...
	// Since we're using Windows API for creating the process, let's use it for files too...
	HANDLE fIn = CreateFile(inFilename.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fIn == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	if (!WriteFile(fIn, source, sourceLen, NULL, NULL))
	{
		// Failed to write
		CloseHandle(fIn);
		return -1;
	}
	CloseHandle(fIn);

	// Then we get the command
	string command = getSetting("Command");
	// ...and substitute the placeholders in it
	replace(command, "%input%", inFilename);
	replace(command, "%output%", outFilename);

	// We need to clone it
	char* commandLine = _strdup(command.c_str());

	// Then we run it and wait for it to complete
	PROCESS_INFORMATION processInformation = {};
	STARTUPINFO startupInfo = {};
	BOOL created = CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &startupInfo, &processInformation);
	free(commandLine);

	if (!created)
	{
		// Failed to run the command
		return -1;
	}

	WaitForSingleObject(processInformation.hProcess, INFINITE);

	// We can delete the input file now
	DeleteFile(inFilename.c_str());

	// Then read in the output file
	HANDLE fOut = CreateFile(outFilename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fOut == INVALID_HANDLE_VALUE)
	{
		DeleteFile(outFilename.c_str());
		return -1;
	}
	// Check the size
	DWORD fileSize = GetFileSize(fOut, NULL);
	if (fileSize > destLen)
	{
		// Dest buffer too small
		CloseHandle(fOut);
		DeleteFile(outFilename.c_str());
		return 0;
	}
	// Read it in
	if (!ReadFile(fOut, dest, fileSize, NULL, NULL))
	{
		// Problem reading
		CloseHandle(fOut);
		DeleteFile(outFilename.c_str());
		return -1;
	}
	CloseHandle(fOut);
	DeleteFile(outFilename.c_str());

	return fileSize;
}

extern "C" __declspec(dllexport) int compressTiles(uint8_t* source, uint32_t numTiles, uint8_t* dest, uint32_t destLen)
{
	return compress(source, numTiles * 32, dest, destLen);
}

extern "C" __declspec(dllexport) int compressTilemap(uint8_t* source, uint32_t width, uint32_t height, uint8_t* dest, uint32_t destLen)
{
	return compress(source, width * height * 2, dest, destLen);
}
