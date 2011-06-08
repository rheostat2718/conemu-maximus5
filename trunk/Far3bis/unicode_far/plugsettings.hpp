#pragma once

/*
plugsettings.hpp

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

#include "array.hpp"
#include "plugin.hpp"
#include "configdb.hpp"

template <class Object> class Vector
{
	private:
		size_t InternalCount, Count, Delta;
		Object *Items;

	public:
		Vector():InternalCount(0), Count(0), Delta(8), Items(nullptr) {}
		~Vector() { delete [] Items; Items=nullptr; }
		size_t GetSize(void) const { return Count; }
		Object& AddItem(const Object &anItem)
		{
			if (InternalCount==Count)
			{
				InternalCount+=Delta;
				Object* newItems=new Object[InternalCount];
				for(size_t ii=0;ii<Count;++ii)
				{
					newItems[ii]=Items[ii];
				}
				delete [] Items;
				Items=newItems;
			}
			Items[Count++]=anItem;
			return Items[Count-1];
		}
		Object* GetItems(void) { return Items; }
};

class PluginSettings
{
	private:
		TPointerArray<unsigned __int64> m_Keys;
		TPointerArray<char*> m_Data;
		TPointerArray<Vector<FarSettingsName> > m_Enum;
		HierarchicalConfig *PluginsCfg;
		PluginSettings();
	public:
		PluginSettings(const GUID& Guid);
		~PluginSettings();
		bool IsValid(void) {return m_Keys.getCount()!=0;}
		int Set(const FarSettingsItem& Item);
		int Get(FarSettingsItem& Item);
		int Enum(FarSettingsEnum& Enum);
		int Delete(const FarSettingsValue& Value);
		int SubKey(const FarSettingsValue& Value, bool bCreate);
};
