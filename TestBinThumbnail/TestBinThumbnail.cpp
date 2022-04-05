#include <Windows.h>
#include <shlwapi.h>
#include <thumbcache.h>
#include <propsys.h>
#include <olectl.h>

#include <wrl.h>

#include <stdio.h>
#include <exception>
#include <stdexcept>
#include <regex>

using namespace Microsoft::WRL;

#pragma comment( lib, "Shlwapi.lib" )

void _throw_failed_hr(HRESULT hr, int line) {

	char buf[1024];

	LPTSTR errorText = NULL;

	FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorText, 0, NULL);
	if (NULL != errorText)
	{
		std::wstring err = std::regex_replace(std::wstring(errorText), std::wregex(L"[\\r\\n]"), L"");

		sprintf_s(buf, "FAILED HRESULT at :%d %S (%x)", line, err.c_str(), hr);
		LocalFree(errorText);
	}
	else
	{
		sprintf_s(buf, "FAILED HRESULT :%d %x", line, hr);
	}

	throw std::runtime_error(buf);
}

#define SUCC(hr) do { HRESULT hr_ = hr; if (FAILED(hr_)) _throw_failed_hr(hr_, __LINE__); } while (0)

static int _main(int argc, wchar_t* argv[])
{
	wchar_t *userprofile;
	_wdupenv_s(&userprofile, NULL, L"USERPROFILE");
	std::wstring inputFilename;
	if (argc > 1)
	{
		inputFilename = argv[1];
	}
	else
	{
		inputFilename = std::wstring(userprofile) + L"\\Google Drive\\SMW\\ExGraphics\\ExGFXBE.bin";
	}
	free(userprofile);

	HRESULT hr;
	hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
	SUCC(hr);
	{
		static const GUID clsid =
		{ 0xb630ff67, 0x75a5, 0x4a1e, { 0x86, 0xb, 0xec, 0x61, 0x9b, 0xdc, 0xe, 0x80 } };

		ComPtr<IStream> input;
		hr = SHCreateStreamOnFileEx(inputFilename.c_str(),
			STGM_READ,
			FILE_ATTRIBUTE_NORMAL,
			FALSE,
			nullptr,
			input.GetAddressOf());
		SUCC(hr);

		ComPtr<IThumbnailProvider> binThumbnail;
		hr = CoCreateInstance(clsid, nullptr, CLSCTX_INPROC, __uuidof(IThumbnailProvider), (void**)binThumbnail.GetAddressOf());
		SUCC(hr);

		ComPtr<IInitializeWithStream> binThumbnailStreamInit;
		binThumbnail.As(&binThumbnailStreamInit);

		hr = binThumbnailStreamInit->Initialize(input.Get(), STGM_READ);
		SUCC(hr);

		HBITMAP bmp;
		WTS_ALPHATYPE alpha;
		hr = binThumbnail->GetThumbnail(1000, &bmp, &alpha);
		SUCC(hr);

		PICTDESC pictdesc = {};
		pictdesc.cbSizeofstruct = sizeof(pictdesc);
		pictdesc.picType = PICTYPE_BITMAP;
		pictdesc.bmp.hbitmap = bmp;

		ComPtr<IPicture> picture;
		OleCreatePictureIndirect(&pictdesc, __uuidof(IPicture), FALSE, (LPVOID*)&picture);

		ComPtr<IPictureDisp> disp;
		picture.As(&disp);
		if (disp) {
			BSTR filename = SysAllocString(L"TestBinThumbnail_debug.bmp");
			hr = OleSavePictureFile(disp.Get(), filename);
			SysReleaseString(filename);
			SUCC(hr);
		}

		DeleteObject(bmp);
	}
	CoUninitialize();

	return 0;
}


int wmain(int argc, wchar_t* argv[])
{
	try
	{
		int ret = _main(argc, argv);
		printf("Output is TestBinThumbnail_debug.bmp\n");
		return ret;
	}
	catch (const std::exception& e) {
		printf("%s\n", e.what());
		fflush(stdout);
		return -2;
	}
}
