#pragma once

/*
Copyright � 1996 Eugene Roshal
Copyright � 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "language.hpp"
#include "bitflags.hpp"
#include "plugin.hpp"

class AncientPlugin
{
	public:
		virtual ~AncientPlugin() {}
		virtual const GUID& GetGUID(void) const = 0;
};

class PluginManager;
struct ExecuteStruct
{
	union
	{
		INT_PTR nResult;
		HANDLE hResult;
		BOOL bResult;
	};

	union
	{
		INT_PTR nDefaultResult;
		HANDLE hDefaultResult;
		BOOL bDefaultResult;
	};
	int id; //function id
	bool bUnloaded;
};

#define EXECUTE_FUNCTION(function, es) \
{ \
	__Prolog(); \
	es.nResult = 0; \
	es.nDefaultResult = 0; \
	es.bUnloaded = false; \
	if ( Opt.ExceptRules ) \
	{ \
		__try \
		{ \
			function; \
		} \
		__except(xfilter(es.id, GetExceptionInformation(), this, 0)) \
		{ \
			m_owner->UnloadPlugin(this, es.id, true); \
			es.bUnloaded = true; \
			es.nResult = es.nDefaultResult; \
			ProcessException=FALSE; \
		} \
	} \
	else \
	{ \
		function; \
	} \
	__Epilog(); \
}

#define EXECUTE_FUNCTION_EX(function, es) EXECUTE_FUNCTION(es.nResult = (INT_PTR)function, es)

#define FUNCTION(id) reinterpret_cast<id##Prototype>(Exports[id])

#define _W(string) L##string
#define W(string) _W(string)

enum EXPORTS_ENUM
{
	iGetGlobalInfo,
	iSetStartupInfo,
	iOpen,
	iClosePanel,
	iGetPluginInfo,
	iGetOpenPanelInfo,
	iGetFindData,
	iFreeFindData,
	iGetVirtualFindData,
	iFreeVirtualFindData,
	iSetDirectory,
	iGetFiles,
	iPutFiles,
	iDeleteFiles,
	iMakeDirectory,
	iProcessHostFile,
	iSetFindList,
	iConfigure,
	iExitFAR,
	iProcessPanelInput,
	iProcessPanelEvent,
	iProcessEditorEvent,
	iCompare,
	iProcessEditorInput,
	iProcessViewerEvent,
	iProcessDialogEvent,
	iProcessSynchroEvent,
#if defined(MANTIS_0000466)
	iProcessMacro,
#endif
#if defined(MANTIS_0001687)
	iProcessConsoleInput,
#endif
	iAnalyse,
	iGetCustomData,
	iFreeCustomData,

	iOpenFilePlugin,
	iGetMinFarVersion,
#ifndef NO_WRAPPER
	iWrapperFunction2, // FarWrapGetProcAddress
#endif // NO_WRAPPER
	i_LAST
};

class Plugin: public AncientPlugin
{
public:
	Plugin(PluginManager *owner, const wchar_t *lpwszModuleName);
	virtual ~Plugin();

	virtual bool GetGlobalInfo(GlobalInfo *Info);
	virtual bool SetStartupInfo(bool &bUnloaded);
	virtual bool CheckMinFarVersion(bool &bUnloaded);
	virtual HANDLE Open(int OpenFrom, const GUID& Guid, INT_PTR Item);
	virtual HANDLE OpenFilePlugin(const wchar_t *Name, const unsigned char *Data, int DataSize, int OpMode);
	virtual int SetFindList(HANDLE hPlugin, const PluginPanelItem *PanelItem, size_t ItemsNumber);
	virtual int GetFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, int OpMode);
	virtual int GetVirtualFindData(HANDLE hPlugin, PluginPanelItem **pPanelItem, size_t *pItemsNumber, const wchar_t *Path);
	virtual int SetDirectory(HANDLE hPlugin, const wchar_t *Dir, int OpMode);
	virtual int GetFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, const wchar_t **DestPath, int OpMode);
	virtual int PutFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, bool Move, int OpMode);
	virtual int DeleteFiles(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode);
	virtual int MakeDirectory(HANDLE hPlugin, const wchar_t **Name, int OpMode);
	virtual int ProcessHostFile(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber, int OpMode);
	virtual int ProcessKey(HANDLE hPlugin, const INPUT_RECORD *Rec, bool Pred);
	virtual int ProcessPanelEvent(HANDLE hPlugin, int Event, PVOID Param);
	virtual int Compare(HANDLE hPlugin, const PluginPanelItem *Item1, const PluginPanelItem *Item2, unsigned long Mode);
	virtual int GetCustomData(const wchar_t *FilePath, wchar_t **CustomData);
	virtual void FreeCustomData(wchar_t *CustomData);
	virtual void GetOpenPanelInfo(HANDLE hPlugin, OpenPanelInfo *Info);
	virtual void FreeFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber);
	virtual void FreeVirtualFindData(HANDLE hPlugin, PluginPanelItem *PanelItem, size_t ItemsNumber);
	virtual void ClosePanel(HANDLE hPlugin);
	virtual int ProcessEditorInput(const INPUT_RECORD *D);
	virtual int ProcessEditorEvent(int Event, PVOID Param);
	virtual int ProcessViewerEvent(int Event, PVOID Param);
	virtual int ProcessDialogEvent(int Event, FarDialogEvent *Param);
	virtual int ProcessSynchroEvent(int Event, PVOID Param);
#if defined(MANTIS_0000466)
	virtual int ProcessMacro(ProcessMacroInfo *Info);
#endif
#if defined(MANTIS_0001687)
	virtual int ProcessConsoleInput(ProcessConsoleInputInfo *Info);
#endif
	virtual int Analyse(const AnalyseInfo *Info);
	virtual bool GetPluginInfo(PluginInfo *pi);
	virtual int Configure(const GUID& Guid);
	virtual void ExitFAR(const ExitInfo *Info);
#ifndef NO_WRAPPER
	virtual bool IsOemPlugin() const { return false; }
#endif // NO_WRAPPER
	virtual bool IsFar2Plugin() const { return HasWrapperFunction2(); }
	virtual const wchar_t *GetHotkeyName() const { return m_strGuid; }

	virtual bool InitLang(const wchar_t *Path) { return PluginLang.Init(Path); }
	void CloseLang() { PluginLang.Close(); }

	bool HasGetGlobalInfo()       const { return Exports[iGetGlobalInfo]!=nullptr; }
	bool HasOpenPanel()           const { return Exports[iOpen]!=nullptr; }
	bool HasMakeDirectory()       const { return Exports[iMakeDirectory]!=nullptr; }
	bool HasDeleteFiles()         const { return Exports[iDeleteFiles]!=nullptr; }
	bool HasPutFiles()            const { return Exports[iPutFiles]!=nullptr; }
	bool HasGetFiles()            const { return Exports[iGetFiles]!=nullptr; }
	bool HasSetStartupInfo()      const { return Exports[iSetStartupInfo]!=nullptr; }
	bool HasClosePanel()          const { return Exports[iClosePanel]!=nullptr; }
	bool HasGetPluginInfo()       const { return Exports[iGetPluginInfo]!=nullptr; }
	bool HasGetOpenPanelInfo()    const { return Exports[iGetOpenPanelInfo]!=nullptr; }
	bool HasGetFindData()         const { return Exports[iGetFindData]!=nullptr; }
	bool HasFreeFindData()        const { return Exports[iFreeFindData]!=nullptr; }
	bool HasGetVirtualFindData()  const { return Exports[iGetVirtualFindData]!=nullptr; }
	bool HasFreeVirtualFindData() const { return Exports[iFreeVirtualFindData]!=nullptr; }
	bool HasSetDirectory()        const { return Exports[iSetDirectory]!=nullptr; }
	bool HasProcessHostFile()     const { return Exports[iProcessHostFile]!=nullptr; }
	bool HasSetFindList()         const { return Exports[iSetFindList]!=nullptr; }
	bool HasConfigure()           const { return Exports[iConfigure]!=nullptr; }
	bool HasExitFAR()             const { return Exports[iExitFAR]!=nullptr; }
	bool HasProcessPanelInput()   const { return Exports[iProcessPanelInput]!=nullptr; }
	bool HasProcessPanelEvent()   const { return Exports[iProcessPanelEvent]!=nullptr; }
	bool HasProcessEditorEvent()  const { return Exports[iProcessEditorEvent]!=nullptr; }
	bool HasCompare()             const { return Exports[iCompare]!=nullptr; }
	bool HasProcessEditorInput()  const { return Exports[iProcessEditorInput]!=nullptr; }
	bool HasProcessViewerEvent()  const { return Exports[iProcessViewerEvent]!=nullptr; }
	bool HasProcessDialogEvent()  const { return Exports[iProcessDialogEvent]!=nullptr; }
	bool HasProcessSynchroEvent() const { return Exports[iProcessSynchroEvent]!=nullptr; }
#if defined(MANTIS_0000466)
	bool HasProcessMacro()        const { return Exports[iProcessMacro]!=nullptr; }
#endif
#if defined(MANTIS_0001687)
	bool HasProcessConsoleInput() const { return Exports[iProcessConsoleInput]!=nullptr; }
#endif
	bool HasAnalyse()             const { return Exports[iAnalyse]!=nullptr; }
	bool HasGetCustomData()       const { return Exports[iGetCustomData]!=nullptr; }
	bool HasFreeCustomData()      const { return Exports[iFreeCustomData]!=nullptr; }

	bool HasOpenFilePlugin()      const { return Exports[iOpenFilePlugin]!=nullptr; }
	bool HasMinFarVersion()       const { return Exports[iGetMinFarVersion]!=nullptr; }

	bool HasWrapperFunction2()    const { return Exports[iWrapperFunction2]!=nullptr; }

	const string &GetModuleName() const { return m_strModuleName; }
	const wchar_t *GetCacheName() const  { return m_strCacheName; }
	const wchar_t* GetTitle(void) const { return strTitle.CPtr(); }
	const wchar_t* GetDescription(void) const { return strDescription.CPtr(); }
	const wchar_t* GetAuthor(void) const { return strAuthor.CPtr(); }
	const GUID& GetGUID(void) const { return m_Guid; }
	const wchar_t *GetMsg(int nID) const { return PluginLang.GetMsg(nID); }

	bool CheckWorkFlags(DWORD flags) const { return WorkFlags.Check(flags)==TRUE; }
	DWORD GetWorkFlags() const { return WorkFlags.Flags; }
	DWORD GetFuncFlags() const { return FuncFlags.Flags; }

	bool Load();
	int Unload(bool bExitFAR = false);
	bool LoadData();
	bool LoadFromCache(const FAR_FIND_DATA_EX &FindData);
	bool SaveToCache();
	bool IsPanelPlugin();

protected:
	virtual void __Prolog() {};
	virtual void __Epilog() {};

	void* Exports[i_LAST];
	const wchar_t **ExportsNamesW;
	const char **ExportsNamesA;

	PluginManager *m_owner; //BUGBUG
	Language PluginLang;

private:
	void InitExports();
	void ClearExports();
	void SetGuid(const GUID& Guid);

	string strTitle;
	string strDescription;
	string strAuthor;

	string m_strModuleName;
	string m_strCacheName;

	BitFlags WorkFlags;      // ������� ����� �������� �������
	BitFlags FuncFlags;      // ������� ����� ������ ����.������� �������

	HMODULE m_hModule;

	VersionInfo MinFarVersion;
	VersionInfo PluginVersion;

	GUID m_Guid;
	string m_strGuid;

	friend class PluginManager;
};

extern PluginStartupInfo NativeInfo;
extern FarStandardFunctions NativeFSF;

size_t WINAPI FarKeyToName(int Key,wchar_t *KeyText,size_t Size);
int WINAPI KeyNameToKeyW(const wchar_t *Name);
