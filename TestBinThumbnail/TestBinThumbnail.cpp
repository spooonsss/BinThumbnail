#include <Windows.h>
#include <shlwapi.h>
#include <thumbcache.h>
#include <propsys.h>
#include <olectl.h>

#include <wrl.h>

#include <stdio.h>
#include <exception>
#include <stdexcept>

using namespace Microsoft::WRL;

#pragma comment( lib, "Shlwapi.lib" )

#define SUCC(hr) do { HRESULT hr_ = hr; if (FAILED(hr)) throw std::runtime_error(""); } while (0)

static int _main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd)
{
	wchar_t *userprofile;
	_wdupenv_s(&userprofile, NULL, L"USERPROFILE");
	int numArgs;
	LPWSTR *args = CommandLineToArgvW(lpCmdLine, &numArgs);
	std::wstring inputFilename;
	if (wcslen(lpCmdLine)) {
		inputFilename = *args;
	}
	else
	{
		inputFilename = std::wstring(userprofile) + L"\\Google Drive\\SMW\\ExGraphics\\ExGFXBE.bin";
	}
	free(userprofile);
	LocalFree(args);

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

	}
	CoUninitialize();

	return 0;
}

extern "C" int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPWSTR lpCmdLine, int nShowCmd)
{
	return _main(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
}
