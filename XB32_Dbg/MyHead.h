#pragma once
#include <winternl.h>
#pragma comment(lib,"ntdll.lib")

#include <vector>


using std::vector;

//条件断点信息 使用哪个寄存器 ，寄存器的值
typedef struct _CONDITION
{
	BOOL OPEN_EAX;
	DWORD EAX;

	BOOL OPEN_EBX;
	DWORD EBX;

	BOOL OPEN_ECX;
	DWORD ECX;

	BOOL OPEN_EDX;
	DWORD EDX;

	BOOL OPEN_ESI;
	DWORD ESI;

	BOOL OPEN_EDI;
	DWORD EDI;

	BOOL OPEN_EBP;
	DWORD EBP;

	BOOL OPEN_ESP;
	DWORD ESP;

}CONDITION, *PCONDITION;

//一个模块的导入导出函数表记录结构
typedef struct _TABLE
{
	CString ModuleName;							//模块的名字
	DWORD ModuleAddress;						//模块的加载地址
	DWORD ModuleSize;							//模块的尺寸

	CString ExportModuleName;					//导出表模块的名字
	vector<CString>ExportFunctionAddress;		//导出函数地址(十六进制),只有通过函数名导出的地址
	vector<CString>ExportFunctionName;			//导出的函数名字

	vector<CString>ImportModuleName;			//导入的模块名字
	vector<CString>ImportFunctionName;			//导入的函数名字
	vector<CString>ImportFunctionAddress;		//导入的函数地址(十六进制)
	vector<CString>ThunkAdress;					//FirstThunk指向的结构体数组中每个结构体的内存地址(十六进制)
}TABLE, *PTABLE;

typedef struct _PROCESS_BASIC_INFORMATION32 {
	NTSTATUS ExitStatus;
	UINT32 PebBaseAddress;
	UINT32 AffinityMask;
	UINT32 BasePriority;
	UINT32 UniqueProcessId;
	UINT32 InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION32;

//typedef struct _PEB_FREE_BLOCK // 2 elements, 0x8 bytes
//{
//	struct _PEB_FREE_BLOCK * pNext;
//	DWORD dwSize;
//} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;
//
//typedef struct _UNICODE_STR
//{
//	USHORT Length;
//	USHORT MaximumLength;
//	PWSTR pBuffer;
//} UNICODE_STR, *PUNICODE_STR;
//
//typedef struct _MYPEB // 65 elements, 0x210 bytes
//{
//	BYTE bInheritedAddressSpace;
//	BYTE bReadImageFileExecOptions;
//	BYTE bBeingDebugged;
//	BYTE bSpareBool;
//	LPVOID lpMutant;
//	LPVOID lpImageBaseAddress;
//	PPEB_LDR_DATA pLdr;
//	LPVOID lpProcessParameters;
//	LPVOID lpSubSystemData;
//	LPVOID lpProcessHeap;
//	PRTL_CRITICAL_SECTION pFastPebLock;
//	LPVOID lpFastPebLockRoutine;
//	LPVOID lpFastPebUnlockRoutine;
//	DWORD dwEnvironmentUpdateCount;
//	LPVOID lpKernelCallbackTable;
//	DWORD dwSystemReserved;
//	DWORD dwAtlThunkSListPtr32;
//	PPEB_FREE_BLOCK pFreeList;
//	DWORD dwTlsExpansionCounter;
//	LPVOID lpTlsBitmap;
//	DWORD dwTlsBitmapBits[2];
//	LPVOID lpReadOnlySharedMemoryBase;
//	LPVOID lpReadOnlySharedMemoryHeap;
//	LPVOID lpReadOnlyStaticServerData;
//	LPVOID lpAnsiCodePageData;
//	LPVOID lpOemCodePageData;
//	LPVOID lpUnicodeCaseTableData;
//	DWORD dwNumberOfProcessors;
//	DWORD dwNtGlobalFlag;
//	LARGE_INTEGER liCriticalSectionTimeout;
//	DWORD dwHeapSegmentReserve;
//	DWORD dwHeapSegmentCommit;
//	DWORD dwHeapDeCommitTotalFreeThreshold;
//	DWORD dwHeapDeCommitFreeBlockThreshold;
//	DWORD dwNumberOfHeaps;
//	DWORD dwMaximumNumberOfHeaps;
//	LPVOID lpProcessHeaps;
//	LPVOID lpGdiSharedHandleTable;
//	LPVOID lpProcessStarterHelper;
//	DWORD dwGdiDCAttributeList;
//	LPVOID lpLoaderLock;
//	DWORD dwOSMajorVersion;
//	DWORD dwOSMinorVersion;
//	WORD wOSBuildNumber;
//	WORD wOSCSDVersion;
//	DWORD dwOSPlatformId;
//	DWORD dwImageSubsystem;
//	DWORD dwImageSubsystemMajorVersion;
//	DWORD dwImageSubsystemMinorVersion;
//	DWORD dwImageProcessAffinityMask;
//	DWORD dwGdiHandleBuffer[34];
//	LPVOID lpPostProcessInitRoutine;
//	LPVOID lpTlsExpansionBitmap;
//	DWORD dwTlsExpansionBitmapBits[32];
//	DWORD dwSessionId;
//	ULARGE_INTEGER liAppCompatFlags;
//	ULARGE_INTEGER liAppCompatFlagsUser;
//	LPVOID lppShimData;
//	LPVOID lpAppCompatInfo;
//	UNICODE_STR usCSDVersion;
//	LPVOID lpActivationContextData;
//	LPVOID lpProcessAssemblyStorageMap;
//	LPVOID lpSystemDefaultActivationContextData;
//	LPVOID lpSystemAssemblyStorageMap;
//	DWORD dwMinimumStackCommit;
//} MYPEB, *PMYPEB;