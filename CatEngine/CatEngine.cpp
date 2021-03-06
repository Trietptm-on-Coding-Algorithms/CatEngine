#include "CatEngine.h"

#pragma warning(disable: 4661)

namespace ce {
  // Private in CatEngine.cpp
  /* The common constants */
  const std::string  CE_TITLE_ANSI           =  "CatEngine";
  const std::wstring CE_TITLE_UNICODE        = L"CatEngine";
  const size_t MAX_SIZE = MAXBYTE;

  /* Additional defination */
#ifndef PSAPI_VERSION
  #define LIST_MODULES_32BIT    0x01
  #define LIST_MODULES_64BIT    0x02
  #define LIST_MODULES_ALL      0x03
  #define LIST_MODULES_DEFAULT  0x04
#endif

  /* -------------------------------------------- Initialize ToolHelp32 -------------------------------------------- */

  bool g_HasToolHelp32 = false;

  typedef HANDLE (WINAPI *PfnCreateToolhelp32Snapshot)(DWORD dwFlags, DWORD th32ProcessID);

  typedef BOOL (WINAPI *PfnProcess32FirstA)(HANDLE hSnapshot, PProcessEntry32A lppe);
  typedef BOOL (WINAPI *PfnProcess32NextA)(HANDLE hSnapshot, PProcessEntry32A lppe);
  typedef BOOL (WINAPI *PfnProcess32FirstW)(HANDLE hSnapshot, PProcessEntry32W lppe);
  typedef BOOL (WINAPI *PfnProcess32NextW)(HANDLE hSnapshot, PProcessEntry32W lppe);

  typedef BOOL (WINAPI *PfnModule32FirstA)(HANDLE hSnapshot, PModuleEntry32A lpme);
  typedef BOOL (WINAPI *PfnModule32NextA)(HANDLE hSnapshot, PModuleEntry32A lpme);
  typedef BOOL (WINAPI *PfnModule32FirstW)(HANDLE hSnapshot, PModuleEntry32W lpme);
  typedef BOOL (WINAPI *PfnModule32NextW)(HANDLE hSnapshot, PModuleEntry32W lpme);

  typedef BOOL (WINAPI *PfnEnumProcessModules)(HANDLE hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded);
  typedef BOOL (WINAPI *PfnEnumProcessModulesEx)(HANDLE  hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded, DWORD dwFilterFlag);

  typedef BOOL (WINAPI *PfnEnumProcesses)(DWORD *pProcessIds, DWORD cb, DWORD *pBytesReturned);

  typedef DWORD (WINAPI *PfnGetModuleBaseNameA)(HANDLE hProcess, HMODULE hModule, LPSTR lpBaseName, DWORD nSize);
  typedef DWORD (WINAPI *PfnGetModuleBaseNameW)(HANDLE hProcess, HMODULE hModule, LPWSTR lpBaseName, DWORD nSize);

  typedef BOOL (WINAPI *PfnQueryFullProcessImageNameA)(HANDLE hProcess, DWORD  dwFlags, LPSTR lpExeName, PDWORD lpdwSize);
  typedef BOOL (WINAPI *PfnQueryFullProcessImageNameW)(HANDLE hProcess, DWORD  dwFlags, LPWSTR lpExeName, PDWORD lpdwSize);

  PfnCreateToolhelp32Snapshot pfnCreateToolhelp32Snapshot = nullptr;

  PfnProcess32FirstA pfnProcess32FirstA = nullptr;
  PfnProcess32NextA pfnProcess32NextA   = nullptr;
  PfnProcess32FirstW pfnProcess32FirstW = nullptr;
  PfnProcess32NextW pfnProcess32NextW   = nullptr;

  PfnModule32FirstA pfnModule32FirstA = nullptr;
  PfnModule32NextA pfnModule32NextA   = nullptr;
  PfnModule32FirstW pfnModule32FirstW = nullptr;
  PfnModule32NextW pfnModule32NextW   = nullptr;

  PfnEnumProcessModules pfnEnumProcessModules     = nullptr;
  PfnEnumProcessModulesEx pfnEnumProcessModulesEx = nullptr;

  PfnEnumProcesses pfnEnumProcesses = nullptr;

  PfnGetModuleBaseNameA pfnGetModuleBaseNameA = nullptr;
  PfnGetModuleBaseNameW pfnGetModuleBaseNameW = nullptr;

  PfnQueryFullProcessImageNameA pfnQueryFullProcessImageNameA = nullptr;
  PfnQueryFullProcessImageNameW pfnQueryFullProcessImageNameW = nullptr;

  CEResult ceapi ceInitTlHelp32()
  {
    if (g_HasToolHelp32) {
      return CE_OK;
    }

    CELibraryA krnl32("kernel32.dll");
    if (!krnl32.ceIsLibraryAvailable()) {
      return 1;
    }

    pfnCreateToolhelp32Snapshot = (PfnCreateToolhelp32Snapshot)krnl32.ceGetRoutineAddress("CreateToolhelp32Snapshot");
    if (pfnCreateToolhelp32Snapshot == nullptr) {
      return 2;
    }

    pfnProcess32FirstA = (PfnProcess32FirstA)krnl32.ceGetRoutineAddress("Process32First");
    if (pfnProcess32FirstA == nullptr) {
      return 3;
    }

    pfnProcess32NextA = (PfnProcess32NextA)krnl32.ceGetRoutineAddress("Process32Next");
    if (pfnProcess32NextA == nullptr) {
      return 4;
    }

    pfnProcess32FirstW = (PfnProcess32FirstW)krnl32.ceGetRoutineAddress("Process32FirstW");
    if (pfnProcess32FirstA == nullptr) {
      return 5;
    }

    pfnProcess32NextW = (PfnProcess32NextW)krnl32.ceGetRoutineAddress("Process32NextW");
    if (pfnProcess32NextW == nullptr) {
      return 6;
    }

    pfnModule32FirstA = (PfnModule32FirstA)krnl32.ceGetRoutineAddress("Module32First");
    if (pfnModule32FirstA == nullptr) {
      return 7;
    }

    pfnModule32NextA = (PfnModule32NextA)krnl32.ceGetRoutineAddress("Module32Next");
    if (pfnModule32NextA == nullptr) {
      return 8;
    }

    pfnModule32FirstW = (PfnModule32FirstW)krnl32.ceGetRoutineAddress("Module32FirstW");
    if (pfnModule32FirstW == nullptr) {
      return 9;
    }

    pfnModule32NextW = (PfnModule32NextW)krnl32.ceGetRoutineAddress("Module32NextW");
    if (pfnModule32NextW == nullptr) {
      return 10;
    }

    pfnEnumProcessModules = (PfnEnumProcessModules)krnl32.ceGetRoutineAddress("EnumProcessModules");
    if (pfnEnumProcessModules == nullptr) {
      pfnEnumProcessModules = (PfnEnumProcessModules)krnl32.ceGetRoutineAddress("K32EnumProcessModules");
      if (pfnEnumProcessModules == nullptr) {
        return 11;
      }
    }

    pfnEnumProcesses = (PfnEnumProcesses)krnl32.ceGetRoutineAddress("EnumProcesses");
    if (pfnEnumProcesses == nullptr) {
      pfnEnumProcesses = (PfnEnumProcesses)krnl32.ceGetRoutineAddress("K32EnumProcesses");
      if (pfnEnumProcesses == nullptr) {
        return 12;
      }
    }

    pfnEnumProcessModulesEx = (PfnEnumProcessModulesEx)krnl32.ceGetRoutineAddress("EnumProcessModulesEx");
    if (pfnEnumProcessModulesEx == nullptr) {
      pfnEnumProcessModulesEx = (PfnEnumProcessModulesEx)krnl32.ceGetRoutineAddress("K32EnumProcessModulesEx");
      if (pfnEnumProcessModulesEx == nullptr) {
        return 13;
      }
    }

    pfnGetModuleBaseNameA = (PfnGetModuleBaseNameA)krnl32.ceGetRoutineAddress("GetModuleBaseNameA");
    if (pfnGetModuleBaseNameA == nullptr) {
      pfnGetModuleBaseNameA = (PfnGetModuleBaseNameA)krnl32.ceGetRoutineAddress("K32GetModuleBaseNameA");
      if (pfnGetModuleBaseNameA == nullptr) {
        return 14;
      }
    }

    pfnGetModuleBaseNameW = (PfnGetModuleBaseNameW)krnl32.ceGetRoutineAddress("GetModuleBaseNameW");
    if (pfnGetModuleBaseNameW == nullptr) {
      pfnGetModuleBaseNameW = (PfnGetModuleBaseNameW)krnl32.ceGetRoutineAddress("K32GetModuleBaseNameW");
      if (pfnGetModuleBaseNameW == nullptr) {
        return 15;
      }
    }

    pfnQueryFullProcessImageNameA = (PfnQueryFullProcessImageNameA)krnl32.ceGetRoutineAddress("QueryFullProcessImageNameA");
    if (pfnQueryFullProcessImageNameA == nullptr) {
      return 16;
    }

    pfnQueryFullProcessImageNameW = (PfnQueryFullProcessImageNameW)krnl32.ceGetRoutineAddress("QueryFullProcessImageNameW");
    if (pfnQueryFullProcessImageNameW == nullptr) {
      return 17;
    }

    g_HasToolHelp32 = true;

    return CE_OK;
  }

  /* ------------------------------------------- Initialize Misc Routines ------------------------------------------- */

  bool g_HasMiscRoutine = false;

  typedef int (__cdecl *Pfn_vscprintf)(const char * format, va_list argptr);
  typedef int (__cdecl *Pfn_vscwprintf)(const wchar_t *format, va_list argptr);
  typedef BOOL (WINAPI *PfnCheckTokenMembership)(HANDLE TokenHandle, PSID SidToCheck, PBOOL IsMember);
  typedef LONG (WINAPI *PfnRegQueryReflectionKey)(HKEY hBase, BOOL *bIsReflectionDisabled);
  typedef LONG (WINAPI *PfnRegEnableReflectionKey)(HKEY hBase);
  typedef LONG (WINAPI *PfnRegDisableReflectionKey)(HKEY hBase);

  Pfn_vscprintf pfn_vscprintf   = nullptr;
  Pfn_vscwprintf pfn_vscwprintf = nullptr;
  PfnCheckTokenMembership pfnCheckTokenMembership = nullptr;
  PfnRegQueryReflectionKey pfnRegQueryReflectionKey = nullptr;
  PfnRegEnableReflectionKey pfnRegEnableReflectionKey = nullptr;
  PfnRegDisableReflectionKey pfnRegDisableReflectionKey = nullptr;

  CEResult ceapi ceInitMiscRoutine()
  {
    if (g_HasMiscRoutine) {
      return CE_OK;
    }

    pfn_vscprintf = (Pfn_vscprintf)CELibrary::ceGetRoutineAddressFast(TEXT("msvcrt.dll"), TEXT("_vscprintf"));
    if (pfn_vscprintf == nullptr) {
      return 1;
    }

    pfn_vscwprintf = (Pfn_vscwprintf)CELibrary::ceGetRoutineAddressFast(TEXT("msvcrt.dll"), TEXT("_vscwprintf"));
    if (pfn_vscwprintf == nullptr) {
      return 2;
    }

    pfnCheckTokenMembership = (PfnCheckTokenMembership)CELibrary::ceGetRoutineAddressFast(
      T("advapi32.dll"),
      T("CheckTokenMembership")
    );
    if (pfnCheckTokenMembership == nullptr) {
      return 3;
    }

    pfnRegQueryReflectionKey = (PfnRegQueryReflectionKey)CELibrary::ceGetRoutineAddressFast(
      T("advapi32.dll"),
      T("RegQueryReflectionKey")
    );
    if (pfnRegQueryReflectionKey == nullptr) {
      return 4;
    }

    pfnRegEnableReflectionKey = (PfnRegEnableReflectionKey)CELibrary::ceGetRoutineAddressFast(
      T("advapi32.dll"),
      T("RegEnableReflectionKey")
    );
    if (pfnRegEnableReflectionKey == nullptr) {
      return 5;
    }

    pfnRegDisableReflectionKey = (PfnRegDisableReflectionKey)CELibrary::ceGetRoutineAddressFast(
      T("advapi32.dll"),
      T("RegDisableReflectionKey")
    );
    if (pfnRegDisableReflectionKey == nullptr) {
      return 6;
    }

    g_HasMiscRoutine = true;

    return CE_OK;
  }

  /***************************************************** FUNCTIONS ****************************************************/

  /* ------------------------------------------------- Misc Working -------------------------------------------------- */

  bool ceapi ceIsAdministrator()
  {
    BOOL IsMember = FALSE;
    SID_IDENTIFIER_AUTHORITY IA = SECURITY_NT_AUTHORITY;
    PSID SID;

    if (ceInitMiscRoutine() != CE_OK) {
      return FALSE;
    }

    if (AllocateAndInitializeSid(&IA, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &SID)) {
      if (!pfnCheckTokenMembership(NULL, SID, &IsMember)) {
        IsMember = FALSE;
      }

      FreeSid(SID);
    }

    return (IsMember != FALSE);
  }

  std::string ceGetEnviromentA(const std::string EnvName)
  {
    std::string s;
    s.clear();

    std::shared_ptr<char> p(new char[MAX_SIZE]);
    if (p == nullptr) {
      return s;
    }

    ZeroMemory(p.get(), MAX_SIZE);

    ulong result = GetEnvironmentVariableA(EnvName.c_str(), p.get(), MAX_SIZE);
    if (result == ERROR_ENVVAR_NOT_FOUND || result == 0)
    {
      return s;
    }
    else if (result > MAX_SIZE) {
      result += 1;
      p.reset(new char[result]);
      if (p == nullptr) {
        return s;
      }

      ZeroMemory(p.get(), result);

      result = GetEnvironmentVariableA(EnvName.c_str(), p.get(), result);
      if (result == 0)
      {
        return s;
      }
    }

    s.assign(p.get());

    return s;
  }

  std::wstring ceGetEnviromentW(const std::wstring EnvName)
  {
    std::wstring ws;
    ws.clear();

    std::shared_ptr<wchar> p(new wchar[MAX_SIZE]);
    if (p == nullptr) {
      return ws;
    }

    ZeroMemory(p.get(), 2*MAX_SIZE);

    ulong result = GetEnvironmentVariableW(EnvName.c_str(), p.get(), 2*MAX_SIZE);
    if (result == ERROR_ENVVAR_NOT_FOUND || result == 0)
    {
      return ws;
    }
    else if (result > MAX_SIZE) {
      result += 1;
      p.reset(new wchar[result]);
      if (p == nullptr) {
        return ws;
      }

      ZeroMemory(p.get(), 2*result);

      result = GetEnvironmentVariableW(EnvName.c_str(), p.get(), 2*result);
      if (result == 0)
      {
        return ws;
      }
    }

    ws.assign(p.get());

    return ws;
  }

  /* ------------------------------------------------- Math Working ------------------------------------------------- */

  bool ceapi ceIsFlagOn(ulongptr ulFlags, ulongptr ulFlag)
  {
    return ((ulFlags & ulFlag) == ulFlag);
  }

  intptr ceapi ceGCD(ulongptr count, ...) /* Greatest Common Divisor -> BCNN */
  {
    va_list args;
    std::vector<intptr> array;
    intptr result = 0;

    array.clear();

    va_start(args, count);

    for (ulongptr i = 0; i < count; i++) {
      array.push_back(va_arg(args, intptr));
    }

    va_end(args);

    intptr min = *std::min_element(array.begin(), array.end());

    intptr pro = 1;
    for (auto i: array) {
      pro *= i;
    }

    if (pro == 0) {
      return 0;
    }

    ulongptr j;
    for (intptr i = min; i <= pro; i++) {
      for (j = 0; j < count; j++) {
        if (i % array[j] != 0) {
          break;
        }
      }
      if (j == count) {
        result = i;
        break;
      }
    }

    return result;
  }

  intptr ceapi ceLCM(ulongptr count, ...)
  { /* Least Common Multiples -> UCLN = |a1*a2*...*an|/GCD(a1, a2, ..., an) */
    va_list args;
    std::vector<intptr> array;
    intptr result = 0;

    array.clear();

    va_start(args, count);

    for (ulongptr i = 0; i < count; i++) {
      array.push_back(va_arg(args, intptr));
    }

    va_end(args);

    intptr min = *std::min_element(array.begin(), array.end());

    ulongptr j;
    for (intptr i = min - 1; i > 1; i--) {
      for (j = 0; j < count; j++) {
        if (array[j] % i != 0) {
          break;
        }
      }
      if (j == count) {
        result = i;
        break;
      }
    }

    return result;
  }

  void ceapi ceHexDump(void * Data, int Size)
  {
    const int DEFAULT_DUMP_COLUMN = 16;

    int i = 0;
    uchar Buffer[DEFAULT_DUMP_COLUMN + 1], *pData = (uchar*)Data;

    for (int i = 0; i < Size; i++) {
      if (i % DEFAULT_DUMP_COLUMN == 0) {
        if (i != 0) {
          printf("  %s\n", Buffer);
        }

        printf("  %04x ", i);
      }

      if (i % DEFAULT_DUMP_COLUMN == 8) printf(" ");

      printf(" %02x", pData[i]);

      if (pData[i] < 0x20 || pData[i] > 0x7E) {
        Buffer[i % DEFAULT_DUMP_COLUMN] = '.';
      }
      else {
        Buffer[i % DEFAULT_DUMP_COLUMN] = pData[i];
      }

      Buffer[(i % DEFAULT_DUMP_COLUMN) + 1] = '\0';
    }

    while (i % DEFAULT_DUMP_COLUMN != 0) {
      printf("   ");
      i++;
    }

    printf("  %s\n", Buffer);
  }

  /* --- Cat: String Formatting --- */

  int ceapi ceGetFormattedLengthA(const std::string Format, va_list args)
  {
    int N = -1;

    #ifdef _MSC_VER
      N = _vscprintf(Format.c_str(), args) + 1;
    #else
      N = pfn_vscprintf(Format.c_str(), args) + 1;
    #endif

    return N;
  }

  int ceapi ceGetFormattedLengthW(const std::wstring Format, va_list args)
  {
    int N = -1;

    #ifdef _MSC_VER
        N = _vscwprintf(Format.c_str(), args) + 1;
    #else
        N = pfn_vscwprintf(Format.c_str(), args) + 1;
    #endif

    return N;
  }

  std::string ceapi ceFormatA(const std::string Format, ...)
  {
    std::string s;
    s.clear();

    va_list args;
    va_start(args, Format);

    int N = ceGetFormattedLengthA(Format, args);
    std::shared_ptr<char> p(new char[N]);
    if (p == nullptr) {
      va_end(args);
      return s;
    }

    ZeroMemory(p.get(), N);

    wvsprintfA(p.get(), Format.c_str(), args);

    va_end(args);

    s.assign(p.get());

    return s;
  }

  std::wstring ceapi ceFormatW(const std::wstring Format, ...)
  {
    std::wstring ws;
    ws.clear();

    va_list args;
    va_start(args, Format);

    int N = ceGetFormattedLengthW(Format, args);
    std::shared_ptr<wchar> p(new wchar[N]);
    if (p == nullptr) {
      va_end(args);
      return ws;
    }

    ZeroMemory(p.get(), 2*N);

    wvsprintfW(p.get(), Format.c_str(), args);

    va_end(args);

    ws.assign(p.get());

    return ws;
  }

  void ceapi ceMsgA(const std::string Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthA(Format, args);
    std::shared_ptr<char> p(new char[N]);
    if (p == nullptr) {
      va_end(args);
      return;
    }

    ZeroMemory(p.get(), N);

    wvsprintfA(p.get(), Format.c_str(), args);

    va_end(args);

    OutputDebugStringA(p.get());
  }

  void ceapi ceMsgW(const std::wstring Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthW(Format, args);
    std::shared_ptr<wchar> p(new wchar[N]);
    if (p == nullptr) {
      va_end(args);
      return;
    }

    ZeroMemory(p.get(), 2*N);

    wvsprintfW(p.get(), Format.c_str(), args);

    va_end(args);

    OutputDebugStringW(p.get());
  }

  int ceapi ceBoxA(const std::string Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthA(Format, args);
    std::shared_ptr<char> p(new char[N]);
    if (p == nullptr) {
      va_end(args);
      return 0;
    }

    ZeroMemory(p.get(), N);

    wvsprintfA(p.get(), Format.c_str(), args);

    va_end(args);

    return MessageBoxA(GetActiveWindow(), p.get(), CE_TITLE_ANSI.c_str(), MB_ICONINFORMATION);
  }

  int ceapi ceBoxA(HWND hWnd, const std::string Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthA(Format, args);
    std::shared_ptr<char> p(new char[N]);
    if (p == nullptr) {
      va_end(args);
      return 0;
    }

    ZeroMemory(p.get(), N);

    wvsprintfA(p.get(), Format.c_str(), args);

    va_end(args);

    return MessageBoxA(hWnd, p.get(), CE_TITLE_ANSI.c_str(), MB_ICONINFORMATION);
  }

  int ceapi ceBoxA(HWND hWnd, uint uType, const std::string Caption, const std::string Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthA(Format, args);
    std::shared_ptr<char> p(new char[N]);
    if (p == nullptr) {
      va_end(args);
      return 0;
    }

    ZeroMemory(p.get(), N);

    wvsprintfA(p.get(), Format.c_str(), args);

    va_end(args);

    return MessageBoxA(hWnd, p.get(), Caption.c_str(), uType);
  }

  int ceapi ceBoxW(const std::wstring Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthW(Format, args);
    std::shared_ptr<wchar> p(new wchar[N]);
    if (p == nullptr) {
      va_end(args);
      return 0;
    }

    ZeroMemory(p.get(), 2*N);

    wvsprintfW(p.get(), Format.c_str(), args);

    va_end(args);

    return MessageBoxW(GetActiveWindow(), p.get(), CE_TITLE_UNICODE.c_str(), MB_ICONINFORMATION);
  }

  int ceapi ceBoxW(HWND hWnd, const std::wstring Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthW(Format, args);
    std::shared_ptr<wchar> p(new wchar[N]);
    if (p == nullptr) {
      va_end(args);
      return 0;
    }

    ZeroMemory(p.get(), 2*N);

    wvsprintfW(p.get(), Format.c_str(), args);

    va_end(args);

    return MessageBoxW(hWnd, p.get(), CE_TITLE_UNICODE.c_str(), MB_ICONINFORMATION);
  }

  int ceapi ceBoxW(HWND hWnd, uint uType, const std::wstring lpcwszCaption, const std::wstring Format, ...)
  {
    va_list args;

    va_start(args, Format);

    int N = ceGetFormattedLengthW(Format, args);
    std::shared_ptr<wchar> p(new wchar[N]);
    if (p == nullptr) {
      va_end(args);
      return 0;
    }

    ZeroMemory(p.get(), 2*N);

    wvsprintfW(p.get(), Format.c_str(), args);

    va_end(args);

    return MessageBoxW(hWnd, p.get(), lpcwszCaption.c_str(), uType);
  }

  std::string ceapi ceLastErrorA(ulong ulErrorCode)
  {
    if (ulErrorCode == -1) {
      ulErrorCode = GetLastError();
    }

    char * lpszErrorMessage = nullptr;

    FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      NULL,
      ulErrorCode,
      LANG_USER_DEFAULT,
      (char*)&lpszErrorMessage,
      0,
      NULL
    );

    std::string s(lpszErrorMessage);
    if (s.length() != 0) {
      s.erase(s.length() - 1);
    }

    return s;
  }

  std::wstring ceapi ceLastErrorW(ulong ulErrorCode)
  {
    if (ulErrorCode == -1) {
      ulErrorCode = GetLastError();
    }

    wchar_t * lpwszErrorMessage = nullptr;

    FormatMessageW(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
      NULL,
      ulErrorCode,
      LANG_USER_DEFAULT,
      (wchar_t*)&lpwszErrorMessage,
      0,
      NULL
    );

    std::wstring ws(lpwszErrorMessage);
    if (ws.length() != 0) {
      ws.erase(ws.length() - 1);
    }

    return ws;
  }

  std::string ceapi ceTimeDateToStringA(const time_t t)
  {
    std::string s = ceFormatTimeDateToStringA(t, "%H:%M:%S %d/%m/%Y");
    return s;
  }

  std::wstring ceapi ceTimeDateToStringW(const time_t t)
  {
    std::wstring ws = ceFormatTimeDateToStringW(t, L"%H:%M:%S %d/%m/%Y");
    return ws;
  }

  std::string ceapi ceFormatTimeDateToStringA(const time_t t, const std::string Format)
  {
    std::string s;
    s.clear();

    std::shared_ptr<char> p(new char[MAX_SIZE]);
    if (p == nullptr) {
      return s;
    }

    tm lt = {0};

    ZeroMemory(p.get(), MAX_SIZE);

    #if defined(_MSC_VER) && (_MSC_VER > 1200) // Above VC++ 6.0
    localtime_s(&lt, &t);
    #else
    memcpy((void*)&lt, localtime(&t), sizeof(tm));
    #endif

    strftime(p.get(), MAX_SIZE, Format.c_str(), &lt);

    s.assign(p.get());

    return s;
  }

  std::wstring ceapi ceFormatTimeDateToStringW(const time_t t, const std::wstring Format)
  {
    std::wstring ws;
    ws.clear();

    std::shared_ptr<wchar> p(new wchar[MAXBYTE]);
    if (p == nullptr) return ws;

    tm lt = {0};

    ZeroMemory(p.get(), 2*MAXBYTE);

    #if defined(_MSC_VER) && (_MSC_VER > 1200) // Above VC++ 6.0
    localtime_s(&lt, &t);
    #else
    memcpy((void*)&lt, localtime(&t), sizeof(tm));
    #endif

    wcsftime(p.get(), 2*MAXBYTE, Format.c_str(), &lt);

    ws.assign(p.get());

    return ws;
  }



  /* ------------------------------------------------ String Working ------------------------------------------------ */

  std::string ceapi ceLowerStringA(const std::string String)
  {
    std::string s(String);
    std::transform(s.begin(), s.end(), s.begin(), tolower);
    return s;
  }

  std::wstring ceapi ceLowerStringW(const std::wstring String)
  {
    std::wstring ws(String);
    std::transform(ws.begin(), ws.end(), ws.begin(), tolower);
    return ws;
  }

  std::string ceapi ceUpperStringA(const std::string String)
  {
    std::string s(String);
    std::transform(s.begin(), s.end(), s.begin(), toupper);
    return s;
  }

  std::wstring ceapi ceUpperStringW(const std::wstring String)
  {
    std::wstring ws(String);
    std::transform(ws.begin(), ws.end(), ws.begin(), toupper);
    return ws;
  }

  std::string ceapi cePwcToPac(const std::wstring String)
  {
    std::string s;
    s.clear();

    int N = (int)String.length() + 1;

    std::shared_ptr<char> p(new char[N]);
    if (p == nullptr) {
      return s;
    }

    ZeroMemory(p.get(), N);

    WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, String.c_str(), -1, p.get(), N, NULL, NULL);

    s.assign(p.get());

    return s;
  }

  std::wstring ceapi cePacToPwc(const std::string String)
  {
    std::wstring ws;
    ws.clear();

    int N = (int)String.length() + 1;

    std::shared_ptr<wchar_t> p(new wchar_t[N]);
    if (p == nullptr) {
      ws.clear();
    }

    ZeroMemory(p.get(), 2*N);

    MultiByteToWideChar(CP_ACP, 0, String.c_str(), -1, p.get(), 2*N);

    ws.assign(p.get());

    return ws;
  }

  std::list<std::string> ceapi ceSplitStringA(const std::string String, std::string Seperate)
  {
    std::string s(String), sep(Seperate), sub;
    ulongptr start, end;
    std::list<std::string> l;

    l.clear();

    start = 0;
    end = s.find(sep);
    while (end != std::string::npos) {
      sub = s.substr(start, end - start);
      l.push_back(std::string(sub.c_str()));
      start = end + sep.length();
      end = s.find(sep, start);
    }

    sub = s.substr(start, end);
    l.push_back(std::string(sub.c_str()));

    return l;
  }

  std::list<std::wstring> ceapi ceSplitStringW(const std::wstring String, const std::wstring Seperate)
  {
    std::wstring s(String), sep(Seperate), sub;
    ulongptr start, end;
    std::list<std::wstring> l;

    l.clear();

    start = 0;
    end = s.find(sep);
    while (end != std::wstring::npos) {
      sub = s.substr(start, end - start);
      l.push_back(std::wstring(sub.c_str()));
      start = end + sep.length();
      end = s.find(sep, start);
    }

    sub = s.substr(start, end);
    l.push_back(std::wstring(sub.c_str()));

    return l;
  }

  std::list<std::string> ceapi ceMultiStringToListA(const char * lpcszMultiString)
  {
    std::list<std::string> l;

    l.clear();

    while (*lpcszMultiString) {
      l.push_back(std::string(lpcszMultiString));
      lpcszMultiString += (lstrlenA(lpcszMultiString) + 1);
    }

    return l;
  }

  std::list<std::wstring> ceapi ceMultiStringToListW(const wchar_t * lpcwszMultiString)
  {
    std::list<std::wstring> l;

    l.clear();

    while (*lpcwszMultiString) {
      l.push_back(std::wstring(lpcwszMultiString));
      lpcwszMultiString += (lstrlenW(lpcwszMultiString) + 1);
    }

    return l;
  }

  std::shared_ptr<char> ceapi ceListToMultiStringA(std::list<std::string> StringList)
  {
    size_t ulLength = 0;
    for (std::string e : StringList) {
      ulLength += (e.length() + 1);
    }

    ulLength += 1; // End of multi string.

    std::shared_ptr<char> p(new char[ulLength]);
    if (p == nullptr) {
      return nullptr;
    }

    ZeroMemory(p.get(), ulLength);

    size_t iCrLength = 0;
    char * pP = p.get();
    for (std::string e : StringList) {
      iCrLength = e.length() + 1;
      lstrcpynA(pP, e.c_str(), (int)iCrLength);
      pP += iCrLength;
    }

    return p;
  }

  std::shared_ptr<wchar> ceapi ceListToMultiStringW(std::list<std::wstring> StringList)
  {
    size_t ulLength = 0;
    for (std::wstring e : StringList) {
      ulLength += (e.length() + 1);
    }

    ulLength += 1; // End of multi string.

    std::shared_ptr<wchar> p(new wchar[ulLength]);
    if (p == nullptr) {
      return nullptr;
    }

    ZeroMemory(p.get(), 2*ulLength);

    size_t iCrLength = 0;
    wchar * pP = p.get();
    for (std::wstring e : StringList) {
      iCrLength = e.length() + 1;
      lstrcpynW(pP, e.c_str(), (int)iCrLength);
      pP += iCrLength;
    }

    return p;
  }

  /* ------------------------------------------------ Process Working ----------------------------------------------- */

  HWND ceapi ceGetConsoleWindow()
  {
    typedef HWND (WINAPI *PfnGetConsoleWindow)();

    HWND hwConsole = NULL;

    PfnGetConsoleWindow pfnGetConsoleWindow = (PfnGetConsoleWindow)CELibrary::ceGetRoutineAddressFast(
      T("kernel32.dll"),
      T("GetConsoleWindow")
    );
    if (pfnGetConsoleWindow) {
      hwConsole = pfnGetConsoleWindow();
    }

    return hwConsole;
  }

  eProcessorArchitecture ceGetProcessorArchitecture()
  {
    typedef void (WINAPI *PfnGetNativeSystemInfo)(LPSYSTEM_INFO lpSystemInfo);

    PfnGetNativeSystemInfo pfnGetNativeSystemInfo = (PfnGetNativeSystemInfo)CELibrary::ceGetRoutineAddressFast(
      T("kernel32.dll"),
      T("GetNativeSystemInfo")
    );

    if (!pfnGetNativeSystemInfo) {
      return PA_UNKNOWN;
    }

    _SYSTEM_INFO si = {0};
    pfnGetNativeSystemInfo(&si);
    return static_cast<eProcessorArchitecture>(si.wProcessorArchitecture);
  }

  eWow64 ceapi ceIsWow64(ulong ulPID)
  {
    typedef BOOL (WINAPI *PfnIsWow64Process)(HANDLE, PBOOL);
    PfnIsWow64Process pfnIsWow64Process = (PfnIsWow64Process)CELibrary::ceGetRoutineAddressFast(
      T("kernel32.dll"),
      T("IsWow64Process")
    );
    if (pfnIsWow64Process == nullptr) {
      return WOW64_ERROR;
    }

    HANDLE hProcess = NULL;

    if (ulPID != (ulong)-1) {
      hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
    }
    else {
      hProcess = GetCurrentProcess();
    }

    BOOL bWow64 = false;
    if (!pfnIsWow64Process(hProcess, &bWow64)) {
      return WOW64_ERROR;
    }

    CloseHandle(hProcess);

    return (bWow64 ? WOW64_YES : WOW64_NO);
  }

  bool ceapi ceRPM(HANDLE hProcess, void* lpAddress, void* lpBuffer, SIZE_T ulSize)
  {
    ulong ulOldProtect = 0;
    SIZE_T ulRead = 0;

    VirtualProtectEx(hProcess, lpAddress, ulSize, PAGE_EXECUTE_READWRITE, &ulOldProtect);

    ReadProcessMemory(hProcess, (const void*)lpAddress, lpBuffer, ulSize, &ulRead);

    VirtualProtectEx(hProcess, lpAddress, ulSize, ulOldProtect, &ulOldProtect);

    if (ulRead != ulSize) {
      return false;
    }

    return true;
  }

  bool ceapi ceRPM(ulong ulPID, void* lpAddress, void* lpBuffer, SIZE_T ulSize)
  {
    ulong ulOldProtect = 0;
    SIZE_T ulRead = 0;

    if (!ulPID) {
      return false;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
    if (!hProcess) {
      return false;
    }

    ceRPM(hProcess, lpAddress, lpBuffer, ulSize);

    CloseHandle(hProcess);

    if (ulRead != ulSize) {
      return false;
    }

    return true;
  }

  bool ceapi ceWPM(HANDLE hProcess, void* lpAddress, const void* lpcBuffer, SIZE_T ulSize)
  {
    ulong ulOldProtect = 0;
    SIZE_T ulWritten = 0;

    VirtualProtectEx(hProcess, lpAddress, ulSize, PAGE_EXECUTE_READWRITE, &ulOldProtect);

    WriteProcessMemory(hProcess, lpAddress, lpcBuffer, ulSize, &ulWritten);

    VirtualProtectEx(hProcess, lpAddress, ulSize, ulOldProtect, &ulOldProtect);

    if (ulWritten != ulSize) {
      return false;
    }

    return true;
  }

  bool ceapi ceWPM(ulong ulPID, void* lpAddress, const void* lpcBuffer, SIZE_T ulSize)
  {
    ulong ulOldProtect = 0;
    SIZE_T ulWritten = 0;

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
    if (!hProcess) {
      return false;
    }

    ceWPM(hProcess, lpAddress, lpcBuffer, ulSize);

    CloseHandle(hProcess);

    if (ulWritten != ulSize) {
      return false;
    }

    return true;
  }

  ulong ceapi ceGetParentPID(ulong ulChildPID)
  {
    if (ceInitTlHelp32() != CE_OK) {
      return (ulong)INVALID_HANDLE_VALUE;
    }

    TProcessEntry32A pe;
    HANDLE hSnapshot;
    BOOL bNext;

    hSnapshot = pfnCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
      return (ulong)-1;
    }

    pe.dwSize = sizeof(TProcessEntry32A);

    bNext = pfnProcess32FirstA(hSnapshot, &pe);

    while (bNext) {
      if (pe.th32ProcessID == ulChildPID) {
        return pe.th32ParentProcessID;
      }
      bNext = pfnProcess32NextA(hSnapshot, &pe);
      pe.dwSize = sizeof(TProcessEntry32A);
    }

    return (ulong)-1;
  }

  std::list<ulong> ceapi ceNameToPidA(const std::string ProcessName, ulong ulMaxProcessNumber)
  {
    std::list<ulong> l;
    l.clear();

    if (ceInitTlHelp32() != CE_OK) {
      return l;
    }

    std::shared_ptr<ulong> pProcesses(new ulong[ulMaxProcessNumber]);
    if (pProcesses == nullptr) {
      return l;
    }

    ZeroMemory(pProcesses.get(), ulMaxProcessNumber*sizeof(ulong));

    ce::ulong cbNeeded = 0;
    pfnEnumProcesses(pProcesses.get(), ulMaxProcessNumber*sizeof(ce::ulong), &cbNeeded);

    if (cbNeeded <= 0) {
      return l;
    }

    ce::ulong nProcesses = cbNeeded / sizeof(ulong);

    std::string s1 = ceLowerStringA(ProcessName), s2;

    ulong PID;
    for (ce::ulong i = 0; i < nProcesses; i++) {
      PID = pProcesses.get()[i];

      s2.clear();
      s2 = ce::ceLowerStringA(ce::cePidToNameA(PID));
      if (s1 == s2) {
        l.push_back(PID);
      }
    }

    return l;
  }

  std::list<ulong> ceapi ceNameToPidW(const std::wstring ProcessName, ulong ulMaxProcessNumber)
  {
    std::list<ulong> l;
    l.clear();

    if (ceInitTlHelp32() != CE_OK) {
      return l;
    }

    std::shared_ptr<ulong> pProcesses(new ulong[ulMaxProcessNumber]);
    if (pProcesses == nullptr) {
      return l;
    }

    ZeroMemory(pProcesses.get(), ulMaxProcessNumber*sizeof(ulong));

    ce::ulong cbNeeded = 0;
    pfnEnumProcesses(pProcesses.get(), ulMaxProcessNumber*sizeof(ce::ulong), &cbNeeded);

    if (cbNeeded <= 0) {
      return l;
    }

    ce::ulong nProcesses = cbNeeded / sizeof(ulong);

    std::wstring s1 = ceLowerStringW(ProcessName), s2;

    ulong PID;
    for (ce::ulong i = 0; i < nProcesses; i++) {
      PID = pProcesses.get()[i];

      s2.clear();
      s2 = ce::ceLowerStringW(ce::cePidToNameW(PID));
      if (s1 == s2) {
        l.push_back(PID);
      }
    }

    return l;
  }

  std::string ceapi cePidToNameA(ulong ulPID)
  {
    std::string s;
    s.clear();

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
    if (!hProcess) {
      return s;
    }

    std::shared_ptr<char> szProcessPath(new char[MAXPATH]);
    ZeroMemory(szProcessPath.get(), MAXPATH);

    ulong ulPathLength = MAXPATH;

    BOOL ret = pfnQueryFullProcessImageNameA(hProcess, 0, szProcessPath.get(), &ulPathLength);

    CloseHandle(hProcess);

    if (ret == 0) {
      return s;
    }

    s.assign(szProcessPath.get());

    s = ceExtractFileNameA(s);

    return s;
  }

  std::wstring ceapi cePidToNameW(ulong ulPID)
  {
    std::wstring ws;
    ws.clear();

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
    if (!hProcess) {
      return ws;
    }

    std::shared_ptr<wchar> wszProcessPath(new wchar[MAXBYTE]);
    ZeroMemory(wszProcessPath.get(), 2*MAXBYTE);

    ulong ulPathLength = 2*MAXPATH;

    BOOL ret = pfnQueryFullProcessImageNameW(hProcess, 0, wszProcessPath.get(), &ulPathLength);

    CloseHandle(hProcess);

    if (ret == 0) {
      return ws;
    }

    ws.assign(wszProcessPath.get());

    ws = ceExtractFileNameW(ws);

    return ws;
  }

  HMODULE ceapi ceRemoteGetModuleHandleA(ulong ulPID, const std::string ModuleName)
  {
    /*TModuleEntry32A me32 = {0};
    HMODULE hModule = nullptr;

    if (ceInitTlHelp32() != CE_OK) {
    return (HMODULE)-1;
    }

    HANDLE hSnap = fnCreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, ulPID);
    if (hSnap == INVALID_HANDLE_VALUE) {
    return nullptr;
    }

    me32.dwSize = sizeof(me32);

    if (!fnModule32FirstA(hSnap, &me32)) {
    CloseHandle(hSnap);
    return nullptr;
    }

    std::string s2, s1 = ceLowerStringA(ModuleName);

    do {
    s2.clear();
    s2 = ceLowerStringA(me32.szModule);
    if (s1 == s2) {
    hModule = me32.hModule;
    break;
    };
    } while (fnModule32NextA(hSnap, &me32));

    CloseHandle(hSnap);

    return hModule;*/

    if (ceInitTlHelp32() != 0) {
      return (HMODULE)-1;
    }

    /*HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, ulPID);
    if (!hProcess) {
    return (HMODULE)-1;
    }

    HMODULE hModules[MAX_NPROCESSES] = {0};
    ulong cbNeeded = 0;
    pfnEnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded);

    ulong nModules = cbNeeded / sizeof(HMODULE);

    tchar ModuleName[MAXPATH];
    for (ulong i = 0; i < nModules; i++) {
    ZeroMemory(ModuleName, sizeof(ModuleName));
    pfnGetModuleFileNameEx(hProcess, hModules[i], ModuleName, sizeof(ModuleName));
    ceMsg(T("%d. %s"), i, ModuleName);
    //OutputDebugString(ModuleName);
    }

    CloseHandle(hProcess);*/

    return (HMODULE)-1;
  }

  HMODULE ceapi ceRemoteGetModuleHandleW(ulong ulPID, const std::wstring ModuleName)
  {
    if (ceInitTlHelp32() != CE_OK) {
      return (HMODULE)-1;
    }

    /*TModuleEntry32W me32 = {0};
    HMODULE hModule = nullptr;

    if (ceInitTlHelp32() != CE_OK) {
    return (HMODULE)-1;
    }

    HANDLE hSnap = pfnCreateToolhelp32Snapshot(TH32CS_SNAPMODULE, ulPID);
    if (hSnap == INVALID_HANDLE_VALUE) {
    return 0;
    }

    me32.dwSize = sizeof(me32);

    if (!pfnModule32FirstW(hSnap, &me32)) {
    CloseHandle(hSnap);
    return 0;
    }

    std::wstring s2, s1 = ceLowerStringW(ModuleName);

    do {
    s2.clear();
    s2 = ceLowerStringW(me32.szModule);
    if (s1 == s2) {
    hModule = me32.hModule;
    break;
    };
    } while (pfnModule32NextW(hSnap, &me32));

    CloseHandle(hSnap);

    return hModule;*/
    return (HMODULE)-1;
  }

  /* -------------------------------------------- File/Directory Working -------------------------------------------- */

  bool ceapi ceDirectoryExistsA(const std::string Directory)
  {
    if (GetFileAttributesA(Directory.c_str()) == INVALID_FILE_ATTRIBUTES) {
      return false;
    }
    else {
      return true;
    }
  }

  bool ceapi ceDirectoryExistsW(const std::wstring Directory)
  {
    if (GetFileAttributesW(Directory.c_str()) == INVALID_FILE_ATTRIBUTES) {
      return false;
    }
    else {
      return true;
    }
  }

  std::string ceapi ceFileTypeA(const std::string FilePath)
  {
    SHFILEINFOA SHINFO = {0};
    std::shared_ptr<char> szFileType(new char[MAXBYTE]);
    std::string s;
    s.clear();

    if (SHGetFileInfoA(FilePath.c_str(), 0, &SHINFO, sizeof(SHFILEINFOA), SHGFI_TYPENAME) != 0) {
      ZeroMemory(szFileType.get(), MAXBYTE);
      s = SHINFO.szTypeName;
    }

    return s;
  }

  std::wstring ceapi ceFileTypeW(const std::wstring FilePath)
  {
    SHFILEINFOW SHINFO = {0};
    std::shared_ptr<wchar> szFileType(new wchar[MAXBYTE]);
    std::wstring ws;
    ws.clear();

    if (SHGetFileInfoW(FilePath.c_str(), 0, &SHINFO, sizeof(SHFILEINFOA), SHGFI_TYPENAME) != 0) {
      ZeroMemory(szFileType.get(), 2*MAXBYTE);
      ws = SHINFO.szTypeName;
    }

    return ws;
  }

  bool ceapi ceFileExistsA(const std::string FilePath)
  {
    bool bResult = false;
    HANDLE hFile;
    WIN32_FIND_DATAA wfd = {0};

    hFile = FindFirstFileA(FilePath.c_str(), &wfd);
    if (hFile != INVALID_HANDLE_VALUE) {
      bResult = true;
      FindClose(hFile);
    }

    return bResult;
  }

  bool ceapi ceFileExistsW(const std::wstring FilePath)
  {
    bool bResult = false;
    HANDLE hFile;
    WIN32_FIND_DATAW wfd = {0};

    hFile = FindFirstFileW(FilePath.c_str(), &wfd);
    if (hFile != INVALID_HANDLE_VALUE) {
      bResult = true;
      FindClose(hFile);
    }

    return bResult;
  }

  std::string ceapi ceExtractFilePathA(const std::string FilePath, bool bIncludeSlash)
  {
    std::string filePath;
    filePath.clear();

    size_t slashPos = FilePath.rfind('\\');
    if (slashPos != std::string::npos) {
      filePath = FilePath.substr(0, slashPos + (bIncludeSlash ? 1 : 0));
    }

    return filePath;
  }

  std::wstring ceapi ceExtractFilePathW(const std::wstring FilePath, bool bIncludeSlash)
  {
    std::wstring filePath;
    filePath.clear();

    size_t slashPos = FilePath.rfind(L'\\');
    if (slashPos != std::string::npos) {
      filePath = FilePath.substr(0, slashPos + (bIncludeSlash ? 1 : 0));
    }

    return filePath;
  }

  std::string ceapi ceExtractFileNameA(const std::string FilePath, bool bIncludeExtension)
  {
    std::string fileName;
    fileName.clear();

    size_t slashPos = FilePath.rfind('\\');
    if (slashPos != std::string::npos) {
      fileName = FilePath.substr(slashPos + 1);
    }

    if (!bIncludeExtension) {
      size_t dotPos = fileName.rfind('.');
      if (dotPos != std::string::npos) {
        fileName = fileName.substr(0, dotPos);
      }
    }

    return fileName;
  }

  std::wstring ceapi ceExtractFileNameW(const std::wstring FilePath, bool bIncludeExtension)
  {
    std::wstring fileName;
    fileName.clear();

    size_t slashPos = FilePath.rfind(L'\\');
    if (slashPos != std::string::npos) {
      fileName = FilePath.substr(slashPos + 1);
    }

    if (!bIncludeExtension) {
      size_t dotPos = fileName.rfind(L'.');
      if (dotPos != std::string::npos) {
        fileName = fileName.substr(0, dotPos);
      }
    }

    return fileName;
  }

  std::string ceapi ceGetCurrentFilePathA()
  {
    std::shared_ptr<char> p(new char[MAXPATH]);

    ZeroMemory(p.get(), MAXPATH);

    HMODULE hModule = GetModuleHandleA(NULL);
    GetModuleFileNameA(hModule, p.get(), MAXPATH);
    FreeLibrary(hModule);

    std::string s(p.get());

    return s;
  }

  std::wstring ceapi ceGetCurrentFilePathW()
  {
    std::shared_ptr<wchar> p(new wchar[MAXPATH]);

    ZeroMemory(p.get(), 2*MAXPATH);

    HMODULE hModule = GetModuleHandleW(NULL);
    GetModuleFileNameW(hModule, p.get(), 2*MAXPATH);
    FreeLibrary(hModule);

    std::wstring ws(p.get());

    return ws;
  }

  std::string ceapi ceGetCurrentDirectoryA(bool bIncludeSlash)
  {
    return ceExtractFilePathA(ceGetCurrentFilePathA(), bIncludeSlash);
  }

  std::wstring ceapi ceGetCurrentDirectoryW(bool bIncludeSlash)
  {
    return ceExtractFilePathW(ceGetCurrentFilePathW(), bIncludeSlash);
  }



  /****************************************************** CLASSES *****************************************************/

  /* ---------------------------------------------------- Library --------------------------------------------------- */

  CELibraryA::CELibraryA()
  {
    m_ModuleName.clear();
    m_RoutineName.clear();

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CELibraryA::CELibraryA(const std::string ModuleName)
  {
    m_ModuleName = ModuleName;
    m_RoutineName.clear();

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CELibraryA::CELibraryA(const std::string ModuleName, const std::string RoutineName)
  {
    m_ModuleName  = ModuleName;
    m_RoutineName = RoutineName;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CELibraryA::~CELibraryA(){}

  bool ceapi CELibraryA::ceIsLibraryAvailable()
  {
    if (!m_ModuleName.empty()) {
      HMODULE hmod = LoadLibraryA(m_ModuleName.c_str());

      m_LastErrorCode = GetLastError();

      if (hmod != NULL) {
        FreeLibrary(hmod);
        return true;
      }
      return false;
    }
    else {
      return false;
    }
  }

  void* ceapi CELibraryA::ceGetRoutineAddress()
  {
    if (!m_ModuleName.empty() && !m_RoutineName.empty()) {
      return this->ceGetRoutineAddress(m_ModuleName, m_RoutineName);
    }

    return nullptr;
  }

  void* ceapi CELibraryA::ceGetRoutineAddress(const std::string RoutineName)
  {
    if (!m_ModuleName.empty() && !RoutineName.empty()) {
      return this->ceGetRoutineAddress(m_ModuleName, RoutineName);
    }

    return nullptr;
  }

  void* ceapi CELibraryA::ceGetRoutineAddress(const std::string ModuleName, const std::string RoutineName)
  {
    HMODULE hmod = 0;
    if (!ModuleName.empty()) hmod = LoadLibraryA(ModuleName.c_str());

    void* p = nullptr;
    if (hmod && !RoutineName.empty()) {
      p = (void*)GetProcAddress(hmod, RoutineName.c_str());
      m_LastErrorCode = GetLastError();
    }

    FreeLibrary(hmod);

    return p;
  }

  void* ceapi CELibraryA::ceGetRoutineAddressFast(const std::string ModuleName, const std::string RoutineName)
  {
    HMODULE hmod = 0;
    if (!ModuleName.empty()) hmod = LoadLibraryA(ModuleName.c_str());

    void* p = nullptr;
    if (hmod && !RoutineName.empty()) {
      p = (void*)GetProcAddress(hmod, RoutineName.c_str());
    }

    FreeLibrary(hmod);

    return p;
  }

  CELibraryW::CELibraryW()
  {
    m_ModuleName.clear();
    m_RoutineName.clear();

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CELibraryW::CELibraryW(const std::wstring ModuleName)
  {
    m_ModuleName = ModuleName;
    m_RoutineName.clear();

    m_LastErrorCode = GetLastError();
  }

  CELibraryW::CELibraryW(const std::wstring ModuleName, const std::wstring RoutineName)
  {
    m_ModuleName  = ModuleName;
    m_RoutineName = RoutineName;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CELibraryW::~CELibraryW(){}

  bool ceapi CELibraryW::ceIsLibraryAvailable()
  {
    if (!m_ModuleName.empty()) {
      HMODULE hmod = LoadLibraryW(m_ModuleName.c_str());

      m_LastErrorCode = GetLastError();

      if (hmod != nullptr) {
        FreeLibrary(hmod);
        return true;
      }
      return false;
    }
    else {
      return false;
    }
  }

  void* ceapi CELibraryW::ceGetRoutineAddress()
  {
    if (!m_ModuleName.empty() && !m_RoutineName.empty()) {
      return this->ceGetRoutineAddress(m_ModuleName, m_RoutineName);
    }

    return nullptr;
  }

  void* ceapi CELibraryW::ceGetRoutineAddress(const std::wstring RoutineName)
  {
    if (!m_ModuleName.empty() && !RoutineName.empty()) {
      return this->ceGetRoutineAddress(m_ModuleName, RoutineName);
    }

    return nullptr;
  }

  void* ceapi CELibraryW::ceGetRoutineAddress(const std::wstring ModuleName, const std::wstring RoutineName)
  {
    HMODULE hmod = 0;
    if (!ModuleName.empty()) hmod = LoadLibraryW(ModuleName.c_str());

    void* p = nullptr;
    if (hmod && !RoutineName.empty()) {
      std::string s = cePwcToPac(RoutineName);
      p = (void*)GetProcAddress(hmod, s.c_str());
      m_LastErrorCode = GetLastError();
    }

    FreeLibrary(hmod);

    return p;
  }

  void* ceapi CELibraryW::ceGetRoutineAddressFast(const std::wstring ModuleName, const std::wstring RoutineName)
  {
    HMODULE hmod = 0;
    if (!ModuleName.empty()) hmod = LoadLibraryW(ModuleName.c_str());

    void* p = nullptr;
    if (hmod && !RoutineName.empty()) {
      std::string s = cePwcToPac(RoutineName);
      p = (void*)GetProcAddress(hmod, s.c_str());
    }

    FreeLibrary(hmod);

    return p;
  }

  /* --- Cat : Socket --- */

  CESocket::CESocket()
  {
    m_Socket = INVALID_SOCKET;

    memset((void*)&m_WSAData, 0, sizeof(m_WSAData));
    memset((void*)&m_Server, 0, sizeof(m_Server));
    memset((void*)&m_Client, 0, sizeof(m_Client));

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CESocket::CESocket(eSocketAF socketAF, eSocketType socketType)
  {
    m_Socket = INVALID_SOCKET;

    memset((void*)&m_WSAData, 0, sizeof(m_WSAData));
    memset((void*)&m_Server, 0, sizeof(m_Server));
    memset((void*)&m_Client, 0, sizeof(m_Client));

    m_LastErrorCode = ERROR_SUCCESS;

    this->ceSocket(socketAF, socketType);
  }

  CESocket::~CESocket() {}

  bool ceapi CESocket::ceIsSocketValid(SOCKET socket)
  {
    if (!socket || socket == INVALID_SOCKET) {
      return false;
    }

    return true;
  }

  CEResult ceapi CESocket::ceSocket(eSocketAF socketAF, eSocketType socketType, eSocketProtocol socketProtocol)
  {
    if (WSAStartup(MAKEWORD(2, 2), &m_WSAData) != 0) {
      m_LastErrorCode = GetLastError();
      return 1;
    }

    m_Socket = socket(socketAF, socketType, socketProtocol);
    if (!this->ceIsSocketValid(m_Socket)) {
      return 2;
    }

    m_Server.sin_family = socketAF;

    return CE_OK;
  }

  CEResult ceapi CESocket::ceBind(TAccessPoint accessPoint)
  {
    return this->ceBind(accessPoint.Host, accessPoint.Port);
  }

  CEResult ceapi CESocket::ceBind(const std::string Address, unsigned short usPort)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return 1;
    }

    std::string IP;

    if (this->ceIsHostName(Address) == true) {
      IP = this->ceGetHostByName(Address);
    }
    else {
      IP = Address;
    }

    if (IP.empty()) {
      return 2;
    }

    m_Server.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
    m_Server.sin_port = htons(usPort);

    if (bind(m_Socket, (const struct sockaddr*)&m_Server, sizeof(m_Server)) == SOCKET_ERROR) {
      m_LastErrorCode = GetLastError();
      return 3;
    }

    return CE_OK;
  }

  CEResult ceapi CESocket::ceListen(int iMaxConnection)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return 1;
    }

    int result = listen(m_Socket, iMaxConnection);

    m_LastErrorCode = GetLastError();

    return (result == SOCKET_ERROR ? 2 : CE_OK);
  }

  CEResult ceapi CESocket::ceAccept(TSocketInfomation& socketInformation)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return 1;
    }

    memset((void*)&socketInformation, 0, sizeof(socketInformation));

    int size = sizeof(socketInformation.sai);

    socketInformation.s = accept(m_Socket, (struct sockaddr*)&socketInformation.sai, &size);

    m_LastErrorCode = GetLastError();

    if (!this->ceIsSocketValid(socketInformation.s)) {
      return 2;
    }

    this->ceBytesToIP(socketInformation);

    return CE_OK;
  }

  CEResult ceapi CESocket::ceConnect(const TAccessPoint ap)
  {
    return this->ceConnect(ap.Host, ap.Port);
  }

  CEResult ceapi CESocket::ceConnect(const std::string Address, unsigned short usPort)
  {
    std::string IP;

    if (this->ceIsHostName(Address) == true) {
      IP = this->ceGetHostByName(Address);
    }
    else {
      IP = Address;
    }

    if (IP.empty()) {
      return 1;
    }

    m_Server.sin_addr.S_un.S_addr = inet_addr(IP.c_str());
    m_Server.sin_port = htons(usPort);

    if (connect(m_Socket, (const struct sockaddr*)&m_Server, sizeof(m_Server)) == SOCKET_ERROR) {
      m_LastErrorCode = GetLastError();
      this->ceClose();
      return 2;
    }

    return CE_OK;
  }

  IResult ceapi CESocket::ceSend(const char * lpData, int iLength, eSocketMessage socketMessage)
  {
    return this->ceSend(m_Socket, lpData, iLength, socketMessage);
  }

  IResult ceapi CESocket::ceSend(SOCKET socket, const char * lpData, int iLength, eSocketMessage socketMessage)
  {
    if (!this->ceIsSocketValid(socket)) {
      return SOCKET_ERROR;
    }

    IResult z;
    if ((z = send(socket, lpData, iLength, socketMessage)) == SOCKET_ERROR) {
      m_LastErrorCode = GetLastError();
      return SOCKET_ERROR;
    }

    return z;
  }

  IResult ceapi CESocket::ceRecv(char* lpData, int iLength, eSocketMessage socketMessage)
  {
    return this->ceRecv(m_Socket, lpData, iLength, socketMessage);
  }

  IResult ceapi CESocket::ceRecv(SOCKET socket, char * lpData, int iLength, eSocketMessage socketMessage)
  {
    if (!this->ceIsSocketValid(socket)) {
      return SOCKET_ERROR;
    }

    IResult z;
    if ((z = recv(socket, lpData, iLength, socketMessage)) == SOCKET_ERROR) {
      m_LastErrorCode = GetLastError();
      return SOCKET_ERROR;
    }

    if (iLength > 0) {
      lpData[iLength - 1] = '\0';
    }

    return z;
  }

  IResult ceapi CESocket::ceSendTo(const char * lpData, int iLength, TSocketInfomation& si)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return SOCKET_ERROR;
    }

    int result = sendto(m_Socket, lpData, iLength, 0, (const struct sockaddr*)&si.sai, sizeof(si.sai));

    m_LastErrorCode = GetLastError();

    return result;
  }

  IResult ceapi CESocket::ceRecvFrom(char * lpData, int iLength, TSocketInfomation& si)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return SOCKET_ERROR;
    }

    int n = sizeof(si.sai), z = SOCKET_ERROR;
    if ((z = recvfrom(m_Socket, lpData, iLength, 0, (struct sockaddr *)&si.sai, &n)) == SOCKET_ERROR) {
      m_LastErrorCode = GetLastError();
      return SOCKET_ERROR;
    }

    this->ceBytesToIP(si);

    return z;
  }

  bool ceapi CESocket::ceClose(SOCKET socket)
  {
    if (socket != 0) {
      if (!this->ceIsSocketValid(socket)) return false;
      closesocket(socket);
    }
    else if (this->ceIsSocketValid(m_Socket)) {
      closesocket(m_Socket);
      m_Socket = 0;
    }

    WSACleanup();

    m_LastErrorCode = GetLastError();

    return true;
  }

  SOCKET ceapi CESocket::ceGetSocket()
  {
    return m_Socket;
  }

  CEResult ceapi CESocket::ceGetOption(int iLevel, int iOptName, std::string OptVal, int * lpiLength)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return 1;
    }

    std::shared_ptr<char> p(new char[MAXBYTE]);
    if (p == nullptr) {
      return 2;
    }

    memset(p.get(), 0, MAXBYTE);

    if (getsockopt(m_Socket, iLevel, iOptName, p.get(), lpiLength) != 0) {
      m_LastErrorCode = GetLastError();
      return 3;
    }

    OptVal = p.get();

    return CE_OK;
  }

  CEResult ceapi CESocket::ceSetOption(int iLevel, int iOptName, const std::string OptVal, int iLength)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return 1;
    }

    if (OptVal.empty()) {
      return 2;
    }

    if (setsockopt(m_Socket, iLevel, iOptName, OptVal.c_str(), iLength) != 0) {
      m_LastErrorCode = GetLastError();
      return 3;
    }

    return CE_OK;
  }

  CEResult ceapi CESocket::ceShutdown(eShutdownFlag shutdownFlag)
  {
    if (!this->ceIsSocketValid(m_Socket)) {
      return 1;
    }

    if (shutdown(m_Socket, (int)shutdownFlag) == SOCKET_ERROR) {
      m_LastErrorCode = GetLastError();
      return 2;
    }

    return CE_OK;
  }

  std::string ceapi CESocket::ceGetLocalHostName()
  {
    std::string r;
    r.clear();

    std::shared_ptr<char> h(new char[MAXBYTE]);
    if (h == nullptr) {
      return r;
    }

    if (!this->ceIsSocketValid(m_Socket)) {
      return r;
    }

    memset(h.get(), 0, MAXBYTE);
    if (gethostname(h.get(), MAXBYTE) == SOCKET_ERROR) {
      m_LastErrorCode = GetLastError();
      return r;
    }

    r.assign(h.get());

    return r;
  }

  std::string ceapi CESocket::ceGetHostByName(const std::string Name)
  {
    std::string r;
    r.clear();

    if (!this->ceIsSocketValid(m_Socket)) {
      WSASetLastError(6);  // WSA_INVALID_HANDLE
      return r;
    }

    if (Name.empty()) {
      WSASetLastError(87); // WSA_INVALID_PARAMETER
      r = CE_LOCALHOST;
      return r;
    }

    if (Name.length() >= MAXBYTE) {
      WSASetLastError(87); // WSA_INVALID_PARAMETER
      return r;
    }

    hostent * h = gethostbyname(Name.c_str());
    if (h == nullptr) {
      return r;
    }

    if (h->h_addr_list[0] == nullptr || strlen(h->h_addr_list[0]) == 0) {
      return r;
    }

    in_addr a = {0};
    memcpy((void*)&a, (void*)h->h_addr_list[0], sizeof(a));
    r = inet_ntoa(a);

    return r;
  }

  bool ceapi CESocket::ceIsHostName(const std::string s)
  {
    bool r = false;
    const std::string MASK = "01234567890.";

    if (s.empty()) {
      return r;
    }

    if (s.length() >= MAXBYTE) {
      return r;
    }

    for (unsigned int i = 0; i < s.length(); i++) {
      if (strchr(MASK.c_str(), s[i]) == nullptr) {
        r = true;
        break;
      }
    }

    return r;
  }

  bool ceapi CESocket::ceBytesToIP(TSocketInfomation& socketInformation)
  {
    if (sprintf(
        (char*)socketInformation.ip,
        "%d.%d.%d.%d\0",
        socketInformation.sai.sin_addr.S_un.S_un_b.s_b1,
        socketInformation.sai.sin_addr.S_un.S_un_b.s_b2,
        socketInformation.sai.sin_addr.S_un.S_un_b.s_b3,
        socketInformation.sai.sin_addr.S_un.S_un_b.s_b4
      ) < 0) return false;
    else return true;
  }

  ulongptr ceapi CEDynHookSupport::ceJumpLen(ulongptr ulSrcAddress, ulongptr ulDestAddress)
  {
    return (ulDestAddress - ulSrcAddress - JMP_OPCODE_SIZE);
  }

  bool ceapi CEDynHookSupport::ceHandleMemoryInstruction(const HDE::tagHDE& hde, const ulong offset)
  {
    bool result = true;

    /* MessageBoxA
    00000000772512B8 | 48 83 EC 38                    | sub rsp,38                                                       |
    00000000772512BC | 45 33 DB                       | xor r11d,r11d                                                    |
    00000000772512BF | 44 39 1D 76 0E 02 00           | cmp dword ptr ds:[7727213C],r11d                                 |
    00000000772512C6 | 74 2E                          | je user32.772512F6                                               |
    00000000772512C8 | 65 48 8B 04 25 30 00 00 00     | mov rax,qword ptr gs:[30]                                        |
    00000000772512D1 | 4C 8B 50 48                    | mov r10,qword ptr ds:[rax+48]                                    |
    00000000772512D5 | 33 C0                          | xor eax,eax                                                      |
    */

    // http://staff.ustc.edu.cn/~xlanchen/cailiao/x86%20Assembly%20Programming.htm (find: Operand Addressing)
    
    /* Memory Instruction
      This  for memory instructions like cmp dword ptr ds:[xxxxxxxx],r11d, mov dword ptr ds:[xxxxxxxx],r11d, ...
    */
    #ifdef _WIN64
      namespace HDE = HDE64;
    #else
      namespace HDE = HDE32;
    #endif // _WIN64

    if ((hde.flags & HDE::F_DISP8) || (hde.flags & HDE::F_DISP16) || (hde.flags & HDE::F_DISP32)) {
      TMemoryInstruction mi = {0};
      mi.Offset   = offset;
      //mi.Position = 3; // Windows 64 API

      ulong ulDispSize = 0;
      if ((hde.flags & HDE::F_DISP8) == HDE::F_DISP8) {
        mi.MemoryAddressType = eMemoryAddressType::MAT_8;
        mi.MAO.A8 = hde.disp.disp8;
        ulDispSize = 1;
      }
      else if ((hde.flags & HDE::F_DISP16) == HDE::F_DISP16) {
        mi.MemoryAddressType = eMemoryAddressType::MAT_16;
        mi.MAO.A16 = hde.disp.disp16;
        ulDispSize = 2;
      }
      else if ((hde.flags & HDE::F_DISP32) == HDE::F_DISP32) {
        mi.MemoryAddressType = eMemoryAddressType::MAT_32;
        mi.MAO.A32 = hde.disp.disp32;
        ulDispSize = 4;
      }

      mi.Position = hde.len - ulDispSize;
      m_ListMemoryInstruction.push_back(mi);
    }

    return result;
  }

  bool ceapi CEDynHookSupport::ceStartDetour(void* pProc, void* pHookProc, void** pOldProc)
  {
    /* x86
    EIP + 0 | FF 25 ?? ?? ?? ??       | JMP DWORD PTR DS:[XXXXXXXX] ; Jump to XXXXXXXX

    // x64
    RIP + 0 | FF 25 ?? ??? ?? ??      | JMP QWORD PTR DS:[RIP+6] ; Jump to [RIP + 6]
    RIP + 6 | ?? ?? ?? ?? ?? ?? ?? ?? | XXXXXXXXXXXXXXXX
    */

    /* MessageBoxA
    --- x64 ---
    00000000772512B8 | 48 83 EC 38                    | sub rsp,38                                                       |
    00000000772512BC | 45 33 DB                       | xor r11d,r11d                                                    |
    00000000772512BF | 44 39 1D 76 0E 02 00           | cmp dword ptr ds:[7727213C],r11d                                 |
    00000000772512C6 | 74 2E                          | je user32.772512F6                                               |
    00000000772512C8 | 65 48 8B 04 25 30 00 00 00     | mov rax,qword ptr gs:[30]                                        |
    00000000772512D1 | 4C 8B 50 48                    | mov r10,qword ptr ds:[rax+48]                                    |
    00000000772512D5 | 33 C0                          | xor eax,eax                                                      |

    --- x86 ---
    750AFD1E 8B FF                mov         edi,edi  
    750AFD20 55                   push        ebp  
    750AFD21 8B EC                mov         ebp,esp  
    750AFD23 6A 00                push        0  
    750AFD25 FF 75 14             push        dword ptr [ebp+14h]  
    750AFD28 FF 75 10             push        dword ptr [ebp+10h]  
    750AFD2B FF 75 0C             push        dword ptr [ebp+0Ch]  
    750AFD2E FF 75 08             push        dword ptr [ebp+8]  
    750AFD31 E8 A0 FF FF FF       call        750AFCD6  
    750AFD36 5D                   pop         ebp  
    750AFD37 C2 10 00             ret         10h  
    */

    TRedirect O2N = {0}, T2O = {0};
    ulong iTrampolineSize = 0;
    HDE::tagHDE hde = {0};
    bool bFoundTrampolineSize = true;

    #ifdef _WIN64
        namespace HDE = HDE64;
    #else
        namespace HDE = HDE32;
    #endif // _WIN64

    do {
      HDEDisasm((const void *)((ulongptr)pProc + iTrampolineSize), &hde);
      if ((hde.flags & HDE::F_ERROR) == HDE::F_ERROR) {
        bFoundTrampolineSize = false;
        break;
      }
      else {
        this->ceHandleMemoryInstruction(hde, iTrampolineSize);
      }

      iTrampolineSize += hde.len;
    } while (iTrampolineSize < MIN_HOOK_SIZE);

    if (!bFoundTrampolineSize)
    {
      return false;
    }

    // pOldProc is a trampoline function
    *pOldProc = (void*)VirtualAlloc(nullptr, iTrampolineSize + sizeof(TRedirect), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    if (*pOldProc == nullptr) {
      return false;
    }

    memcpy(*pOldProc, pProc, iTrampolineSize);

    // fix memory instruction
    if (m_ListMemoryInstruction.size() > 0) {
      for (auto e: m_ListMemoryInstruction) {
        if (e.MemoryAddressType == eMemoryAddressType::MAT_32) {
          auto v = (ulongptr)pProc - (ulongptr)*pOldProc + (ulongptr)e.MAO.A32;
          *(ulongptr*)((ulongptr)*pOldProc + e.Offset + e.Position) = (ulongptr)v;
        }
      }
    }

    T2O.JMP = 0x25FF;
    #ifdef _WIN64
      T2O.Unknown = 0; // JMP_OPCODE_SIZE
    #else  // _WIN32
      T2O.Unknown = ((ulongptr)*pOldProc + iTrampolineSize) + sizeof(T2O.JMP) + sizeof(T2O.Unknown);
    #endif // _WIN64
    T2O.Address = ((ulongptr)pProc + iTrampolineSize);

    // Write the jump code (trampoline -> original) on the bottom of the trampoline function
    memcpy((void*)((ulongptr)*pOldProc + iTrampolineSize), (void*)&T2O, sizeof(T2O));

    ulong ulOldProtect = 0;
    VirtualProtect(pProc, iTrampolineSize, PAGE_EXECUTE_READWRITE, &ulOldProtect);

    O2N.JMP = 0x25FF;
    #ifdef _WIN64
      O2N.Unknown = 0; // JMP_OPCODE_SIZE;
    #else  // _WIN32
      O2N.Unknown = ((ulongptr)pProc + sizeof(O2N.JMP) + sizeof(O2N.Unknown));
    #endif // _WIN64
    O2N.Address = (ulongptr)pHookProc;

    // Write the jump code (original -> new) on the top of the target function
    memcpy(pProc, (void*)&O2N, sizeof(O2N));

    return true;
  }

  bool ceapi CEDynHookSupport::ceStopDetour(void* pProc, void** pOldProc)
  {
    if (!m_Hooked) {
      return false;
    }

    // fix memory instruction
    if (m_ListMemoryInstruction.size() > 0) {
      for (auto e: m_ListMemoryInstruction) {
        if (e.MemoryAddressType == eMemoryAddressType::MAT_32) {
          *(ulongptr*)((ulongptr)*pOldProc + e.Offset + e.Position) = (ulongptr)e.MAO.A32;
        }
      }
    }

    memcpy(pProc, *pOldProc, MIN_HOOK_SIZE);

    VirtualFree(*pOldProc, MIN_HOOK_SIZE + sizeof(TRedirect), MEM_RELEASE);

    *pOldProc = nullptr;

    return true;
  }

  bool ceapi CEDynHookA::ceAPIAttach(
    const std::string ModuleName,
    const std::string ProcName,
    void* lpHookProc,
    void** lpOldProc
  )
  {
    void* lpProc = CELibraryA::ceGetRoutineAddressFast(ModuleName, ProcName);
    if (lpProc == nullptr) {
      return false;
    }

    m_Hooked = this->ceStartDetour(lpProc, lpHookProc, lpOldProc);

    return m_Hooked;
  }

  bool ceapi CEDynHookA::ceAPIDetach(const std::string ModuleName, const std::string ProcName, void** lpOldProc
  )
  {
    if (!m_Hooked) {
      return false;
    }

    void* lpProc = CELibraryA::ceGetRoutineAddressFast(ModuleName, ProcName);
    if (lpProc == nullptr) {
      return false;
    }

    return this->ceStopDetour(lpProc, lpOldProc);
  }

  bool ceapi CEDynHookW::ceAPIAttach(
    const std::wstring ModuleName,
    const std::wstring ProcName,
    void* lpHookProc,
    void** lpOldProc
    )
  {
    void* lpProc = CELibraryW::ceGetRoutineAddressFast(ModuleName, ProcName);
    if (lpProc == nullptr) {
      return false;
    }

    m_Hooked = this->ceStartDetour(lpProc, lpHookProc, lpOldProc);

    return m_Hooked;
  }

  bool ceapi CEDynHookW::ceAPIDetach(const std::wstring ModuleName, const std::wstring ProcName, void** lpOldProc)
  {
    if (!m_Hooked) {
      return false;
    }

    void* lpProc = CELibraryW::ceGetRoutineAddressFast(ModuleName, ProcName);
    if (lpProc == nullptr) {
      return false;
    }

    return this->ceStopDetour(lpProc, lpOldProc);
  }

  /* --- Cat : File Mapping --- */

  CEFileMappingA::CEFileMappingA()
  {
    m_FileHandle = INVALID_HANDLE_VALUE;
    m_MapHandle  = INVALID_HANDLE_VALUE;
    m_pData      = nullptr;
    m_HasInit    = false;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CEFileMappingA::~CEFileMappingA()
  {
    this->ceClose();
  }

  CEResult ceapi CEFileMappingA::ceInit(const std::string FileName, eFileGenericFlags fgFlag, eFileShareFlags fsFlag)
  {
    if (FileName.empty()) {
      return 1;
    }

    m_FileHandle = CreateFileA(FileName.c_str(), fgFlag, fsFlag, NULL, OPEN_EXISTING, 0, 0);

    m_LastErrorCode = GetLastError();

    if (!this->ceIsValidHandle(m_FileHandle)) {
      return 2;
    }

    m_HasInit = true;

    return CE_OK;
  }

  CEResult ceapi CEFileMappingA::ceCreate(const std::string MapName, ulong ulMaxSizeHigh, ulong ulMaxSizeLow)
  {
    if (MapName.empty()) {
      return 1;
    }

    if (!m_HasInit || !this->ceIsValidHandle(m_FileHandle)) {
      return 2;
    }

    m_MapHandle = CreateFileMappingA(
      m_FileHandle,
      NULL,
      PAGE_READWRITE,
      ulMaxSizeHigh,
      ulMaxSizeLow,
      MapName.c_str()
    );

    m_LastErrorCode = GetLastError();

    if (!this->ceIsValidHandle(m_MapHandle)) {
      return 3;
    }

    return CE_OK;
  }

  CEResult ceapi CEFileMappingA::ceOpen(const std::string MapName, bool bInheritHandle)
  {
    if (MapName.empty()) {
      return 1;
    }

    m_MapHandle = OpenFileMappingA(FILE_MAP_ALL_ACCESS, bInheritHandle, MapName.c_str());

    m_LastErrorCode = GetLastError();

    if (!this->ceIsValidHandle(m_MapHandle)) {
      return 2;
    }

    return CE_OK;
  }

  void* ceapi CEFileMappingA::ceView(
    ulong ulMaxFileOffsetHigh,
    ulong ulMaxFileOffsetLow,
    ulong ulNumberOfBytesToMap
  )
  {
    if (!this->ceIsValidHandle(m_MapHandle)) {
      return nullptr;
    }

    m_pData = MapViewOfFile(
      m_MapHandle,
      FILE_MAP_ALL_ACCESS,
      ulMaxFileOffsetHigh,
      ulMaxFileOffsetLow,
      ulNumberOfBytesToMap
    );

    m_LastErrorCode = GetLastError();

    return m_pData;
  }

  void ceapi CEFileMappingA::ceClose()
  {
    if (m_pData != nullptr) {
      UnmapViewOfFile(m_pData);
      m_pData = nullptr;
    }

    if (this->ceIsValidHandle(m_MapHandle)) {
      CloseHandle(m_MapHandle);
      m_MapHandle = INVALID_HANDLE_VALUE;
    }

    if (this->ceIsValidHandle(m_FileHandle)) {
      CloseHandle(m_FileHandle);
      m_FileHandle = INVALID_HANDLE_VALUE;
    }
  }

  ulong ceapi CEFileMappingA::ceGetFileSize()
  {
    if (!this->ceIsValidHandle(m_FileHandle)) {
      return INVALID_FILE_SIZE;
    }

    ulong result = GetFileSize(m_FileHandle, NULL);

    m_LastErrorCode = GetLastError();

    return result;
  }

  CEFileMappingW::CEFileMappingW()
  {
    m_FileHandle = INVALID_HANDLE_VALUE;
    m_MapHandle  = INVALID_HANDLE_VALUE;
    m_pData      = nullptr;
    m_HasInit    = false;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CEFileMappingW::~CEFileMappingW()
  {
    this->ceClose();
  }

  CEResult ceapi CEFileMappingW::ceInit(
    const std::wstring FileName,
    eFileGenericFlags fgFlag,
    eFileShareFlags fsFlag
  )
  {
    if (FileName.empty()) {
      return 1;
    }

    m_FileHandle = CreateFileW(FileName.c_str(), fgFlag, fsFlag, NULL, OPEN_EXISTING, 0, 0);
    if (!this->ceIsValidHandle(m_FileHandle)) {
      return 2;
    }

    m_LastErrorCode = GetLastError();

    m_HasInit = true;

    return CE_OK;
  }

  CEResult ceapi CEFileMappingW::ceCreate(
    const std::wstring MapName,
    ulong ulMaxSizeHigh,
    ulong ulMaxSizeLow
  )
  {
    if (MapName.empty()) {
      return 1;
    }

    if (!m_HasInit || !this->ceIsValidHandle(m_FileHandle)) {
      return 2;
    }

    m_MapHandle = CreateFileMappingW(
      m_FileHandle,
      NULL,
      PAGE_READWRITE,
      ulMaxSizeHigh,
      ulMaxSizeLow,
      MapName.c_str()
    );

    m_LastErrorCode = GetLastError();

    if (!this->ceIsValidHandle(m_MapHandle)) {
      return 3;
    }

    return CE_OK;
  }

  CEResult ceapi CEFileMappingW::ceOpen(const std::wstring MapName, bool bInheritHandle)
  {
    if (MapName.empty()) {
      return 1;
    }

    m_MapHandle = OpenFileMappingW(FILE_MAP_ALL_ACCESS, bInheritHandle, MapName.c_str());

    m_LastErrorCode = GetLastError();

    if (!this->ceIsValidHandle(m_MapHandle)) {
      return 2;
    }

    return CE_OK;
  }

  void* ceapi CEFileMappingW::ceView(
    ulong ulMaxFileOffsetHigh,
    ulong ulMaxFileOffsetLow,
    ulong ulNumberOfBytesToMap
  )
  {
    if (!this->ceIsValidHandle(m_MapHandle)) {
      return nullptr;
    }

    m_pData = MapViewOfFile(
      m_MapHandle,
      FILE_MAP_ALL_ACCESS,
      ulMaxFileOffsetHigh,
      ulMaxFileOffsetLow,
      ulNumberOfBytesToMap
    );

    m_LastErrorCode = GetLastError();

    return m_pData;
  }

  void ceapi CEFileMappingW::ceClose()
  {
    if (m_pData != nullptr) {
      UnmapViewOfFile(m_pData);
      m_pData = nullptr;
    }

    if (this->ceIsValidHandle(m_MapHandle)) {
      CloseHandle(m_MapHandle);
      m_MapHandle = INVALID_HANDLE_VALUE;
    }

    if (this->ceIsValidHandle(m_FileHandle)) {
      CloseHandle(m_FileHandle);
      m_FileHandle = INVALID_HANDLE_VALUE;
    }
  }

  ulong ceapi CEFileMappingW::ceGetFileSize()
  {
    if (!this->ceIsValidHandle(m_FileHandle)) {
      return INVALID_FILE_SIZE;
    }

    ulong result = GetFileSize(m_FileHandle, NULL);

    m_LastErrorCode = GetLastError();

    return result;
  }

  /* --- Cat : INI File --- */

  // ANSI Version

  CEIniFileA::CEIniFileA(const std::string FilePath)
  {
    m_FilePath = FilePath;
  }

  CEIniFileA::CEIniFileA(const std::string FilePath, const std::string Section)
  {
    m_FilePath = FilePath;
    m_Section  = Section;
  }

  void CEIniFileA::ceValidFilePath()
  {
    if (m_FilePath.empty()) {
      std::string filePath = ceGetCurrentFilePathA();
      std::string fileDir  = ceExtractFilePathA(filePath, true);
      std::string fileName = ceExtractFileNameA(filePath, false);
      m_FilePath = fileDir + fileName + ".INI";
    }
  }

  void CEIniFileA::ceSetCurrentFilePath(const std::string FilePath)
  {
    m_FilePath = FilePath;
  }

  void CEIniFileA::ceSetCurrentSection(const std::string Section)
  {
    m_Section = Section;
  }

  // Long-Read

  std::list<std::string> ceapi CEIniFileA::ceReadSectionNames(ulong ulMaxSize)
  {
    std::list<std::string> l;
    l.clear();

    std::shared_ptr<char> p(new char[ulMaxSize]);
    if (p == nullptr) {
      return l;
    }

    ZeroMemory(p.get(), ulMaxSize);

    this->ceValidFilePath();

    GetPrivateProfileSectionNamesA(p.get(), ulMaxSize, m_FilePath.c_str());

    m_LastErrorCode = GetLastError();

    for (char * s = p.get(); *s; s++) {
      l.push_back(s); // l.push_back((new std::string(s))->c_str());
      s += lstrlenA(s);
    }

    return l;
  }

  std::list<std::string> ceapi CEIniFileA::ceReadSection(const std::string Section, ulong ulMaxSize)
  {
    std::list<std::string> l;
    l.clear();

    std::shared_ptr<char> p(new char[ulMaxSize]);
    if (p == nullptr) {
      return l;
    }

    ZeroMemory(p.get(), ulMaxSize);

    this->ceValidFilePath();

    GetPrivateProfileSectionA(Section.c_str(), p.get(), ulMaxSize, m_FilePath.c_str());

    m_LastErrorCode = GetLastError();

    for (char * s = p.get(); *s; s++) {
      l.push_back(s); // l.push_back((new std::string(s))->c_str());
      s += lstrlenA(s);
    }

    return l;
  }

  int ceapi CEIniFileA::ceReadInteger(const std::string Section, const std::string Key, int Default)
  {
    this->ceValidFilePath();
    uint result = GetPrivateProfileIntA(Section.c_str(), Key.c_str(), Default, m_FilePath.c_str());
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileA::ceReadBool(const std::string Section, const std::string Key, bool Default)
  {
    this->ceValidFilePath();
    bool result = (GetPrivateProfileIntA(Section.c_str(), Key.c_str(), Default, m_FilePath.c_str()) == 1 ? true : false);
    m_LastErrorCode = GetLastError();
    return result;
  }

  float ceapi CEIniFileA::ceReadFloat(const std::string Section, const std::string Key, float Default)
  {
    const std::string sDefault = std::to_string(Default);
    char lpszResult[MAX_SIZE];

    ZeroMemory(lpszResult, sizeof(lpszResult));

    this->ceValidFilePath();

    GetPrivateProfileStringA(Section.c_str(), Key.c_str(), sDefault.c_str(), lpszResult, MAX_SIZE, m_FilePath.c_str());

    m_LastErrorCode = GetLastError();

    return (float)atof(lpszResult);
  }

  std::string ceapi CEIniFileA::ceReadString(const std::string Section, const std::string Key, const std::string Default)
  {
    std::string s;
    s.clear();

    std::shared_ptr<char> p(new char[MAX_SIZE]);
    if (p == nullptr) {
      return s;
    }

    ZeroMemory(p.get(), MAX_SIZE);

    this->ceValidFilePath();

    ulong result = GetPrivateProfileStringA(
      Section.c_str(),
      Key.c_str(),
      Default.c_str(),
      p.get(),
      MAX_SIZE,
      m_FilePath.c_str()
    );

    if (result == 0) {
      m_LastErrorCode = GetLastError();
      return s;
    }

    s.assign(p.get());

    return s;
  }

  std::shared_ptr<void> ceapi CEIniFileA::ceReadStruct(const std::string Section, const std::string Key, ulong ulSize)
  {
    std::shared_ptr<void> p(malloc(ulSize));
    if (p == nullptr) {
      return nullptr;
    }

    ZeroMemory(p.get(), ulSize);

    this->ceValidFilePath();

    if (GetPrivateProfileStructA(Section.c_str(), Key.c_str(), p.get(), ulSize, m_FilePath.c_str()) == 0) {
      m_LastErrorCode = GetLastError();
      return nullptr;
    }

    return p;
  }

  // Short-Read

  std::list<std::string> ceapi CEIniFileA::ceReadSection(ulong ulMaxSize)
  {
    return this->ceReadSection(m_Section, ulMaxSize);
  }

  int ceapi CEIniFileA::ceReadInteger(const std::string Key, int Default)
  {
    return this->ceReadInteger(m_Section, Key, Default);
  }

  bool ceapi CEIniFileA::ceReadBool(const std::string Key, bool Default)
  {
    return this->ceReadBool(m_Section, Key, Default);
  }

  float ceapi CEIniFileA::ceReadFloat(const std::string Key, float Default)
  {
    return this->ceReadFloat(m_Section, Key, Default);
  }

  std::string ceapi CEIniFileA::ceReadString(const std::string Key, const std::string Default)
  {
    return this->ceReadString(m_Section, Key, Default);
  }

  std::shared_ptr<void> ceapi CEIniFileA::ceReadStruct(const std::string Key, ulong ulSize)
  {
    return this->ceReadStruct(m_Section, Key, ulSize);
  }

  // Long-Write

  bool ceapi CEIniFileA::ceWriteInteger(const std::string Section, const std::string Key, int Value)
  {
    this->ceValidFilePath();
    const std::string s = std::to_string(Value);
    bool result = (WritePrivateProfileStringA(Section.c_str(), Key.c_str(), s.c_str(), m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileA::ceWriteBool(const std::string Section, const std::string Key, bool Value)
  {
    this->ceValidFilePath();
    const std::string s(Value ? "1" : "0");
    bool result = (WritePrivateProfileStringA(Section.c_str(), Key.c_str(), s.c_str(), m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileA::ceWriteFloat(const std::string Section, const std::string Key, float Value)
  {
    this->ceValidFilePath();
    const std::string s = std::to_string(Value);
    bool result = (WritePrivateProfileStringA(Section.c_str(), Key.c_str(), s.c_str(), m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileA::ceWriteString(const std::string Section, const std::string Key, const std::string Value)
  {
    this->ceValidFilePath();
    bool result = (WritePrivateProfileStringA(Section.c_str(), Key.c_str(), Value.c_str(), m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileA::ceWriteStruct(const std::string Section, const std::string Key, void * pStruct, ulong ulSize)
  {
    this->ceValidFilePath();
    bool result = (WritePrivateProfileStructA(Section.c_str(), Key.c_str(), pStruct, ulSize, m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  // Short-Write

  bool ceapi CEIniFileA::ceWriteInteger(const std::string Key, int Value)
  {
    return this->ceWriteInteger(m_Section, Key, Value);
  }

  bool ceapi CEIniFileA::ceWriteBool(const std::string Key, bool Value)
  {
    return this->ceWriteBool(m_Section, Key, Value);
  }

  bool ceapi CEIniFileA::ceWriteFloat(const std::string Key, float Value)
  {
    return this->ceWriteFloat(m_Section, Key, Value);
  }

  bool ceapi CEIniFileA::ceWriteString(const std::string Key, const std::string Value)
  {
    return this->ceWriteString(m_Section, Key, Value);
  }

  bool ceapi CEIniFileA::ceWriteStruct(const std::string Key, void * pStruct, ulong ulSize)
  {
    return this->ceWriteStruct(m_Section, Key, pStruct, ulSize);
  }

  // UNICODE Version

  CEIniFileW::CEIniFileW(const std::wstring FilePath)
  {
    m_FilePath = FilePath;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CEIniFileW::CEIniFileW(const std::wstring FilePath, const std::wstring Section)
  {
    m_FilePath = FilePath;
    m_Section  = Section;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  void CEIniFileW::ceValidFilePath()
  {
    if (m_FilePath.empty()) {
      std::wstring filePath = ceGetCurrentFilePathW();
      std::wstring fileDir = ceExtractFilePathW(filePath, true);
      std::wstring fileName = ceExtractFileNameW(filePath, false);
      m_FilePath = fileDir + fileName + L".INI";
    }
  }

  void CEIniFileW::ceSetCurrentFilePath(const std::wstring FilePath)
  {
    m_FilePath = FilePath;
  }

  void CEIniFileW::ceSetCurrentSection(const std::wstring Section)
  {
    m_Section = Section;
  }

  // Long-Read

  std::list<std::wstring> ceapi CEIniFileW::ceReadSectionNames(ulong ulMaxSize)
  {
    std::list<std::wstring> l;
    l.clear();

    std::shared_ptr<wchar> p(new wchar[ulMaxSize]);
    if (p == nullptr) {
      return l;
    }

    ZeroMemory(p.get(), 2*ulMaxSize);

    this->ceValidFilePath();

    GetPrivateProfileSectionNamesW(p.get(), 2*ulMaxSize, m_FilePath.c_str());

    m_LastErrorCode = GetLastError();

    for (wchar * s = p.get(); *s; s++) {
      l.push_back(s); // l.push_back((new std::wstring(s))->c_str());
      s += lstrlenW(s);
    }

    return l;
  }

  std::list<std::wstring> ceapi CEIniFileW::ceReadSection(const std::wstring Section, ulong ulMaxSize)
  {
    std::list<std::wstring> l;
    l.clear();

    std::shared_ptr<wchar> p(new wchar[ulMaxSize]);
    if (p == nullptr) {
      return l;
    }

    ZeroMemory(p.get(), 2*ulMaxSize);

    this->ceValidFilePath();

    GetPrivateProfileSectionW(Section.c_str(), p.get(), 2*ulMaxSize, m_FilePath.c_str());

    m_LastErrorCode = GetLastError();

    for (wchar * s = p.get(); *s; s++) {
      l.push_back(s); // l.push_back((new std::wstring(s))->c_str());
      s += lstrlenW(s);
    }

    return l;
  }

  int ceapi CEIniFileW::ceReadInteger(const std::wstring Section, const std::wstring Key, int Default)
  {
    this->ceValidFilePath();
    int result = GetPrivateProfileIntW(Section.c_str(), Key.c_str(), Default, m_FilePath.c_str());
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileW::ceReadBool(const std::wstring Section, const std::wstring Key, bool Default)
  {
    this->ceValidFilePath();
    bool result = (GetPrivateProfileIntW(Section.c_str(), Key.c_str(), Default, m_FilePath.c_str()) == 1 ? true : false);
    m_LastErrorCode = GetLastError();
    return result;
  }

  float ceapi CEIniFileW::ceReadFloat(const std::wstring Section, const std::wstring Key, float Default)
  {
    this->ceValidFilePath();

    const std::wstring sDefault = std::to_wstring(Default);
    wchar lpwszResult[MAX_SIZE];

    ZeroMemory(lpwszResult, sizeof(lpwszResult));

    GetPrivateProfileStringW(Section.c_str(), Key.c_str(), sDefault.c_str(), lpwszResult, sizeof(lpwszResult), m_FilePath.c_str());

    m_LastErrorCode = GetLastError();

    const std::string s = cePwcToPac(lpwszResult);

    return (float)atof(s.c_str());
  }

  std::wstring ceapi CEIniFileW::ceReadString(const std::wstring Section, const std::wstring Key, const std::wstring Default)
  {
    std::wstring ws;
    ws.clear();

    std::shared_ptr<wchar> p(new wchar[MAX_SIZE]);
    if (p == nullptr) {
      return ws;
    }

    ZeroMemory(p.get(), 2*MAX_SIZE);

    this->ceValidFilePath();

    ulong result = GetPrivateProfileStringW(
      Section.c_str(),
      Key.c_str(),
      Default.c_str(),
      p.get(),
      2*MAX_SIZE,
      m_FilePath.c_str()
    );
    if (result == 0) {
      m_LastErrorCode = GetLastError();
      return ws;
    }

    ws.assign(p.get());

    return ws;
  }

  std::shared_ptr<void> ceapi CEIniFileW::ceReadStruct(const std::wstring Section, const std::wstring Key, ulong ulSize)
  {
    std::shared_ptr<void> p(malloc(ulSize));
    if (p == nullptr) {
      return nullptr;
    }

    ZeroMemory(p.get(), ulSize);

    this->ceValidFilePath();

    if (GetPrivateProfileStructW(Section.c_str(), Key.c_str(), p.get(), ulSize, m_FilePath.c_str()) == 0) {
      m_LastErrorCode = GetLastError();
      return nullptr;
    }

    return p;
  }

  // Short-Read

  std::list<std::wstring> ceapi CEIniFileW::ceReadSection(ulong ulMaxSize)
  {
    return this->ceReadSection(m_Section, ulMaxSize);
  }

  int ceapi CEIniFileW::ceReadInteger(const std::wstring Key, int Default)
  {
    return this->ceReadInteger(m_Section, Key, Default);
  }

  bool ceapi CEIniFileW::ceReadBool(const std::wstring Key, bool Default)
  {
    return this->ceReadBool(m_Section, Key, Default);
  }

  float ceapi CEIniFileW::ceReadFloat(const std::wstring Key, float Default)
  {
    return this->ceReadFloat(m_Section, Key, Default);
  }

  std::wstring ceapi CEIniFileW::ceReadString(const std::wstring Key, const std::wstring Default)
  {
    return this->ceReadString(m_Section, Key, Default);
  }

  std::shared_ptr<void> ceapi CEIniFileW::ceReadStruct(const std::wstring Key, ulong ulSize)
  {
    return this->ceReadStruct(m_Section, Key, ulSize);
  }

  // Long-Write

  bool ceapi CEIniFileW::ceWriteInteger(const std::wstring Section, const std::wstring Key, int Value)
  {
    this->ceValidFilePath();
    const std::wstring ws = std::to_wstring(Value);
    bool result = (WritePrivateProfileStringW(Section.c_str(), Key.c_str(), ws.c_str(), m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileW::ceWriteBool(const std::wstring Section, const std::wstring Key, bool Value)
  {
    this->ceValidFilePath();
    const std::wstring ws(Value ? L"1" : L"0");
    return (WritePrivateProfileStringW(Section.c_str(), Key.c_str(), ws.c_str(), m_FilePath.c_str()) != 0);
  }

  bool ceapi CEIniFileW::ceWriteFloat(const std::wstring Section, const std::wstring Key, float Value)
  {
    this->ceValidFilePath();
    const std::wstring ws = std::to_wstring(Value);
    bool result = (WritePrivateProfileStringW(Section.c_str(), Key.c_str(), ws.c_str(), m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileW::ceWriteString(const std::wstring Section, const std::wstring Key, const std::wstring Value)
  {
    this->ceValidFilePath();
    bool result = (WritePrivateProfileStringW(Section.c_str(), Key.c_str(), Value.c_str(), m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  bool ceapi CEIniFileW::ceWriteStruct(const std::wstring Section, const std::wstring Key, void * pStruct, ulong ulSize)
  {
    this->ceValidFilePath();
    bool result = (WritePrivateProfileStructW(Section.c_str(), Key.c_str(), pStruct, ulSize, m_FilePath.c_str()) != 0);
    m_LastErrorCode = GetLastError();
    return result;
  }

  // Short-Write

  bool ceapi CEIniFileW::ceWriteInteger(const std::wstring Key, int Value)
  {
    return this->ceWriteInteger(m_Section.c_str(), Key.c_str(), Value);
  }

  bool ceapi CEIniFileW::ceWriteBool(const std::wstring Key, bool Value)
  {
    return this->ceWriteBool(m_Section.c_str(), Key.c_str(), Value);
  }

  bool ceapi CEIniFileW::ceWriteFloat(const std::wstring Key, float Value)
  {
    return this->ceWriteFloat(m_Section.c_str(), Key.c_str(), Value);
  }

  bool ceapi CEIniFileW::ceWriteString(const std::wstring Key, const std::wstring Value)
  {
    return this->ceWriteString(m_Section.c_str(), Key.c_str(), Value);
  }

  bool ceapi CEIniFileW::ceWriteStruct(const std::wstring Key, void * pStruct, ulong ulSize)
  {
    return this->ceWriteStruct(m_Section.c_str(), Key.c_str(), pStruct, ulSize);
  }

  /* --- Cat : Registry --- */

  HKEY ceapi CERegistrySupport::ceGetCurrentKeyHandle()
  {
    return m_HKSubKey;
  }

  eRegReflection ceapi CERegistrySupport::ceQueryReflectionKey()
  {
    BOOL bReflectedDisabled = FALSE;

    if (ceInitMiscRoutine() != CE_OK) {
      return eRegReflection::RR_ERROR;
    }

    if (pfnRegQueryReflectionKey(m_HKSubKey, &bReflectedDisabled) != ERROR_SUCCESS) {
      return eRegReflection::RR_ERROR;
    }

    if (bReflectedDisabled == TRUE)
    {
      return eRegReflection::RR_DISABLED;
    }
    else {
      return eRegReflection::RR_ENABLED;
    }
  }

  bool ceapi CERegistrySupport::ceSetReflectionKey(eRegReflection RegReflection)
  {
    bool result = false;

    if (ceInitMiscRoutine() != CE_OK) {
      return result;
    }

    switch (RegReflection)
    {
    case eRegReflection::RR_DISABLE:
      result = (pfnRegDisableReflectionKey(m_HKSubKey) == ERROR_SUCCESS);
      break;
    case eRegReflection::RR_ENABLE:
      result = (pfnRegEnableReflectionKey(m_HKSubKey) == ERROR_SUCCESS);
      break;
    default:
      result = false;
      break;
    }

    return result;
  }

  bool ceapi CERegistrySupport::ceCloseKey()
  {
    m_LastErrorCode = RegCloseKey(m_HKSubKey);
    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  // Registry - ANSI Version

  CERegistryA::CERegistryA()
  {
    m_HKRootKey = NULL;
    m_HKSubKey = NULL;
    m_SubKey.clear();

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CERegistryA::CERegistryA(eRegRoot RegRoot)
  {
    m_HKRootKey = (HKEY)RegRoot;
    m_HKSubKey = NULL;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CERegistryA::CERegistryA(eRegRoot RegRoot, const std::string SubKey)
  {
    m_HKRootKey = (HKEY)RegRoot;
    m_HKSubKey = NULL;
    m_SubKey = SubKey;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  ulong ceapi CERegistryA::ceGetSizeOfMultiString(const char * lpcszMultiString)
  {
    ulong ulLength = 0;

    while (*lpcszMultiString) {
      ulong crLength = lstrlenA(lpcszMultiString) + sizeof(char);
      ulLength += crLength;
      lpcszMultiString = lpcszMultiString + crLength;
    }

    return ulLength;
  }

  ulong ceapi CERegistryA::ceGetDataSize(const std::string ValueName, ulong ulType)
  {
    ulong ulDataSize = 0;

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, NULL, &ulDataSize);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return 0;
    }

    return ulDataSize;
  }

  bool ceapi CERegistryA::ceCreateKey()
  {
    return this->ceCreateKey(m_SubKey);
  }

  bool ceapi CERegistryA::ceCreateKey(const std::string SubKey)
  {
    m_LastErrorCode = RegCreateKeyA(m_HKRootKey, SubKey.c_str(), &m_HKSubKey);
    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceKeyExists()
  {
    return this->ceKeyExists(m_SubKey);
  }

  bool ceapi CERegistryA::ceKeyExists(const std::string SubKey)
  {
    m_LastErrorCode = RegOpenKeyA(m_HKRootKey, SubKey.c_str(), &m_HKSubKey);
    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceOpenKey(eRegAccess RegAccess)
  {
    return this->ceOpenKey(m_SubKey, RegAccess);
  }

  bool ceapi CERegistryA::ceOpenKey(const std::string SubKey, eRegAccess RegAccess)
  {
    m_LastErrorCode = RegOpenKeyExA(m_HKRootKey, SubKey.c_str(), 0, (REGSAM)RegAccess, &m_HKSubKey);
    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceDeleteKey()
  {
    return this->ceDeleteKey(m_SubKey);
  }

  bool ceapi CERegistryA::ceDeleteKey(const std::string SubKey)
  {
    if (m_HKRootKey == 0) {
      return false;
    }

    m_LastErrorCode = RegDeleteKeyA(m_HKRootKey, SubKey.c_str());

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceDeleteValue(const std::string Value)
  {
    if (m_HKSubKey == 0) {
      return false;
    }

    m_LastErrorCode = RegDeleteValueA(m_HKSubKey, Value.c_str());

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  std::list<std::string> ceapi CERegistryA::ceEnumKeys()
  {
    std::list<std::string> l;
    l.clear();

    char Class[MAXPATH] = { 0 };
    ulong cchClass = MAXPATH;
    ulong cSubKeys = 0;
    ulong cbMaxSubKeyLen = 0;
    ulong cbMaxClassLen = 0;
    ulong cValues = 0;
    ulong cbMaxValueNameLen = 0;
    ulong cbMaxValueLen = 0;
    ulong cbSecurityDescriptor = 0;
    FILETIME ftLastWriteTime = { 0 };

    ZeroMemory(&Class, sizeof(Class));
    ZeroMemory(&ftLastWriteTime, sizeof(ftLastWriteTime));

    m_LastErrorCode = RegQueryInfoKeyA(
      m_HKSubKey,
      (char*)&Class,
      &cchClass,
      NULL,
      &cSubKeys,
      &cbMaxSubKeyLen,
      &cbMaxClassLen,
      &cValues,
      &cbMaxValueNameLen,
      &cbMaxValueLen,
      &cbSecurityDescriptor,
      &ftLastWriteTime
    );

    if (m_LastErrorCode != ERROR_SUCCESS || cSubKeys == 0) return l;

    cbMaxSubKeyLen += 1;

    char * pSubKeyName = new char[cbMaxSubKeyLen];

    for (ulong i = 0; i < cSubKeys; i++) {
      ZeroMemory(pSubKeyName, cbMaxSubKeyLen);
      m_LastErrorCode = RegEnumKeyA(m_HKSubKey, i, pSubKeyName, cbMaxSubKeyLen);
      l.push_back(pSubKeyName);
    }

    delete[] pSubKeyName;

    return l;
  }

  std::list<std::string> ceapi CERegistryA::ceEnumValues()
  {
    std::list<std::string> l;
    l.clear();

    char Class[MAXPATH] = { 0 };
    ulong cchClass = MAXPATH;
    ulong cSubKeys = 0;
    ulong cbMaxSubKeyLen = 0;
    ulong cbMaxClassLen = 0;
    ulong cValues = 0;
    ulong cbMaxValueNameLen = 0;
    ulong cbMaxValueLen = 0;
    ulong cbSecurityDescriptor = 0;
    FILETIME ftLastWriteTime = { 0 };

    ZeroMemory(&Class, sizeof(Class));
    ZeroMemory(&ftLastWriteTime, sizeof(ftLastWriteTime));

    m_LastErrorCode = RegQueryInfoKeyA(
      m_HKSubKey,
      (char*)&Class,
      &cchClass,
      NULL,
      &cSubKeys,
      &cbMaxSubKeyLen,
      &cbMaxClassLen,
      &cValues,
      &cbMaxValueNameLen,
      &cbMaxValueLen,
      &cbSecurityDescriptor,
      &ftLastWriteTime
    );

    if (m_LastErrorCode != ERROR_SUCCESS || cValues == 0) return l;

    cbMaxValueNameLen += 1;

    char * pValueName = new char[cbMaxValueNameLen];
    ulong ulValueNameLength;
    for (ulong i = 0; i < cValues; i++) {
      ulValueNameLength = cbMaxValueNameLen;
      ZeroMemory(pValueName, cbMaxValueNameLen);
      m_LastErrorCode = RegEnumValueA(m_HKSubKey, i, pValueName, &ulValueNameLength, NULL, NULL, NULL, NULL);
      l.push_back(pValueName);
    }

    delete[] pValueName;

    return l;
  }

  // Write

  bool ceapi CERegistryA::ceWriteInteger(const std::string ValueName, int Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExA(m_HKSubKey, ValueName.c_str(), 0, REG_DWORD, (const uchar*)&Value, sizeof(Value));

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceWriteBool(const std::string ValueName, bool Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExA(m_HKSubKey, ValueName.c_str(), 0, REG_BINARY, (const uchar*)&Value, sizeof(Value));

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceWriteFloat(const std::string ValueName, float Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExA(m_HKSubKey, ValueName.c_str(), 0, REG_BINARY, (const uchar*)&Value, sizeof(Value));

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  // For REG_SZ, REG_MULTI_SZ and REG_EXPAND_SZ:
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724884(v=vs.85).aspx

  bool ceapi CERegistryA::ceWriteString(const std::string ValueName, const std::string Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExA(
      m_HKSubKey,
      ValueName.c_str(),
      0,
      REG_SZ,
      (const uchar*)Value.c_str(),
      (ulong)Value.length() + sizeof(char)
    );

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceWriteMultiString(const std::string ValueName, const char * lpValue)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExA(
      m_HKSubKey,
      ValueName.c_str(),
      0,
      REG_MULTI_SZ,
      (const uchar*)lpValue,
      this->ceGetSizeOfMultiString(lpValue) + sizeof(char)
    );

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceWriteMultiString(const std::string ValueName, const std::list<std::string> Value)
  {
    std::shared_ptr<char> p = ceListToMultiStringA(Value);
    return this->ceWriteMultiString(ValueName, p.get());
  }

  bool ceapi CERegistryA::ceWriteExpandString(const std::string ValueName, const std::string Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExA(
      m_HKSubKey,
      ValueName.c_str(),
      0,
      REG_EXPAND_SZ,
      (const uchar*)Value.c_str(),
      (ulong)Value.length() + sizeof(char)
    );

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryA::ceWriteBinary(const std::string ValueName, void * lpData, ulong ulSize)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExA(m_HKSubKey, ValueName.c_str(), 0, REG_BINARY, (const uchar*)lpData, ulSize);

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  // Read

  int ceapi CERegistryA::ceReadInteger(const std::string ValueName, int Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    int ret = 0;
    ulong ulType = REG_DWORD, ulReturn = sizeof(ret);

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)&ret, &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    return ret;
  }

  bool ceapi CERegistryA::ceReadBool(const std::string ValueName, bool Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    bool ret = false;
    ulong ulType = REG_BINARY, ulReturn = sizeof(ret);

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)&ret, &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    return ret;
  }

  float ceapi CERegistryA::ceReadFloat(const std::string ValueName, float Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    float ret = 0;
    ulong ulType = REG_BINARY, ulReturn = sizeof(ret);

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)&ret, &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    return ret;
  }

  std::string ceapi CERegistryA::ceReadString(const std::string ValueName, const std::string Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    ulong ulType = REG_SZ, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      return Default;
    }

    ulReturn += sizeof(char);

    std::shared_ptr<char> p(new char[ulReturn]);
    if (p == nullptr) {
      return Default;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    std::string s(p.get());

    return s;
  }

  std::list<std::string> ceapi CERegistryA::ceReadMultiString(const std::string ValueName, std::list<std::string> Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    ulong ulType = REG_MULTI_SZ, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      return Default;
    }

    ulReturn += sizeof(char);

    std::shared_ptr<char> p(new char[ulReturn]);
    if (p == nullptr) {
      return Default;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    std::list<std::string> l = ceMultiStringToListA(p.get());

    return l;
  }

  std::string ceapi CERegistryA::ceReadExpandString(const std::string ValueName, const std::string Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    ulong ulType = REG_EXPAND_SZ, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      return Default;
    }

    ulReturn += sizeof(char);

    std::shared_ptr<char> p(new char[ulReturn]);
    if (p == nullptr) {
      return Default;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    std::string s(p.get());

    return s;
  }

  std::shared_ptr<void> ceapi CERegistryA::ceReadBinary(const std::string ValueName, const void * pDefault)
  {
    std::shared_ptr<void> pDef((void*)pDefault);

    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return pDef;
    }

    ulong ulType = REG_BINARY, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      m_LastErrorCode = ERROR_INCORRECT_SIZE;
      SetLastError(m_LastErrorCode);
      return pDef;
    }

    ulReturn += 1;

    std::shared_ptr<void> p(malloc(ulReturn));
    if (p == nullptr) {
      return pDef;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExA(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return pDef;
    }

    return p;
  }

  // Registry - UNICODE Version

  CERegistryW::CERegistryW()
  {
    m_HKRootKey = NULL;
    m_HKRootKey = NULL;
    m_SubKey.clear();

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CERegistryW::CERegistryW(eRegRoot RegRoot)
  {
    m_HKRootKey = (HKEY)RegRoot;
    m_HKSubKey = NULL;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CERegistryW::CERegistryW(eRegRoot RegRoot, const std::wstring SubKey)
  {
    m_HKRootKey = (HKEY)RegRoot;
    m_HKSubKey = NULL;
    m_SubKey = SubKey;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  ulong ceapi CERegistryW::ceGetSizeOfMultiString(const wchar * lpcwszMultiString)
  {
    ulong ulLength = 0;

    while (*lpcwszMultiString) {
      ulong crLength = sizeof(wchar)*(lstrlenW(lpcwszMultiString) + 1);
      ulLength += crLength;
      lpcwszMultiString = lpcwszMultiString + crLength / 2;
    }

    return ulLength;
  }

  ulong ceapi CERegistryW::ceGetDataSize(const std::wstring ValueName, ulong ulType)
  {
    ulong ulDataSize = 0;

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, NULL, &ulDataSize);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return 0;
    }

    return ulDataSize;
  }

  bool ceapi CERegistryW::ceCreateKey()
  {
    return this->ceCreateKey(m_SubKey);
  }

  bool ceapi CERegistryW::ceCreateKey(const std::wstring SubKey)
  {
    m_LastErrorCode = RegCreateKeyW(m_HKRootKey, SubKey.c_str(), &m_HKRootKey);
    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceKeyExists()
  {
    return this->ceKeyExists(m_SubKey);
  }

  bool ceapi CERegistryW::ceKeyExists(const std::wstring SubKey)
  {
    m_LastErrorCode = RegOpenKeyW(m_HKRootKey, SubKey.c_str(), &m_HKSubKey);
    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceOpenKey(eRegAccess RegAccess)
  {
    return this->ceOpenKey(m_SubKey, RegAccess);
  }

  bool ceapi CERegistryW::ceOpenKey(const std::wstring SubKey, eRegAccess RegAccess)
  {
    m_LastErrorCode = RegOpenKeyExW(m_HKRootKey, SubKey.c_str(), 0, (REGSAM)RegAccess, &m_HKSubKey);
    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceDeleteKey()
  {
    return this->ceDeleteKey(m_SubKey);
  }

  bool ceapi CERegistryW::ceDeleteKey(const std::wstring SubKey)
  {
    if (m_HKRootKey == 0) {
      return false;
    }

    m_LastErrorCode = RegDeleteKeyW(m_HKRootKey, SubKey.c_str());

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceDeleteValue(const std::wstring Value)
  {
    if (m_HKSubKey == 0) {
      return false;
    }

    m_LastErrorCode = RegDeleteValueW(m_HKSubKey, Value.c_str());

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  std::list<std::wstring> ceapi CERegistryW::ceEnumKeys()
  {
    std::list<std::wstring> l;
    l.clear();

    wchar Class[MAXPATH] = { 0 };
    ulong cchClass = MAXPATH;
    ulong cSubKeys = 0;
    ulong cbMaxSubKeyLen = 0;
    ulong cbMaxClassLen = 0;
    ulong cValues = 0;
    ulong cbMaxValueNameLen = 0;
    ulong cbMaxValueLen = 0;
    ulong cbSecurityDescriptor = 0;
    FILETIME ftLastWriteTime = { 0 };

    ZeroMemory(&Class, sizeof(Class));
    ZeroMemory(&ftLastWriteTime, sizeof(ftLastWriteTime));

    m_LastErrorCode = RegQueryInfoKeyW(
      m_HKSubKey,
      (wchar*)&Class,
      &cchClass,
      NULL,
      &cSubKeys,
      &cbMaxSubKeyLen,
      &cbMaxClassLen,
      &cValues,
      &cbMaxValueNameLen,
      &cbMaxValueLen,
      &cbSecurityDescriptor,
      &ftLastWriteTime
      );

    if (m_LastErrorCode != ERROR_SUCCESS || cSubKeys == 0) return l;

    cbMaxSubKeyLen += 1;

    wchar * pSubKeyName = new wchar[cbMaxSubKeyLen];

    for (ulong i = 0; i < cSubKeys; i++) {
      ZeroMemory(pSubKeyName, sizeof(wchar)*cbMaxSubKeyLen);
      m_LastErrorCode = RegEnumKeyW(m_HKSubKey, i, pSubKeyName, sizeof(wchar)*cbMaxSubKeyLen);
      l.push_back(pSubKeyName);
    }

    delete[] pSubKeyName;

    return l;
  }

  std::list<std::wstring> ceapi CERegistryW::ceEnumValues()
  {
    std::list<std::wstring> l;
    l.clear();

    wchar Class[MAXPATH] = { 0 };
    ulong cchClass = MAXPATH;
    ulong cSubKeys = 0;
    ulong cbMaxSubKeyLen = 0;
    ulong cbMaxClassLen = 0;
    ulong cValues = 0;
    ulong cbMaxValueNameLen = 0;
    ulong cbMaxValueLen = 0;
    ulong cbSecurityDescriptor = 0;
    FILETIME ftLastWriteTime = { 0 };

    ZeroMemory(&Class, sizeof(Class));
    ZeroMemory(&ftLastWriteTime, sizeof(ftLastWriteTime));

    m_LastErrorCode = RegQueryInfoKeyA(
      m_HKSubKey,
      (char*)&Class,
      &cchClass,
      NULL,
      &cSubKeys,
      &cbMaxSubKeyLen,
      &cbMaxClassLen,
      &cValues,
      &cbMaxValueNameLen,
      &cbMaxValueLen,
      &cbSecurityDescriptor,
      &ftLastWriteTime
    );

    if (m_LastErrorCode != ERROR_SUCCESS || cValues == 0) return l;

    cbMaxValueNameLen += 1;

    wchar * pValueName = new wchar[cbMaxValueNameLen];
    ulong ulValueNameLength;
    for (ulong i = 0; i < cValues; i++) {
      ulValueNameLength = sizeof(wchar)*cbMaxValueNameLen;
      ZeroMemory(pValueName, cbMaxValueNameLen);
      m_LastErrorCode = RegEnumValueW(m_HKSubKey, i, pValueName, &ulValueNameLength, NULL, NULL, NULL, NULL);
      l.push_back(pValueName);
    }

    delete[] pValueName;

    return l;
  }

  // Write

  bool ceapi CERegistryW::ceWriteInteger(const std::wstring ValueName, int Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExW(m_HKSubKey, ValueName.c_str(), 0, REG_DWORD, (const uchar *)&Value, sizeof(Value));

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceWriteBool(const std::wstring ValueName, bool Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExW(m_HKSubKey, ValueName.c_str(), 0, REG_BINARY, (const uchar *)&Value, sizeof(Value));

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceWriteFloat(const std::wstring ValueName, float Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExW(m_HKSubKey, ValueName.c_str(), 0, REG_BINARY, (const uchar *)&Value, sizeof(Value));

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  // For REG_SZ, REG_MULTI_SZ and REG_EXPAND_SZ:
  // https://msdn.microsoft.com/en-us/library/windows/desktop/ms724884(v=vs.85).aspx

  bool ceapi CERegistryW::ceWriteString(const std::wstring ValueName, const std::wstring Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExW(
      m_HKSubKey,
      ValueName.c_str(),
      0,
      REG_SZ,
      (const uchar *)Value.c_str(),
      sizeof(wchar)*((ulong)Value.length() + 1)
    );

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceWriteMultiString(const std::wstring ValueName, const wchar * lpValue)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExW(
      m_HKSubKey,
      ValueName.c_str(),
      0,
      REG_MULTI_SZ,
      (const uchar *)lpValue,
      this->ceGetSizeOfMultiString(lpValue) + sizeof(wchar_t)
    );

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceWriteMultiString(const std::wstring ValueName, const std::list<std::wstring> Value)
  {
    std::shared_ptr<wchar> p = ceListToMultiStringW(Value);
    return this->ceWriteMultiString(ValueName, p.get());
  }

  bool ceapi CERegistryW::ceWriteExpandString(const std::wstring ValueName, const std::wstring Value)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExW(
      m_HKSubKey,
      ValueName.c_str(),
      0,
      REG_EXPAND_SZ,
      (const uchar *)Value.c_str(),
      sizeof(wchar)*((ulong)Value.length() + 1)
    );

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  bool ceapi CERegistryW::ceWriteBinary(const std::wstring ValueName, void * lpData, ulong ulSize)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return false;
    }

    m_LastErrorCode = RegSetValueExW(m_HKSubKey, ValueName.c_str(), 0, REG_BINARY, (const uchar *)lpData, ulSize);

    return (m_LastErrorCode == ERROR_SUCCESS);
  }

  // Read

  int ceapi CERegistryW::ceReadInteger(const std::wstring ValueName, int Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    int ret = 0;
    ulong ulType = REG_DWORD, ulReturn = sizeof(ret);

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)&ret, &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    return ret;
  }

  bool ceapi CERegistryW::ceReadBool(const std::wstring ValueName, bool Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    bool ret = false;
    ulong ulType = REG_BINARY, ulReturn = sizeof(ret);

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)&ret, &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    return ret;
  }

  float ceapi CERegistryW::ceReadFloat(const std::wstring ValueName, float Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    float ret = 0;
    ulong ulType = REG_BINARY, ulReturn = sizeof(ret);

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)&ret, &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    return ret;
  }

  std::wstring ceapi CERegistryW::ceReadString(const std::wstring ValueName, const std::wstring Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    ulong ulType = REG_SZ, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      return Default;
    }

    ulReturn += sizeof(wchar);

    std::shared_ptr<wchar> p(new wchar[ulReturn / sizeof(wchar)]);
    if (p == nullptr) {
      return Default;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    std::wstring ws(p.get());

    return ws;
  }

  std::list<std::wstring> ceapi CERegistryW::ceReadMultiString(const std::wstring ValueName, std::list<std::wstring> Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    ulong ulType = REG_MULTI_SZ, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      return Default;
    }

    ulReturn += sizeof(wchar);

    std::shared_ptr<wchar> p(new wchar[ulReturn / sizeof(wchar)]);
    if (p == nullptr) {
      return Default;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    std::list<std::wstring> l = ceMultiStringToListW(p.get());

    return l;
  }

  std::wstring ceapi CERegistryW::ceReadExpandString(const std::wstring ValueName, const std::wstring Default)
  {
    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return Default;
    }

    ulong ulType = REG_EXPAND_SZ, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      return Default;
    }

    ulReturn += sizeof(wchar);

    std::shared_ptr<wchar> p(new wchar[ulReturn / 2]);
    if (p == nullptr) {
      return Default;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return Default;
    }

    std::wstring ws(p.get());

    return ws;
  }

  std::shared_ptr<void> ceapi CERegistryW::ceReadBinary(const std::wstring ValueName, const void * pDefault)
  {
    std::shared_ptr<void> pDef((void*)pDefault);

    if (m_HKSubKey == 0) {
      m_LastErrorCode = ERROR_INVALID_HANDLE;
      SetLastError(m_LastErrorCode);
      return pDef;
    }

    ulong ulType = REG_BINARY, ulReturn = this->ceGetDataSize(ValueName, ulType);
    if (ulReturn == 0) {
      m_LastErrorCode = ERROR_INCORRECT_SIZE;
      SetLastError(m_LastErrorCode);
      return pDef;
    }

    ulReturn += 1;

    std::shared_ptr<void> p(malloc(ulReturn));
    if (p == nullptr) {
      return pDef;
    }

    ZeroMemory(p.get(), ulReturn);

    m_LastErrorCode = RegQueryValueExW(m_HKSubKey, ValueName.c_str(), NULL, &ulType, (uchar*)p.get(), &ulReturn);
    if (m_LastErrorCode != ERROR_SUCCESS) {
      return pDef;
    }

    return p;
  }

  /* --- Cat : Critical Section --- */

  void ceapi CECriticalSection::ceInit()
  {
    InitializeCriticalSection(&m_CriticalSection);
  }

  void ceapi CECriticalSection::ceEnter()
  {
    EnterCriticalSection(&m_CriticalSection);
  }

  void ceapi CECriticalSection::ceLeave()
  {
    LeaveCriticalSection(&m_CriticalSection);
  }

  void ceapi CECriticalSection::ceDestroy()
  {
    DeleteCriticalSection(&m_CriticalSection);
  }

  TCriticalSection& ceapi CECriticalSection::ceGetCurrentSection()
  {
    return m_CriticalSection;
  }

  /* --- Cat : File Working --- */

  bool ceapi CEFileSupport::ceIsFileHandleValid(HANDLE fileHandle)
  {
    if (!fileHandle || fileHandle == INVALID_HANDLE_VALUE) {
      return false;
    }

    return true;
  }

  bool ceapi CEFileSupport::ceRead(
    ulong ulOffset,
    void* Buffer,
    ulong ulSize,
    eMoveMethodFlags mmFlag
  )
  {
    if (!this->ceSeek(ulOffset, mmFlag)) return false;

    BOOL result = ReadFile(m_FileHandle, Buffer, ulSize, (LPDWORD)&m_ReadSize, NULL);
    if (!result && ulSize != m_ReadSize) {
      m_LastErrorCode = GetLastError();
      return false;
    }

    return true;
  }

  bool ceapi CEFileSupport::ceRead(void* Buffer, ulong ulSize)
  {
    BOOL result = ReadFile(m_FileHandle, Buffer, ulSize, (LPDWORD)&m_ReadSize, NULL);
    if (!result && ulSize != m_ReadSize) {
      m_LastErrorCode = GetLastError();
      return false;
    }

    return true;
  }

  bool ceapi CEFileSupport::ceWrite(
    ulong ulOffset,
    const void* cBuffer,
    ulong ulSize,
    eMoveMethodFlags mmFlag
  )
  {
    if (!this->ceSeek(ulOffset, mmFlag)) return false;

    BOOL result = WriteFile(m_FileHandle, cBuffer, ulSize, (LPDWORD)&m_WroteSize, NULL);
    if (!result && ulSize != m_WroteSize) {
      m_LastErrorCode = GetLastError();
      return false;
    }

    return true;
  }

  bool ceapi CEFileSupport::ceWrite(const void* cBuffer, ulong ulSize)
  {
    BOOL result = WriteFile(m_FileHandle, cBuffer, ulSize, (LPDWORD)&m_WroteSize, NULL);
    if (!result && ulSize != m_WroteSize) {
      m_LastErrorCode = GetLastError();
      return false;
    }

    return true;
  }

  bool ceapi CEFileSupport::ceSeek(ulong ulOffset, eMoveMethodFlags mmFlag)
  {
    if (!this->ceIsFileHandleValid(m_FileHandle)) {
      return false;
    }

    ulong result = SetFilePointer(m_FileHandle, ulOffset, NULL, mmFlag);

    m_LastErrorCode = GetLastError();

    return (result != INVALID_SET_FILE_POINTER);
  }

  ulong ceapi CEFileSupport::ceGetFileSize()
  {
    if (!this->ceIsFileHandleValid(m_FileHandle)) {
      return 0;
    }

    ulong result = GetFileSize(m_FileHandle, NULL);

    m_LastErrorCode = GetLastError();

    return result;
  }

  bool ceapi CEFileSupport::ceIOControl(
    ulong ulControlCode,
    void* lpSendBuffer,
    ulong ulSendSize,
    void* lpReveiceBuffer,
    ulong ulReveiceSize
  )
  {
    ulong ulReturnedLength = 0;

    bool result = (DeviceIoControl(
      m_FileHandle,
      ulControlCode,
      lpSendBuffer,
      ulSendSize,
      lpReveiceBuffer,
      ulReveiceSize,
      &ulReturnedLength,
      NULL
    ) != 0);

    m_LastErrorCode = GetLastError();

    return result;
  }

  bool ceapi CEFileSupport::ceClose()
  {
    if (!this->ceIsFileHandleValid(m_FileHandle)) {
      return false;
    }

    if (!CloseHandle(m_FileHandle)) {
      return false;
    }

    return true;
  }

  bool ceapi CEFileA::ceInit(
    const std::string FilePath,
    eFileModeFlags fmFlag,
    eFileGenericFlags fgFlag,
    eFileShareFlags fsFlag,
    eFileAttributeFlags faFlag
  )
  {
    m_FileHandle = CreateFileA(FilePath.c_str(), fgFlag, fsFlag, NULL, fmFlag, faFlag, NULL);
    if (!this->ceIsFileHandleValid(m_FileHandle)) {
      m_LastErrorCode = GetLastError();
      return false;
    }

    return true;
  }

  bool ceapi CEFileW::ceInit(
    const std::wstring FilePath,
    eFileModeFlags fmFlag,
    eFileGenericFlags fgFlag,
    eFileShareFlags fsFlag,
    eFileAttributeFlags faFlag
  )
  {
    m_FileHandle = CreateFileW(FilePath.c_str(), fgFlag, fsFlag, NULL, fmFlag, faFlag, NULL);
    if (!this->ceIsFileHandleValid(m_FileHandle)) {
      m_LastErrorCode = GetLastError();
      return false;
    }

    return true;
  }

  /* --- Cat : Service Working --- */

  bool ceapi CEServiceSupport::ceInitService(eSCAccessType SCAccessType)
  {
    m_SCMHandle = OpenSCManager(
      NULL, // Local computer
      NULL, // ServicesActive database
      (ulong)SCAccessType
    );

    m_LastErrorCode = GetLastError();

    if (!m_SCMHandle) {
      m_Initialized = false;
      return false;
    }

    m_Initialized = true;

    return true;
  }

  bool ceapi CEServiceSupport::ceReleaseService()
  {
    if (!m_Initialized) {
      return false;
    }

    BOOL result = CloseServiceHandle(m_SCMHandle);

    m_LastErrorCode = GetLastError();

    if (result == FALSE) {
      return false;
    }

    return true;
  }

  bool ceapi CEServiceSupport::ceStartService()
  {
    if (!m_ServiceHandle) {
      return false;
    }

    BOOL result = StartServiceW(m_ServiceHandle, 0, NULL);

    m_LastErrorCode = GetLastError();

    return (result != FALSE);
  }

  bool ceapi CEServiceSupport::ceStopService()
  {
    return this->ceControlService(SC_STOP);
  }

  bool ceapi CEServiceSupport::ceControlService(eServiceControl ServiceControl)
  {
    if (!m_ServiceHandle) {
      return false;
    }

    BOOL result = ControlService(m_ServiceHandle, (ulong)ServiceControl, (LPSERVICE_STATUS)&m_Status);

    m_LastErrorCode = GetLastError();

    return (result != FALSE);
  }

  bool ceapi CEServiceSupport::ceCloseService()
  {
    if (!m_ServiceHandle) {
      return false;
    }

    BOOL result = DeleteService(m_ServiceHandle);

    m_LastErrorCode = GetLastError();

    if (result == FALSE) {
      return false;
    }

    result = CloseServiceHandle(m_ServiceHandle);

    m_LastErrorCode = GetLastError();

    return (result != FALSE);
  }

  bool ceapi CEServiceSupport::ceQueryServiceStatus(TServiceStatus& ServiceStatus)
  {
    if (!m_ServiceHandle) {
      return false;
    }

    BOOL result = QueryServiceStatus(m_ServiceHandle, &ServiceStatus);

    m_LastErrorCode = GetLastError();

    if (result == FALSE) {
      return false;
    }

    return true;
  }

  eServiceType ceapi CEServiceSupport::ceGetServiceType()
  {
    if (!m_ServiceHandle) {
      return ce::eServiceType::ST_UNKNOWN;
    }

    if (this->ceQueryServiceStatus(m_Status) == FALSE) {
      return ce::eServiceType::ST_UNKNOWN;
    }

    return (ce::eServiceType)m_Status.dwServiceType;
  }

  eServiceState ceapi CEServiceSupport::ceGetServiceState()
  {
    if (!m_ServiceHandle) {
      return ce::eServiceState::SS_UNKNOWN;
    }

    if (this->ceQueryServiceStatus(m_Status) == FALSE) {
      return ce::eServiceState::SS_UNKNOWN;
    }

    return (ce::eServiceState)m_Status.dwCurrentState;
  }

  // A

  CEServiceA::CEServiceA()
  {
    m_ServiceName.clear();
    m_DisplayName.clear();
    m_ServiceFilePath.clear();

    m_SCMHandle     = 0;
    m_ServiceHandle = 0;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CEServiceA::~CEServiceA()
  {
    m_SCMHandle     = 0;
    m_ServiceHandle = 0;
  }

  bool ceapi CEServiceA::ceCreateService (
    const std::string ServiceFilePath,
    eServiceAccessType ServiceAccessType,
    eServiceType ServiceType,
    eServiceStartType ServiceStartType,
    eServiceErrorControlType ServiceErrorControlType
  )
  {
    // Is a full file path?
    if (ServiceFilePath.find('\\') == std::string::npos) {
      m_ServiceFilePath = ceGetCurrentDirectoryA(true);
      m_ServiceFilePath.append(ServiceFilePath);
    }
    else {
      m_ServiceFilePath.assign(ServiceFilePath);
    }

    if (!ceFileExistsA(m_ServiceFilePath)) {
      SetLastError(ERROR_FILE_NOT_FOUND);
      return false;
    }

    m_ServiceFileName = ceExtractFileNameA(m_ServiceFilePath);

    m_ServiceName = ceExtractFileNameA(m_ServiceFilePath, false);

    time_t t = time(NULL);
    std::string crTime = ceFormatTimeDateToStringA(t, "%H:%M:%S");
    std::string crDateTime = ceFormatTimeDateToStringA(t, "%H:%M:%S %d/%m/%Y");

    // Name
    m_Name.clear();
    m_Name.assign(m_ServiceName);
    m_Name.append(" - ");
    m_Name.append(crTime);

    // Display Name
    m_DisplayName.clear();
    m_DisplayName.assign(m_ServiceName);
    m_DisplayName.append(" - ");
    m_DisplayName.append(crDateTime);

    m_ServiceHandle = CreateServiceA(
      m_SCMHandle,
      m_Name.c_str(),
      m_DisplayName.c_str(),
      (ulong)ServiceAccessType,
      (ulong)ServiceType,
      (ulong)ServiceStartType,
      (ulong)ServiceErrorControlType,
      m_ServiceFilePath.c_str(),
      NULL, NULL, NULL, NULL, NULL
    );

    m_LastErrorCode = GetLastError();

    if (!m_ServiceHandle) {
      CloseServiceHandle(m_SCMHandle);
      return false;
    }

    return true;
  }

  bool ceapi CEServiceA::ceOpenService(const std::string ServiceName, eServiceAccessType ServiceAccessType)
  {
    if (!m_SCMHandle) {
      return false;
    }

    m_ServiceName = ServiceName;

    m_ServiceHandle = OpenServiceA(m_SCMHandle, m_ServiceName.c_str(), (ulong)ServiceAccessType);

    m_LastErrorCode = GetLastError();

    if (!m_ServiceHandle) {
      return false;
    }

    return true;
  }

  std::string ceapi CEServiceA::ceGetName(const std::string AnotherServiceDisplayName)
  {
    if (AnotherServiceDisplayName.empty()) {
      return m_Name;
    }

    std::string s;
    s.clear();
    ulong nSize = MAX_SIZE;

    std::shared_ptr<char> p(new char[MAX_SIZE]);
    ZeroMemory(p.get(), MAX_SIZE);

    BOOL result = GetServiceKeyNameA(m_SCMHandle, AnotherServiceDisplayName.c_str(), p.get(), &nSize);

    m_LastErrorCode = GetLastError();

    if (result == FALSE) {
      return s;
    }

    s.assign(p.get());

    return s;
  }

  std::string ceapi CEServiceA::ceGetDisplayName(const std::string AnotherServiceName)
  {
    if (AnotherServiceName.empty())
    {
      return m_DisplayName;
    }

    std::string s;
    s.clear();
    ulong nSize = MAX_SIZE;

    std::shared_ptr<char> p(new char[MAX_SIZE]);
    ZeroMemory(p.get(), MAX_SIZE);

    BOOL result = GetServiceDisplayNameA(m_SCMHandle, AnotherServiceName.c_str(), p.get(), &nSize);

    m_LastErrorCode = GetLastError();

    if (result == FALSE) {
      return s;
    }

    s.assign(p.get());

    return s;
  }

  // W

  CEServiceW::CEServiceW()
  {
    m_ServiceName.clear();
    m_DisplayName.clear();
    m_ServiceFilePath.clear();

    m_SCMHandle     = 0;
    m_ServiceHandle = 0;

    m_LastErrorCode = ERROR_SUCCESS;
  }

  CEServiceW::~CEServiceW()
  {
    m_SCMHandle     = 0;
    m_ServiceHandle = 0;
  }

  bool ceapi CEServiceW::ceCreateService(
    const std::wstring ServiceFilePath,
    eServiceAccessType ServiceAccessType,
    eServiceType ServiceType,
    eServiceStartType ServiceStartType,
    eServiceErrorControlType ServiceErrorControlType
  )
  {
    // Is a full file path?
    if (ServiceFilePath.find(L'\\') == std::wstring::npos) {
      m_ServiceFilePath = ceGetCurrentDirectoryW(true);
      m_ServiceFilePath.append(ServiceFilePath);
    }
    else {
      m_ServiceFilePath.assign(ServiceFilePath);
    }

    if (!ceFileExistsW(m_ServiceFilePath)) {
      return false;
    }

    m_ServiceFileName = ceExtractFileNameW(m_ServiceFilePath);

    m_ServiceName = ceExtractFileNameW(m_ServiceFilePath, false);

    time_t t = time(NULL);
    std::wstring crTime = ceFormatTimeDateToStringW(t, L"%H:%M:%S");
    std::wstring crDateTime = ceFormatTimeDateToStringW(t, L"%H:%M:%S %d/%m/%Y");

    // Name
    m_Name.clear();
    m_Name.assign(m_ServiceName);
    m_Name.append(L" - ");
    m_Name.append(crTime);

    // Display Name
    m_DisplayName.clear();
    m_DisplayName.assign(m_ServiceName);
    m_DisplayName.append(L" - ");
    m_DisplayName.append(crDateTime);

    m_ServiceHandle = CreateServiceW(
      m_SCMHandle,
      m_Name.c_str(),
      m_DisplayName.c_str(),
      (ulong)ServiceAccessType,
      (ulong)ServiceType,
      (ulong)ServiceStartType,
      (ulong)ServiceErrorControlType,
      m_ServiceFilePath.c_str(),
      NULL, NULL, NULL, NULL, NULL
    );

    m_LastErrorCode = GetLastError();

    if (!m_ServiceHandle) {
      return false;
    }

    return true;
  }

  bool ceapi CEServiceW::ceOpenService(const std::wstring ServiceName, eServiceAccessType ServiceAccessType)
  {
    if (!m_SCMHandle) {
      return false;
    }

    m_ServiceName = ServiceName;

    m_ServiceHandle = OpenServiceW(m_SCMHandle, m_ServiceName.c_str(), (ulong)ServiceAccessType);

    m_LastErrorCode = GetLastError();

    if (!m_ServiceHandle) {
      return false;
    }

    return true;
  }

  std::wstring ceapi CEServiceW::ceGetName(const std::wstring AnotherServiceDisplayName)
  {
    if (AnotherServiceDisplayName.empty()) {
      return m_Name;
    }

    std::wstring ws;
    ws.clear();
    ulong nSize = 2*MAX_SIZE;

    std::shared_ptr<wchar> p(new wchar[MAX_SIZE]);
    ZeroMemory(p.get(), 2*MAX_SIZE);

    BOOL result = GetServiceKeyNameW(m_SCMHandle, AnotherServiceDisplayName.c_str(), p.get(), &nSize);

    m_LastErrorCode = GetLastError();

    if (result == FALSE) {
      return ws;
    }

    ws.assign(p.get());

    return ws;
  }

  std::wstring ceapi CEServiceW::ceGetDisplayName(const std::wstring AnotherServiceName)
  {
    if (AnotherServiceName.empty())
    {
      return m_DisplayName;
    }

    std::wstring ws;
    ws.clear();
    ulong nSize = 2*MAX_SIZE;

    std::shared_ptr<wchar> p(new wchar[MAX_SIZE]);
    ZeroMemory(p.get(), 2*MAX_SIZE);

    BOOL result = GetServiceDisplayNameW(m_SCMHandle, AnotherServiceName.c_str(), p.get(), &nSize);

    m_LastErrorCode = GetLastError();

    if (result == FALSE) {
      return ws;
    }

    ws.assign(p.get());

    return ws;
  }

  /* --- Cat : PE File --- */

  // CEPEFileSupportT

  template class CEPEFileSupportT<ulong32>;
  template class CEPEFileSupportT<ulong64>;

  template<typename T>
  CEPEFileSupportT<T>::CEPEFileSupportT()
  {
    m_Initialized = false;

    m_pBase = nullptr;
    m_pDosHeader = nullptr;
    m_pPEHeader  = nullptr;
    m_SectionHeaderList.clear();
    m_ImportDescriptorList.clear();
    m_ExIDDList.clear();
    m_FunctionInfoList.clear();

    if (sizeof(T) == 4) {
      m_OrdinalFlag = (T)IMAGE_ORDINAL_FLAG32;
    }
    else {
      m_OrdinalFlag = (T)IMAGE_ORDINAL_FLAG64;
    }
  }

  template<typename T>
  CEPEFileSupportT<T>::~CEPEFileSupportT(){};

  template<typename T>
  void* ceapi CEPEFileSupportT<T>::ceGetpBase()
  {
    return m_pBase;
  }

  template<typename T>
  TPEHeaderT<T>* ceapi CEPEFileSupportT<T>::ceGetpPEHeader()
  {
    if (!m_Initialized) {
      return nullptr;
    }

    return m_pPEHeader;
  }

  template<typename T>
  std::list<PSectionHeader>& ceapi CEPEFileSupportT<T>::ceGetSetionHeaderList(bool Reget)
  {
    m_SectionHeaderList.clear();

    if (!m_Initialized) {
      return m_SectionHeaderList;
    }

    if (!Reget && (m_SectionHeaderList.size() != 0)) {
      return m_SectionHeaderList;
    }

    ce::PSectionHeader pSH = (PSectionHeader)((T)m_pPEHeader + sizeof(ce::TNtHeaderT<T>));
    if (pSH == nullptr) {
      return m_SectionHeaderList;
    }

    m_SectionHeaderList.clear();
    for (int i = 0; i < m_pPEHeader->NumberOfSections; i++) {
      m_SectionHeaderList.push_back(pSH);
      pSH++;
    }

    return m_SectionHeaderList;
  }

  template<typename T>
  std::list<TExIID>& ceapi CEPEFileSupportT<T>::ceGetExIIDList()
  {
    m_ExIDDList.clear();

    if (!m_Initialized) {
      return m_ExIDDList;
    }

    T ulIIDOffset = this->ceRVA2Offset(m_pPEHeader->Import.VirtualAddress);
    if (ulIIDOffset == (T)-1) {
      return m_ExIDDList;
    }

    PImportDescriptor pIID = (PImportDescriptor)((T)m_pBase + ulIIDOffset);
    if (pIID == nullptr) {
      return m_ExIDDList;
    }

    m_ExIDDList.clear();
    for (int i = 0; pIID->FirstThunk != 0; i++, pIID++) {
      std::string DllName = (char*)((ce::ulong32)m_pBase + this->ceRVA2Offset(pIID->Name));

      TExIID ExIID;
      ExIID.IIDID = i;
      ExIID.Name = DllName;
      ExIID.pIID = pIID;

      m_ExIDDList.push_back(ExIID);
    }

    return m_ExIDDList;
  }

  template<typename T>
  std::list<PImportDescriptor>& ceapi CEPEFileSupportT<T>::ceGetImportDescriptorList(bool Reget)
  {
    m_ImportDescriptorList.clear();

    if (!m_Initialized) {
      return m_ImportDescriptorList;
    }

    if (Reget || (m_ExIDDList.size() == 0)) {
      this->ceGetExIIDList();
    }

    if (m_ExIDDList.size() == 0) {
      return m_ImportDescriptorList;
    }

    m_ImportDescriptorList.clear();
    for (auto e: m_ExIDDList) {
      m_ImportDescriptorList.push_back(e.pIID);
    }

    return m_ImportDescriptorList;
  }

  template<typename T>
  std::list<TDLLInfo> ceapi CEPEFileSupportT<T>::ceGetDLLInfoList(bool Reget)
  {
    std::list<TDLLInfo> DLLInfoList;
    DLLInfoList.clear();

    if (!m_Initialized) {
      return DLLInfoList;
    }

    if (Reget || (m_ExIDDList.size() == 0)) {
      this->ceGetExIIDList();
    }

    if (m_ExIDDList.size() == 0) {
      return DLLInfoList;
    }

    for (auto e: m_ExIDDList) {
      TDLLInfo DLLInfo;
      DLLInfo.IIDID = e.IIDID;
      DLLInfo.Name = e.Name;
      //DLLInfo.NumberOfFuctions = 0;

      DLLInfoList.push_back(DLLInfo);
    }

    return DLLInfoList;
  }

  template<typename T>
  std::list<TFunctionInfoT<T>> ceapi CEPEFileSupportT<T>::ceGetFunctionInfoList(bool Reget)
  {
    if (!m_Initialized) {
      return m_FunctionInfoList;
    }

    if (Reget || (m_ExIDDList.size() == 0)) {
      this->ceGetExIIDList();
    }

    if (m_ExIDDList.size() == 0) {
      return m_FunctionInfoList;
    }

    m_FunctionInfoList.clear();
    TThunkDataT<T>* pThunkData = nullptr;
    TFunctionInfoT<T> funcInfo;
    for (auto e: m_ExIDDList) {
      T ulOffset = this->ceRVA2Offset(e.pIID->FirstThunk);
      if (ulOffset == -1 || (pThunkData = (TThunkDataT<T>*)((T)m_pBase + ulOffset)) == nullptr) continue;
      do {
        if ((pThunkData->u1.AddressOfData & m_OrdinalFlag) == m_OrdinalFlag) { // Imported by ordinal
          funcInfo.Hint = -1;
          funcInfo.Ordinal = pThunkData->u1.AddressOfData & ~m_OrdinalFlag;
          funcInfo.Name = "";
        }
        else { // Imported by name
          ulOffset = this->ceRVA2Offset(pThunkData->u1.AddressOfData);
          PImportByName p = (PImportByName)((ulong)m_pBase + ulOffset);
          if (ulOffset != -1 && p != nullptr) {
            funcInfo.Hint = p->Hint;
            funcInfo.Ordinal = (T)-1;
            funcInfo.Name = (char*)p->Name;
          }
        }

        funcInfo.IIDID = e.IIDID;
        funcInfo.RVA = pThunkData->u1.AddressOfData;
        m_FunctionInfoList.push_back(funcInfo);

        /*ulOffset = this->ceRVA2Offset(pThunkData->u1.AddressOfData);
        PImportByName p = (PImportByName)((ulong)m_pBase + ulOffset);
        if (ulOffset != -1 && p != nullptr) {
          funcInfo.IIDID = e.IIDID;
          funcInfo.Hint = p->Hint;
          funcInfo.Name = (char*)p->Name;
          funcInfo.RVA = pThunkData->u1.AddressOfData;
          m_FunctionInfoList.push_back(funcInfo);
        }*/

        pThunkData++;
      } while (pThunkData->u1.AddressOfData != 0);
    }

    return m_FunctionInfoList;
  }

  template<typename T>
  TDLLInfo ceapi CEPEFileSupportT<T>::ceFindImportedDLL(const std::string DLLName)
  {
    TDLLInfo o ={0};

    if (m_ExIDDList.size() == 0) {
      this->ceGetDLLInfoList();
    }

    if (m_ExIDDList.size() == 0) {
      return o;
    }

    for (auto e: m_ExIDDList) {
      if (e.Name == DLLName) {
        o.IIDID = e.IIDID;
        o.Name = e.Name;
        break;
      }
    }

    return o;
  }

  template<typename T>
  TFunctionInfoT<T> ceapi CEPEFileSupportT<T>::ceFindImportedFunction(
    const TFunctionInfoT<T>& FunctionInfo,
    eImportedFunctionFindMethod Method
  )
  {
    TFunctionInfoT<T> o ={0};

    if (m_FunctionInfoList.size() == 0) {
      this->ceGetFunctionInfoList();
    }

    if (m_FunctionInfoList.size() == 0) {
      return o;
    }

    switch (Method) {
    case eImportedFunctionFindMethod::IFFM_HINT:
      for (auto e: m_FunctionInfoList) {
        if (e.Hint == FunctionInfo.Hint) {
          o = e;
          break;
        }
      }
      break;
    case eImportedFunctionFindMethod::IFFM_NAME:
      for (auto e: m_FunctionInfoList) {
        if (e.Name == FunctionInfo.Name) {
          o = e;
          break;
        }
      }
      break;
    default:
      break;
    }

    return o;
  }

  template<typename T>
  TFunctionInfoT<T> ceapi CEPEFileSupportT<T>::ceFindImportedFunction(const std::string FunctionName)
  {
    TFunctionInfoT<T> o = {0};
    o.Name = FunctionName;

    return this->ceFindImportedFunction(o, eImportedFunctionFindMethod::IFFM_NAME);
  }

  template<typename T>
  TFunctionInfoT<T> ceapi CEPEFileSupportT<T>::ceFindImportedFunction(const ushort FunctionHint)
  {
    TFunctionInfoT<T> o = {0};
    o.Hint = FunctionHint;

    return this->ceFindImportedFunction(o, eImportedFunctionFindMethod::IFFM_HINT);
  }

  template<typename T>
  T ceapi CEPEFileSupportT<T>::ceRVA2Offset(T RVA)
  {
    if (!m_Initialized) {
      return (T)-1;
    }

    if (m_SectionHeaderList.size() == 0) {
      this->ceGetSetionHeaderList();
    }

    if (m_SectionHeaderList.size() == 0) {
      return (T)-1;
    }

    PSectionHeader foundSection = nullptr;
    for (auto e: m_SectionHeaderList) {
      if (RVA >= e->VirtualAddress && RVA <= e->VirtualAddress + e->Misc.VirtualSize) {
        foundSection = e;
        break;
      }
    }

    if (foundSection == nullptr) {
      return (T)-1;
    }

    T ulOffset = RVA - (T)foundSection->VirtualAddress + (T)foundSection->PointerToRawData;

    return ulOffset;
  }

  template<typename T>
  ulong ceapi CEPEFileSupportT<T>::ceOffset2RVA(ulong Offset)
  {
    // Write your code here
    return 0;
  }

  // CEPEFileTA

  template class CEPEFileTA<ulong32>;
  template class CEPEFileTA<ulong64>;

  template<typename T>
  CEPEFileTA<T>::CEPEFileTA(const std::string PEFilePath)
  {
    CEPEFileSupportT<T>::m_Initialized = false;

    CEPEFileSupportT<T>::m_pBase = nullptr;
    CEPEFileSupportT<T>::m_pDosHeader = nullptr;
    CEPEFileSupportT<T>::m_pPEHeader  = nullptr;

    m_FilePath = PEFilePath;
  }

  template<typename T>
  CEPEFileTA<T>::~CEPEFileTA()
  {
    if (CEPEFileSupportT<T>::m_Initialized) {
      m_FileMap.ceClose();
    }
  }

  template<typename T>
  CEResult ceapi CEPEFileTA<T>::ceParse(const std::string PEFilePath)
  {
    if (PEFilePath.length() != 0) {
      m_FilePath = PEFilePath;
    }

    if (m_FilePath.length() == 0) {
      return 1;
    }

    if (!ceFileExistsA(m_FilePath)) {
      return 2;
    }

    if (m_FileMap.ceInit(m_FilePath) != ce::CE_OK) {
      return 3;
    }

    if (m_FileMap.ceCreate("PE-CEPEFileA") != ce::CE_OK) {
      return 4;
    }

    CEPEFileSupportT<T>::m_pBase = m_FileMap.ceView();
    if (CEPEFileSupportT<T>::m_pBase == nullptr) {
      return 5;
    }

    CEPEFileSupportT<T>::m_pDosHeader = (PDosHeader)CEPEFileSupportT<T>::m_pBase;
    if (CEPEFileSupportT<T>::m_pDosHeader == nullptr) {
      return 6;
    }

    CEPEFileSupportT<T>::m_pPEHeader = (TPEHeaderT<T>*)(
      (T)CEPEFileSupportT<T>::m_pBase +
      CEPEFileSupportT<T>::m_pDosHeader->e_lfanew
    );
    if (CEPEFileSupportT<T>::m_pPEHeader == nullptr) {
      return 7;
    }

    if (sizeof(T) == sizeof(pe32)) {
      if (CEPEFileSupportT<T>::m_pPEHeader->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return 8; // Used wrong type data for the current PE file
      }
    }
    else if (sizeof(T) == sizeof(pe64)) {
      if (CEPEFileSupportT<T>::m_pPEHeader->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return 8; // Used wrong type data for the current PE file
      }
    }
    else {
      return 9; // The curent type data was not supported
    }

    CEPEFileSupportT<T>::m_Initialized = true;

    return CE_OK;
  }

  /*template<typename T>
  void ceapi CEPEFileTA<T>::ceX()
  {
    // ...
  }*/

  // CEPEFileTW

  template class CEPEFileTW<ulong32>;
  template class CEPEFileTW<ulong64>;

  template<typename T>
  CEPEFileTW<T>::CEPEFileTW(const std::wstring PEFilePath)
  {
    CEPEFileSupportT<T>::m_Initialized = false;

    CEPEFileSupportT<T>::m_pBase = nullptr;
    CEPEFileSupportT<T>::m_pDosHeader = nullptr;
    CEPEFileSupportT<T>::m_pPEHeader  = nullptr;

    m_FilePath = PEFilePath;
  }

  template<typename T>
  CEPEFileTW<T>::~CEPEFileTW()
  {
    if (CEPEFileSupportT<T>::m_Initialized) {
      m_FileMap.ceClose();
    }
  }

  template<typename T>
  CEResult ceapi CEPEFileTW<T>::ceParse(const std::wstring PEFilePath)
  {
    if (PEFilePath.length() != 0) {
      m_FilePath = PEFilePath;
    }

    if (m_FilePath.length() == 0) {
      return 1;
    }

    if (!ceFileExistsW(m_FilePath)) {
      return 2;
    }

    if (m_FileMap.ceInit(m_FilePath) != ce::CE_OK) {
      return 3;
    }

    if (m_FileMap.ceCreate(L"PE-CEPEFileW") != ce::CE_OK) {
      return 4;
    }

    CEPEFileSupportT<T>::m_pBase = m_FileMap.ceView();
    if (CEPEFileSupportT<T>::m_pBase == nullptr) {
      return 5;
    }

    CEPEFileSupportT<T>::m_pDosHeader = (PDosHeader)CEPEFileSupportT<T>::m_pBase;
    if (CEPEFileSupportT<T>::m_pDosHeader == nullptr) {
      return 6;
    }

    CEPEFileSupportT<T>::m_pPEHeader = (TPEHeaderT<T>*)(
      (T)CEPEFileSupportT<T>::m_pBase +
      CEPEFileSupportT<T>::m_pDosHeader->e_lfanew
    );
    if (CEPEFileSupportT<T>::m_pPEHeader == nullptr) {
      return 7;
    }

    if (sizeof(T) == sizeof(pe32)) {
      if (CEPEFileSupportT<T>::m_pPEHeader->Magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC) {
        return 8; // Used wrong type data for the current PE file
      }
    }
    else if (sizeof(T) == sizeof(pe64)) {
      if (CEPEFileSupportT<T>::m_pPEHeader->Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
        return 8; // Used wrong type data for the current PE file
      }
    }
    else {
      return 9; // The curent type data was not supported
    }

    CEPEFileSupportT<T>::m_Initialized = true;

    return CE_OK;
  }
}

/* HDE */
// Hacker Disassembler Engine 32/64 C
// Copyright (c) 2008-2009, Vyacheslav Patkov
// Vyacheslav Patkov, thanks you so much !

// --- Begin of HDE --- //

unsigned int hde32_disasm(const void *code, HDE32::hde32s *hs)
{
  using namespace HDE32;

  static unsigned char hde32_table[] = {
    0xa3,0xa8,0xa3,0xa8,0xa3,0xa8,0xa3,0xa8,0xa3,0xa8,0xa3,0xa8,0xa3,0xa8,0xa3,
    0xa8,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xaa,0xac,0xaa,0xb2,0xaa,0x9f,0x9f,
    0x9f,0x9f,0xb5,0xa3,0xa3,0xa4,0xaa,0xaa,0xba,0xaa,0x96,0xaa,0xa8,0xaa,0xc3,
    0xc3,0x96,0x96,0xb7,0xae,0xd6,0xbd,0xa3,0xc5,0xa3,0xa3,0x9f,0xc3,0x9c,0xaa,
    0xaa,0xac,0xaa,0xbf,0x03,0x7f,0x11,0x7f,0x01,0x7f,0x01,0x3f,0x01,0x01,0x90,
    0x82,0x7d,0x97,0x59,0x59,0x59,0x59,0x59,0x7f,0x59,0x59,0x60,0x7d,0x7f,0x7f,
    0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x9a,0x88,0x7d,
    0x59,0x50,0x50,0x50,0x50,0x59,0x59,0x59,0x59,0x61,0x94,0x61,0x9e,0x59,0x59,
    0x85,0x59,0x92,0xa3,0x60,0x60,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,0x59,
    0x59,0x59,0x9f,0x01,0x03,0x01,0x04,0x03,0xd5,0x03,0xcc,0x01,0xbc,0x03,0xf0,
    0x10,0x10,0x10,0x10,0x50,0x50,0x50,0x50,0x14,0x20,0x20,0x20,0x20,0x01,0x01,
    0x01,0x01,0xc4,0x02,0x10,0x00,0x00,0x00,0x00,0x01,0x01,0xc0,0xc2,0x10,0x11,
    0x02,0x03,0x11,0x03,0x03,0x04,0x00,0x00,0x14,0x00,0x02,0x00,0x00,0xc6,0xc8,
    0x02,0x02,0x02,0x02,0x00,0x00,0xff,0xff,0xff,0xff,0x00,0x00,0x00,0xff,0xca,
    0x01,0x01,0x01,0x00,0x06,0x00,0x04,0x00,0xc0,0xc2,0x01,0x01,0x03,0x01,0xff,
    0xff,0x01,0x00,0x03,0xc4,0xc4,0xc6,0x03,0x01,0x01,0x01,0xff,0x03,0x03,0x03,
    0xc8,0x40,0x00,0x0a,0x00,0x04,0x00,0x00,0x00,0x00,0x7f,0x00,0x33,0x01,0x00,
    0x00,0x00,0x00,0x00,0x00,0xff,0xbf,0xff,0xff,0x00,0x00,0x00,0x00,0x07,0x00,
    0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0xff,0xff,0x00,0x00,0x00,0xbf,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x7f,0x00,0x00,0xff,0x4a,0x4a,0x4a,0x4a,0x4b,0x52,0x4a,0x4a,0x4a,0x4a,0x4f,
    0x4c,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x55,0x45,0x40,0x4a,0x4a,0x4a,
    0x45,0x59,0x4d,0x46,0x4a,0x5d,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,0x4a,
    0x4a,0x4a,0x4a,0x4a,0x4a,0x61,0x63,0x67,0x4e,0x4a,0x4a,0x6b,0x6d,0x4a,0x4a,
    0x45,0x6d,0x4a,0x4a,0x44,0x45,0x4a,0x4a,0x00,0x00,0x00,0x02,0x0d,0x06,0x06,
    0x06,0x06,0x0e,0x00,0x00,0x00,0x00,0x06,0x06,0x06,0x00,0x06,0x06,0x02,0x06,
    0x00,0x0a,0x0a,0x07,0x07,0x06,0x02,0x05,0x05,0x02,0x02,0x00,0x00,0x04,0x04,
    0x04,0x04,0x00,0x00,0x00,0x0e,0x05,0x06,0x06,0x06,0x01,0x06,0x00,0x00,0x08,
    0x00,0x10,0x00,0x18,0x00,0x20,0x00,0x28,0x00,0x30,0x00,0x80,0x01,0x82,0x01,
    0x86,0x00,0xf6,0xcf,0xfe,0x3f,0xab,0x00,0xb0,0x00,0xb1,0x00,0xb3,0x00,0xba,
    0xf8,0xbb,0x00,0xc0,0x00,0xc1,0x00,0xc7,0xbf,0x62,0xff,0x00,0x8d,0xff,0x00,
    0xc4,0xff,0x00,0xc5,0xff,0x00,0xff,0xff,0xeb,0x01,0xff,0x0e,0x12,0x08,0x00,
    0x13,0x09,0x00,0x16,0x08,0x00,0x17,0x09,0x00,0x2b,0x09,0x00,0xae,0xff,0x07,
    0xb2,0xff,0x00,0xb4,0xff,0x00,0xb5,0xff,0x00,0xc3,0x01,0x00,0xc7,0xff,0xbf,
    0xe7,0x08,0x00,0xf0,0x02,0x00
  };

  uint8_t x, c, *p = (uint8_t *)code, cflags, opcode, pref = 0;
  uint8_t *ht = hde32_table, m_mod, m_reg, m_rm, disp_size = 0;

  // Avoid using memset to reduce the footprint.
#ifndef _MSC_VER
  memset((LPBYTE)hs, 0, sizeof(hde32s));
#else
  __stosb((LPBYTE)hs, 0, sizeof(hde32s));
#endif

  for (x = 16; x; x--)
    switch (c = *p++) {
    case 0xf3:
      hs->p_rep = c;
      pref |= PRE_F3;
      break;
    case 0xf2:
      hs->p_rep = c;
      pref |= PRE_F2;
      break;
    case 0xf0:
      hs->p_lock = c;
      pref |= PRE_LOCK;
      break;
    case 0x26: case 0x2e: case 0x36:
    case 0x3e: case 0x64: case 0x65:
      hs->p_seg = c;
      pref |= PRE_SEG;
      break;
    case 0x66:
      hs->p_66 = c;
      pref |= PRE_66;
      break;
    case 0x67:
      hs->p_67 = c;
      pref |= PRE_67;
      break;
    default:
      goto pref_done;
  }
pref_done:

  hs->flags = (uint32_t)pref << 23;

  if (!pref)
    pref |= PRE_NONE;

  if ((hs->opcode = c) == 0x0f) {
    hs->opcode2 = c = *p++;
    ht += DELTA_OPCODES;
  } else if (c >= 0xa0 && c <= 0xa3) {
    if (pref & PRE_67)
      pref |= PRE_66;
    else
      pref &= ~PRE_66;
  }

  opcode = c;
  cflags = ht[ht[opcode / 4] + (opcode % 4)];

  if (cflags == C_ERROR) {
    hs->flags |= F_ERROR | F_ERROR_OPCODE;
    cflags = 0;
    if ((opcode & -3) == 0x24)
      cflags++;
  }

  x = 0;
  if (cflags & C_GROUP) {
    uint16_t t;
    t = *(uint16_t *)(ht + (cflags & 0x7f));
    cflags = (uint8_t)t;
    x = (uint8_t)(t >> 8);
  }

  if (hs->opcode2) {
    ht = hde32_table + DELTA_PREFIXES;
    if (ht[ht[opcode / 4] + (opcode % 4)] & pref)
      hs->flags |= F_ERROR | F_ERROR_OPCODE;
  }

  if (cflags & C_MODRM) {
    hs->flags |= F_MODRM;
    hs->modrm = c = *p++;
    hs->modrm_mod = m_mod = c >> 6;
    hs->modrm_rm = m_rm = c & 7;
    hs->modrm_reg = m_reg = (c & 0x3f) >> 3;

    if (x && ((x << m_reg) & 0x80))
      hs->flags |= F_ERROR | F_ERROR_OPCODE;

    if (!hs->opcode2 && opcode >= 0xd9 && opcode <= 0xdf) {
      uint8_t t = opcode - 0xd9;
      if (m_mod == 3) {
        ht = hde32_table + DELTA_FPU_MODRM + t*8;
        t = ht[m_reg] << m_rm;
      } else {
        ht = hde32_table + DELTA_FPU_REG;
        t = ht[t] << m_reg;
      }
      if (t & 0x80)
        hs->flags |= F_ERROR | F_ERROR_OPCODE;
    }

    if (pref & PRE_LOCK) {
      if (m_mod == 3) {
        hs->flags |= F_ERROR | F_ERROR_LOCK;
      } else {
        uint8_t *table_end, op = opcode;
        if (hs->opcode2) {
          ht = hde32_table + DELTA_OP2_LOCK_OK;
          table_end = ht + DELTA_OP_ONLY_MEM - DELTA_OP2_LOCK_OK;
        } else {
          ht = hde32_table + DELTA_OP_LOCK_OK;
          table_end = ht + DELTA_OP2_LOCK_OK - DELTA_OP_LOCK_OK;
          op &= -2;
        }
        for (; ht != table_end; ht++)
          if (*ht++ == op) {
            if (!((*ht << m_reg) & 0x80))
              goto no_lock_error;
            else
              break;
          }
          hs->flags |= F_ERROR | F_ERROR_LOCK;
no_lock_error:
          ;
      }
    }

    if (hs->opcode2) {
      switch (opcode) {
      case 0x20: case 0x22:
        m_mod = 3;
        if (m_reg > 4 || m_reg == 1)
          goto error_operand;
        else
          goto no_error_operand;
      case 0x21: case 0x23:
        m_mod = 3;
        if (m_reg == 4 || m_reg == 5)
          goto error_operand;
        else
          goto no_error_operand;
      }
    } else {
      switch (opcode) {
      case 0x8c:
        if (m_reg > 5)
          goto error_operand;
        else
          goto no_error_operand;
      case 0x8e:
        if (m_reg == 1 || m_reg > 5)
          goto error_operand;
        else
          goto no_error_operand;
      }
    }

    if (m_mod == 3) {
      uint8_t *table_end;
      if (hs->opcode2) {
        ht = hde32_table + DELTA_OP2_ONLY_MEM;
        table_end = ht + sizeof(hde32_table) - DELTA_OP2_ONLY_MEM;
      } else {
        ht = hde32_table + DELTA_OP_ONLY_MEM;
        table_end = ht + DELTA_OP2_ONLY_MEM - DELTA_OP_ONLY_MEM;
      }
      for (; ht != table_end; ht += 2)
        if (*ht++ == opcode) {
          if (*ht++ & pref && !((*ht << m_reg) & 0x80))
            goto error_operand;
          else
            break;
        }
        goto no_error_operand;
    } else if (hs->opcode2) {
      switch (opcode) {
      case 0x50: case 0xd7: case 0xf7:
        if (pref & (PRE_NONE | PRE_66))
          goto error_operand;
        break;
      case 0xd6:
        if (pref & (PRE_F2 | PRE_F3))
          goto error_operand;
        break;
      case 0xc5:
        goto error_operand;
      }
      goto no_error_operand;
    } else
      goto no_error_operand;

error_operand:
    hs->flags |= F_ERROR | F_ERROR_OPERAND;
no_error_operand:

    c = *p++;
    if (m_reg <= 1) {
      if (opcode == 0xf6)
        cflags |= C_IMM8;
      else if (opcode == 0xf7)
        cflags |= C_IMM_P66;
    }

    switch (m_mod) {
    case 0:
      if (pref & PRE_67) {
        if (m_rm == 6)
          disp_size = 2;
      } else
        if (m_rm == 5)
          disp_size = 4;
      break;
    case 1:
      disp_size = 1;
      break;
    case 2:
      disp_size = 2;
      if (!(pref & PRE_67))
        disp_size <<= 1;
    }

    if (m_mod != 3 && m_rm == 4 && !(pref & PRE_67)) {
      hs->flags |= F_SIB;
      p++;
      hs->sib = c;
      hs->sib_scale = c >> 6;
      hs->sib_index = (c & 0x3f) >> 3;
      if ((hs->sib_base = c & 7) == 5 && !(m_mod & 1))
        disp_size = 4;
    }

    p--;
    switch (disp_size) {
    case 1:
      hs->flags |= F_DISP8;
      hs->disp.disp8 = *p;
      break;
    case 2:
      hs->flags |= F_DISP16;
      hs->disp.disp16 = *(uint16_t *)p;
      break;
    case 4:
      hs->flags |= F_DISP32;
      hs->disp.disp32 = *(uint32_t *)p;
    }
    p += disp_size;
  } else if (pref & PRE_LOCK)
    hs->flags |= F_ERROR | F_ERROR_LOCK;

  if (cflags & C_IMM_P66) {
    if (cflags & C_REL32) {
      if (pref & PRE_66) {
        hs->flags |= F_IMM16 | F_RELATIVE;
        hs->imm.imm16 = *(uint16_t *)p;
        p += 2;
        goto disasm_done;
      }
      goto rel32_ok;
    }
    if (pref & PRE_66) {
      hs->flags |= F_IMM16;
      hs->imm.imm16 = *(uint16_t *)p;
      p += 2;
    } else {
      hs->flags |= F_IMM32;
      hs->imm.imm32 = *(uint32_t *)p;
      p += 4;
    }
  }

  if (cflags & C_IMM16) {
    if (hs->flags & F_IMM32) {
      hs->flags |= F_IMM16;
      hs->disp.disp16 = *(uint16_t *)p;
    } else if (hs->flags & F_IMM16) {
      hs->flags |= F_2IMM16;
      hs->disp.disp16 = *(uint16_t *)p;
    } else {
      hs->flags |= F_IMM16;
      hs->imm.imm16 = *(uint16_t *)p;
    }
    p += 2;
  }
  if (cflags & C_IMM8) {
    hs->flags |= F_IMM8;
    hs->imm.imm8 = *p++;
  }

  if (cflags & C_REL32) {
rel32_ok:
    hs->flags |= F_IMM32 | F_RELATIVE;
    hs->imm.imm32 = *(uint32_t *)p;
    p += 4;
  } else if (cflags & C_REL8) {
    hs->flags |= F_IMM8 | F_RELATIVE;
    hs->imm.imm8 = *p++;
  }

disasm_done:

  if ((hs->len = (uint8_t)(p-(uint8_t *)code)) > 15) {
    hs->flags |= F_ERROR | F_ERROR_LENGTH;
    hs->len = 15;
  }

  return (unsigned int)hs->len;
}

unsigned int hde64_disasm(const void *code, HDE64::hde64s *hs)
{
  using namespace HDE64;

  unsigned char hde64_table[] = {
    0xa5,0xaa,0xa5,0xb8,0xa5,0xaa,0xa5,0xaa,0xa5,0xb8,0xa5,0xb8,0xa5,0xb8,0xa5,
    0xb8,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xac,0xc0,0xcc,0xc0,0xa1,0xa1,
    0xa1,0xa1,0xb1,0xa5,0xa5,0xa6,0xc0,0xc0,0xd7,0xda,0xe0,0xc0,0xe4,0xc0,0xea,
    0xea,0xe0,0xe0,0x98,0xc8,0xee,0xf1,0xa5,0xd3,0xa5,0xa5,0xa1,0xea,0x9e,0xc0,
    0xc0,0xc2,0xc0,0xe6,0x03,0x7f,0x11,0x7f,0x01,0x7f,0x01,0x3f,0x01,0x01,0xab,
    0x8b,0x90,0x64,0x5b,0x5b,0x5b,0x5b,0x5b,0x92,0x5b,0x5b,0x76,0x90,0x92,0x92,
    0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x6a,0x73,0x90,
    0x5b,0x52,0x52,0x52,0x52,0x5b,0x5b,0x5b,0x5b,0x77,0x7c,0x77,0x85,0x5b,0x5b,
    0x70,0x5b,0x7a,0xaf,0x76,0x76,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,0x5b,
    0x5b,0x5b,0x86,0x01,0x03,0x01,0x04,0x03,0xd5,0x03,0xd5,0x03,0xcc,0x01,0xbc,
    0x03,0xf0,0x03,0x03,0x04,0x00,0x50,0x50,0x50,0x50,0xff,0x20,0x20,0x20,0x20,
    0x01,0x01,0x01,0x01,0xc4,0x02,0x10,0xff,0xff,0xff,0x01,0x00,0x03,0x11,0xff,
    0x03,0xc4,0xc6,0xc8,0x02,0x10,0x00,0xff,0xcc,0x01,0x01,0x01,0x00,0x00,0x00,
    0x00,0x01,0x01,0x03,0x01,0xff,0xff,0xc0,0xc2,0x10,0x11,0x02,0x03,0x01,0x01,
    0x01,0xff,0xff,0xff,0x00,0x00,0x00,0xff,0x00,0x00,0xff,0xff,0xff,0xff,0x10,
    0x10,0x10,0x10,0x02,0x10,0x00,0x00,0xc6,0xc8,0x02,0x02,0x02,0x02,0x06,0x00,
    0x04,0x00,0x02,0xff,0x00,0xc0,0xc2,0x01,0x01,0x03,0x03,0x03,0xca,0x40,0x00,
    0x0a,0x00,0x04,0x00,0x00,0x00,0x00,0x7f,0x00,0x33,0x01,0x00,0x00,0x00,0x00,
    0x00,0x00,0xff,0xbf,0xff,0xff,0x00,0x00,0x00,0x00,0x07,0x00,0x00,0xff,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,
    0x00,0x00,0x00,0xbf,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7f,0x00,0x00,
    0xff,0x40,0x40,0x40,0x40,0x41,0x49,0x40,0x40,0x40,0x40,0x4c,0x42,0x40,0x40,
    0x40,0x40,0x40,0x40,0x40,0x40,0x4f,0x44,0x53,0x40,0x40,0x40,0x44,0x57,0x43,
    0x5c,0x40,0x60,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x40,
    0x40,0x40,0x64,0x66,0x6e,0x6b,0x40,0x40,0x6a,0x46,0x40,0x40,0x44,0x46,0x40,
    0x40,0x5b,0x44,0x40,0x40,0x00,0x00,0x00,0x00,0x06,0x06,0x06,0x06,0x01,0x06,
    0x06,0x02,0x06,0x06,0x00,0x06,0x00,0x0a,0x0a,0x00,0x00,0x00,0x02,0x07,0x07,
    0x06,0x02,0x0d,0x06,0x06,0x06,0x0e,0x05,0x05,0x02,0x02,0x00,0x00,0x04,0x04,
    0x04,0x04,0x05,0x06,0x06,0x06,0x00,0x00,0x00,0x0e,0x00,0x00,0x08,0x00,0x10,
    0x00,0x18,0x00,0x20,0x00,0x28,0x00,0x30,0x00,0x80,0x01,0x82,0x01,0x86,0x00,
    0xf6,0xcf,0xfe,0x3f,0xab,0x00,0xb0,0x00,0xb1,0x00,0xb3,0x00,0xba,0xf8,0xbb,
    0x00,0xc0,0x00,0xc1,0x00,0xc7,0xbf,0x62,0xff,0x00,0x8d,0xff,0x00,0xc4,0xff,
    0x00,0xc5,0xff,0x00,0xff,0xff,0xeb,0x01,0xff,0x0e,0x12,0x08,0x00,0x13,0x09,
    0x00,0x16,0x08,0x00,0x17,0x09,0x00,0x2b,0x09,0x00,0xae,0xff,0x07,0xb2,0xff,
    0x00,0xb4,0xff,0x00,0xb5,0xff,0x00,0xc3,0x01,0x00,0xc7,0xff,0xbf,0xe7,0x08,
    0x00,0xf0,0x02,0x00
  };

  uint8_t x, c, *p = (uint8_t *)code, cflags, opcode, pref = 0;
  uint8_t *ht = hde64_table, m_mod, m_reg, m_rm, disp_size = 0;
  uint8_t op64 = 0;

  // Avoid using memset to reduce the footprint.
#ifndef _MSC_VER
  memset((LPBYTE)hs, 0, sizeof(hde64s));
#else
  __stosb((LPBYTE)hs, 0, sizeof(hde64s));
#endif

  for (x = 16; x; x--)
    switch (c = *p++) {
    case 0xf3:
      hs->p_rep = c;
      pref |= PRE_F3;
      break;
    case 0xf2:
      hs->p_rep = c;
      pref |= PRE_F2;
      break;
    case 0xf0:
      hs->p_lock = c;
      pref |= PRE_LOCK;
      break;
    case 0x26: case 0x2e: case 0x36:
    case 0x3e: case 0x64: case 0x65:
      hs->p_seg = c;
      pref |= PRE_SEG;
      break;
    case 0x66:
      hs->p_66 = c;
      pref |= PRE_66;
      break;
    case 0x67:
      hs->p_67 = c;
      pref |= PRE_67;
      break;
    default:
      goto pref_done;
  }
pref_done:

  hs->flags = (uint32_t)pref << 23;

  if (!pref)
    pref |= PRE_NONE;

  if ((c & 0xf0) == 0x40) {
    hs->flags |= F_PREFIX_REX;
    if ((hs->rex_w = (c & 0xf) >> 3) && (*p & 0xf8) == 0xb8)
      op64++;
    hs->rex_r = (c & 7) >> 2;
    hs->rex_x = (c & 3) >> 1;
    hs->rex_b = c & 1;
    if (((c = *p++) & 0xf0) == 0x40) {
      opcode = c;
      goto error_opcode;
    }
  }

  if ((hs->opcode = c) == 0x0f) {
    hs->opcode2 = c = *p++;
    ht += DELTA_OPCODES;
  } else if (c >= 0xa0 && c <= 0xa3) {
    op64++;
    if (pref & PRE_67)
      pref |= PRE_66;
    else
      pref &= ~PRE_66;
  }

  opcode = c;
  cflags = ht[ht[opcode / 4] + (opcode % 4)];

  if (cflags == C_ERROR) {
error_opcode:
    hs->flags |= F_ERROR | F_ERROR_OPCODE;
    cflags = 0;
    if ((opcode & -3) == 0x24)
      cflags++;
  }

  x = 0;
  if (cflags & C_GROUP) {
    uint16_t t;
    t = *(uint16_t *)(ht + (cflags & 0x7f));
    cflags = (uint8_t)t;
    x = (uint8_t)(t >> 8);
  }

  if (hs->opcode2) {
    ht = hde64_table + DELTA_PREFIXES;
    if (ht[ht[opcode / 4] + (opcode % 4)] & pref)
      hs->flags |= F_ERROR | F_ERROR_OPCODE;
  }

  if (cflags & C_MODRM) {
    hs->flags |= F_MODRM;
    hs->modrm = c = *p++;
    hs->modrm_mod = m_mod = c >> 6;
    hs->modrm_rm = m_rm = c & 7;
    hs->modrm_reg = m_reg = (c & 0x3f) >> 3;

    if (x && ((x << m_reg) & 0x80))
      hs->flags |= F_ERROR | F_ERROR_OPCODE;

    if (!hs->opcode2 && opcode >= 0xd9 && opcode <= 0xdf) {
      uint8_t t = opcode - 0xd9;
      if (m_mod == 3) {
        ht = hde64_table + DELTA_FPU_MODRM + t*8;
        t = ht[m_reg] << m_rm;
      } else {
        ht = hde64_table + DELTA_FPU_REG;
        t = ht[t] << m_reg;
      }
      if (t & 0x80)
        hs->flags |= F_ERROR | F_ERROR_OPCODE;
    }

    if (pref & PRE_LOCK) {
      if (m_mod == 3) {
        hs->flags |= F_ERROR | F_ERROR_LOCK;
      } else {
        uint8_t *table_end, op = opcode;
        if (hs->opcode2) {
          ht = hde64_table + DELTA_OP2_LOCK_OK;
          table_end = ht + DELTA_OP_ONLY_MEM - DELTA_OP2_LOCK_OK;
        } else {
          ht = hde64_table + DELTA_OP_LOCK_OK;
          table_end = ht + DELTA_OP2_LOCK_OK - DELTA_OP_LOCK_OK;
          op &= -2;
        }
        for (; ht != table_end; ht++)
          if (*ht++ == op) {
            if (!((*ht << m_reg) & 0x80))
              goto no_lock_error;
            else
              break;
          }
          hs->flags |= F_ERROR | F_ERROR_LOCK;
no_lock_error:
          ;
      }
    }

    if (hs->opcode2) {
      switch (opcode) {
      case 0x20: case 0x22:
        m_mod = 3;
        if (m_reg > 4 || m_reg == 1)
          goto error_operand;
        else
          goto no_error_operand;
      case 0x21: case 0x23:
        m_mod = 3;
        if (m_reg == 4 || m_reg == 5)
          goto error_operand;
        else
          goto no_error_operand;
      }
    } else {
      switch (opcode) {
      case 0x8c:
        if (m_reg > 5)
          goto error_operand;
        else
          goto no_error_operand;
      case 0x8e:
        if (m_reg == 1 || m_reg > 5)
          goto error_operand;
        else
          goto no_error_operand;
      }
    }

    if (m_mod == 3) {
      uint8_t *table_end;
      if (hs->opcode2) {
        ht = hde64_table + DELTA_OP2_ONLY_MEM;
        table_end = ht + sizeof(hde64_table) - DELTA_OP2_ONLY_MEM;
      } else {
        ht = hde64_table + DELTA_OP_ONLY_MEM;
        table_end = ht + DELTA_OP2_ONLY_MEM - DELTA_OP_ONLY_MEM;
      }
      for (; ht != table_end; ht += 2)
        if (*ht++ == opcode) {
          if (*ht++ & pref && !((*ht << m_reg) & 0x80))
            goto error_operand;
          else
            break;
        }
        goto no_error_operand;
    } else if (hs->opcode2) {
      switch (opcode) {
      case 0x50: case 0xd7: case 0xf7:
        if (pref & (PRE_NONE | PRE_66))
          goto error_operand;
        break;
      case 0xd6:
        if (pref & (PRE_F2 | PRE_F3))
          goto error_operand;
        break;
      case 0xc5:
        goto error_operand;
      }
      goto no_error_operand;
    } else
      goto no_error_operand;

error_operand:
    hs->flags |= F_ERROR | F_ERROR_OPERAND;
no_error_operand:

    c = *p++;
    if (m_reg <= 1) {
      if (opcode == 0xf6)
        cflags |= C_IMM8;
      else if (opcode == 0xf7)
        cflags |= C_IMM_P66;
    }

    switch (m_mod) {
    case 0:
      if (pref & PRE_67) {
        if (m_rm == 6)
          disp_size = 2;
      } else
        if (m_rm == 5)
          disp_size = 4;
      break;
    case 1:
      disp_size = 1;
      break;
    case 2:
      disp_size = 2;
      if (!(pref & PRE_67))
        disp_size <<= 1;
    }

    if (m_mod != 3 && m_rm == 4) {
      hs->flags |= F_SIB;
      p++;
      hs->sib = c;
      hs->sib_scale = c >> 6;
      hs->sib_index = (c & 0x3f) >> 3;
      if ((hs->sib_base = c & 7) == 5 && !(m_mod & 1))
        disp_size = 4;
    }

    p--;
    switch (disp_size) {
    case 1:
      hs->flags |= F_DISP8;
      hs->disp.disp8 = *p;
      break;
    case 2:
      hs->flags |= F_DISP16;
      hs->disp.disp16 = *(uint16_t *)p;
      break;
    case 4:
      hs->flags |= F_DISP32;
      hs->disp.disp32 = *(uint32_t *)p;
    }
    p += disp_size;
  } else if (pref & PRE_LOCK)
    hs->flags |= F_ERROR | F_ERROR_LOCK;

  if (cflags & C_IMM_P66) {
    if (cflags & C_REL32) {
      if (pref & PRE_66) {
        hs->flags |= F_IMM16 | F_RELATIVE;
        hs->imm.imm16 = *(uint16_t *)p;
        p += 2;
        goto disasm_done;
      }
      goto rel32_ok;
    }
    if (op64) {
      hs->flags |= F_IMM64;
      hs->imm.imm64 = *(uint64_t *)p;
      p += 8;
    } else if (!(pref & PRE_66)) {
      hs->flags |= F_IMM32;
      hs->imm.imm32 = *(uint32_t *)p;
      p += 4;
    } else
      goto imm16_ok;
  }


  if (cflags & C_IMM16) {
imm16_ok:
    hs->flags |= F_IMM16;
    hs->imm.imm16 = *(uint16_t *)p;
    p += 2;
  }
  if (cflags & C_IMM8) {
    hs->flags |= F_IMM8;
    hs->imm.imm8 = *p++;
  }

  if (cflags & C_REL32) {
rel32_ok:
    hs->flags |= F_IMM32 | F_RELATIVE;
    hs->imm.imm32 = *(uint32_t *)p;
    p += 4;
  } else if (cflags & C_REL8) {
    hs->flags |= F_IMM8 | F_RELATIVE;
    hs->imm.imm8 = *p++;
  }

disasm_done:

  if ((hs->len = (uint8_t)(p-(uint8_t *)code)) > 15) {
    hs->flags |= F_ERROR | F_ERROR_LENGTH;
    hs->len = 15;
  }

  return (unsigned int)hs->len;
}

// --- End of HDE --- //
