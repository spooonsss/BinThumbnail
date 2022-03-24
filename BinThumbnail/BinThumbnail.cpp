/*
Used by Windows Explorer to create thumbnails for SNES 4bpp .bin files

Copyright 2022 spooonsss
License: https://opensource.org/licenses/BSD-2-Clause

Current user installation/uninstall:
See install.cmd and uninstall.cmd


All users installation (requires admin)
reg add HKCR\.bin\shellex\{E357FCCD-A995-4576-B01F-234630154E96} /reg:64 /f /d {B630FF67-75A5-4A1E-860B-EC619BDC0E80}
reg add HKCR\CLSID\{B630FF67-75A5-4A1E-860B-EC619BDC0E80}\InprocServer32 /reg:64 /f /d "%CD%\x64\Release\BinThumbnail.dll"
reg add HKCR\CLSID\{B630FF67-75A5-4A1E-860B-EC619BDC0E80}\InprocServer32 /reg:64 /f /v ThreadingModel /d Apartment

Uninstall:
reg delete HKCR\CLSID\{B630FF67-75A5-4A1E-860B-EC619BDC0E80} /f /reg:64
reg delete HKCR\.bin\shellex\{E357FCCD-A995-4576-B01F-234630154E96} /f /reg:64

*/

#include <wrl.h>
#include <propsys.h>
#include <thumbcache.h>
#include <olectl.h>

#include <memory>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

using namespace Microsoft::WRL;

template<typename T, auto F>
struct deleter {
    T* o;
    deleter(T* o) {
        this->o = o;
    }
    ~deleter() {
        F(*o);
    }
};

/*
pal = open(r"SMW\pal default.pal", 'rb').read()
', '.join([f"{{{', '.join(map(str, p))}}}" for p in zip(*([iter(pal)] * (3 * 16))) ])

*/
unsigned char default_pal[][16*3] = {
{0, 96, 184, 232, 240, 248, 16, 80, 112, 64, 128, 160, 112, 176, 208, 144, 192, 192, 168, 208, 208, 192, 224, 224, 152, 224, 224, 0, 0, 0, 216, 56, 24, 88, 248, 88, 152, 224, 224, 0, 0, 0, 232, 240, 248, 248, 88, 88}, {0, 96, 184, 232, 240, 248, 16, 80, 112, 64, 128, 160, 112, 176, 208, 16, 80, 112, 64, 128, 160, 112, 176, 208, 152, 224, 224, 0, 0, 0, 88, 168, 240, 248, 248, 248, 152, 224, 224, 0, 0, 0, 216, 160, 56, 248, 216, 112}, {0, 96, 184, 232, 240, 248, 0, 0, 0, 120,
104, 24, 200, 152, 88, 0, 120, 72, 224, 192, 80, 0, 200, 0, 0, 0, 0, 248, 248, 248, 0, 0, 0, 0, 200, 0, 176, 0, 0, 248, 0, 0, 248, 88, 0, 248, 160, 0 }, { 0, 96,
184, 232, 240, 248, 0, 0, 0, 88, 88, 88, 120, 120, 120, 152, 152, 152, 192, 192, 192, 224, 224, 224, 0, 0, 0, 248, 248, 248, 0, 0, 0, 0, 200, 0, 232, 24, 104, 240, 64, 168, 248, 120, 200, 248, 192, 240 }, { 0, 96, 184, 232, 240, 248, 112, 112, 112, 0, 0, 0, 192, 192, 192, 160, 200, 248, 168, 224, 248, 192, 248, 248, 0, 0, 0, 248, 248, 248, 0, 0, 0, 0, 200, 0, 0, 224, 0, 136, 248, 56, 200, 248, 0, 248, 248, 152 }, { 0, 96, 184, 232, 240, 248, 0, 0, 0, 184, 168, 96, 216, 248, 200,
0, 128, 0, 0, 200, 0, 0, 248, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 96, 184, 232, 240, 248, 0, 0, 0, 136, 88, 24, 248, 8, 248, 216, 160, 56, 248, 216, 32, 248, 248, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 96, 184, 232, 240, 248, 0, 0, 0,
184, 0, 80, 248, 0, 128, 72, 72, 136, 104, 104, 176, 128, 128, 200, 0, 0, 0, 0,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 96, 184, 248, 248, 248, 0, 0, 0, 136, 88, 24, 216, 160, 56, 248, 216, 112, 248, 208, 192, 232,
0, 176, 80, 0, 0, 248, 64, 112, 32, 48, 136, 64, 128, 152, 128, 216, 200, 176, 40, 96, 248, 112, 104, 248, 248, 0 }, { 0, 96, 184, 248, 248, 248, 0, 0, 0, 112, 112, 112, 160, 160, 160, 192, 192, 192, 224, 224, 224, 248, 16, 88, 0, 0, 0, 248,
248, 248, 0, 0, 0, 0, 200, 0, 176, 0, 0, 248, 0, 0, 248, 88, 0, 248, 160, 0 }, { 0, 96, 184, 248, 248, 248, 0, 0, 0, 248, 120, 0, 248, 192, 0, 248, 248, 0, 184, 40, 0, 248, 136, 0, 0, 0, 0, 248, 248, 248, 0, 0, 0, 0, 200, 0, 232, 24, 104, 240, 64, 168, 248, 120, 200, 248, 192, 240 }, { 0, 96, 184, 248, 248, 248, 0, 0, 0, 64, 64, 216, 104, 104, 216, 136, 136, 248, 184, 40, 0, 248, 136, 0, 0, 0, 0, 248, 248, 248, 0, 0, 0, 0, 200, 0, 0, 224, 0, 136, 248, 56, 200, 248, 0, 248, 248, 152 }, { 0, 96, 184, 248, 248, 248, 0, 0, 0, 136, 0, 0, 184, 0, 0, 248, 0, 0, 184,
40, 0, 248, 136, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
0, 0, 0, 0 }, { 0, 96, 184, 248, 248, 248, 0, 0, 0, 0, 120, 0, 0, 184, 0, 0, 248,
0, 184, 40, 0, 248, 136, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 96, 184, 248, 248, 248, 0, 0, 0, 40, 48, 72, 72, 80, 88, 104, 104, 88, 152, 144, 64, 192, 192, 120, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, { 0, 96, 184, 248, 248, 248, 24, 72, 72, 32, 112, 104, 40, 136, 120, 48, 160, 136, 56, 184, 152, 248, 0, 128, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

// {B630FF67-75A5-4A1E-860B-EC619BDC0E80}
static const GUID clsid =
{ 0xb630ff67, 0x75a5, 0x4a1e, { 0x86, 0xb, 0xec, 0x61, 0x9b, 0xdc, 0xe, 0x80 } };

class
    __declspec(uuid("B630FF67-75A5-4A1E-860B-EC619BDC0E80"))
    BinThumbnail : public RuntimeClass<RuntimeClassFlags<ClassicCom>, IThumbnailProvider, IInitializeWithStream>
{
public:

    virtual HRESULT STDMETHODCALLTYPE GetThumbnail(
        /* [in] */ UINT cx,
        /* [out] */ __RPC__deref_out_opt HBITMAP* phbmp,
        /* [out] */ __RPC__out WTS_ALPHATYPE* pdwAlpha) {

        if (this->stream == nullptr) {
            return E_FAIL;
        }

        BITMAPINFO bmi = {0};
        bmi.bmiHeader.biSize = sizeof(bmi.bmiHeader);
        bmi.bmiHeader.biWidth = 8 * 16;
        bmi.bmiHeader.biHeight = -8 * 8;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        unsigned char* bytes;
        HBITMAP bmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS,
            reinterpret_cast<void**>(&bytes),
            nullptr, 0);
        if (bmp == nullptr) {
            return E_OUTOFMEMORY;
        }
        deleter<HBITMAP, DeleteObject> bmp_deleter(&bmp);

        // assume input file is 4k
        ULONG buf_size = 4 * 1024;
        std::unique_ptr<unsigned char[]> buf_ptr(new unsigned char[buf_size]);
        unsigned char* buf = buf_ptr.get();
        memset(buf, 0, buf_size);
        ULONG buf_pos = 0;
        ULONG bRead;
        HRESULT hr;
        while (buf_pos < buf_size) {
            hr = this->stream->Read((void*)buf, buf_size - buf_pos, &bRead);
            if (hr != E_PENDING) {
                break;
            }
            buf_pos += bRead;
        }
        if (FAILED(hr)) {
            return hr;
        }
        // hr == S_FALSE: we didn't read 4k, but that's ok

        // copy SNES 4bpp `buf` to `bmi`
        for (int out_y = 0; out_y < 8 * 8; out_y += 8)
        {
            for (int out_x = 0; out_x < 8 * 16; out_x += 8)
            {
                int bpp = 4;
                unsigned char* intile = &buf[out_x * 8 * bpp / 8 + (out_y * 8 * bpp / 8 * 16)];

                for (int xx = 0; xx < 8; xx++)
                {
                    for (int yy = 0; yy < 8; yy++)
                    {
                        unsigned char ind = // decodedtile[(yy * 8) + 7 - xx] =
                            ((intile[yy << 1] >> xx) & 1) +
                            (((intile[(yy << 1) | 1] >> xx) & 1) << 1) +
                            (((intile[(yy << 1) | 16] >> xx) & 1) << 2) +
                            (((intile[(yy << 1) | 17] >> xx) & 1) << 3);

                        auto pos_in_8x8 = (yy * bmi.bmiHeader.biWidth * 4) + (4 * (7 - xx)) + (out_x * 4) + (out_y * 4 * bmi.bmiHeader.biWidth);
                        if (ind > 0) {
                            // bytes format is BGRA aka RGBQUAD
                            bytes[pos_in_8x8+3] = 0xFF;
                            for (int i = 0; i < 3; i++) {
                                auto rgb_component = default_pal[0xa][ind * 3 + (2-i)];
                                bytes[pos_in_8x8 + i] = rgb_component;
                            }
                        }
                        else {
                            bytes[pos_in_8x8 + 0] = 0x00;
                            bytes[pos_in_8x8 + 1] = 0x00;
                            bytes[pos_in_8x8 + 2] = 0x00;
                            bytes[pos_in_8x8 + 3] = 0x00;
                            // leaving RGB=0 ensures premultiplied alpha for stbir_resize_uint8
                        }
                    }
                }
            }
        }

        // scale bmi to bmiOutput with desired size, and rearranged rightmost 32px
        BITMAPINFO bmiOutput = { 0 };
        bmiOutput.bmiHeader.biSize = sizeof(bmiOutput.bmiHeader);
        bmiOutput.bmiHeader.biWidth = cx;
        bmiOutput.bmiHeader.biHeight = -((LONG)cx);
        bmiOutput.bmiHeader.biPlanes = 1;
        bmiOutput.bmiHeader.biBitCount = 32;
        bmiOutput.bmiHeader.biCompression = BI_RGB;

        unsigned char* bytesOutput;
        HBITMAP bmpOutput = CreateDIBSection(nullptr, &bmiOutput, DIB_RGB_COLORS,
            reinterpret_cast<void**>(&bytesOutput),
            nullptr, 0);
        if (bmpOutput == nullptr) {
            return E_OUTOFMEMORY;
        }
        // caller will delete bmpOutput deleter<HBITMAP, DeleteObject> bmpd(&bmpOutput);

        int bpp = 4; // n.b. bytes per pixel here
        stbir_resize_uint8(bytes, 12 * 8, 8 * 8, bmi.bmiHeader.biWidth * bpp,
            bytesOutput, bmiOutput.bmiHeader.biWidth, abs(bmiOutput.bmiHeader.biHeight) * 2 / 3, bmiOutput.bmiHeader.biWidth * bpp, 4);
        // copy rightmost 32x32s under first 2 rows of 32x32s
        stbir_resize_uint8(bytes + (12 * 8 * bpp), 4 * 8, 4 * 8, bmi.bmiHeader.biWidth * bpp,
            bytesOutput + bmiOutput.bmiHeader.biWidth * (abs(bmiOutput.bmiHeader.biHeight) * 2 / 3) * bpp, bmiOutput.bmiHeader.biWidth / 3, abs(bmiOutput.bmiHeader.biHeight) * 1 / 3, bmiOutput.bmiHeader.biWidth * 4, 4);
        stbir_resize_uint8(bytes + (12 * 8 * bpp) + (bmi.bmiHeader.biWidth * 8 * 4 * bpp), 4 * 8, 4 * 8, bmi.bmiHeader.biWidth * bpp,
            bytesOutput + ((bmiOutput.bmiHeader.biWidth * bpp / 3) | 3) + 1 + bmiOutput.bmiHeader.biWidth * (abs(bmiOutput.bmiHeader.biHeight) * 2 / 3) * bpp, bmiOutput.bmiHeader.biWidth / 3, abs(bmiOutput.bmiHeader.biHeight) * 1 / 3, bmiOutput.bmiHeader.biWidth * 4, 4);

        *phbmp = bmpOutput;
        *pdwAlpha = WTSAT_ARGB;

        return S_OK;
    }

    ComPtr<IStream> stream;

    virtual /* [local] */ HRESULT STDMETHODCALLTYPE Initialize(
        /* [annotation][in] */
        _In_  IStream* pstream,
        /* [annotation][in] */
        _In_  DWORD grfMode) {
        // assert grfMode == STGM_READ;
        if (this->stream) {
            return HRESULT_FROM_WIN32(ERROR_ALREADY_INITIALIZED);
        }
        this->stream = pstream;
        return S_OK;
    }

};

CoCreatableClass(BinThumbnail);
