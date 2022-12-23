#ifndef _COMMON_H_
#define _COMMON_H_
#include <iostream>
#include <Windows.h>
#include <vector>


namespace common {

#define OB_TYPE_INDEX_TYPE				1  //[ObjT] "Type"
#define OB_TYPE_INDEX_DIRECTORY			2  //[Dire] "Directory"
#define OB_TYPE_INDEX_SYMBOLIC_LINK		3  //[Symb] "SymbolicLink"
#define OB_TYPE_INDEX_TOKEN				4  //[Toke] "Token"
#define OB_TYPE_INDEX_PROCESS			5  //[Proc] "Process"
#define OB_TYPE_INDEX_THREAD			6  //[Thre] "Thread"
#define OB_TYPE_INDEX_JOB				7  //[Job ] "Job"
#define OB_TYPE_INDEX_EVENT				8  //[Even] "Event"
#define OB_TYPE_INDEX_EVENT_PAIR		9  //[Even] "EventPair"
#define OB_TYPE_INDEX_MUTANT			10 //[Muta] "Mutant"
#define OB_TYPE_INDEX_CALLBACK			11 //[Call] "Callback"
#define OB_TYPE_INDEX_SEMAPHORE			12 //[Sema] "Semaphore"
#define OB_TYPE_INDEX_TIMER				13 //[Time] "Timer"
#define OB_TYPE_INDEX_PROFILE			14 //[Prof] "Profile"
#define OB_TYPE_INDEX_WINDOW_STATION	15 //[Wind] "WindowStation"
#define OB_TYPE_INDEX_DESKTOP			16 //[Desk] "Desktop"
#define OB_TYPE_INDEX_SECTION			17 //[Sect] "Section"
#define OB_TYPE_INDEX_KEY				18 //[Key ] "Key"
#define OB_TYPE_INDEX_PORT				19 //[Port] "Port"
#define OB_TYPE_INDEX_WAITABLE_PORT		20 //[Wait] "WaitablePort"
#define OB_TYPE_INDEX_ADAPTER			21 //[Adap] "Adapter"
#define OB_TYPE_INDEX_CONTROLLER		22 //[Cont] "Controller"
#define OB_TYPE_INDEX_DEVICE			23 //[Devi] "Device"
#define OB_TYPE_INDEX_DRIVER			24 //[Driv] "Driver"
#define OB_TYPE_INDEX_IO_COMPLETION		25 //[IoCo] "IoCompletion"
#define OB_TYPE_INDEX_FILE				26 //[File] "File"
#define OB_TYPE_INDEX_WMI_GUID			27 //[WmiG] "WmiGuid"


#define STATUS_ERROR_PARAM_NULL			1001


#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)
#define STATUS_INFO_LENGTH_MISMATCH      ((NTSTATUS)0xC0000004L)

#define MINCHAR		0x80
#define MAXCHAR     0x7f
#define MINSHORT    0x8000
#define MAXSHORT    0x7fff
#define MINLONG     0x80000000
#define MAXLONG     0x7fffffff
#define MAXUCHAR    0xff
#define MAXUSHORT   0xffff
#define MAXULONG    0xffffffff

enum class _SystemInformationClass_
{
	SystemBasicInformation = 0x0,
	SystemProcessorInformation = 0x1,
	SystemPerformanceInformation = 0x2,
	SystemTimeOfDayInformation = 0x3,
	SystemPathInformation = 0x4,
	SystemProcessInformation = 0x5,			//进程对象信息
	SystemCallCountInformation = 0x6,
	SystemDeviceInformation = 0x7,
	SystemProcessorPerformanceInformation = 0x8,
	SystemFlagsInformation = 0x9,
	SystemCallTimeInformation = 0xa,
	SystemModuleInformation = 0xb,
	SystemLocksInformation = 0xc,
	SystemStackTraceInformation = 0xd,
	SystemPagedPoolInformation = 0xe,
	SystemNonPagedPoolInformation = 0xf,
	SystemHandleInformation = 0x10,			
	SystemObjectInformation = 0x11,			
	SystemPageFileInformation = 0x12,
	SystemVdmInstemulInformation = 0x13,
	SystemVdmBopInformation = 0x14,
	SystemFileCacheInformation = 0x15,
	SystemPoolTagInformation = 0x16,
	SystemInterruptInformation = 0x17,
	SystemDpcBehaviorInformation = 0x18,
	SystemFullMemoryInformation = 0x19,
	SystemLoadGdiDriverInformation = 0x1a,
	SystemUnloadGdiDriverInformation = 0x1b,
	SystemTimeAdjustmentInformation = 0x1c,
	SystemSummaryMemoryInformation = 0x1d,
	SystemMirrorMemoryInformation = 0x1e,
	SystemPerformanceTraceInformation = 0x1f,
	SystemObsolete0 = 0x20,
	SystemExceptionInformation = 0x21,
	SystemCrashDumpStateInformation = 0x22,
	SystemKernelDebuggerInformation = 0x23,
	SystemContextSwitchInformation = 0x24,
	SystemRegistryQuotaInformation = 0x25,
	SystemExtendServiceTableInformation = 0x26,
	SystemPrioritySeperation = 0x27,
	SystemVerifierAddDriverInformation = 0x28,
	SystemVerifierRemoveDriverInformation = 0x29,
	SystemProcessorIdleInformation = 0x2a,
	ystemLegacyDriverInformation = 0x2b,
	SystemCurrentTimeZoneInformation = 0x2c,
	SystemLookasideInformation = 0x2d,
	SystemTimeSlipNotification = 0x2e,
	SystemSessionCreate = 0x2f,
	SystemSessionDetach = 0x30,
	SystemSessionInformation = 0x31,
	SystemRangeStartInformation = 0x32,
	SystemVerifierInformation = 0x33,
	SystemVerifierThunkExtend = 0x34,
	SystemSessionProcessInformation = 0x35,
	SystemLoadGdiDriverInSystemSpace = 0x36,
	SystemNumaProcessorMap = 0x37,
	SystemPrefetcherInformation = 0x38,
	SystemExtendedProcessInformation = 0x39,
	SystemRecommendedSharedDataAlignment = 0x3a,
	SystemComPlusPackage = 0x3b,
	SystemNumaAvailableMemory = 0x3c,
	SystemProcessorPowerInformation = 0x3d,
	SystemEmulationBasicInformation = 0x3e,
	SystemEmulationProcessorInformation = 0x3f,
	SystemExtendedHandleInformation = 0x40,
	SystemLostDelayedWriteInformation = 0x41,
	SystemBigPoolInformation = 0x42,
	SystemSessionPoolTagInformation = 0x43,
	SystemSessionMappedViewInformation = 0x44,
	SystemHotpatchInformation = 0x45,
	SystemObjectSecurityMode = 0x46,
	SystemWatchdogTimerHandler = 0x47,
	SystemWatchdogTimerInformation = 0x48,
	SystemLogicalProcessorInformation = 0x49,
	SystemWow64SharedInformationObsolete = 0x4a,
	SystemRegisterFirmwareTableInformationHandler = 0x4b,
	SystemFirmwareTableInformation = 0x4c,
	SystemModuleInformationEx = 0x4d,
	SystemVerifierTriageInformation = 0x4e,
	SystemSuperfetchInformation = 0x4f,
	SystemMemoryListInformation = 0x50,
	SystemFileCacheInformationEx = 0x51,
	SystemThreadPriorityClientIdInformation = 0x52,
	SystemProcessorIdleCycleTimeInformation = 0x53,
	SystemVerifierCancellationInformation = 0x54,
	SystemProcessorPowerInformationEx = 0x55,
	SystemRefTraceInformation = 0x56,
	SystemSpecialPoolInformation = 0x57,
	SystemProcessIdInformation = 0x58,
	SystemErrorPortInformation = 0x59,
	SystemBootEnvironmentInformation = 0x5a,
	SystemHypervisorInformation = 0x5b,
	SystemVerifierInformationEx = 0x5c,
	SystemTimeZoneInformation = 0x5d,
	SystemImageFileExecutionOptionsInformation = 0x5e,
	SystemCoverageInformation = 0x5f,
	SystemPrefetchPatchInformation = 0x60,
	SystemVerifierFaultsInformation = 0x61,
	SystemSystemPartitionInformation = 0x62,
	SystemSystemDiskInformation = 0x63,
	SystemProcessorPerformanceDistribution = 0x64,
	SystemNumaProximityNodeInformation = 0x65,
	SystemDynamicTimeZoneInformation = 0x66,
	SystemCodeIntegrityInformation = 0x67,
	SystemProcessorMicrocodeUpdateInformation = 0x68,
	SystemProcessorBrandString = 0x69,
	SystemVirtualAddressInformation = 0x6a,
	SystemLogicalProcessorAndGroupInformation = 0x6b,
	SystemProcessorCycleTimeInformation = 0x6c,
	SystemStoreInformation = 0x6d,
	SystemRegistryAppendString = 0x6e,
	SystemAitSamplingValue = 0x6f,
	SystemVhdBootInformation = 0x70,
	SystemCpuQuotaInformation = 0x71,
	SystemNativeBasicInformation = 0x72,
	SystemErrorPortTimeouts = 0x73,
	SystemLowPriorityIoInformation = 0x74,
	SystemBootEntropyInformation = 0x75,
	SystemVerifierCountersInformation = 0x76,
	SystemPagedPoolInformationEx = 0x77,
	SystemSystemPtesInformationEx = 0x78,
	SystemNodeDistanceInformation = 0x79,
	SystemAcpiAuditInformation = 0x7a,
	SystemBasicPerformanceInformation = 0x7b,
	SystemQueryPerformanceCounterInformation = 0x7c,
	SystemSessionBigPoolInformation = 0x7d,
	SystemBootGraphicsInformation = 0x7e,
	SystemScrubPhysicalMemoryInformation = 0x7f,
	SystemBadPageInformation = 0x80,
	SystemProcessorProfileControlArea = 0x81,
	SystemCombinePhysicalMemoryInformation = 0x82,
	SystemEntropyInterruptTimingInformation = 0x83,
	SystemConsoleInformation = 0x84,
	SystemPlatformBinaryInformation = 0x85,
	SystemThrottleNotificationInformation = 0x86,
	SystemHypervisorProcessorCountInformation = 0x87,
	SystemDeviceDataInformation = 0x88,
	SystemDeviceDataEnumerationInformation = 0x89,
	SystemMemoryTopologyInformation = 0x8a,
	SystemMemoryChannelInformation = 0x8b,
	SystemBootLogoInformation = 0x8c,
	SystemProcessorPerformanceInformationEx = 0x8d,
	SystemSpare0 = 0x8e,
	SystemSecureBootPolicyInformation = 0x8f,
	SystemPageFileInformationEx = 0x90,
	SystemSecureBootInformation = 0x91,
	SystemEntropyInterruptTimingRawInformation = 0x92,
	SystemPortableWorkspaceEfiLauncherInformation = 0x93,
	SystemFullProcessInformation = 0x94,
	SystemKernelDebuggerInformationEx = 0x95,
	SystemBootMetadataInformation = 0x96,
	SystemSoftRebootInformation = 0x97,
	SystemElamCertificateInformation = 0x98,
	SystemOfflineDumpConfigInformation = 0x99,
	SystemProcessorFeaturesInformation = 0x9a,
	SystemRegistryReconciliationInformation = 0x9b,
	MaxSystemInfoClass = 0x9c,
};
using SystemInformationClass = _SystemInformationClass_;

enum class _ObjectInformationClass_
{
	ObjectBasicInformation,
	ObjectNameInformation,
	ObjectTypeInformation,
	ObjectTypesInformation,
	ObjectHandleFlagInformation,
	ObjectSessionInformation,
	MaxObjectInfoClass
};
using ObjectInformationClass = _ObjectInformationClass_;


typedef struct _UnicodeString_ {
    USHORT length;
    USHORT maximumLength;
    PWSTR  buffer;
} Unicode, *UnicodePtr; 

typedef struct _SystemHandleTableEntryInfo_
{
	USHORT uniqueProcessID;			    //所属进程
	USHORT creatorBackTraceIndex;		//???
	UCHAR  objectTypeIndex;				//句柄对象类型索引
	UCHAR  handleAttributes;			//???
	USHORT handleValue;					//句柄值
	PVOID  object;						//句柄对应对象？？
	ULONG  grantedAccess;				//???
}SystemHandleTableEntryInfo, *SystemHandleTableEntryInfoPtr;

typedef struct _SystemHandleInformation_
{
	ULONG					   numberOfHandles;
	SystemHandleTableEntryInfo handles[0];
}SystemHandleInformation, *SystemHandleInformationPtr;

typedef struct _ObjectTypeInformation_
{
	Unicode			typeName;
	ULONG			totalNumberOfObjects;
	ULONG			totalNumberOfHandles;
	ULONG			totalPagedPoolUsage;
	ULONG			totalNonpagedPoolUsage;
	ULONG			totalNamePoolUsage;
	ULONG			totalHandleTableUsage;
	ULONG			highWaterNumberOfObjects;
	ULONG			highWaterNumberOfHandles;
	ULONG			highWaterPagedPoolUsage;
	ULONG			highWaterNonpagedPoolUsage;
	ULONG			highWaterNamePoolUsage;
	ULONG			highWaterHandleTableUsage;
	ULONG			invalidAttributes;
	GENERIC_MAPPING genericMapping;
	ULONG			validAccessMask;
	BOOLEAN			securityRequired;
	BOOLEAN			maintainHandleCount;
	ULONG			poolType;
	ULONG			defaultPagedPoolCharge;
	ULONG			defaultNonpagedPoolCharge;
}ObjectTypeInformation, * ObjectTypeInformationPtr;

typedef struct _ObjectNameInformation_ 
{
    Unicode name;
} ObjectNameInformation, *ObjectNameInformationPtr;


struct _HandleInfo_
{
	DWORD	pid;
	USHORT	handle;
	std::wstring kernelFileName;
};
using HandleInfo = _HandleInfo_;

struct _SystemInfoData_
{
	NTSTATUS status = 0;
	std::vector<BYTE> memory;
};
using SystemInfoData = _SystemInfoData_;


typedef
NTSTATUS 
(NTAPI *fnNtQuerySystemInformation)(
  ULONG   systemInformationClass,
  PVOID   systemInformation,
  ULONG   systemInformationLength,
  PULONG  returnLen
);

typedef
NTSTATUS
(NTAPI* fnNtQueryObject)(
	HANDLE objectHandle,
	ULONG  objectInformationClass,
	PVOID  objectInformation,
	ULONG  objectInformationLength,
	PULONG returnLength);


typedef
NTSTATUS 
(NTAPI* fnNtDuplicateObject)(
	HANDLE		sourceProcessHandle, 
	HANDLE		sourceHandle, 
	HANDLE		targetProcessHandle, 
	PHANDLE		targetHandle, 
	ACCESS_MASK desiredAccess, 
	ULONG		attributes, 
	ULONG		options);

}

#endif // !_COMMON_H_

