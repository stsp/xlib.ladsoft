
#include <shlobj.h>

extern char szInstallPath[1024];

// Macros for interface casts
#ifdef __cplusplus
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void **>(ppType)
#else
#define IID_PPV_ARG(IType, ppType) &IID_##IType, (void**)(ppType)
#endif

// Retrieves the UIObject interface for the specified full PIDL
STDAPI SHGetUIObjectFromFullPIDL(LPCITEMIDLIST pidl, HWND hwnd, REFIID riid, void **ppv)
{
    LPCITEMIDLIST pidlChild;
    IShellFolder* psf;

    HRESULT hr = SHBindToParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlChild);

    *ppv = NULL;

    if (SUCCEEDED(hr))
    {
        hr = psf->lpVtbl->GetUIObjectOf(psf, hwnd, 1, &pidlChild, riid, NULL, ppv);
        psf->lpVtbl->Release(psf);
    }
    return hr;
}
 
#define ILSkip(pidl, cb)       ((LPITEMIDLIST)(((BYTE*)(pidl))+cb))
#define ILNext(pidl)           ILSkip(pidl, (pidl)->mkid.cb)
 
static HRESULT SHILClone(LPCITEMIDLIST pidl, LPITEMIDLIST *ppidl)
{
    DWORD cbTotal = 0;

    if (pidl)
    {
        LPCITEMIDLIST pidl_temp = pidl;
        cbTotal += sizeof (pidl_temp->mkid.cb);

        while (pidl_temp->mkid.cb) 
        {
            cbTotal += pidl_temp->mkid.cb;
            pidl_temp = ILNext (pidl_temp);
        }
    }
    
    *ppidl = (LPITEMIDLIST)CoTaskMemAlloc(cbTotal);
    
    if (*ppidl)
        CopyMemory(*ppidl, pidl, cbTotal);
 
    return  *ppidl ? S_OK: E_OUTOFMEMORY;
}
 
// Get the target PIDL for a folder PIDL. This also deals with cases of a folder  
// shortcut or an alias to a real folder.
STDAPI SHGetTargetFolderIDList(LPCITEMIDLIST pidlFolder, LPITEMIDLIST *ppidl)
{
    IShellLink *psl;
	
    HRESULT hr = SHGetUIObjectFromFullPIDL(pidlFolder, NULL, IID_PPV_ARG(IShellLink, &psl));
    
    *ppidl = NULL;
    
    if (SUCCEEDED(hr))
    {
        hr = psl->lpVtbl->GetIDList(psl, ppidl);
        psl->lpVtbl->Release(psl);
    }
    
    // It's not a folder shortcut so get the PIDL normally.
    if (FAILED(hr))
        hr = SHILClone(pidlFolder, ppidl);
    
    return hr;
}

// Get the target folder for a folder PIDL. This deals with cases where a folder
// is an alias to a real folder, folder shortcuts, the My Documents folder, and 
// other items of that nature.
STDAPI SHGetTargetFolderPath(LPCITEMIDLIST pidlFolder, LPWSTR pszPath, UINT cchPath)
{
    LPITEMIDLIST pidlTarget;
	

    HRESULT hr = SHGetTargetFolderIDList(pidlFolder, &pidlTarget);
    
    *pszPath = 0;

    if (SUCCEEDED(hr))
    {
        SHGetPathFromIDListW(pidlTarget, pszPath);   // Make sure it is a path
        CoTaskMemFree(pidlTarget);
    }
    
    return *pszPath ? S_OK : E_FAIL;
}
void GetDefaultProjectsPath(LPSTR pszPath)
{
    HANDLE hndl;
    WIN32_FIND_DATA data;
    char ispth[MAX_PATH];
    sprintf(ispth,"%s\\appdata",szInstallPath);
    hndl = FindFirstFile(ispth, &data);
    if (hndl != INVALID_HANDLE_VALUE)
    {
	sprintf(pszPath, "%s\\CC386 Projects\\", szInstallPath);
        FindClose(hndl);
    }
    else if (SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, pszPath) == S_OK)
    {
        strcat(pszPath,"\\CC386 Projects");
    }
    else
    {
        pszPath[0] = 0;
    }
}
DWORD BrowseForFile(HWND hwnd, LPSTR pszDisplayName, LPSTR pszPath, UINT cchPath)
{
    DWORD rv = E_FAIL;
    LPITEMIDLIST pidlSelected = NULL;
    WCHAR intermedPath[MAX_PATH];
    BROWSEINFO bi = {0};

    bi.hwndOwner = hwnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = pszDisplayName;
    bi.lpszTitle = "Choose a folder";
    bi.ulFlags = 0;
    bi.lpfn = NULL;
    bi.lParam = 0;

    pidlSelected = SHBrowseForFolder(&bi);

    
    if (pidlSelected)
    {
        rv = SHGetTargetFolderPath(pidlSelected, intermedPath, MAX_PATH);
        if (rv == S_OK)
        {
            WCHAR *s =intermedPath;
            char *d = pszPath;
            while (*s)
                *d++ = *s++;
            *d = 0;
        }
        CoTaskMemFree(pidlSelected);
    }
    return rv;
}

