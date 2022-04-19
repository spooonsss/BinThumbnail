reg add HKCU\Software\Classes\.bin\shellex\{E357FCCD-A995-4576-B01F-234630154E96} /reg:64 /f /d {B630FF67-75A5-4A1E-860B-EC619BDC0E80} || pause
reg add HKCU\Software\Classes\CLSID\{B630FF67-75A5-4A1E-860B-EC619BDC0E80}\InprocServer32 /reg:64 /f /d "%~dp0BinThumbnail.dll" || pause
reg add HKCU\Software\Classes\CLSID\{B630FF67-75A5-4A1E-860B-EC619BDC0E80}\InprocServer32 /reg:64 /f /v ThreadingModel /d Apartment || pause
reg add HKCU\Software\Classes\CLSID\{B630FF67-75A5-4A1E-860B-EC619BDC0E80}\InprocServer32 /reg:32 /f /d "%~dp0BinThumbnail_32.dll" || pause
reg add HKCU\Software\Classes\CLSID\{B630FF67-75A5-4A1E-860B-EC619BDC0E80}\InprocServer32 /reg:32 /f /v ThreadingModel /d Apartment || pause
Rundll32 BinThumbnail.dll,PostInstall || pause
