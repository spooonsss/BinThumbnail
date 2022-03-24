// dllmain.h : Declaration of module class.

class CBinThumbnailModule : public ATL::CAtlDllModuleT< CBinThumbnailModule >
{
public :
	DECLARE_LIBID(LIBID_BinThumbnailLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_BINTHUMBNAIL, "{7b03c102-4506-47e9-aafa-c3e59c9bbdee}")
};

extern class CBinThumbnailModule _AtlModule;
