/*
configdb.cpp

�������� �������� � ���� sqlite.
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

#include "configdb.hpp"
#include "strmix.hpp"
#include "pathmix.hpp"
#include "config.hpp"
#include "datetime.hpp"
#include "tinyxml.hpp"
#include "farversion.hpp"
#include "RegExp.hpp"

GeneralConfig *GeneralCfg;
AssociationsConfig *AssocConfig;
PluginsCacheConfig *PlCacheCfg;
PluginsHotkeysConfig *PlHotkeyCfg;
HistoryConfig *HistoryCfg;

int IntToHex(int h)
{
	if (h >= 10)
		return 'A' + h - 10;
	return '0' + h;
}

int HexToInt(int h)
{
	if (h >= 'a' && h <= 'f')
		return h - 'a' + 10;

	if (h >= 'A' && h <= 'F')
		return h - 'A' + 10;

	if (h >= '0' && h <= '9')
		return h - '0';

	return 0;
}

char *BlobToHexString(const char *Blob, int Size)
{
	char *Hex = (char *)xf_malloc(Size*2+Size+1);
	for (int i=0, j=0; i<Size; i++, j+=3)
	{
		Hex[j] = IntToHex((Blob[i]&0xF0) >> 4);
		Hex[j+1] = IntToHex(Blob[i]&0x0F);
		Hex[j+2] = ',';
	}
	Hex[Size ? Size*2+Size-1 : 0] = 0;
	return Hex;

}

char *HexStringToBlob(const char *Hex, int *Size)
{
	*Size=0;
	char *Blob = (char *)xf_malloc(strlen(Hex)/2+1);
	if (!Blob)
		return nullptr;

	while (*Hex && *(Hex+1))
	{
		Blob[(*Size)++] = (HexToInt(*Hex)<<4) | HexToInt(*(Hex+1));
		Hex+=2;
		if (!*Hex)
			break;
		Hex++;
	}

	return Blob;
}

const char *Int64ToHexString(unsigned __int64 X)
{
	static char Bin[16+1];
	for (int i=15; i>=0; i--, X>>=4)
		Bin[i] = IntToHex(X&0xFull);
	return Bin;
}

unsigned __int64 HexStringToInt64(const char *Hex)
{
	unsigned __int64 x = 0;
	while (*Hex)
	{
		x <<= 4;
		x += HexToInt(*Hex);
		Hex++;
	}
	return x;
}

void GetDatabasePath(const wchar_t *FileName, string &strOut, bool Local)
{
	if(StrCmp(FileName, L":memory:"))
	{
		strOut = Local?Opt.LocalProfilePath:Opt.ProfilePath;
		AddEndSlash(strOut);
		strOut += FileName;
	}
	else
	{
		strOut = FileName;
	}
}

bool SQLiteDb::Open(const wchar_t *DbFile, bool Local)
{
	GetDatabasePath(DbFile, strPath, Local);
	return sqlite3_open16(strPath.CPtr(),&pDb) == SQLITE_OK;
}

void SQLiteDb::Initialize(const wchar_t* DbName)
{
	if (!InitializeImpl(DbName))
	{
		Close();
		if (!apiMoveFileEx(strPath, strPath+L".bad", MOVEFILE_REPLACE_EXISTING) || !InitializeImpl(DbName))
		{
			InitializeImpl(L":memory:");
		}
	}
}

class GeneralConfigDb: public GeneralConfig {
	SQLiteStmt stmtUpdateValue;
	SQLiteStmt stmtInsertValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtDelValue;
	SQLiteStmt stmtEnumValues;

public:

	GeneralConfigDb()
	{
		Initialize(L"generalconfig.db");
	}

	bool InitializeImpl(const wchar_t* DbName)
	{
		return 
			Open(DbName) && 

			//schema
			Exec("CREATE TABLE IF NOT EXISTS general_config(key TEXT NOT NULL, name TEXT NOT NULL, value BLOB, PRIMARY KEY (key, name));") &&

			//update value statement
			InitStmt(stmtUpdateValue, L"UPDATE general_config SET value=?1 WHERE key=?2 AND name=?3;") &&

			//insert value statement
			InitStmt(stmtInsertValue, L"INSERT INTO general_config VALUES (?1,?2,?3);") &&

			//get value statement
			InitStmt(stmtGetValue, L"SELECT value FROM general_config WHERE key=?1 AND name=?2;") &&

			//delete value statement
			InitStmt(stmtDelValue, L"DELETE FROM general_config WHERE key=?1 AND name=?2;") &&

			//enum values statement
			InitStmt(stmtEnumValues, L"SELECT name, value FROM general_config WHERE key=?1;")
		;
	}

	virtual ~GeneralConfigDb() { }

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const wchar_t *Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 Value)
	{
		bool b = stmtUpdateValue.Bind(Value).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value).StepAndReset();
		return b;
	}

	bool SetValue(const wchar_t *Key, const wchar_t *Name, const void *Value, size_t Size)
	{
		bool b = stmtUpdateValue.Bind(Value,Size).Bind(Key).Bind(Name).StepAndReset();
		if (!b || Changes() == 0)
			b = stmtInsertValue.Bind(Key).Bind(Name).Bind(Value,Size).StepAndReset();
		return b;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, unsigned __int64 *Value)
	{
		bool b = stmtGetValue.Bind(Key).Bind(Name).Step();
		if (b)
			*Value = stmtGetValue.GetColInt64(0);
		stmtGetValue.Reset();
		return b;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue)
	{
		bool b = stmtGetValue.Bind(Key).Bind(Name).Step();
		if (b)
			strValue = stmtGetValue.GetColText(0);
		stmtGetValue.Reset();
		return b;
	}

	int GetValue(const wchar_t *Key, const wchar_t *Name, void *Value, size_t Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Key).Bind(Name).Step())
		{
			const char *blob = stmtGetValue.GetColBlob(0);
			realsize = stmtGetValue.GetColBytes(0);
			if (Value)
				memcpy(Value,blob,Min(realsize,static_cast<int>(Size)));
		}
		stmtGetValue.Reset();
		return realsize;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, DWORD *Value, DWORD Default)
	{
		unsigned __int64 v;
		if (GetValue(Key,Name,&v))
		{   *Value = (DWORD)v;
			return true;
		}
		*Value = Default;
		return false;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, int *Value, int Default)
	{
		unsigned __int64 v;
		if (GetValue(Key,Name,&v))
		{   *Value = (int)v;
			return true;
		}
		*Value = Default;
		return false;
	}

	int GetValue(const wchar_t *Key, const wchar_t *Name, int Default)
	{
		unsigned __int64 v;
		if (GetValue(Key,Name,&v))
			return (int)v;
		return Default;
	}

	bool GetValue(const wchar_t *Key, const wchar_t *Name, string &strValue, const wchar_t *Default)
	{
		if (GetValue(Key,Name,strValue))
			return true;
		strValue=Default;
		return false;
	}

	int GetValue(const wchar_t *Key, const wchar_t *Name, void *Value, size_t Size, const void *Default)
	{
		int s = GetValue(Key,Name,Value,Size);
		if (s)
			return s;
		if (Default)
		{
			memcpy(Value,Default,Size);
			return static_cast<int>(Size);
		}
		return 0;
	}

	bool DeleteValue(const wchar_t *Key, const wchar_t *Name)
	{
		return stmtDelValue.Bind(Key).Bind(Name).StepAndReset();
	}

	bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, string &strValue)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Key,false);

		if (stmtEnumValues.Step())
		{
			strName = stmtEnumValues.GetColText(0);
			strValue = stmtEnumValues.GetColText(1);
			return true;
		}

		stmtEnumValues.Reset();
		return false;
	}

	bool EnumValues(const wchar_t *Key, DWORD Index, string &strName, DWORD *Value)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Key,false);

		if (stmtEnumValues.Step())
		{
			strName = stmtEnumValues.GetColText(0);
			*Value = (DWORD)stmtEnumValues.GetColInt(1);
			return true;
		}

		stmtEnumValues.Reset();
		return false;
	}

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("generalconfig");
		if (!root)
			return nullptr;

		SQLiteStmt stmtEnumAllValues;
		InitStmt(stmtEnumAllValues, L"SELECT key, name, value FROM general_config ORDER BY key, name;");

		while (stmtEnumAllValues.Step())
		{
			TiXmlElement *e = new TiXmlElement("setting");
			if (!e)
				break;

			e->SetAttribute("key", stmtEnumAllValues.GetColTextUTF8(0));
			e->SetAttribute("name", stmtEnumAllValues.GetColTextUTF8(1));

			switch (stmtEnumAllValues.GetColType(2))
			{
				case SQLITE_INTEGER:
					e->SetAttribute("type", "qword");
					e->SetAttribute("value", Int64ToHexString(stmtEnumAllValues.GetColInt64(2)));
					break;
				case SQLITE_TEXT:
					e->SetAttribute("type", "text");
					e->SetAttribute("value", stmtEnumAllValues.GetColTextUTF8(2));
					break;
				default:
				{
					char *hex = BlobToHexString(stmtEnumAllValues.GetColBlob(2),stmtEnumAllValues.GetColBytes(2));
					e->SetAttribute("type", "hex");
					e->SetAttribute("value", hex);
					xf_free(hex);
				}
			}

			root->LinkEndChild(e);
		}

		stmtEnumAllValues.Reset();

		return root;
	}

	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		for (const TiXmlElement *e = root.FirstChild("generalconfig").FirstChildElement("setting").Element(); e; e=e->NextSiblingElement("setting"))
		{
			const char *key = e->Attribute("key");
			const char *name = e->Attribute("name");
			const char *type = e->Attribute("type");
			const char *value = e->Attribute("value");

			if (!key || !name || !type || !value)
				continue;

			string Key(key, CP_UTF8);
			string Name(name, CP_UTF8);

			if (!strcmp(type,"qword"))
			{
				SetValue(Key, Name, HexStringToInt64(value));
			}
			else if (!strcmp(type,"text"))
			{
				string Value(value, CP_UTF8);
				SetValue(Key, Name, Value);
			}
			else if (!strcmp(type,"hex"))
			{
				int Size = 0;
				char *Blob = HexStringToBlob(value, &Size);
				if (Blob)
				{
					SetValue(Key, Name, Blob, Size);
					xf_free(Blob);
				}
			}
			else
			{
				continue;
			}
		}
		EndTransaction();

		return true;
	}
};

class HierarchicalConfigDb: public HierarchicalConfig {
	SQLiteStmt stmtCreateKey;
	SQLiteStmt stmtFindKey;
	SQLiteStmt stmtSetKeyDescription;
	SQLiteStmt stmtSetValue;
	SQLiteStmt stmtGetValue;
	SQLiteStmt stmtEnumKeys;
	SQLiteStmt stmtEnumValues;
	SQLiteStmt stmtDelValue;
	SQLiteStmt stmtDeleteTree;

	HierarchicalConfigDb() {}

public:

	explicit HierarchicalConfigDb(const wchar_t *DbName)
	{
		Initialize(DbName);
	}

	bool InitializeImpl(const wchar_t* DbName)
	{
		Close();
		return
			Open(DbName) &&

			//schema
			EnableForeignKeysConstraints() &&

			Exec(
				"CREATE TABLE IF NOT EXISTS table_keys(id INTEGER PRIMARY KEY, parent_id INTEGER NOT NULL, name TEXT NOT NULL, description TEXT, FOREIGN KEY(parent_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, UNIQUE (parent_id,name));"
				"CREATE TABLE IF NOT EXISTS table_values(key_id INTEGER NOT NULL, name TEXT NOT NULL, value BLOB, FOREIGN KEY(key_id) REFERENCES table_keys(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (key_id, name), CHECK (key_id <> 0));"
			) &&

			//root key (needs to be before the transaction start)
			Exec("INSERT OR IGNORE INTO table_keys VALUES (0,0,\"\",\"Root - do not edit\");") &&

			BeginTransaction() &&

			//create key statement
			InitStmt(stmtCreateKey, L"INSERT INTO table_keys VALUES (NULL,?1,?2,?3);") &&

			//find key statement
			InitStmt(stmtFindKey, L"SELECT id FROM table_keys WHERE parent_id=?1 AND name=?2 AND id<>0;") &&

			//set key description statement
			InitStmt(stmtSetKeyDescription, L"UPDATE table_keys SET description=?1 WHERE id=?2 AND id<>0 AND description<>?1;") &&

			//set value statement
			InitStmt(stmtSetValue, L"INSERT OR REPLACE INTO table_values VALUES (?1,?2,?3);") &&

			//get value statement
			InitStmt(stmtGetValue, L"SELECT value FROM table_values WHERE key_id=?1 AND name=?2;") &&

			//enum keys statement
			InitStmt(stmtEnumKeys, L"SELECT name FROM table_keys WHERE parent_id=?1 AND id<>0;") &&

			//enum values statement
			InitStmt(stmtEnumValues, L"SELECT name, value FROM table_values WHERE key_id=?1;") &&

			//delete value statement
			InitStmt(stmtDelValue, L"DELETE FROM table_values WHERE key_id=?1 AND name=?2;") &&

			//delete tree statement
			InitStmt(stmtDeleteTree, L"DELETE FROM table_keys WHERE id=?1 AND id<>0;")
		;
	}

	virtual ~HierarchicalConfigDb() { EndTransaction(); }

	bool Flush()
	{
		bool b = EndTransaction();
		BeginTransaction();
		return b;
	}

	unsigned __int64 CreateKey(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Description=nullptr)
	{
		if (stmtCreateKey.Bind(Root).Bind(Name).Bind(Description).StepAndReset())
			return LastInsertRowID();
		unsigned __int64 id = GetKeyID(Root,Name);
		if (id && Description)
			SetKeyDescription(id,Description);
		return id;
	}

	unsigned __int64 GetKeyID(unsigned __int64 Root, const wchar_t *Name)
	{
		unsigned __int64 id = 0;
		if (stmtFindKey.Bind(Root).Bind(Name).Step())
			id = stmtFindKey.GetColInt64(0);
		stmtFindKey.Reset();
		return id;
	}

	bool SetKeyDescription(unsigned __int64 Root, const wchar_t *Description)
	{
		return stmtSetKeyDescription.Bind(Description).Bind(Root).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, const wchar_t *Value)
	{
		if (!Name)
			return SetKeyDescription(Root,Value);
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 Value)
	{
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value).StepAndReset();
	}

	bool SetValue(unsigned __int64 Root, const wchar_t *Name, const void *Value, size_t Size)
	{
		return stmtSetValue.Bind(Root).Bind(Name).Bind(Value,Size).StepAndReset();
	}

	bool GetValue(unsigned __int64 Root, const wchar_t *Name, unsigned __int64 *Value)
	{
		bool b = stmtGetValue.Bind(Root).Bind(Name).Step();
		if (b)
			*Value = stmtGetValue.GetColInt64(0);
		stmtGetValue.Reset();
		return b;
	}

	bool GetValue(unsigned __int64 Root, const wchar_t *Name, string &strValue)
	{
		bool b = stmtGetValue.Bind(Root).Bind(Name).Step();
		if (b)
			strValue = stmtGetValue.GetColText(0);
		stmtGetValue.Reset();
		return b;
	}

	int GetValue(unsigned __int64 Root, const wchar_t *Name, void *Value, size_t Size)
	{
		int realsize = 0;
		if (stmtGetValue.Bind(Root).Bind(Name).Step())
		{
			const char *blob = stmtGetValue.GetColBlob(0);
			realsize = stmtGetValue.GetColBytes(0);
			if (Value)
				memcpy(Value,blob,Min(realsize,static_cast<int>(Size)));
		}
		stmtGetValue.Reset();
		return realsize;
	}

	bool DeleteKeyTree(unsigned __int64 KeyID)
	{
		//All subtree is automatically deleted because of foreign key constraints
		return stmtDeleteTree.Bind(KeyID).StepAndReset();
	}

	bool DeleteValue(unsigned __int64 Root, const wchar_t *Name)
	{
		return stmtDelValue.Bind(Root).Bind(Name).StepAndReset();
	}

	bool EnumKeys(unsigned __int64 Root, DWORD Index, string &strName)
	{
		if (Index == 0)
			stmtEnumKeys.Reset().Bind(Root);

		if (stmtEnumKeys.Step())
		{
			strName = stmtEnumKeys.GetColText(0);
			return true;
		}

		stmtEnumKeys.Reset();
		return false;
	}

	bool EnumValues(unsigned __int64 Root, DWORD Index, string &strName, DWORD *Type)
	{
		if (Index == 0)
			stmtEnumValues.Reset().Bind(Root);

		if (stmtEnumValues.Step())
		{
			strName = stmtEnumValues.GetColText(0);
			switch (stmtEnumValues.GetColType(1))
			{
				case SQLITE_INTEGER: *Type = TYPE_INTEGER; break;
				case SQLITE_TEXT: *Type = TYPE_TEXT; break;
				case SQLITE_BLOB: *Type = TYPE_BLOB; break;
				default: *Type = TYPE_UNKNOWN;
			}

			return true;
		}

		stmtEnumValues.Reset();
		return false;

	}

	void Export(unsigned __int64 id, TiXmlElement *key)
	{
		stmtEnumValues.Bind(id);
		while (stmtEnumValues.Step())
		{
			TiXmlElement *e = new TiXmlElement("value");
			if (!e)
				break;

			e->SetAttribute("name", stmtEnumValues.GetColTextUTF8(0));

			switch (stmtEnumValues.GetColType(1))
			{
				case SQLITE_INTEGER:
					e->SetAttribute("type", "qword");
					e->SetAttribute("value", Int64ToHexString(stmtEnumValues.GetColInt64(1)));
					break;
				case SQLITE_TEXT:
					e->SetAttribute("type", "text");
					e->SetAttribute("value", stmtEnumValues.GetColTextUTF8(1));
					break;
				default:
				{
					char *hex = BlobToHexString(stmtEnumValues.GetColBlob(1),stmtEnumValues.GetColBytes(1));
					e->SetAttribute("type", "hex");
					e->SetAttribute("value", hex);
					xf_free(hex);
				}
			}

			key->LinkEndChild(e);
		}
		stmtEnumValues.Reset();

		SQLiteStmt stmtEnumSubKeys;
		InitStmt(stmtEnumSubKeys, L"SELECT id, name, description FROM table_keys WHERE parent_id=?1 AND id<>0;");

		stmtEnumSubKeys.Bind(id);
		while (stmtEnumSubKeys.Step())
		{
			TiXmlElement *e = new TiXmlElement("key");
			if (!e)
				break;

			e->SetAttribute("name", stmtEnumSubKeys.GetColTextUTF8(1));
			const char *description = stmtEnumSubKeys.GetColTextUTF8(2);
			if (description)
				e->SetAttribute("description", description);

			Export(stmtEnumSubKeys.GetColInt64(0), e);

			key->LinkEndChild(e);
		}
		stmtEnumSubKeys.Reset();
	}

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("hierarchicalconfig");
		if (!root)
			return nullptr;

		Export(0, root);

		return root;
	}

	void Import(unsigned __int64 root, const TiXmlElement *key)
	{
		unsigned __int64 id;
		{
			const char *name = key->Attribute("name");
			const char *description = key->Attribute("description");
			if (!name)
				return;

			string Name(name, CP_UTF8);
			string Description(description, CP_UTF8);
			id = CreateKey(root, Name, description ? Description.CPtr() : nullptr);
			if (!id)
				return;
		}

		for (const TiXmlElement *e = key->FirstChildElement("value"); e; e=e->NextSiblingElement("value"))
		{
			const char *name = e->Attribute("name");
			const char *type = e->Attribute("type");
			const char *value = e->Attribute("value");

			if (!name || !type || !value)
				continue;

			string Name(name, CP_UTF8);

			if (!strcmp(type,"qword"))
			{
				SetValue(id, Name, HexStringToInt64(value));
			}
			else if (!strcmp(type,"text"))
			{
				string Value(value, CP_UTF8);
				SetValue(id, Name, Value);
			}
			else if (!strcmp(type,"hex"))
			{
				int Size = 0;
				char *Blob = HexStringToBlob(value, &Size);
				if (Blob)
				{
					SetValue(id, Name, Blob, Size);
					xf_free(Blob);
				}
			}
			else
			{
				continue;
			}
		}

		for (const TiXmlElement *e = key->FirstChildElement("key"); e; e=e->NextSiblingElement("key"))
		{
			Import(id, e);
		}

	}

	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		for (const TiXmlElement *e = root.FirstChild("hierarchicalconfig").FirstChildElement("key").Element(); e; e=e->NextSiblingElement("key"))
		{
			Import(0, e);
		}
		EndTransaction();
		return true;
	}
};

class AssociationsConfigDb: public AssociationsConfig {
	SQLiteStmt stmtReorder;
	SQLiteStmt stmtAddType;
	SQLiteStmt stmtGetMask;
	SQLiteStmt stmtGetDescription;
	SQLiteStmt stmtUpdateType;
	SQLiteStmt stmtSetCommand;
	SQLiteStmt stmtGetCommand;
	SQLiteStmt stmtEnumTypes;
	SQLiteStmt stmtEnumMasks;
	SQLiteStmt stmtEnumMasksForType;
	SQLiteStmt stmtDelType;
	SQLiteStmt stmtGetWeight;
	SQLiteStmt stmtSetWeight;

public:

	AssociationsConfigDb()
	{
		Initialize(L"associations.db");
	}

	virtual ~AssociationsConfigDb() { }

	bool InitializeImpl(const wchar_t* DbName)
	{
		Close();
		return
			Open(DbName) &&

			//schema
			EnableForeignKeysConstraints() &&
			Exec(
			"CREATE TABLE IF NOT EXISTS filetypes(id INTEGER PRIMARY KEY, weight INTEGER NOT NULL, mask TEXT, description TEXT);"
			"CREATE TABLE IF NOT EXISTS commands(ft_id INTEGER NOT NULL, type INTEGER NOT NULL, enabled INTEGER NOT NULL, command TEXT, FOREIGN KEY(ft_id) REFERENCES filetypes(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (ft_id, type));"
			) &&

			//add new type and reorder statements
			InitStmt(stmtReorder, L"UPDATE filetypes SET weight=weight+1 WHERE weight>(CASE ?1 WHEN 0 THEN 0 ELSE (SELECT weight FROM filetypes WHERE id=?1) END);") &&
			InitStmt(stmtAddType, L"INSERT INTO filetypes VALUES (NULL,(CASE ?1 WHEN 0 THEN 1 ELSE (SELECT weight FROM filetypes WHERE id=?1)+1 END),?2,?3);") &&

			//get mask statement
			InitStmt(stmtGetMask, L"SELECT mask FROM filetypes WHERE id=?1;") &&

			//get description statement
			InitStmt(stmtGetDescription, L"SELECT description FROM filetypes WHERE id=?1;") &&

			//update type statement
			InitStmt(stmtUpdateType, L"UPDATE filetypes SET mask=?1, description=?2 WHERE id=?3;") &&

			//set association statement
			InitStmt(stmtSetCommand, L"INSERT OR REPLACE INTO commands VALUES (?1,?2,?3,?4);") &&

			//get association statement
			InitStmt(stmtGetCommand, L"SELECT command, enabled FROM commands WHERE ft_id=?1 AND type=?2;") &&

			//enum types statement
			InitStmt(stmtEnumTypes, L"SELECT id, description FROM filetypes ORDER BY weight;") &&

			//enum masks statement
			InitStmt(stmtEnumMasks, L"SELECT id, mask FROM filetypes ORDER BY weight;") &&

			//enum masks with a specific type on statement
			InitStmt(stmtEnumMasksForType, L"SELECT id, mask FROM filetypes, commands WHERE id=ft_id AND type=?1 AND enabled<>0 ORDER BY weight;") &&

			//delete type statement
			InitStmt(stmtDelType, L"DELETE FROM filetypes WHERE id=?1;") &&

			//get weight and set weight statements
			InitStmt(stmtGetWeight, L"SELECT weight FROM filetypes WHERE id=?1;") &&
			InitStmt(stmtSetWeight, L"UPDATE filetypes SET weight=?1 WHERE id=?2;")
		;
	}


	bool EnumMasks(DWORD Index, unsigned __int64 *id, string &strMask)
	{
		if (Index == 0)
			stmtEnumMasks.Reset();

		if (stmtEnumMasks.Step())
		{
			*id = stmtEnumMasks.GetColInt64(0);
			strMask = stmtEnumMasks.GetColText(1);
			return true;
		}

		stmtEnumMasks.Reset();
		return false;
	}

	bool EnumMasksForType(int Type, DWORD Index, unsigned __int64 *id, string &strMask)
	{
		if (Index == 0)
			stmtEnumMasksForType.Reset().Bind(Type);

		if (stmtEnumMasksForType.Step())
		{
			*id = stmtEnumMasksForType.GetColInt64(0);
			strMask = stmtEnumMasksForType.GetColText(1);
			return true;
		}

		stmtEnumMasksForType.Reset();
		return false;
	}

	bool GetMask(unsigned __int64 id, string &strMask)
	{
		bool b = stmtGetMask.Bind(id).Step();
		if (b)
			strMask = stmtGetMask.GetColText(0);
		stmtGetMask.Reset();
		return b;
	}

	bool GetDescription(unsigned __int64 id, string &strDescription)
	{
		bool b = stmtGetDescription.Bind(id).Step();
		if (b)
			strDescription = stmtGetDescription.GetColText(0);
		stmtGetDescription.Reset();
		return b;
	}

	bool GetCommand(unsigned __int64 id, int Type, string &strCommand, bool *Enabled=nullptr)
	{
		bool b = stmtGetCommand.Bind(id).Bind(Type).Step();
		if (b)
		{
			strCommand = stmtGetCommand.GetColText(0);
			if (Enabled)
				*Enabled = stmtGetCommand.GetColInt(1) ? true : false;
		}
		stmtGetCommand.Reset();
		return b;
	}

	bool SetCommand(unsigned __int64 id, int Type, const wchar_t *Command, bool Enabled)
	{
		return stmtSetCommand.Bind(id).Bind(Type).Bind(Enabled?1:0).Bind(Command).StepAndReset();
	}

	bool SwapPositions(unsigned __int64 id1, unsigned __int64 id2)
	{
		if (stmtGetWeight.Bind(id1).Step())
		{
			unsigned __int64 weight1 = stmtGetWeight.GetColInt64(0);
			stmtGetWeight.Reset();
			if (stmtGetWeight.Bind(id2).Step())
			{
				unsigned __int64 weight2 = stmtGetWeight.GetColInt64(0);
				stmtGetWeight.Reset();
				return stmtSetWeight.Bind(weight1).Bind(id2).StepAndReset() && stmtSetWeight.Bind(weight2).Bind(id1).StepAndReset();
			}
		}
		stmtGetWeight.Reset();
		return false;
	}

	unsigned __int64 AddType(unsigned __int64 after_id, const wchar_t *Mask, const wchar_t *Description)
	{
		if (stmtReorder.Bind(after_id).StepAndReset() && stmtAddType.Bind(after_id).Bind(Mask).Bind(Description).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	bool UpdateType(unsigned __int64 id, const wchar_t *Mask, const wchar_t *Description)
	{
		return stmtUpdateType.Bind(Mask).Bind(Description).Bind(id).StepAndReset();
	}

	bool DelType(unsigned __int64 id)
	{
		return stmtDelType.Bind(id).StepAndReset();
	}

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("associations");
		if (!root)
			return nullptr;

		SQLiteStmt stmtEnumAllTypes;
		InitStmt(stmtEnumAllTypes, L"SELECT id, mask, description FROM filetypes ORDER BY weight;");
		SQLiteStmt stmtEnumCommandsPerFiletype;
		InitStmt(stmtEnumCommandsPerFiletype, L"SELECT type, enabled, command FROM commands WHERE ft_id=?1 ORDER BY type;");

		while (stmtEnumAllTypes.Step())
		{
			TiXmlElement *e = new TiXmlElement("filetype");
			if (!e)
				break;

			e->SetAttribute("mask", stmtEnumAllTypes.GetColTextUTF8(1));
			e->SetAttribute("description", stmtEnumAllTypes.GetColTextUTF8(2));

			stmtEnumCommandsPerFiletype.Bind(stmtEnumAllTypes.GetColInt64(0));
			while (stmtEnumCommandsPerFiletype.Step())
			{
				TiXmlElement *se = new TiXmlElement("command");
				if (!se)
					break;

				se->SetAttribute("type", stmtEnumCommandsPerFiletype.GetColInt(0));
				se->SetAttribute("enabled", stmtEnumCommandsPerFiletype.GetColInt(1));
				se->SetAttribute("command", stmtEnumCommandsPerFiletype.GetColTextUTF8(2));
				e->LinkEndChild(se);
			}
			stmtEnumCommandsPerFiletype.Reset();

			root->LinkEndChild(e);
		}

		stmtEnumAllTypes.Reset();

		return root;
	}

	bool Import(const TiXmlHandle &root)
	{
		const TiXmlHandle base = root.FirstChild("associations");
		if (!base.ToElement())
			return false;

		BeginTransaction();
		Exec("DELETE FROM filetypes;"); //delete all before importing
		unsigned __int64 id = 0;
		for (const TiXmlElement *e = base.FirstChildElement("filetype").Element(); e; e=e->NextSiblingElement("filetype"))
		{
			const char *mask = e->Attribute("mask");
			const char *description = e->Attribute("description");

			if (!mask)
				continue;

			string Mask(mask, CP_UTF8);
			string Description(description, CP_UTF8);

			id = AddType(id, Mask, Description);
			if (!id)
				continue;

			for (const TiXmlElement *se = e->FirstChildElement("command"); se; se=se->NextSiblingElement("command"))
			{
				const char *command = se->Attribute("command");
				int type=0;
				const char *stype = se->Attribute("type", &type);
				int enabled=0;
				const char *senabled = se->Attribute("enabled", &enabled);

				if (!command || !stype || !senabled)
					continue;

				string Command(command, CP_UTF8);
				SetCommand(id, type, Command, enabled ? true : false);
			}

		}
		EndTransaction();

		return true;
	}
};

class PluginsCacheConfigDb: public PluginsCacheConfig {
	SQLiteStmt stmtCreateCache;
	SQLiteStmt stmtFindCacheName;
	SQLiteStmt stmtDelCache;
	SQLiteStmt stmtCountCacheNames;
	SQLiteStmt stmtGetPreloadState;
	SQLiteStmt stmtGetSignature;
	SQLiteStmt stmtGetExportState;
	SQLiteStmt stmtGetGuid;
	SQLiteStmt stmtGetTitle;
	SQLiteStmt stmtGetAuthor;
	SQLiteStmt stmtGetPrefix;
	SQLiteStmt stmtGetDescription;
	SQLiteStmt stmtGetFlags;
	SQLiteStmt stmtGetMinFarVersion;
	SQLiteStmt stmtGetVersion;
	SQLiteStmt stmtSetPreloadState;
	SQLiteStmt stmtSetSignature;
	SQLiteStmt stmtSetExportState;
	SQLiteStmt stmtSetGuid;
	SQLiteStmt stmtSetTitle;
	SQLiteStmt stmtSetAuthor;
	SQLiteStmt stmtSetPrefix;
	SQLiteStmt stmtSetDescription;
	SQLiteStmt stmtSetFlags;
	SQLiteStmt stmtSetMinFarVersion;
	SQLiteStmt stmtSetVersion;
	SQLiteStmt stmtEnumCache;
	SQLiteStmt stmtGetMenuItem;
	SQLiteStmt stmtSetMenuItem;

	enum MenuItemTypeEnum {
		PLUGINS_MENU,
		CONFIG_MENU,
		DRIVE_MENU
	};

	bool GetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, int index, string &Text, string &Guid)
	{
		bool b = stmtGetMenuItem.Bind(id).Bind((int)type).Bind(index).Step();
		if (b)
		{
			Text = stmtGetMenuItem.GetColText(0);
			Guid = stmtGetMenuItem.GetColText(1);
		}
		stmtGetMenuItem.Reset();
		return b;
	}

	bool SetMenuItem(unsigned __int64 id, MenuItemTypeEnum type, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return stmtSetMenuItem.Bind(id).Bind((int)type).Bind(index).Bind(Guid).Bind(Text).StepAndReset();
	}

	string GetTextFromID(SQLiteStmt &stmt, unsigned __int64 id)
	{
		string strText;
		if (stmt.Bind(id).Step())
			strText = stmt.GetColText(0);
		stmt.Reset();
		return strText;
	}

public:

	PluginsCacheConfigDb()
	{
		Initialize(L"plugincache.db");
	}

	bool InitializeImpl(const wchar_t* DbName)
	{
		Close();
		return
			Open(DbName, true) &&

			//schema
			SetWALJournalingMode() &&

			EnableForeignKeysConstraints() &&

			Exec(
				"CREATE TABLE IF NOT EXISTS cachename(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE);"
				"CREATE TABLE IF NOT EXISTS preload(cid INTEGER NOT NULL PRIMARY KEY, enabled INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS signatures(cid INTEGER NOT NULL PRIMARY KEY, signature TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS guids(cid INTEGER NOT NULL PRIMARY KEY, guid TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS titles(cid INTEGER NOT NULL PRIMARY KEY, title TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS authors(cid INTEGER NOT NULL PRIMARY KEY, author TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS descriptions(cid INTEGER NOT NULL PRIMARY KEY, description TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS minfarversions(cid INTEGER NOT NULL PRIMARY KEY, version BLOB NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS pluginversions(cid INTEGER NOT NULL PRIMARY KEY, version BLOB NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS flags(cid INTEGER NOT NULL PRIMARY KEY, bitmask INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS prefixes(cid INTEGER NOT NULL PRIMARY KEY, prefix TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE);"
				"CREATE TABLE IF NOT EXISTS exports(cid INTEGER NOT NULL, export TEXT NOT NULL, enabled INTEGER NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, export));"
				"CREATE TABLE IF NOT EXISTS menuitems(cid INTEGER NOT NULL, type INTEGER NOT NULL, number INTEGER NOT NULL, guid TEXT NOT NULL, name TEXT NOT NULL, FOREIGN KEY(cid) REFERENCES cachename(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (cid, type, number));"
			) &&

			//get menu item text and guid statement
			InitStmt(stmtGetMenuItem, L"SELECT name, guid FROM menuitems WHERE cid=?1 AND type=?2 AND number=?3;") &&

			//set menu item statement
			InitStmt(stmtSetMenuItem, L"INSERT OR REPLACE INTO menuitems VALUES (?1,?2,?3,?4,?5);") &&

			//add new cache name statement
			InitStmt(stmtCreateCache, L"INSERT INTO cachename VALUES (NULL,?1);") &&

			//get cache id by name statement
			InitStmt(stmtFindCacheName, L"SELECT id FROM cachename WHERE name=?1;") &&

			//del cache by name statement
			InitStmt(stmtDelCache, L"DELETE FROM cachename WHERE name=?1;") &&

			//count cache names statement
			InitStmt(stmtCountCacheNames, L"SELECT count(name) FROM cachename") &&

			//get preload state statement
			InitStmt(stmtGetPreloadState, L"SELECT enabled FROM preload WHERE cid=?1;") &&

			//get signature statement
			InitStmt(stmtGetSignature, L"SELECT signature FROM signatures WHERE cid=?1;") &&

			//get export state statement
			InitStmt(stmtGetExportState, L"SELECT enabled FROM exports WHERE cid=?1 and export=?2;") &&

			//get guid statement
			InitStmt(stmtGetGuid, L"SELECT guid FROM guids WHERE cid=?1;") &&

			//get title statement
			InitStmt(stmtGetTitle, L"SELECT title FROM titles WHERE cid=?1;") &&

			//get author statement
			InitStmt(stmtGetAuthor, L"SELECT author FROM authors WHERE cid=?1;") &&

			//get description statement
			InitStmt(stmtGetDescription, L"SELECT description FROM descriptions WHERE cid=?1;") &&

			//get command prefix statement
			InitStmt(stmtGetPrefix, L"SELECT prefix FROM prefixes WHERE cid=?1;") &&

			//get flags statement
			InitStmt(stmtGetFlags, L"SELECT bitmask FROM flags WHERE cid=?1;") &&

			//get MinFarVersion statement
			InitStmt(stmtGetMinFarVersion, L"SELECT version FROM minfarversions WHERE cid=?1;") &&

			//get plugin version statement
			InitStmt(stmtGetVersion, L"SELECT version FROM pluginversions WHERE cid=?1;") &&

			//set preload state statement
			InitStmt(stmtSetPreloadState, L"INSERT OR REPLACE INTO preload VALUES (?1,?2);") &&

			//set signature statement
			InitStmt(stmtSetSignature, L"INSERT OR REPLACE INTO signatures VALUES (?1,?2);") &&

			//set export state statement
			InitStmt(stmtSetExportState, L"INSERT OR REPLACE INTO exports VALUES (?1,?2,?3);") &&

			//set guid statement
			InitStmt(stmtSetGuid, L"INSERT OR REPLACE INTO guids VALUES (?1,?2);") &&

			//set title statement
			InitStmt(stmtSetTitle, L"INSERT OR REPLACE INTO titles VALUES (?1,?2);") &&

			//set author statement
			InitStmt(stmtSetAuthor, L"INSERT OR REPLACE INTO authors VALUES (?1,?2);") &&

			//set description statement
			InitStmt(stmtSetDescription, L"INSERT OR REPLACE INTO descriptions VALUES (?1,?2);") &&

			//set command prefix statement
			InitStmt(stmtSetPrefix, L"INSERT OR REPLACE INTO prefixes VALUES (?1,?2);") &&

			//set flags statement
			InitStmt(stmtSetFlags, L"INSERT OR REPLACE INTO flags VALUES (?1,?2);") &&

			//set MinFarVersion statement
			InitStmt(stmtSetMinFarVersion, L"INSERT OR REPLACE INTO minfarversions VALUES (?1,?2);") &&

			//set plugin version statement
			InitStmt(stmtSetVersion, L"INSERT OR REPLACE INTO pluginversions VALUES (?1,?2);") &&

			//enum cache names statement
			InitStmt(stmtEnumCache, L"SELECT name FROM cachename ORDER BY name;")
		;
	}

	virtual ~PluginsCacheConfigDb() {}

	unsigned __int64 CreateCache(const wchar_t *CacheName)
	{
		if (stmtCreateCache.Bind(CacheName).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetCacheID(const wchar_t *CacheName)
	{
		unsigned __int64 id = 0;
		if (stmtFindCacheName.Bind(CacheName).Step())
			id = stmtFindCacheName.GetColInt64(0);
		stmtFindCacheName.Reset();
		return id;
	}

	bool DeleteCache(const wchar_t *CacheName)
	{
		//All related entries are automatically deleted because of foreign key constraints
		return stmtDelCache.Bind(CacheName).StepAndReset();
	}

	bool IsPreload(unsigned __int64 id)
	{
		bool preload = false;
		if (stmtGetPreloadState.Bind(id).Step())
			preload = stmtGetPreloadState.GetColInt(0) ? true : false;
		stmtGetPreloadState.Reset();
		return preload;
	}

	string GetSignature(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetSignature, id);
	}

	void *GetExport(unsigned __int64 id, const wchar_t *ExportName)
	{
		void *enabled = nullptr;
		if (stmtGetExportState.Bind(id).Bind(ExportName).Step())
			if (stmtGetExportState.GetColInt(0) > 0)
				enabled = ToPtr(1);
		stmtGetExportState.Reset();
		return enabled;
	}

	string GetGuid(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetGuid, id);
	}

	string GetTitle(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetTitle, id);
	}

	string GetAuthor(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetAuthor, id);
	}

	string GetDescription(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetDescription, id);
	}

	bool GetMinFarVersion(unsigned __int64 id, VersionInfo *Version)
	{
		bool b = stmtGetMinFarVersion.Bind(id).Step();
		if (b)
		{
			const char *blob = stmtGetMinFarVersion.GetColBlob(0);
			int realsize = stmtGetMinFarVersion.GetColBytes(0);
			memcpy(Version,blob,Min(realsize,(int)sizeof(VersionInfo)));
		}
		stmtGetMinFarVersion.Reset();
		return b;
	}

	bool GetVersion(unsigned __int64 id, VersionInfo *Version)
	{
		bool b = stmtGetVersion.Bind(id).Step();
		if (b)
		{
			const char *blob = stmtGetVersion.GetColBlob(0);
			int realsize = stmtGetVersion.GetColBytes(0);
			memcpy(Version,blob,Min(realsize,(int)sizeof(VersionInfo)));
		}
		stmtGetVersion.Reset();
		return b;
	}

	bool GetDiskMenuItem(unsigned __int64 id, int index, string &Text, string &Guid)
	{
		return GetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool GetPluginsMenuItem(unsigned __int64 id, int index, string &Text, string &Guid)
	{
		return GetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool GetPluginsConfigMenuItem(unsigned __int64 id, int index, string &Text, string &Guid)
	{
		return GetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	string GetCommandPrefix(unsigned __int64 id)
	{
		return GetTextFromID(stmtGetPrefix, id);
	}

	unsigned __int64 GetFlags(unsigned __int64 id)
	{
		unsigned __int64 flags = 0;
		if (stmtGetFlags.Bind(id).Step())
			flags = stmtGetFlags.GetColInt64(0);
		stmtGetFlags.Reset();
		return flags;
	}

	bool SetPreload(unsigned __int64 id, bool Preload)
	{
		return stmtSetPreloadState.Bind(id).Bind(Preload?1:0).StepAndReset();
	}

	bool SetSignature(unsigned __int64 id, const wchar_t *Signature)
	{
		return stmtSetSignature.Bind(id).Bind(Signature).StepAndReset();
	}

	bool SetDiskMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, DRIVE_MENU, index, Text, Guid);
	}

	bool SetPluginsMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, PLUGINS_MENU, index, Text, Guid);
	}

	bool SetPluginsConfigMenuItem(unsigned __int64 id, int index, const wchar_t *Text, const wchar_t *Guid)
	{
		return SetMenuItem(id, CONFIG_MENU, index, Text, Guid);
	}

	bool SetCommandPrefix(unsigned __int64 id, const wchar_t *Prefix)
	{
		return stmtSetPrefix.Bind(id).Bind(Prefix).StepAndReset();
	}

	bool SetFlags(unsigned __int64 id, unsigned __int64 Flags)
	{
		return stmtSetFlags.Bind(id).Bind(Flags).StepAndReset();
	}

	bool SetExport(unsigned __int64 id, const wchar_t *ExportName, bool Exists)
	{
		return stmtSetExportState.Bind(id).Bind(ExportName).Bind(Exists?1:0).StepAndReset();
	}

	bool SetMinFarVersion(unsigned __int64 id, const VersionInfo *Version)
	{
		return stmtSetMinFarVersion.Bind(id).Bind(Version, sizeof(VersionInfo)).StepAndReset();
	}

	bool SetVersion(unsigned __int64 id, const VersionInfo *Version)
	{
		return stmtSetVersion.Bind(id).Bind(Version,sizeof(VersionInfo)).StepAndReset();
	}

	bool SetGuid(unsigned __int64 id, const wchar_t *Guid)
	{
		return stmtSetGuid.Bind(id).Bind(Guid).StepAndReset();
	}

	bool SetTitle(unsigned __int64 id, const wchar_t *Title)
	{
		return stmtSetTitle.Bind(id).Bind(Title).StepAndReset();
	}

	bool SetAuthor(unsigned __int64 id, const wchar_t *Author)
	{
		return stmtSetAuthor.Bind(id).Bind(Author).StepAndReset();
	}

	bool SetDescription(unsigned __int64 id, const wchar_t *Description)
	{
		return stmtSetDescription.Bind(id).Bind(Description).StepAndReset();
	}

	bool EnumPlugins(DWORD index, string &CacheName)
	{
		if (index == 0)
			stmtEnumCache.Reset();

		if (stmtEnumCache.Step())
		{
			CacheName = stmtEnumCache.GetColText(0);
			return true;
		}

		stmtEnumCache.Reset();
		return false;
	}

	bool DiscardCache()
	{
		BeginTransaction();
		bool ret = Exec("DELETE FROM cachename");
		EndTransaction();
		return ret;
	}

	bool IsCacheEmpty()
	{
		int count = 0;
		if (stmtCountCacheNames.Step())
			count = stmtCountCacheNames.GetColInt(0);
		stmtCountCacheNames.Reset();
		return count==0;
	}
};

class PluginsHotkeysConfigDb: public PluginsHotkeysConfig {
	SQLiteStmt stmtGetHotkey;
	SQLiteStmt stmtSetHotkey;
	SQLiteStmt stmtDelHotkey;
	SQLiteStmt stmtCheckForHotkeys;

public:

	PluginsHotkeysConfigDb()
	{
		Initialize(L"pluginhotkeys.db");
	}

	bool InitializeImpl(const wchar_t* DbName)
	{
		Close();
		return
			Open(DbName) &&

			//schema
			Exec("CREATE TABLE IF NOT EXISTS pluginhotkeys(pluginkey TEXT NOT NULL, menuguid TEXT NOT NULL, type INTEGER NOT NULL, hotkey TEXT, PRIMARY KEY(pluginkey, menuguid, type));") &&

			//get hotkey statement
			InitStmt(stmtGetHotkey, L"SELECT hotkey FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;") &&

			//set hotkey statement
			InitStmt(stmtSetHotkey, L"INSERT OR REPLACE INTO pluginhotkeys VALUES (?1,?2,?3,?4);") &&

			//delete hotkey statement
			InitStmt(stmtDelHotkey, L"DELETE FROM pluginhotkeys WHERE pluginkey=?1 AND menuguid=?2 AND type=?3;") &&

			//check if exist hotkeys of specific type statement
			InitStmt(stmtCheckForHotkeys, L"SELECT count(hotkey) FROM pluginhotkeys WHERE type=?1")
		;
	}

	virtual ~PluginsHotkeysConfigDb() {}

	bool HotkeysPresent(HotKeyTypeEnum HotKeyType)
	{
		int count = 0;
		if (stmtCheckForHotkeys.Bind((int)HotKeyType).Step())
			count = stmtCheckForHotkeys.GetColInt(0);
		stmtCheckForHotkeys.Reset();
		return count!=0;
	}

	string GetHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType)
	{
		string strHotKey;
		if (stmtGetHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).Step())
			strHotKey = stmtGetHotkey.GetColText(0);
		stmtGetHotkey.Reset();
		return strHotKey;
	}

	bool SetHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType, const wchar_t *HotKey)
	{
		return stmtSetHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).Bind(HotKey).StepAndReset();
	}

	bool DelHotkey(const wchar_t *PluginKey, const wchar_t *MenuGuid, HotKeyTypeEnum HotKeyType)
	{
		return stmtDelHotkey.Bind(PluginKey).Bind(MenuGuid).Bind((int)HotKeyType).StepAndReset();
	}

	TiXmlElement *Export()
	{
		TiXmlElement * root = new TiXmlElement("pluginhotkeys");
		if (!root)
			return nullptr;

		SQLiteStmt stmtEnumAllPluginKeys;
		InitStmt(stmtEnumAllPluginKeys, L"SELECT pluginkey FROM pluginhotkeys GROUP BY pluginkey;");
		SQLiteStmt stmtEnumAllHotkeysPerKey;
		InitStmt(stmtEnumAllHotkeysPerKey, L"SELECT menuguid, type, hotkey FROM pluginhotkeys WHERE pluginkey=$1;");

		while (stmtEnumAllPluginKeys.Step())
		{
			TiXmlElement *p = new TiXmlElement("plugin");
			if (!p)
				break;

			string Key = stmtEnumAllPluginKeys.GetColText(0);
			p->SetAttribute("key", stmtEnumAllPluginKeys.GetColTextUTF8(0));

			stmtEnumAllHotkeysPerKey.Bind(Key);
			while (stmtEnumAllHotkeysPerKey.Step())
			{
				TiXmlElement *e = new TiXmlElement("hotkey");
				if (!e)
					break;

				const char *type;
				switch (stmtEnumAllHotkeysPerKey.GetColInt(1))
				{
					case DRIVE_MENU: type = "drive"; break;
					case CONFIG_MENU: type = "config"; break;
					default: type = "plugins";
				}
				e->SetAttribute("menu", type);
				e->SetAttribute("guid", stmtEnumAllHotkeysPerKey.GetColTextUTF8(0));
				const char *hotkey = stmtEnumAllHotkeysPerKey.GetColTextUTF8(2);
				e->SetAttribute("hotkey", hotkey ? hotkey : "");
				p->LinkEndChild(e);
			}
			stmtEnumAllHotkeysPerKey.Reset();

			root->LinkEndChild(p);
		}

		stmtEnumAllPluginKeys.Reset();

		return root;
	}

	bool Import(const TiXmlHandle &root)
	{
		BeginTransaction();
		for (const TiXmlElement *e = root.FirstChild("pluginhotkeys").FirstChildElement("plugin").Element(); e; e=e->NextSiblingElement("plugin"))
		{
			const char *key = e->Attribute("key");

			if (!key)
				continue;

			string Key(key, CP_UTF8);

			for (const TiXmlElement *se = e->FirstChildElement("hotkey"); se; se=se->NextSiblingElement("hotkey"))
			{
				const char *stype = se->Attribute("menu");
				const char *guid = se->Attribute("guid");
				const char *hotkey = se->Attribute("hotkey");

				if (!guid || !stype)
					continue;

				string Guid(guid, CP_UTF8);
				string Hotkey(hotkey, CP_UTF8);
				HotKeyTypeEnum type;
				if (!strcmp(stype,"drive"))
					type = DRIVE_MENU;
				else if (!strcmp(stype,"config"))
					type = CONFIG_MENU;
				else
					type = PLUGINS_MENU;
				SetHotkey(Key, Guid, type, Hotkey);
			}

		}
		EndTransaction();

		return true;
	}
};

class HistoryConfigDb: public HistoryConfig {
	SQLiteStmt stmtEnum;
	SQLiteStmt stmtEnumDesc;
	SQLiteStmt stmtDel;
	SQLiteStmt stmtDeleteOldUnlocked;
	SQLiteStmt stmtEnumLargeHistories;
	SQLiteStmt stmtAdd;
	SQLiteStmt stmtGetName;
	SQLiteStmt stmtGetNameAndType;
	SQLiteStmt stmtGetNewestName;
	SQLiteStmt stmtCount;
	SQLiteStmt stmtDelUnlocked;
	SQLiteStmt stmtGetLock;
	SQLiteStmt stmtSetLock;
	SQLiteStmt stmtGetNext;
	SQLiteStmt stmtGetPrev;
	SQLiteStmt stmtGetNewest;
	SQLiteStmt stmtSetEditorPos;
	SQLiteStmt stmtSetEditorBookmark;
	SQLiteStmt stmtGetEditorPos;
	SQLiteStmt stmtGetEditorBookmark;
	SQLiteStmt stmtSetViewerPos;
	SQLiteStmt stmtSetViewerBookmark;
	SQLiteStmt stmtGetViewerPos;
	SQLiteStmt stmtGetViewerBookmark;
	SQLiteStmt stmtDeleteOldEditor;
	SQLiteStmt stmtDeleteOldViewer;

	unsigned __int64 CalcDays(int Days)
	{
		return ((unsigned __int64)Days) * 24ull * 60ull * 60ull * 10000000ull;
	}

public:

	HistoryConfigDb()
	{
		Initialize(L"history.db");
	}

	bool InitializeImpl(const wchar_t* DbName)
	{
		Close();
		return
			Open(DbName, true) &&

			//schema
			SetWALJournalingMode() &&

			EnableForeignKeysConstraints() &&
			//command,view,edit,folder,dialog history
			Exec(
				"CREATE TABLE IF NOT EXISTS history(id INTEGER PRIMARY KEY, kind INTEGER NOT NULL, key TEXT NOT NULL, type INTEGER NOT NULL, lock INTEGER NOT NULL, name TEXT NOT NULL, time INTEGER NOT NULL, guid TEXT NOT NULL, file TEXT NOT NULL, data TEXT NOT NULL);"
				"CREATE INDEX IF NOT EXISTS history_idx1 ON history (kind, key);"
				"CREATE INDEX IF NOT EXISTS history_idx2 ON history (kind, key, time);"
				"CREATE INDEX IF NOT EXISTS history_idx3 ON history (kind, key, lock DESC, time DESC);"
				"CREATE INDEX IF NOT EXISTS history_idx4 ON history (kind, key, time DESC);"
			) &&
			//view,edit file positions and bookmarks history
			Exec(
				"CREATE TABLE IF NOT EXISTS editorposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, time INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, codepage INTEGER NOT NULL);"
				"CREATE TABLE IF NOT EXISTS editorbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, line INTEGER NOT NULL, linepos INTEGER NOT NULL, screenline INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES editorposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
				"CREATE INDEX IF NOT EXISTS editorposition_history_idx1 ON editorposition_history (time DESC);"
				"CREATE TABLE IF NOT EXISTS viewerposition_history(id INTEGER PRIMARY KEY, name TEXT NOT NULL UNIQUE, time INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, hex INTEGER NOT NULL, codepage INTEGER NOT NULL);"
				"CREATE TABLE IF NOT EXISTS viewerbookmarks_history(pid INTEGER NOT NULL, num INTEGER NOT NULL, filepos INTEGER NOT NULL, leftpos INTEGER NOT NULL, FOREIGN KEY(pid) REFERENCES viewerposition_history(id) ON UPDATE CASCADE ON DELETE CASCADE, PRIMARY KEY (pid, num));"
				"CREATE INDEX IF NOT EXISTS viewerposition_history_idx1 ON viewerposition_history (time DESC);"
			) &&

			//enum items order by time statement
			InitStmt(stmtEnum, L"SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY time;") &&

			//enum items order by time DESC and lock DESC statement
			InitStmt(stmtEnumDesc, L"SELECT id, name, type, lock, time, guid, file, data FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC;") &&

			//delete item statement
			InitStmt(stmtDel, L"DELETE FROM history WHERE id=?1;") &&

			//delete old unlocked items statement
			InitStmt(stmtDeleteOldUnlocked, L"DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0 AND time<?3 AND id NOT IN (SELECT id FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT ?4);") &&

			//enum histories with more than X entries statement
			InitStmt(stmtEnumLargeHistories, L"SELECT key FROM (SELECT key, num FROM (SELECT key, count(id) as num FROM history WHERE kind=?1 GROUP BY key)) WHERE num > ?2;") &&

			//add item statement
			InitStmt(stmtAdd, L"INSERT INTO history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7,?8,?9);") &&

			//get item name statement
			InitStmt(stmtGetName, L"SELECT name FROM history WHERE id=?1;") &&

			//get item name and type statement
			InitStmt(stmtGetNameAndType, L"SELECT name, type, guid, file, data FROM history WHERE id=?1;") &&

			//get newest item (locked items go first) name statement
			InitStmt(stmtGetNewestName, L"SELECT name FROM history WHERE kind=?1 AND key=?2 ORDER BY lock DESC, time DESC LIMIT 1;") &&

			//count items statement
			InitStmt(stmtCount, L"SELECT count(id) FROM history WHERE kind=?1 AND key=?2;") &&

			//delete unlocked items statement
			InitStmt(stmtDelUnlocked, L"DELETE FROM history WHERE kind=?1 AND key=?2 AND lock=0;") &&

			//get item lock statement
			InitStmt(stmtGetLock, L"SELECT lock FROM history WHERE id=?1;") &&

			//set item lock statement
			InitStmt(stmtSetLock, L"UPDATE history SET lock=?1 WHERE id=?2") &&

			//get next (newer than current) item statement
			InitStmt(stmtGetNext, L"SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time>b.time ORDER BY a.time LIMIT 1;") &&

			//get prev (older than current) item statement
			InitStmt(stmtGetPrev, L"SELECT a.id, a.name FROM history AS a, history AS b WHERE b.id=?1 AND a.kind=?2 AND a.key=?3 AND a.time<b.time ORDER BY a.time DESC LIMIT 1;") &&

			//get newest item name statement
			InitStmt(stmtGetNewest, L"SELECT id, name FROM history WHERE kind=?1 AND key=?2 ORDER BY time DESC LIMIT 1;") &&

			//set editor position statement
			InitStmt(stmtSetEditorPos, L"INSERT OR REPLACE INTO editorposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6,?7);") &&

			//set editor bookmark statement
			InitStmt(stmtSetEditorBookmark, L"INSERT OR REPLACE INTO editorbookmarks_history VALUES (?1,?2,?3,?4,?5,?6);") &&

			//get editor position statement
			InitStmt(stmtGetEditorPos, L"SELECT id, line, linepos, screenline, leftpos, codepage FROM editorposition_history WHERE name=?1;") &&

			//get editor bookmark statement
			InitStmt(stmtGetEditorBookmark, L"SELECT line, linepos, screenline, leftpos FROM editorbookmarks_history WHERE pid=?1 AND num=?2;") &&

			//set viewer position statement
			InitStmt(stmtSetViewerPos, L"INSERT OR REPLACE INTO viewerposition_history VALUES (NULL,?1,?2,?3,?4,?5,?6);") &&

			//set viewer bookmark statement
			InitStmt(stmtSetViewerBookmark, L"INSERT OR REPLACE INTO viewerbookmarks_history VALUES (?1,?2,?3,?4);") &&

			//get viewer position statement
			InitStmt(stmtGetViewerPos, L"SELECT id, filepos, leftpos, hex, codepage FROM viewerposition_history WHERE name=?1;") &&

			//get viewer bookmark statement
			InitStmt(stmtGetViewerBookmark, L"SELECT filepos, leftpos FROM viewerbookmarks_history WHERE pid=?1 AND num=?2;") &&

			//delete old editor positions statement
			InitStmt(stmtDeleteOldEditor, L"DELETE FROM editorposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM editorposition_history ORDER BY time DESC LIMIT ?2);") &&

			//delete old viewer positions statement
			InitStmt(stmtDeleteOldViewer, L"DELETE FROM viewerposition_history WHERE time<?1 AND id NOT IN (SELECT id FROM viewerposition_history ORDER BY time DESC LIMIT ?2);")
		;
	}

	virtual ~HistoryConfigDb() {}

	bool Enum(DWORD index, DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 *id, string &strName, int *Type, bool *Lock, unsigned __int64 *Time, string &strGuid, string &strFile, string &strData, bool Reverse=false)
	{
		SQLiteStmt &stmt = Reverse ? stmtEnumDesc : stmtEnum;

		if (index == 0)
			stmt.Reset().Bind((int)TypeHistory).Bind(HistoryName,false);

		if (stmt.Step())
		{
			*id = stmt.GetColInt64(0);
			strName = stmt.GetColText(1);
			*Type = stmt.GetColInt(2);
			*Lock = stmt.GetColInt(3) ? true : false;
			*Time = stmt.GetColInt64(4);
			strGuid = stmt.GetColText(5);
			strFile = stmt.GetColText(6);
			strData = stmt.GetColText(7);
			return true;
		}

		stmt.Reset();
		return false;
	}

	bool Delete(unsigned __int64 id)
	{
		return stmtDel.Bind(id).StepAndReset();
	}

	bool DeleteOldUnlocked(DWORD TypeHistory, const wchar_t *HistoryName, int DaysToKeep, int MinimunEntries)
	{
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= CalcDays(DaysToKeep);
		return stmtDeleteOldUnlocked.Bind((int)TypeHistory).Bind(HistoryName).Bind(older).Bind(MinimunEntries).StepAndReset();
	}

	bool EnumLargeHistories(DWORD index, int MinimunEntries, DWORD TypeHistory, string &strHistoryName)
	{
		if (index == 0)
			stmtEnumLargeHistories.Reset().Bind((int)TypeHistory).Bind(MinimunEntries);

		if (stmtEnumLargeHistories.Step())
		{
			strHistoryName = stmtEnumLargeHistories.GetColText(0);
			return true;
		}

		stmtEnumLargeHistories.Reset();
		return false;
	}

	bool Add(DWORD TypeHistory, const wchar_t *HistoryName, string strName, int Type, bool Lock, string &strGuid, string &strFile, string &strData)
	{
		return stmtAdd.Bind((int)TypeHistory).Bind(HistoryName).Bind(Type).Bind(Lock?1:0).Bind(strName).Bind(GetCurrentUTCTimeInUI64()).Bind(strGuid).Bind(strFile).Bind(strData).StepAndReset();
	}

	bool GetNewest(DWORD TypeHistory, const wchar_t *HistoryName, string &strName)
	{
		bool b = stmtGetNewestName.Bind((int)TypeHistory).Bind(HistoryName).Step();
		if (b)
		{
			strName = stmtGetNewestName.GetColText(0);
		}
		stmtGetNewestName.Reset();
		return b;
	}

	bool Get(unsigned __int64 id, string &strName)
	{
		bool b = stmtGetName.Bind(id).Step();
		if (b)
		{
			strName = stmtGetName.GetColText(0);
		}
		stmtGetName.Reset();
		return b;
	}

	bool Get(unsigned __int64 id, string &strName, int *Type, string &strGuid, string &strFile, string &strData)
	{
		bool b = stmtGetNameAndType.Bind(id).Step();
		if (b)
		{
			strName = stmtGetNameAndType.GetColText(0);
			*Type = stmtGetNameAndType.GetColInt(1);
			strGuid = stmtGetNameAndType.GetColText(2);
			strFile = stmtGetNameAndType.GetColText(3);
			strData = stmtGetNameAndType.GetColText(4);
		}
		stmtGetNameAndType.Reset();
		return b;
	}

	DWORD Count(DWORD TypeHistory, const wchar_t *HistoryName)
	{
		DWORD c = 0;
		if (stmtCount.Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			 c = (DWORD) stmtCount.GetColInt(0);
		}
		stmtCount.Reset();
		return c;
	}

	bool FlipLock(unsigned __int64 id)
	{
		return stmtSetLock.Bind(IsLocked(id)?0:1).Bind(id).StepAndReset();
	}

	bool IsLocked(unsigned __int64 id)
	{
		bool l = false;
		if (stmtGetLock.Bind(id).Step())
		{
			 l = stmtGetLock.GetColInt(0) ? true : false;
		}
		stmtGetLock.Reset();
		return l;
	}

	bool DeleteAllUnlocked(DWORD TypeHistory, const wchar_t *HistoryName)
	{
		return stmtDelUnlocked.Bind((int)TypeHistory).Bind(HistoryName).StepAndReset();
	}

	unsigned __int64 GetNext(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName)
	{
		strName.Clear();
		unsigned __int64 nid = 0;
		if (!id)
			return nid;
		if (stmtGetNext.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetNext.GetColInt64(0);
			strName = stmtGetNext.GetColText(1);
		}
		stmtGetNext.Reset();
		return nid;
	}

	unsigned __int64 GetPrev(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName)
	{
		strName.Clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (stmtGetNewest.Bind((int)TypeHistory).Bind(HistoryName).Step())
			{
				nid = stmtGetNewest.GetColInt64(0);
				strName = stmtGetNewest.GetColText(1);
			}
			stmtGetNewest.Reset();
			return nid;
		}
		if (stmtGetPrev.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetPrev.GetColInt64(0);
			strName = stmtGetPrev.GetColText(1);
		}
		else if (Get(id, strName))
		{
			nid = id;
		}
		stmtGetPrev.Reset();
		return nid;
	}

	unsigned __int64 CyclicGetPrev(DWORD TypeHistory, const wchar_t *HistoryName, unsigned __int64 id, string &strName)
	{
		strName.Clear();
		unsigned __int64 nid = 0;
		if (!id)
		{
			if (stmtGetNewest.Bind((int)TypeHistory).Bind(HistoryName).Step())
			{
				nid = stmtGetNewest.GetColInt64(0);
				strName = stmtGetNewest.GetColText(1);
			}
			stmtGetNewest.Reset();
			return nid;
		}
		if (stmtGetPrev.Bind(id).Bind((int)TypeHistory).Bind(HistoryName).Step())
		{
			nid = stmtGetPrev.GetColInt64(0);
			strName = stmtGetPrev.GetColText(1);
		}
		stmtGetPrev.Reset();
		return nid;
	}

	unsigned __int64 SetEditorPos(const wchar_t *Name, int Line, int LinePos, int ScreenLine, int LeftPos, UINT CodePage)
	{
		if (stmtSetEditorPos.Bind(Name).Bind(GetCurrentUTCTimeInUI64()).Bind(Line).Bind(LinePos).Bind(ScreenLine).Bind(LeftPos).Bind((int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetEditorPos(const wchar_t *Name, int *Line, int *LinePos, int *ScreenLine, int *LeftPos, UINT *CodePage)
	{
		unsigned __int64 id=0;
		if (stmtGetEditorPos.Bind(Name).Step())
		{
			id = stmtGetEditorPos.GetColInt64(0);
			*Line = stmtGetEditorPos.GetColInt(1);
			*LinePos = stmtGetEditorPos.GetColInt(2);
			*ScreenLine = stmtGetEditorPos.GetColInt(3);
			*LeftPos = stmtGetEditorPos.GetColInt(4);
			*CodePage = stmtGetEditorPos.GetColInt(5);
		}
		stmtGetEditorPos.Reset();
		return id;
	}

	bool SetEditorBookmark(unsigned __int64 id, int i, int Line, int LinePos, int ScreenLine, int LeftPos)
	{
		return stmtSetEditorBookmark.Bind(id).Bind(i).Bind(Line).Bind(LinePos).Bind(ScreenLine).Bind(LeftPos).StepAndReset();
	}

	bool GetEditorBookmark(unsigned __int64 id, int i, int *Line, int *LinePos, int *ScreenLine, int *LeftPos)
	{
		bool b = stmtGetEditorBookmark.Bind(id).Bind(i).Step();
		if (b)
		{
			*Line = stmtGetEditorBookmark.GetColInt(0);
			*LinePos = stmtGetEditorBookmark.GetColInt(1);
			*ScreenLine = stmtGetEditorBookmark.GetColInt(2);
			*LeftPos = stmtGetEditorBookmark.GetColInt(3);
		}
		stmtGetEditorBookmark.Reset();
		return b;
	}

	unsigned __int64 SetViewerPos(const wchar_t *Name, __int64 FilePos, __int64 LeftPos, int Hex, UINT CodePage)
	{
		if (stmtSetViewerPos.Bind(Name).Bind(GetCurrentUTCTimeInUI64()).Bind(FilePos).Bind(LeftPos).Bind(Hex).Bind((int)CodePage).StepAndReset())
			return LastInsertRowID();
		return 0;
	}

	unsigned __int64 GetViewerPos(const wchar_t *Name, __int64 *FilePos, __int64 *LeftPos, int *Hex, UINT *CodePage)
	{
		unsigned __int64 id=0;
		if (stmtGetViewerPos.Bind(Name).Step())
		{
			id = stmtGetViewerPos.GetColInt64(0);
			*FilePos = stmtGetViewerPos.GetColInt64(1);
			*LeftPos = stmtGetViewerPos.GetColInt64(2);
			*Hex = stmtGetViewerPos.GetColInt(3);
			*CodePage = stmtGetViewerPos.GetColInt(4);
		}
		stmtGetViewerPos.Reset();
		return id;
	}

	bool SetViewerBookmark(unsigned __int64 id, int i, __int64 FilePos, __int64 LeftPos)
	{
		return stmtSetViewerBookmark.Bind(id).Bind(i).Bind(FilePos).Bind(LeftPos).StepAndReset();
	}

	bool GetViewerBookmark(unsigned __int64 id, int i, __int64 *FilePos, __int64 *LeftPos)
	{
		bool b = stmtGetViewerBookmark.Bind(id).Bind(i).Step();
		if (b)
		{
			*FilePos = stmtGetViewerBookmark.GetColInt64(0);
			*LeftPos = stmtGetViewerBookmark.GetColInt64(1);
		}
		stmtGetViewerBookmark.Reset();
		return b;
	}

	void DeleteOldPositions(int DaysToKeep, int MinimunEntries)
	{
		unsigned __int64 older = GetCurrentUTCTimeInUI64();
		older -= CalcDays(DaysToKeep);
		stmtDeleteOldEditor.Bind(older).Bind(MinimunEntries).StepAndReset();
		stmtDeleteOldViewer.Bind(older).Bind(MinimunEntries).StepAndReset();
	}

};

HierarchicalConfig *CreatePluginsConfig(const wchar_t *guid)
{
	string strDbName = L"PluginsData\\";
	strDbName += guid;
	strDbName += L".db";
	return new HierarchicalConfigDb(strDbName);
}

HierarchicalConfig *CreateFiltersConfig()
{
	return new HierarchicalConfigDb(L"filters.db");
}

HierarchicalConfig *CreateHighlightConfig()
{
	return new HierarchicalConfigDb(L"highlight.db");
}

HierarchicalConfig *CreateShortcutsConfig()
{
	return new HierarchicalConfigDb(L"shortcuts.db");
}

HierarchicalConfig *CreatePanelModeConfig()
{
	return new HierarchicalConfigDb(L"panelmodes.db");
}

void InitDb()
{
	GeneralCfg = new GeneralConfigDb();
	AssocConfig = new AssociationsConfigDb();
	PlCacheCfg = new PluginsCacheConfigDb();
	PlHotkeyCfg = new PluginsHotkeysConfigDb();
	HistoryCfg = new HistoryConfigDb();
}

void ReleaseDb()
{
	delete GeneralCfg;
	delete AssocConfig;
	delete PlCacheCfg;
	delete PlHotkeyCfg;
	delete HistoryCfg;
}

bool ExportImportConfig(bool Export, const wchar_t *XML)
{
	FILE* XmlFile = _wfopen(NTPath(XML), Export?L"wb":L"rb");
	if(!XmlFile)
		return false;

	bool ret = false;

	int mc;
	SMatch m[2];
	RegExp re;
	re.Compile(L"/^[0-9A-F]{8}-([0-9A-F]{4}-){3}[0-9A-F]{12}$/", OP_PERLSTYLE|OP_OPTIMIZE);

	if (Export)
	{
		TiXmlDocument doc;
		doc.LinkEndChild(new TiXmlDeclaration("1.0", "UTF-8", ""));

		FormatString strVer;
		strVer << FAR_VERSION.Major << L"." << FAR_VERSION.Minor << L"." << FAR_VERSION.Build;
		char ver[50];
		strVer.GetCharString(ver, ARRAYSIZE(ver));
		TiXmlElement *root = new TiXmlElement("farconfig");
		root->SetAttribute("version", ver);

		root->LinkEndChild(GeneralCfg->Export());

		root->LinkEndChild(AssocConfig->Export());

		root->LinkEndChild(PlHotkeyCfg->Export());

		HierarchicalConfig *cfg = CreateFiltersConfig();
		TiXmlElement *e = new TiXmlElement("filters");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		cfg = CreateHighlightConfig();
		e = new TiXmlElement("highlight");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		cfg = CreatePanelModeConfig();
		e = new TiXmlElement("panelmodes");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		cfg = CreateShortcutsConfig();
		e = new TiXmlElement("shortcuts");
		e->LinkEndChild(cfg->Export());
		root->LinkEndChild(e);
		delete cfg;

		{
			string strPlugins = Opt.ProfilePath;
			strPlugins += L"\\PluginsData\\*.db";
			FAR_FIND_DATA_EX fd;
			FindFile ff(strPlugins);
			e = new TiXmlElement("pluginsconfig");
			while (ff.Get(fd))
			{
				fd.strFileName.SetLength(fd.strFileName.GetLength()-3);
				fd.strFileName.Upper();
				mc=2;
				if (re.Match(fd.strFileName, fd.strFileName.CPtr() + fd.strFileName.GetLength(), m, mc))
				{
					char guid[37];
					for (size_t i=0; i<ARRAYSIZE(guid); i++)
						guid[i] = fd.strFileName[i]&0xFF;

					TiXmlElement *plugin = new TiXmlElement("plugin");
					plugin->SetAttribute("guid", guid);
					cfg = CreatePluginsConfig(fd.strFileName);
					plugin->LinkEndChild(cfg->Export());
					e->LinkEndChild(plugin);
					delete cfg;
				}
			}
			root->LinkEndChild(e);
		}

		doc.LinkEndChild(root);
		ret = doc.SaveFile(XmlFile);
	}
	else
	{
		TiXmlDocument doc;
		if (doc.LoadFile(XmlFile))
		{
			TiXmlElement *farconfig = doc.FirstChildElement("farconfig");
			if (farconfig)
			{
				const TiXmlHandle root(farconfig);

				GeneralCfg->Import(root);

				AssocConfig->Import(root);

				PlHotkeyCfg->Import(root);

				HierarchicalConfig *cfg = CreateFiltersConfig();
				cfg->Import(root.FirstChildElement("filters"));
				delete cfg;

				cfg = CreateHighlightConfig();
				cfg->Import(root.FirstChildElement("highlight"));
				delete cfg;

				cfg = CreatePanelModeConfig();
				cfg->Import(root.FirstChildElement("panelmodes"));
				delete cfg;

				cfg = CreateShortcutsConfig();
				cfg->Import(root.FirstChildElement("shortcuts"));
				delete cfg;

				for (TiXmlElement *plugin=root.FirstChild("pluginsconfig").FirstChildElement("plugin").Element(); plugin; plugin=plugin->NextSiblingElement("plugin"))
				{
					const char *guid = plugin->Attribute("guid");
					if (!guid)
						continue;
					string Guid(guid, CP_UTF8);
					Guid.Upper();

					mc=2;
					if (re.Match(Guid, Guid.CPtr() + Guid.GetLength(), m, mc))
					{
						cfg = CreatePluginsConfig(Guid);
						const TiXmlHandle h(plugin);
						cfg->Import(h);
						delete cfg;
					}
				}

				ret = true;
			}
		}
	}

	fclose(XmlFile);
	return ret;
}

void ClearPluginsCache()
{
	PluginsCacheConfigDb *p = new PluginsCacheConfigDb();
	p->DiscardCache();
	delete p;
}
