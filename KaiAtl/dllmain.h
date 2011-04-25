// dllmain.h : Declaration of module class.

class CKaiAtlModule : public CAtlDllModuleT< CKaiAtlModule >
{
public :
	DECLARE_LIBID(LIBID_KaiAtlLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_KAIATL, "{7F73DB02-CE05-4FDE-99B7-38806F45A9DC}")
};

extern class CKaiAtlModule _AtlModule;
