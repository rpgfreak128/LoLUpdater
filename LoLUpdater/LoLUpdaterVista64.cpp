// Minimum supported processor = Intel Pentium 4 (Or anything with at least SSE2)

#include <Windows.h>
#include <wininet.h>
#include <fstream>
#include <Shlwapi.h>
#include <Shlobj.h>
#include "resource.h"
#include <sstream>
#include <Msi.h>
#include <atomic>
#include <versionhelpers.h>

class LimitSingleInstance
{
protected:
	DWORD LastError;
	HANDLE Mutex;

public:
	explicit LimitSingleInstance(std::wstring const& strMutexName)
	{
		Mutex = CreateMutex(nullptr, 0, strMutexName.c_str());

		if (Mutex == nullptr)
			throw std::runtime_error("failed to create mutex");

		LastError = GetLastError();
	}

	~LimitSingleInstance()
	{
		if (Mutex)
		{
			if (!CloseHandle(Mutex))
				throw std::runtime_error("failed to close handle");

			Mutex = nullptr;
		}
	}

	BOOL AnotherInstanceRunning()
	{
		return ERROR_ALREADY_EXISTS == LastError;
	}
};

wchar_t airclientd[MAX_PATH + 1];
wchar_t gameclient[MAX_PATH + 1];
wchar_t patchclient[MAX_PATH + 1];
wchar_t svc[MAX_PATH + 1];
wchar_t airdest[MAX_PATH + 1];
wchar_t flashdest[MAX_PATH + 1];
wchar_t instdir[MAX_PATH + 1];
wchar_t instdirGarena[MAX_PATH + 1];
wchar_t instdirQQ[MAX_PATH + 1];

wchar_t air[MAX_PATH + 1];
wchar_t cgdest[MAX_PATH + 1];
wchar_t cgGLdest[MAX_PATH + 1];
wchar_t cgD3D9dest[MAX_PATH + 1];
wchar_t tbb[MAX_PATH + 1];
wchar_t finalurl[INTERNET_MAX_URL_LENGTH];

const std::wstring EXE = L".exe";
const std::wstring DLL = L".dll";
const std::wstring unblocktag = L":Zone.Identifier";

wchar_t msvcp[MAX_PATH + 1] = L"msvcp120";
wchar_t msvcr[MAX_PATH + 1] = L"msvcr120";
wchar_t msvcp0[MAX_PATH + 1] = L"msvcp110";
wchar_t msvcr0[MAX_PATH + 1] = L"msvcr110";
wchar_t adobedir[MAX_PATH + 1] = L"Adobe AIR";
wchar_t cg[MAX_PATH + 1] = L"cg";
wchar_t cgGL[MAX_PATH + 1] = L"cgGL";
wchar_t cgD3D9[MAX_PATH + 1] = L"cgD3D9";

wchar_t* cwd(_wgetcwd(nullptr, 0));

DWORD dwLength;

HWND hwnd;
HWND hwnd2;
HWND hwndButton;
HWND hwndButton2;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ButtonProc2(HWND, UINT, WPARAM, LPARAM);

WNDPROC OldButtonProc;
WNDPROC OldButtonProc2;

SHELLEXECUTEINFO ei;
MSG Msg;

FILE* f;

class ZoneIdentifier
{
public:
	ZoneIdentifier(const wchar_t* fileName)
		: mFilename(fileName ? fileName : L"")
	{
		if (!fileExists(mFilename)) {
			mFilename.clear();
		}
	}
	bool validFile() const
	{
		return !mFilename.empty();
	}
	static bool fileExists(const std::wstring& fileName)
	{
		DWORD attr = GetFileAttributesW(fileName.c_str());
		return INVALID_FILE_ATTRIBUTES != attr && FILE_ATTRIBUTE_DIRECTORY != attr;
	}
	bool hasZoneID() const
	{
		if (validFile()) {
			std::wstring file = mFilename;
			file.append(unblocktag.c_str());
			return fileExists(file);
		}
		return false;
	}
	bool strip() const
	{
		if (validFile()) {
			std::wstring file = mFilename;
			file.append(unblocktag.c_str());
			return !!DeleteFile(file.c_str());
		}
		return false;
	}
private:
	std::wstring mFilename;
};

bool StandardLoL()
{
	return std::wifstream(instdir).good() & std::wifstream(instdirGarena).fail() & std::wifstream(instdirQQ).fail();
}

void PCombine(PWSTR pszPathOut, std::wstring const& pszPathIn, std::wstring const& pszMore)
{
	if (PathCombine(pszPathOut, pszPathIn.c_str(), pszMore.c_str()) == nullptr)
		throw std::runtime_error("failed to combine path");
}

void PAppend(PWSTR pszPath, std::wstring const& pszMore)
{
	if (!PathAppend(pszPath, pszMore.c_str()))
		throw std::runtime_error("failed to append path");
}

void UnblockFile(std::wstring const& filename)
{
	ZoneIdentifier id(filename.c_str());
	if (id.hasZoneID())
	{
		id.strip();
	}
}

void downloadFile(std::wstring const& url, std::wstring const& file)
{
	if (URLDownloadToFile(nullptr, url.c_str(), file.c_str(), 0, nullptr) != S_OK)
		throw std::runtime_error("failed to initialize download");

	UnblockFile(file);
}

void ExtractResource(std::wstring const& RCDATAID, std::wstring const& filename)
{
	auto hRes = FindResource(nullptr, RCDATAID.c_str(), RT_RCDATA);

	if (hRes == nullptr)
		throw std::runtime_error("failed to find resource");

	if (_wfopen_s(&f, filename.c_str(), L"wb") != NULL)
		throw std::runtime_error("failed to open resource");

	if (!fwrite(LockResource(LoadResource(nullptr, hRes)), SizeofResource(nullptr, hRes), 1, f))
		throw std::runtime_error("failed to write resource");

	if (fclose(f) != NULL)
		throw std::runtime_error("failed to close resource");

	UnblockFile(filename);
}

void URlComb(std::wstring const& pszBase, std::wstring const& pszRelative, std::wstring const& file)
{
	*finalurl = '\0';
	dwLength = sizeof(finalurl);

	if (UrlCombine(std::wstring(L"http://labsdownload.adobe.com/pub/labs/flashruntimes/" + pszBase).c_str(), pszRelative.c_str(), finalurl, &dwLength, 0) != S_OK)
		throw std::runtime_error("failed to combine Url");

	downloadFile(finalurl, file.c_str());
}

void CpFile(std::wstring const& lpExistingFileName, std::wstring const& lpNewFileName)
{
	if (!CopyFile(lpExistingFileName.c_str(), lpNewFileName.c_str(), false))
		throw std::runtime_error("failed to copy file");
}

void Fldrpath(int CSIDL, PWSTR buffer)
{
	if (SHGetFolderPath(nullptr, CSIDL, nullptr, 0, buffer) != S_OK)
		throw std::runtime_error("Unable to get folder path");
}

void msvccopy(std::wstring const& MSVCP, std::wstring const& MSVCR, std::wstring const& MSVCP0, std::wstring const& MSVCR0)
{
	if (StandardLoL())
	{
		*svc = '\0';
		PCombine(svc, patchclient, msvcp);
		ExtractResource(MSVCP, svc);

		*svc = '\0';
		PCombine(svc, patchclient, msvcr);
		ExtractResource(MSVCR, svc);
	}

	*svc = '\0';
	PCombine(svc, gameclient, msvcp);
	ExtractResource(MSVCP, svc);

	*svc = '\0';
	PCombine(svc, gameclient, msvcr);
	ExtractResource(MSVCR, svc);

	*svc = '\0';
	PCombine(svc, airclientd, msvcp0);
	ExtractResource(MSVCP0, svc);

	*svc = '\0';
	PCombine(svc, airclientd, msvcr0);
	ExtractResource(MSVCR0, svc);

}

void Launch()
{
	if (std::wifstream(instdir).good())
	{
		ei.lpFile = instdir;

		if (!ShellExecuteEx(&ei))
			throw std::runtime_error("failed to execute the executable");
	}
}

LRESULT CALLBACK ButtonProc(HWND, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		Msg = {};
		SendMessage(hwndButton, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(L"Patching..."));

		if (MsiQueryProductState(L"{A749D8E6-B613-3BE3-8F5F-045C84EBA29B}") != INSTALLSTATE_DEFAULT)
		{
			wchar_t msvc[MAX_PATH + 1] = L"vcredist_x64";
			wchar_t runmsvc[MAX_PATH + 1];
			wcsncat_s(msvc, _countof(msvc), EXE.c_str(), _TRUNCATE);
			PCombine(runmsvc, cwd, msvc);
			*finalurl = '\0';
			dwLength = sizeof(finalurl);

			if (UrlCombine(L"http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/", msvc, finalurl, &dwLength, 0) != S_OK)
				throw std::runtime_error("failed to combine Url");

			downloadFile(finalurl, runmsvc);

			ei.lpParameters = L"/q /norestart";
			ei.lpFile = runmsvc;

			if (!ShellExecuteEx(&ei))
				throw std::runtime_error("failed to execute the executable");

			while (WAIT_OBJECT_0 != MsgWaitForMultipleObjects(1, &ei.hProcess, FALSE, INFINITE, QS_ALLINPUT))
			{
				while (PeekMessage(&Msg, nullptr, 0, 0, PM_REMOVE))
				{
					DispatchMessage(&Msg);
				}
			}
			DeleteFile(runmsvc);
		}

		msvccopy(L"x20", L"x30", L"x201", L"x301");

		if (IsWindows8OrGreater())
		{
			ExtractResource(L"x02", tbb);
		}
		else
		{
			if (IsWindows7OrGreater())
			{
				ExtractResource(L"x01", tbb);
			}
			else
			{
				ExtractResource(L"x00", tbb);
			}

		}

		ExtractResource(L"xfff", flashdest);

		ExtractResource(L"xa1", cgdest);
		ExtractResource(L"xa2", cgGLdest);
		ExtractResource(L"xa3", cgD3D9dest);

		ExtractResource(L"x666", airdest);

		EnableWindow(hwndButton, FALSE);
		SendMessage(hwndButton, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(L"Finished!"));
		Launch();
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
		return 0;
	}
	return CallWindowProc(OldButtonProc, hwndButton, msg, wp, lp);
}

LRESULT CALLBACK ButtonProc2(HWND, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		SendMessage(hwndButton2, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(L"Uninstalling..."));

		ExtractResource(L"x101", airdest);
		msvccopy(L"x501", L"x601", L"x502", L"x602");
		ExtractResource(L"x701", flashdest);

		ExtractResource(L"xu1", cgdest);
		ExtractResource(L"xu2", cgGLdest);
		ExtractResource(L"xu3", cgD3D9dest);
		DeleteFile(tbb);
		EnableWindow(hwndButton2, FALSE);
		SendMessage(hwndButton2, WM_SETTEXT, NULL, reinterpret_cast<LPARAM>(L"Finished!"));
		Launch();
		break;
	}
	return CallWindowProc(OldButtonProc2, hwndButton2, msg, wp, lp);
}

// Todo: add content
void AboutBox()
{
	ShowWindow(hwnd2, SW_SHOW);
	UpdateWindow(hwnd2);
}

struct Version
{
	int major, minor, revision, build;

	explicit Version(const std::wstring& version)
	{
		swscanf_s(version.c_str(), L"%d.%d.%d.%d", &major, &minor, &revision, &build, sizeof(wchar_t));
		if (major < 0) major = 0;
		if (minor < 0) minor = 0;
		if (revision < 0) revision = 0;
		if (build < 0) build = 0;
	}

	bool operator <(const Version& other)
	{
		if (major < other.major)
			return true;
		if (minor < other.minor)
			return true;
		if (revision < other.revision)
			return true;
		if (build < other.build)
			return true;
		return false;
	}

	friend std::wostream& operator <<(std::wostream& stream, const Version& ver)
	{
		stream << ver.major;
		stream << '.';
		stream << ver.minor;
		stream << '.';
		stream << ver.revision;
		stream << '.';
		stream << ver.build;
		return stream;
	}
};

void AutoUpdater()
{
	wchar_t versiontxt[MAX_PATH + 1];
	const std::wstring version = L"version.txt";
	PCombine(versiontxt, cwd, version.c_str());

	*finalurl = '\0';
	dwLength = sizeof(finalurl);

	if (UrlCombine(L"http://lol.jdhpro.com/", version.c_str(), finalurl, &dwLength, 0) != S_OK)
		throw std::runtime_error("failed to combine Url");

	downloadFile(finalurl, versiontxt);

	std::wifstream t0(versiontxt);
	std::wstringstream buffer[5];
	buffer[0] << t0.rdbuf();

	wchar_t ownPth[MAX_PATH + 1];
	auto hModule = GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		GetModuleFileName(hModule, ownPth, sizeof(ownPth));
	}

	DWORD verHandle;
	UINT size = 0;
	LPBYTE lpBuffer;
	auto verSize = GetFileVersionInfoSize(ownPth, &verHandle);

	if (verSize != NULL)
	{
		auto verData = new wchar_t[verSize];
		if (GetFileVersionInfo(ownPth, verHandle, verSize, verData) && VerQueryValue(verData, L"\\", reinterpret_cast<VOID FAR* FAR*>(&lpBuffer), &size) && size)
		{
			auto verInfo = reinterpret_cast<VS_FIXEDFILEINFO*>(lpBuffer);
			if (verInfo->dwSignature == 0xfeef04bd)
			{
				buffer[1] << HIWORD(verInfo->dwProductVersionMS);
				buffer[2] << LOWORD(verInfo->dwProductVersionMS);
				buffer[3] << HIWORD(verInfo->dwProductVersionLS);
				buffer[4] << LOWORD(verInfo->dwProductVersionLS);
			}
		}
	}

	if (Version(std::wstring(buffer[1].str() + L"." + buffer[2].str() + L"." + buffer[3].str() + L"." + buffer[4].str())) < Version(buffer[0].str()))
	{
		wchar_t fullpath[MAX_PATH + 1];
		PCombine(fullpath, cwd, std::wstring(L"LoLUpdater64 v" + buffer[0].str() + EXE).c_str());
		downloadFile(L"http://www.smoothdev.org/mirrors/download.php?user=Loggan&file=WDQzWlVxYmpUZ3RVc3RCUmlQalN0UWNBbGZTK2hreVdvSGxjb0RudjhLcz0=", fullpath);
		MessageBox(hwnd, std::wstring(L"Update saved to " + std::wstring(cwd)).c_str(), L"LoLUpdater AutoUpdater", MB_OK | MB_APPLMODAL);
	}
	else
	{
		MessageBox(hwnd, L"No update found!", L"LoLUpdater AutoUpdater", MB_OK | MB_APPLMODAL);
	}

	t0.close();
	DeleteFile(versiontxt);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		hwndButton = CreateWindow(L"BUTTON", L"Install", WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 10, 100, 50, hwnd, NULL, nullptr, nullptr);

		if (hwndButton == nullptr)
			throw std::runtime_error("failed to create Button");

		OldButtonProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwndButton, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ButtonProc)));

		hwndButton2 = CreateWindow(L"BUTTON", L"Uninstall", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 120, 10, 100, 50, hwnd, NULL, nullptr, nullptr);
		if (hwndButton2 == nullptr)
			throw std::runtime_error("failed to create Button");

		OldButtonProc2 = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hwndButton2, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ButtonProc2)));

		CLIENTCREATESTRUCT MDIClientCreateStruct;

		hwnd2 = CreateWindowEx(WS_EX_TOOLWINDOW, L"MDICLIENT", L"About LoLUpdater", WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 250, 130, hwnd, nullptr, nullptr, &MDIClientCreateStruct);
		if (hwnd2 == nullptr)
			throw std::runtime_error("failed to create window");
		break;

	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_HELP_CHECKFORUPDATES:
			AutoUpdater();
			break;

		case ID_HELP_ABOUT:
			AboutBox();
			break;
		}
	}
	break;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		exit(0);
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

void FindLatest(PWSTR PATH)
{
	WIN32_FIND_DATA data2;
	auto hFind = FindFirstFile(std::wstring(PATH + std::wstring(L"\\*.*")).c_str(), &data2);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFile(hFind, &data2))
		{
		}

		PAppend(PATH, data2.cFileName);

		if (!FindClose(hFind))
			throw std::runtime_error("failed to close file handle");
	}
	else
		throw std::runtime_error("failed to find file/directory");
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	LimitSingleInstance appGUID(L"Global\\{101UPD473R-BYL0GG4N08@G17HUB-V3RYR4ND0M4NDR4R3MUCH}");
	if (appGUID.AnotherInstanceRunning())
		return 0;

	Msg = {};
	const std::wstring wndClassName(L"LoLUpdater mainwindow");
	WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_DROPSHADOW | CS_PARENTDC, WndProc, 0, 0, hInstance, LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON)), nullptr, static_cast<HBRUSH>(GetSysColorBrush(COLOR_3DFACE)), MAKEINTRESOURCE(IDR_MENU1), wndClassName.c_str(), LoadIcon(hInstance, MAKEINTRESOURCE(MAINICON)) };
	if (wc.hIcon == nullptr)
		throw std::runtime_error("failed to load icon");

	if (wc.hbrBackground == nullptr)
		throw std::runtime_error("failed to load background");

	if (wc.hIconSm == nullptr)
		throw std::runtime_error("failed to load icon");

	if (!RegisterClassEx(&wc))
		throw std::runtime_error("failed to register windowclass");

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, wndClassName.c_str(), L"LoLUpdater", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 250, 130, nullptr, nullptr, hInstance, nullptr);
	if (hwnd == nullptr)
		throw std::runtime_error("failed to create window");

	BROWSEINFO bi{};
	bi.lpszTitle = L"Select your GarenaLoL/League of Legends/LoLQQ installation directory:";
	bi.ulFlags = BIF_USENEWUI;

	auto pidl = SHBrowseForFolder(&bi);

	if (pidl == nullptr)
		return 0;

	wchar_t loldir[MAX_PATH + 1];

	if (!SHGetPathFromIDList(pidl, loldir))
		throw std::runtime_error("failed to get browse path");

	wcsncat_s(air, _countof(air), adobedir, 9);
	wcsncat_s(air, _countof(air), DLL.c_str(), _TRUNCATE);

	PCombine(instdir, loldir, L"lol.launcher.exe");
	PCombine(instdirQQ, loldir, L"lol.launcher_tencent.exe");
	PCombine(instdirGarena, loldir, L"lol.exe");

	PAppend(adobedir, L"Versions");
	PAppend(adobedir, L"1.0");
	wchar_t airclient[MAX_PATH + 1];
	if (StandardLoL())
	{
		const std::wstring rads = L"RADS";
		PCombine(airclient, loldir, rads.c_str());
		const std::wstring proj = L"projects";
		PAppend(airclient, proj.c_str());
		PAppend(airclient, L"lol_air_client");
		const std::wstring rel = L"releases";
		PAppend(airclient, rel.c_str());
		FindLatest(airclient);
		const std::wstring dep = L"deploy";
		PAppend(airclient, dep.c_str());
		PAppend(airclientd, airclient);
		PAppend(airclient, adobedir);

		PCombine(patchclient, loldir, rads.c_str());
		PAppend(patchclient, proj.c_str());
		PAppend(patchclient, L"lol_patcher");
		PAppend(patchclient, rel.c_str());
		FindLatest(patchclient);
		PAppend(patchclient, dep.c_str());

		PCombine(gameclient, loldir, rads.c_str());
		PAppend(gameclient, L"solutions");
		PAppend(gameclient, L"lol_game_client_sln");
		PAppend(gameclient, rel.c_str());
		FindLatest(gameclient);
		PAppend(gameclient, dep.c_str());
	}
	if (std::wifstream(instdir).fail() & (std::wifstream(instdirGarena).good() || std::wifstream(instdirQQ).good()))
	{
		PCombine(gameclient, loldir, L"Game");

		PCombine(airclient, loldir, L"Air");
		PAppend(airclient, adobedir);
	}

	wchar_t tbbfile[MAX_PATH + 1] = L"tbb";
	wcsncat_s(tbbfile, _countof(tbbfile), DLL.c_str(), _TRUNCATE);
	wcsncat_s(msvcp, _countof(msvcp), DLL.c_str(), _TRUNCATE);
	wcsncat_s(msvcr, _countof(msvcr), DLL.c_str(), _TRUNCATE);
	wcsncat_s(msvcp0, _countof(msvcp0), DLL.c_str(), _TRUNCATE);
	wcsncat_s(msvcr0, _countof(msvcr0), DLL.c_str(), _TRUNCATE);
	wcsncat_s(cg, _countof(cg), DLL.c_str(), _TRUNCATE);
	wcsncat_s(cgGL, _countof(cgGL), DLL.c_str(), _TRUNCATE);
	wcsncat_s(cgD3D9, _countof(cgD3D9), DLL.c_str(), _TRUNCATE);

	PCombine(airdest, airclient, air);
	PCombine(flashdest, airclient, L"Resources");
	wchar_t flash[MAX_PATH + 1] = L"NPSWF32";
	wcsncat_s(flash, _countof(flash), DLL.c_str(), _TRUNCATE);
	PAppend(flashdest, flash);
	PCombine(cgdest, gameclient, cg);
	PCombine(cgGLdest, gameclient, cgGL);
	PCombine(cgD3D9dest, gameclient, cgD3D9);
	PCombine(tbb, gameclient, tbbfile);

	ei.cbSize = sizeof(SHELLEXECUTEINFO);
	ei.fMask = SEE_MASK_NOCLOSEPROCESS;
	ei.nShow = SW_SHOW;

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, nullptr, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return static_cast<int>(Msg.wParam);
}