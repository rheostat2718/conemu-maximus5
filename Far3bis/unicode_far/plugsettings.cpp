/*
plugsettings.cpp

API ��� �������� ��������� ��������.
*/
/*
Copyright � 2011 Far Group
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

#include "headers.hpp"
#pragma hdrstop

#include "plugsettings.hpp"
#include "ctrlobj.hpp"
#include "strmix.hpp"
#include "history.hpp"
#include "datetime.hpp"

AbstractSettings::~AbstractSettings()
{
}

bool AbstractSettings::IsValid(void)
{
	return true;
}

PluginSettings::PluginSettings(const GUID& Guid) : PluginsCfg(nullptr)
{
	//��� ���� SCTL_* ����� �������� ��� ExitFarW.
	extern PluginManager *PluginManagerForExitFar;
	Plugin* pPlugin=CtrlObject?CtrlObject->Plugins.FindPlugin(Guid):(PluginManagerForExitFar?PluginManagerForExitFar->FindPlugin(Guid):nullptr);
	if (pPlugin)
	{
		string strGuid = GuidToStr(Guid);
		PluginsCfg = CreatePluginsConfig(strGuid);
		unsigned __int64& root(*m_Keys.insertItem(0));
		root=PluginsCfg->CreateKey(0, strGuid, pPlugin->GetTitle());
	}
}

PluginSettings::~PluginSettings()
{
	if (PluginsCfg)
		delete PluginsCfg;

	for(size_t ii=0;ii<m_Data.getCount();++ii)
	{
		delete [] *m_Data.getItem(ii);
	}
}

bool PluginSettings::IsValid(void)
{
	return m_Keys.getCount()!=0;
}

int PluginSettings::Set(const FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.getCount())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				break;
			case FST_QWORD:
				if (PluginsCfg->SetValue(*m_Keys.getItem(Item.Root),Item.Name,Item.Number)) result=TRUE;
				break;
			case FST_STRING:
				//�� ��� �������� "description" �����, �� ������������ � plugin title �����
				if (Item.Root==0 && !Item.Name) break;
				if (PluginsCfg->SetValue(*m_Keys.getItem(Item.Root),Item.Name,Item.String)) result=TRUE;
				break;
			case FST_DATA:
				if (PluginsCfg->SetValue(*m_Keys.getItem(Item.Root),Item.Name,(const char *)Item.Data.Data,(int)Item.Data.Size)) result=TRUE;
				break;
			default:
				break;
		}
	}
	return result;
}

int PluginSettings::Get(FarSettingsItem& Item)
{
	int result=FALSE;
	if(Item.Root<m_Keys.getCount())
	{
		switch(Item.Type)
		{
			case FST_SUBKEY:
				break;
			case FST_QWORD:
				{
					unsigned __int64 value;
					if (PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,&value))
					{
						result=TRUE;
						Item.Number=value;
					}
				}
				break;
			case FST_STRING:
				{
					string data;
					if (PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,data))
					{
						result=TRUE;
						char** item=m_Data.addItem();
						size_t size=(data.GetLength()+1)*sizeof(wchar_t);
						*item=new char[size];
						memcpy(*item,data.CPtr(),size);
						Item.String=(wchar_t*)*item;
					}
				}
				break;
			case FST_DATA:
				{
					int size=PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,nullptr,0);
					if (size)
					{
						char** item=m_Data.addItem();
						*item=new char[size];
						int checkedSize=PluginsCfg->GetValue(*m_Keys.getItem(Item.Root),Item.Name,*item,size);
						if (size==checkedSize)
						{
							result=TRUE;
							Item.Data.Data=*item;
							Item.Data.Size=size;
						}
					}
				}
				break;
			default:
				break;
		}
	}
	return result;
}

template <class Object> void AddString(Vector<Object>& Array, Object& Item, string& String)
{
	size_t size=String.GetLength()+1;
	Item.Name=new wchar_t[size];
	wmemcpy((wchar_t*)Item.Name,String.CPtr(),size);
	Array.AddItem(Item);
}

int PluginSettings::Enum(FarSettingsEnum& Enum)
{
	int result=FALSE;
	if(Enum.Root<m_Keys.getCount())
	{
		Vector<FarSettingsName>& array=*m_Enum.addItem();
		FarSettingsName item;
		DWORD Index=0,Type;
		string strName;

		unsigned __int64 root = *m_Keys.getItem(Enum.Root);
		item.Type=FST_SUBKEY;
		while (PluginsCfg->EnumKeys(root,Index++,strName))
		{
			AddString(array,item,strName);
		}
		Index=0;
		while (PluginsCfg->EnumValues(root,Index++,strName,&Type))
		{
			item.Type=FST_UNKNOWN;
			switch (Type)
			{
				case HierarchicalConfig::TYPE_INTEGER:
					item.Type=FST_QWORD;
					break;
				case HierarchicalConfig::TYPE_TEXT:
					item.Type=FST_STRING;
					break;
				case HierarchicalConfig::TYPE_BLOB:
					item.Type=FST_DATA;
					break;
			}
			if(item.Type!=FST_UNKNOWN)
			{
				AddString(array,item,strName);
			}
		}
		Enum.Count=array.GetSize();
		Enum.Items=array.GetItems();
		result=TRUE;
	}
	return result;
}

int PluginSettings::Delete(const FarSettingsValue& Value)
{
	int result=FALSE;
	if(Value.Root<m_Keys.getCount())
	{
		if (!Value.Value)
		{
			if (PluginsCfg->DeleteKeyTree(*m_Keys.getItem(Value.Root)))
				result=TRUE;
		}
		else
		{
			if (PluginsCfg->DeleteValue(*m_Keys.getItem(Value.Root),Value.Value))
				result=TRUE;
		}
	}
	return result;
}

int PluginSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	int result=0;
	if(Value.Root<m_Keys.getCount()&&!wcschr(Value.Value,'\\'))
	{
		unsigned __int64 root = 0;
		if (bCreate)
			root = PluginsCfg->CreateKey(*m_Keys.getItem(Value.Root),Value.Value);
		else
			root = PluginsCfg->GetKeyID(*m_Keys.getItem(Value.Root),Value.Value);
		if (root)
		{
			result=static_cast<int>(m_Keys.getCount());
			*m_Keys.insertItem(result) = root;
		}
	}
	return result;
}

FarSettings::FarSettings()
{
}

FarSettings::~FarSettings()
{
}

int FarSettings::Set(const FarSettingsItem& Item)
{
	return FALSE;
}

int FarSettings::Get(FarSettingsItem& Item)
{
	return FALSE;
}

static bool FilterNone(int)
{
	return true;
}

static bool FilterView(int Type)
{
	return (Type==0)?true:false;
}

static bool FilterEdit(int Type)
{
	return (Type==1||Type==4)?true:false;
}

static bool FilterExt(int Type)
{
	return (Type==2||Type==3)?true:false;
}

int FarSettings::Enum(FarSettingsEnum& Enum)
{
	switch(Enum.Root)
	{
		case FSSF_HISTORY_CMD:
			return FillHistory(HISTORYTYPE_CMD,Enum,FilterNone);
		case FSSF_HISTORY_FOLDER:
			return FillHistory(HISTORYTYPE_FOLDER,Enum,FilterNone);
		case FSSF_HISTORY_VIEW:
			return FillHistory(HISTORYTYPE_VIEW,Enum,FilterView);
		case FSSF_HISTORY_EDIT:
			return FillHistory(HISTORYTYPE_VIEW,Enum,FilterEdit);
		case FSSF_HISTORY_EXTERNAL:
			return FillHistory(HISTORYTYPE_VIEW,Enum,FilterExt);
	}
	return FALSE;
}

int FarSettings::Delete(const FarSettingsValue& Value)
{
	return FALSE;
}

int FarSettings::SubKey(const FarSettingsValue& Value, bool bCreate)
{
	return FALSE;
}

int FarSettings::FillHistory(int Type,FarSettingsEnum& Enum,HistoryFilter Filter)
{
	Vector<FarSettingsHistory>& array=*m_Enum.addItem();
	FarSettingsHistory item;
	DWORD Index=0;
	string strName,strHistoryName,strGuid,strFile,strData;

	unsigned __int64 id;
	int HType;
	bool HLock;
	unsigned __int64 Time;
	while(HistoryCfg->Enum(Index++,Type,strHistoryName,&id,strName,&HType,&HLock,&Time,strGuid,strFile,strData,false))
	{
		if(Filter(HType))
		{
			UI64ToFileTime(Time,&item.Time);
			item.Lock=HLock;
			AddString(array,item,strName);
		}
	}
	Enum.Count=array.GetSize();
	Enum.Histories=array.GetItems();
	return TRUE;
}
