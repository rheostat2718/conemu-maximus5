#pragma once

/*
format.hpp

�������������� �����
*/
/*
Copyright � 2009 Far Group
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
namespace fmt
{
	template<typename T, T Default> class ManipulatorTemplate
	{
	public:
		ManipulatorTemplate(T Value=DefaultValue):Value(Value) {}
		T GetValue() const { return Value; }
		static T GetDefault() { return DefaultValue; }
	private:
		const T Value;
		static const T DefaultValue = Default;
	};

	typedef ManipulatorTemplate<size_t, 0> Width;
	typedef ManipulatorTemplate<size_t, static_cast<size_t>(-1)> Precision;
	typedef ManipulatorTemplate<wchar_t, L' '> FillChar;
	typedef ManipulatorTemplate<int, 10> Radix;

	enum AlignType
	{
		A_LEFT,
		A_RIGHT,
	};
	typedef ManipulatorTemplate<AlignType, A_RIGHT> Align;

	template<AlignType T>class SimpleAlign {};
	typedef SimpleAlign<A_LEFT> LeftAlign;
	typedef SimpleAlign<A_RIGHT> RightAlign;

	class Flush {};
};

class BaseFormat
{
public:
	BaseFormat();
	virtual ~BaseFormat() {}

	virtual BaseFormat& Flush() { return *this; }

	// attributes
	BaseFormat& SetPrecision(size_t Precision=fmt::Precision::GetDefault());
	BaseFormat& SetWidth(size_t Width=fmt::Width::GetDefault());
	BaseFormat& SetAlign(fmt::AlignType Align=fmt::Align::GetDefault());
	BaseFormat& SetFillChar(wchar_t Char=fmt::FillChar::GetDefault());
	BaseFormat& SetRadix(int Radix=fmt::Radix::GetDefault());

	BaseFormat& Put(LPCWSTR Data, size_t Length);

	// data
	BaseFormat& operator<<(INT64 Value);
	BaseFormat& operator<<(UINT64 Value);
	BaseFormat& operator<<(short Value);
	BaseFormat& operator<<(USHORT Value);
	BaseFormat& operator<<(int Value);
	BaseFormat& operator<<(UINT Value);
	BaseFormat& operator<<(long Value);
	BaseFormat& operator<<(ULONG Value);
	BaseFormat& operator<<(wchar_t Value);
	BaseFormat& operator<<(LPCWSTR Data);
	BaseFormat& operator<<(const string& String);

	// manipulators
	BaseFormat& operator<<(const fmt::Width& Manipulator);
	BaseFormat& operator<<(const fmt::Precision& Manipulator);
	BaseFormat& operator<<(const fmt::FillChar& Manipulator);
	BaseFormat& operator<<(const fmt::Radix& Manipulator);
	BaseFormat& operator<<(const fmt::Align& Manipulator);
	BaseFormat& operator<<(const fmt::LeftAlign& Manipulator);
	BaseFormat& operator<<(const fmt::RightAlign& Manipulator);
	BaseFormat& operator<<(const fmt::Flush& Manipulator);

protected:
	virtual void Commit(const string& Data)=0;

private:
	size_t Width;
	size_t Precision;
	wchar_t FillChar;
	fmt::AlignType Align;
	int Radix;

	void Reset();
	BaseFormat& ToString(INT64 Value, bool Signed);
};

class FormatString:public BaseFormat, public string
{
	virtual void Commit(const string& Data);
};

class FormatScreen:public BaseFormat
{
	virtual void Commit(const string& Data);
};
