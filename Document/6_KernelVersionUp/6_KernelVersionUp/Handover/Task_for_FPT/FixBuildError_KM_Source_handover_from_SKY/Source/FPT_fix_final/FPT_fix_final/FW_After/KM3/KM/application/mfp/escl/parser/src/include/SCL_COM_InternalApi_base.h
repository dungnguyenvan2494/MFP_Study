#ifndef __SCL_COM_INTERNALAPI_BASE_H__
#define __SCL_COM_INTERNALAPI_BASE_H__

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <math.h> // IT2マージ No.111 
#include "SCL_COM_define.h"

#include "system/sys/APIC_DummyIF.h"


//// TypeTraits lib
// tag
enum TypeTagEnum 
{
	TypeTagEnum_Struct = 1 ,
	TypeTagEnum_Enum = 2 ,
	TypeTagEnum_String = 3 ,
	TypeTagEnum_Number = 4 ,
	TypeTagEnum_Igc = 10 ,
};

template <int I>
struct int2type
{
	enum { value = I , };
};

struct TypeTag_Null{enum s{value = 0,};};
struct TypeTag_Struct{enum s{value = 1,};};
struct TypeTag_Enum{enum s{value = 2,};};
struct TypeTag_String{enum s{value = 3,};};
struct TypeTag_Number{enum s{value = 4,};};

extern const int			MAX_CHAR_ESCL;
extern const unsigned int   MAX_UCHAR_ESCL;
extern const int			MAX_SHORT_ESCL;
extern const unsigned int	MAX_USHORT_ESCL;
extern const int			MAX_LONG_ESCL;
extern const unsigned int	MAX_ULONG_ESCL;

// Cash対応 >> .
struct SCLC_DataObject
{
	size_t sz_Len_;
	const char* pc_Data_;
};
// Cash対応 << .
///////////////////////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////////////////////// 
// 
// 
//											Formats 
// 
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////// 
///////////////////////////////////////////////////////////////////////////////////////////////////////// 
//==========================================================================.
// 変更履歴.
// 2007/08/18	Y.Sanda		Donau1st	Ver対応 VER10-VER31削除、VER40追加.
// 2008/06/11	NAKAGAWA	Amur		Ver対応 VER41追加.
// 2008/12/11	heima		Donau3rd	Ver対応 VER42追加.
// 2009/04/16 	nishida		Taiga		Ver対応 VER421追加.
// 2010/01/07	heima		Minerva1st(Zeus1st)	Ver対応 VER32-VER421削除、VER50追加.
//==========================================================================.
// ！！！！！Ver追加時の修正必要箇所！！！！！.
// \buildenv\version_standard.txt						各種バージョンテーブル.
// \src\database\OAP_COM_database.cpp					SCLC_InternalDataBaseのコンストラクタ.
// \src\client_service\builtin\OAP_TRN_packagedata.h	static const package_version tblVersionInfo[].
//==========================================================================.
// ヘッダー情報.
struct SCLS_MsgHeaderInfo 
{
// バージョンラベル (current/item_define用）.
#define VERNON	0x00
#define VERALL	0xff
#define VERC1	0x80
#define VER50	0x01

// 強制省略ラベル(rem用)
// 通常はNONREM(非省略)で指定すること 
// ある構造体Aが複数のバージョンで使用されるとき、 
// Aをメンバーとして持つ構造体Bがバージョンによって
// Aを使用したりしなかった場合に利用する
// この例では以下のような実装が想定される 
// struct B {
// 		......
// 		A	a;
// 		......
//
// 		B(cver) :
// 		a(cver, (VERxx==cver) ? REM : NONREM)
//		{}
// };
#define REM		0x01
#define NONREM	0x00

	unsigned char	status;				// 省略状態				(0:省略状態/1:何らかの値が代入された)  
	unsigned char	current;			// カレントバージョン	(受信したメッセージのバージョン) 
	unsigned char	item_define;		// 定義バージョン		(対象メンバー変数が使用されるバージョン。バージョンラベルの論理和で表される) 
	unsigned char	rem;				// 強制省略				(上記参照のこと) 
	bool			MustHonor;

	SCLS_MsgHeaderInfo( const unsigned char stat ,
				   const unsigned char cver ,
				   const unsigned char def ,
				   const unsigned char r );
	SCLS_MsgHeaderInfo();
};

// 数値型(signed)
struct SCLS_MsgInteger 
{
	enum { TypeValue = TypeTagEnum_Number , };
	SCLS_MsgHeaderInfo	header;				// ヘッダー情報 
	int32_t value;
	int32_t limitmax;
	int32_t limitmin;

	bool empty() const { return (header.status) ? false : true; }
	bool getMustHonor() const { return (header.MustHonor) ? true : false;}
	int32_t get() const { return value; }
	const char* getformat() const;
	bool range() const { return (limitmin<=value && value<=limitmax) ? true : false; }
	bool range(int32_t min, int32_t max) const { return (min<=value && value<=max) ? true : false; }
	void clean() { value=0; header.status = 0; }
	SCLS_MsgInteger& operator=( const int& n );
	SCLS_MsgInteger& operator=( const SCLS_MsgInteger& n );

	bool set( const char* n );

	bool operator==(const SCLS_MsgInteger& x) const { return (value==x.value) ? true : false; }
	bool operator!=(const SCLS_MsgInteger& x) const { return (value!=x.value) ? true : false; }
	bool operator<(const SCLS_MsgInteger& x) const { return (value<x.value) ? true : false; }
	bool operator>(const SCLS_MsgInteger& x) const { return (value>x.value) ? true : false; }
	bool operator<=(const SCLS_MsgInteger& x) const { return (value<=x.value) ? true : false; }
	bool operator>=(const SCLS_MsgInteger& x) const { return (value>=x.value) ? true : false; }

	bool operator==(const short x) const { return (value==x) ? true : false; }
	bool operator!=(const short x) const { return (value!=x) ? true : false; }
	bool operator<(const short x) const { return (value<x) ? true : false; }
	bool operator>(const short x) const { return (value>x) ? true : false; }
	bool operator<=(const short x) const { return (value<=x) ? true : false; }
	bool operator>=(const short x) const { return (value>=x) ? true : false; }
	bool operator==(const int32_t x) const { return (value==x) ? true : false; }
	bool operator!=(const int32_t x) const { return (value!=x) ? true : false; }
	bool operator<(const int32_t x) const { return (value<x) ? true : false; }
	bool operator>(const int32_t x) const { return (value>x) ? true : false; }
	bool operator<=(const int32_t x) const { return (value<=x) ? true : false; }
	bool operator>=(const int32_t x) const { return (value>=x) ? true : false; }

	bool operator!() const { return (this->get()) ? true : false; }
	SCLS_MsgInteger& operator++() { ++value; return *this; }
	SCLS_MsgInteger& operator++(int) { value++; return *this; }
	SCLS_MsgInteger& operator--() { --value; return *this; }
	SCLS_MsgInteger& operator--(int) { value--; return *this; }
	operator int() const { return value; }
	//operator int32_t() const { return (int32_t)value; }
	operator short() const { return (short)value; }
	operator float() const { return (float)value; }
	operator char() const { return (char)value; }

	// コンストラクタ 
	SCLS_MsgInteger( const unsigned char cver ,			// カレントバージョン 
				const unsigned char def ,			// 定義バージョン 
				const unsigned char rem ,			// 強制省略 
				const int32_t min ,					// 最小値 
				const int32_t max );					// 最大値 

	SCLS_MsgInteger();
	SCLS_MsgInteger( const int& x );
};
inline bool operator==(const short x, const SCLS_MsgInteger& m) { return (m==x); }
inline bool operator!=(const short x, const SCLS_MsgInteger& m) { return (m!=x); }
inline bool operator<(const short x, const SCLS_MsgInteger& m) { return (m>x); }
inline bool operator>(const short x, const SCLS_MsgInteger& m) { return (m<x); }
inline bool operator<=(const short x, const SCLS_MsgInteger& m) { return (m>=x); }
inline bool operator>=(const short x, const SCLS_MsgInteger& m) { return (m<=x); }
inline bool operator==(const int32_t x, const SCLS_MsgInteger& m) { return (m==x); }
inline bool operator!=(const int32_t x, const SCLS_MsgInteger& m) { return (m!=x); }
inline bool operator<(const int32_t x, const SCLS_MsgInteger& m) { return (m>x); }
inline bool operator>(const int32_t x, const SCLS_MsgInteger& m) { return (m<x); }
inline bool operator<=(const int32_t x, const SCLS_MsgInteger& m) { return (m>=x); }
inline bool operator>=(const int32_t x, const SCLS_MsgInteger& m) { return (m<=x); }

inline int operator+(const int x, const SCLS_MsgInteger& m) { return m.get()+x; }
inline int operator+(const SCLS_MsgInteger& m, const int& x) { return m.get()+x; }
inline short operator+(const short x, const SCLS_MsgInteger& m) { return (short)m.get()+x; }
inline short operator+(const SCLS_MsgInteger& m, const short& x) { return (short)m.get()+x; }
inline int operator-(const int x, const SCLS_MsgInteger& m) { return x-m.get(); }
inline int operator-(const SCLS_MsgInteger& m, const int& x) { return m.get()-x; }
inline short operator-(const short x, const SCLS_MsgInteger& m) { return x-(short)m.get(); }
inline short operator-(const SCLS_MsgInteger& m, const short& x) { return (short)m.get()-x; }

// 数値型(unsigned)
struct SCLS_MsgUinteger 
{
	enum { TypeValue = TypeTagEnum_Number , };
	SCLS_MsgHeaderInfo	header;				// ヘッダー情報 
	uint32_t value;
	uint32_t limitmax;
	uint32_t limitmin;

	bool empty() const { return (header.status) ? false : true; }
	bool getMustHonor() const { return (header.MustHonor) ? true : false;}
	uint32_t get() const { return value; }	// CQNo344269 Minerva.
	const char* getformat() const;
	bool range() const { return (limitmin<=value && value<=limitmax) ? true : false; }
	bool range(uint32_t min, uint32_t max) const { return (min<=value && value<=max) ? true : false; }
	void clean() { value=0; header.status = 0; }
	SCLS_MsgUinteger& operator=(const unsigned int& n);
	SCLS_MsgUinteger& operator=(const SCLS_MsgUinteger& n);

	bool set(const char* n);

	bool operator==(const SCLS_MsgUinteger& x) const { return (value==x.value) ? true : false; }
	bool operator!=(const SCLS_MsgUinteger& x) const { return (value!=x.value) ? true : false; }
	bool operator<(const SCLS_MsgUinteger& x) const { return (value<x.value) ? true : false; }
	bool operator>(const SCLS_MsgUinteger& x) const { return (value>x.value) ? true : false; }
	bool operator<=(const SCLS_MsgUinteger& x) const { return (value<=x.value) ? true : false; }
	bool operator>=(const SCLS_MsgUinteger& x) const { return (value>=x.value) ? true : false; }

	bool operator==(const unsigned short x) const { return (value==x) ? true : false; }
	bool operator!=(const unsigned short x) const { return (value!=x) ? true : false; }
	bool operator<(const unsigned short x) const { return (value<x) ? true : false; }
	bool operator>(const unsigned short x) const { return (value>x) ? true : false; }
	bool operator<=(const unsigned short x) const { return (value<=x) ? true : false; }
	bool operator>=(const unsigned short x) const { return (value>=x) ? true : false; }
	bool operator==(const uint32_t x) const { return (value==x) ? true : false; }
	bool operator!=(const uint32_t x) const { return (value!=x) ? true : false; }
	bool operator<(const uint32_t x) const { return (value<x) ? true : false; }
	bool operator>(const uint32_t x) const { return (value>x) ? true : false; }
	bool operator<=(const uint32_t x) const { return (value<=x) ? true : false; }
	bool operator>=(const uint32_t x) const { return (value>=x) ? true : false; }

	bool operator!() const { return (this->get()) ? true : false; }
	SCLS_MsgUinteger& operator++() { ++value; return *this; }
	SCLS_MsgUinteger& operator++(int) { value++; return *this; }
	SCLS_MsgUinteger& operator--() { --value; return *this; }
	SCLS_MsgUinteger& operator--(int) { value--; return *this; }
	operator unsigned int() const { return (unsigned int)value; }
	//operator uint32_t() const { return (uint32_t)value; }
	operator unsigned short() const { return (unsigned short)value; }
	operator float() const { return (float)value; }

	// コンストラクタ 
	SCLS_MsgUinteger( const unsigned char cver,			// カレントバージョン 
				 const unsigned char def ,			// 定義バージョン 
				 const unsigned char rem ,			// 強制省略 
				 const uint32_t min ,			// 最小値 
				 const uint32_t max );			// 最大値 
	SCLS_MsgUinteger();
	SCLS_MsgUinteger(const unsigned int& x);
};
inline bool operator==(const unsigned short x, const SCLS_MsgUinteger& m) { return (m==x); }
inline bool operator!=(const unsigned short x, const SCLS_MsgUinteger& m) { return (m!=x); }
inline bool operator<(const unsigned short x, const SCLS_MsgUinteger& m) { return (m>x); }
inline bool operator>(const unsigned short x, const SCLS_MsgUinteger& m) { return (m<x); }
inline bool operator<=(const unsigned short x, const SCLS_MsgUinteger& m) { return (m>=x); }
inline bool operator>=(const unsigned short x, const SCLS_MsgUinteger& m) { return (m<=x); }
inline bool operator==(const uint32_t x, const SCLS_MsgUinteger& m) { return (m==x); }
inline bool operator!=(const uint32_t x, const SCLS_MsgUinteger& m) { return (m!=x); }
inline bool operator<(const uint32_t x, const SCLS_MsgUinteger& m) { return (m>x); }
inline bool operator>(const uint32_t x, const SCLS_MsgUinteger& m) { return (m<x); }
inline bool operator<=(const uint32_t x, const SCLS_MsgUinteger& m) { return (m>=x); }
inline bool operator>=(const uint32_t x, const SCLS_MsgUinteger& m) { return (m<=x); }

inline unsigned int operator+(const unsigned int x, const SCLS_MsgUinteger& m) { return m.get()+x; }
inline unsigned int operator+(const SCLS_MsgUinteger& m, const unsigned int& x) { return m.get()+x; }
inline unsigned short operator+(const short x, const SCLS_MsgUinteger& m) { return (unsigned short)m.get()+x; }
inline unsigned short operator+(const SCLS_MsgUinteger& m, const unsigned short& x) { return (unsigned short)m.get()+x; }
inline unsigned int operator-(const unsigned int x, const SCLS_MsgUinteger& m) { return x-m.get(); }
inline unsigned int operator-(const SCLS_MsgUinteger& m, const unsigned int& x) { return m.get()-x; }
inline unsigned short operator-(const unsigned short x, const SCLS_MsgUinteger& m) { return x-(unsigned short)m.get(); }
inline unsigned short operator-(const SCLS_MsgUinteger& m, const unsigned short& x) { return (unsigned short)m.get()-x; }

// 文字列型
template<int ARRAY>
struct SCLS_MsgString 
{
	enum { TypeValue = TypeTagEnum_String , };
	enum { stirng_size = ARRAY };
	SCLS_MsgHeaderInfo	header;
	char value[ARRAY];
	bool empty() const { return (header.status) ? false : true; }
	bool getMustHonor() const { return (header.MustHonor) ? true : false;}
	bool isnull() const { return (!value[0]) ? true : false; }

	void set( const SCLC_DataObject* pc_DataObject ) 
	{
		value[0] = 0;
		if( pc_DataObject->pc_Data_ )
		{
			clean();
			::memcpy( value , pc_DataObject->pc_Data_ , pc_DataObject->sz_Len_ );
		}
		header.status = 1;  // 2007/7/11 Mizoguchi 空タグの場合empty扱いになるので修正。 
	}

	const char* get() const
	{
		return value;
	}
	const char* out() const
	{
		return value;
	}
	void clean() { ::memset(value, 0, sizeof(value)); header.status=0; }	// Donau CQNo.303968 NAKAGAWA 2008/05/23 Oap共通問題.
	unsigned int size() const { return empty()? 0 : ::strlen(value); }
	SCLS_MsgString& operator=(const char* n)
	{
		value[0] = 0;
		if (n) {
			clean();
			::strncpy( value, n, (ARRAY-1) );
			value[ARRAY-1] = '\0';
			header.status = 1;
		}
		return *this;
	}

	template<int ARRAY2>
	SCLS_MsgString& operator=(const SCLS_MsgString<ARRAY2>& n)
	{
		::memcpy(this, &n, sizeof(n));
		return *this;
	}
	
	SCLS_MsgString& operator=(const uchar* n)
	{
		value[0] = 0;
		if (n) {
			clean();
			::strncpy(value, (const char*)n, ARRAY);
			header.status = 1;
		}
		return *this;
	}

	SCLS_MsgString<0> operator[](const unsigned int x) const { return (x < ARRAY) ? value[x] : value; }
	bool operator==(const char* str) const { return (strcmp(value, str)) ? false : true; }
	bool operator!=(const char* str) const { return (strcmp(value, str)) ? true : false; }
	template<int C>
	bool operator==(const SCLS_MsgString<C>& str) const { return (strcmp(value, str.value)) ? false : true; }
	template<int C>
	bool operator!=(const SCLS_MsgString<C>& str) const { return (strcmp(value, str.value)) ? true : false; }
	SCLS_MsgString& operator+=(const char* str)
	{
		unsigned int addlen = ::strlen(str);
		unsigned int len = ::strlen(value);
		if ((len+addlen) >= sizeof(value)) addlen = sizeof(value)-len-1;
		char* p = &value[0];
		p+=len;
		strncpy(p, str, addlen);
		header.status = 1;
		return *this;
	}

	// コンストラクタ 
	SCLS_MsgString<ARRAY>( const unsigned char cver ,		// カレントバージョン 
					  const unsigned char def ,			// 定義バージョン 
					  const unsigned char rem ) :		// 強制省略 
		header( 0 , cver , def , rem )
	{ ::memset(value, 0, sizeof(value)); }

	SCLS_MsgString<ARRAY>() :
		header( 0 , 0 , 0 , NONREM )
	{
		::memset( value , 0 , sizeof(value) );
	}
};
template<int ARRAY>
inline bool operator==(const char* str, const SCLS_MsgString<ARRAY>& s) { return (s==str) ? true : false; }
template<int ARRAY>
inline bool operator!=(const char* str, const SCLS_MsgString<ARRAY>& s) { return (s!=str) ? true : false; }

//	"1.0 != 1.0+OAP_FLT_EPSILON" となる値 IT2マージ No.111 
//	この値は、実際に使われる値の精度にあわせるようにする。
//	例えば、0.01の精度であれば、この値は小さすぎ、期待した結果を得られない可能性が高い。
//	SCLS_MsgFloatでoperator==を使うようになったとき、十分確認されたい。
#define OAP_FLT_EPSILON (1.192092896e-07F)

// 実数型
struct SCLS_MsgFloat 
{
	enum { TypeValue = TypeTagEnum_Number , };
	SCLS_MsgHeaderInfo	header;
	char format[12];
	float value;
	bool empty() const { return (header.status) ? false : true; }
	bool getMustHonor() const { return (header.MustHonor) ? true : false;}
	float get() const { return value; }
	const char* getformat() const { return format; }
	void clean() { value=0.0; header.status = 0; }
	// GaiaFum sone 2009/8/17 追加.
	bool range(float min, float max) const { return (min<=value && value<=max) ? true : false; }

	SCLS_MsgFloat& operator=(const float& n);
	SCLS_MsgFloat& operator=(const SCLS_MsgFloat& n);

	bool set(const char* n);

	bool operator==(const SCLS_MsgFloat& f) const { return (fabs(value - f.value) <= OAP_FLT_EPSILON * value); }
	bool operator!=(const SCLS_MsgFloat& f) const { return !operator==(f); }
	bool operator<(const SCLS_MsgFloat& f) const { return (value<f.value) ? true : false; }
	bool operator>(const SCLS_MsgFloat& f) const { return (value>f.value) ? true : false; }
	bool operator<=(const SCLS_MsgFloat& f) const { return (value<=f.value) ? true : false; }
	bool operator>=(const SCLS_MsgFloat& f) const { return (value>=f.value) ? true : false; }

	bool operator==(const float f) const { return (fabs(value - f) <= OAP_FLT_EPSILON * value); }
	bool operator!=(const float f) const { return !operator==(f); }
	bool operator<(const float f) const { return (value<f) ? true : false; }
	bool operator>(const float f) const { return (value>f) ? true : false; }
	bool operator<=(const float f) const { return (value<=f) ? true : false; }
	bool operator>=(const float f) const { return (value>=f) ? true : false; }

	operator float() const { return value; }

	// コンストラクタ 
	SCLS_MsgFloat( const unsigned char cver ,
			  const unsigned char def ,
			  const unsigned char rem ,
			  const char* fm = "%f" );
	SCLS_MsgFloat();
	SCLS_MsgFloat(const float& f);
};

// Enum型 
template<class T>
struct SCLS_MsgEnum 
{
	enum { TypeValue = TypeTagEnum_Enum , };
	typedef T EnumType;
	SCLS_MsgHeaderInfo	header;
	T value;
	bool empty() const { return (header.status) ? false : true; }
	bool getMustHonor() const { return (header.MustHonor) ? true : false;}
	T get() const { return value; }
	void clean() { value = static_cast<T>(0); header.status=0; }

	SCLS_MsgEnum<T>& operator=(const T& n)
	{
		value=n;
		header.status = 1;
		return *this;
	}
	SCLS_MsgEnum<T>& operator=(const SCLS_MsgEnum<T>& n)
	{
		::memcpy(this, &n, sizeof(n));
		return *this;
	}

	bool operator==(const T& t) const { return (value==t); }
	bool operator!=(const T& t) const { return (value!=t); }
	bool operator==(const SCLS_MsgEnum<T>& t) const { return (value==t.value); }
	bool operator!=(const SCLS_MsgEnum<T>& t) const { return (value!=t.value); }
	operator T() const { return value; }

	// コンストラクタ 
	SCLS_MsgEnum( const unsigned char cver,			// カレントバージョン 
			 const unsigned char def,			// 定義バージョン 
			 const unsigned char rem ) :		// 強制省略 
		header( 0 , cver , def , rem ) ,
		value( static_cast<T>(0) )
	{}

	SCLS_MsgEnum() :
		header( 0 , 0 , 0 , NONREM ) ,
		value( static_cast<T>(0) )
	{}

	SCLS_MsgEnum( const T& t ) :
		header( 0 , 0 , 0 , NONREM ) ,
		value( t )
	{
		header.status = 1;
	}
};
template<class T>
inline bool operator==(const T& e1, const SCLS_MsgEnum<T>& e2) { return (e2==e1) ? true : false; }
template<class T >
inline bool operator!=(const T& e1, const SCLS_MsgEnum<T>& e2) { return (e2!=e1) ? true : false; }

//概要　　：構造体が必須パラメータを含む時、そのパラメータを含んで正常かどうかを判定をする 
//戻り値　：パラメータがダメな構造体はtrueを返す 
//備考　　：値の設定されてない構造体はfalseを返す 
//作成者　：ま。 
//レビュー：全然 
//変更履歴：2005/02/14:	初版 
//  
template<class T>
bool empty_check( const T& obj )
{
	return !obj.empty() && !obj.check();
}

//概要　　：配列の全パラメータに対してemptyを呼ぶ 
//戻り値　：途中でemptyが真ならtrueを返してループを抜ける 
//備考　　： 
//作成者　：ま。 
//レビュー：全然 
//変更履歴：2005/08/06:	初版 
//  
template<typename TYPE>
bool emptyArray( const TYPE* array , const size_t size )
{
	for( unsigned int i( 0 ); i < size; i++ ) 
	{
		if( !array[i].empty() ) 
		{
			return false;
		}
	}
	return true;
}

//概要　　：配列の全パラメータのコンストラクタを呼ぶ 
//戻り値　：なし
//備考　　： 
//作成者　：ま。 
//レビュー：全然 
//変更履歴：2005/08/06:	初版 
//  
template<typename TYPE>
void constructorArrayMsgString( TYPE* array , 
								const size_t size , 
								const unsigned char cver ,
								const unsigned char def ,
								const unsigned char rem )
{
	for( unsigned int i( 0 ); i < size; i++ ) 
	{
		array[i] = TYPE( cver , def ,  rem );
	}
}

template<typename TYPE>
void constructorArrayStructure( TYPE* array , 
								const size_t size , 
								const unsigned char cver ,
								const unsigned char rem )
{
	for( unsigned int i( 0 ); i < size; i++ ) 
	{
		array[i] = TYPE( cver , rem );
	}
}

template<typename TYPE>
void constructorArrayStructure( TYPE* array , 
								const size_t size , 
								const unsigned char cver )
{
	for( unsigned int i( 0 ); i < size; i++ ) 
	{
		array[i] = TYPE( cver );
	}
}

template<typename TYPE>
void constructorArrayMsgInteger( TYPE* array , 
								const size_t size , 
								const unsigned char cver ,
								const unsigned char def ,
								const unsigned char rem ,
								const int32_t min ,
								const int32_t max )
{
	for( unsigned int i( 0 ); i < size; i++ ) 
	{
		array[i] = TYPE( cver , def , rem , min , max );
	}
}

struct SCLS_StructureBase
{
	SCLS_MsgHeaderInfo	header;

	SCLS_StructureBase() {}
	SCLS_StructureBase( const unsigned char cver ) :				// カレントバージョン 
		header							( 0 , cver , 0 , NONREM )
	{}

	unsigned char getVersion() const { return header.current; }
};

#endif // __SCL_COM_INTERNALAPI_BASE_H__
