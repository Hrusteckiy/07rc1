#include "stdafx.h"
#pragma hdrstop

LPSTR _TrimLeft(LPSTR str)
{
	LPSTR p_str = str;
	while (*p_str && (u8(*p_str) <= u8(' ')))
		p_str++;

	if (p_str != str)
	{
		LPSTR t = NULL;
		for (t = str; *p_str; t++, p_str++)
			*t = *p_str;

		*t = 0;
	}
	return str;
}

LPSTR _TrimRight(LPSTR str)
{
	LPSTR p_str = str + xr_strlen(str);
	while ((p_str != str) && (u8(*p_str) <= u8(' ')))
		p_str--;
	*(++p_str) = 0;
	return str;
}

LPSTR _Trim(LPSTR str)
{
	_TrimLeft(str);
	_TrimRight(str);
	return str;
}

LPCSTR _SetPos(LPCSTR src, u32 pos, char separator)
{
	LPCSTR res = src;
	u32 p_str = 0;
	while ((p_str < pos) && (0 != (res = strchr(res, separator))))
	{
		res++;
		p_str++;
	}
	return res;
}

LPCSTR _CopyVal(LPCSTR src, LPSTR dst, char separator)
{
	LPCSTR p_str;
	size_t n;
	p_str = strchr(src, separator);
	n = (p_str) ? (p_str - src) : xr_strlen(src);
	strncpy(dst, src, n);
	dst[n] = 0;
	return dst;
}

int _GetItemCount(LPCSTR src, char separator)
{
	u32 cnt = 0;

	if (src && src[0])
	{
		LPCSTR res = src;
		LPCSTR last_res = res;

		while (0 != (res = strchr(res, separator)))
		{
			res++;
			last_res = res;
			cnt++;

			if (res[0] == separator)
				break;
		}

		if (xr_strlen(last_res))
			cnt++;
	}

	return cnt;
}

LPSTR _GetItem(LPCSTR src, int index, LPSTR dst, char separator, LPCSTR def, bool trim)
{
	LPCSTR	ptr = _SetPos(src, index, separator);
	if (ptr)
		_CopyVal(ptr, dst, separator);
	else
		strcpy(dst, def);
	if (trim)
		_Trim(dst);
	return		dst;
}

LPSTR _GetItems(LPCSTR src, int idx_start, int idx_end, LPSTR dst, char separator)
{
	LPSTR n = dst;
	int level = 0;
	for (LPCSTR p_str = src; *p_str != 0; p_str++)
	{
		if ((level >= idx_start) && (level < idx_end))
			*n++ = *p_str;
		if (*p_str == separator)
			level++;
		if (level >= idx_end)
			break;
	}
	*n = '\0';
	return dst;
}

u32 _ParseItem(LPCSTR src, xr_token* token_list)
{
	for (int i = 0; token_list[i].name; i++)
		if (!stricmp(src, token_list[i].name))
			return token_list[i].id;
	return u32(-1);
}

u32 _ParseItem(LPSTR src, int ind, xr_token* token_list)
{
	char dst[128];
	_GetItem(src, ind, dst);
	return _ParseItem(dst, token_list);
}

LPSTR _ReplaceItems(LPCSTR src, int idx_start, int idx_end, LPCSTR new_items, LPSTR dst, char separator)
{
	LPSTR n = dst;
	int level = 0;
	bool bCopy = true;
	for (LPCSTR p_str = src; *p_str != 0; p_str++)
	{
		if ((level >= idx_start) && (level < idx_end))
		{
			if (bCopy)
			{
				for (LPCSTR itm = new_items; *itm != 0;)
					*n++ = *itm++;
				bCopy = false;
			}
			if (*p_str == separator)
				*n++ = separator;
		}
		else
		{
			*n++ = *p_str;
		}
		if (*p_str == separator)
			level++;
	}
	*n = '\0';
	return dst;
}

LPSTR _ReplaceItem(LPCSTR src, int index, LPCSTR new_item, LPSTR dst, char separator)
{
	LPSTR n = dst;
	int level = 0;
	bool bCopy = true;
	for (LPCSTR p_str = src; *p_str != 0; p_str++)
	{
		if (level == index)
		{
			if (bCopy)
			{
				for (LPCSTR itm = new_item; *itm != 0;)
					*n++ = *itm++;
				bCopy = false;
			}
			if (*p_str == separator)
				*n++ = separator;
		}
		else
		{
			*n++ = *p_str;
		}
		if (*p_str == separator)
			level++;
	}
	*n = '\0';
	return dst;
}

LPSTR _ChangeSymbol(LPSTR name, char src, char dest)
{
	char* sTmpName = name;
	while (sTmpName[0])
	{
		if (sTmpName[0] == src)
			sTmpName[0] = dest;
		sTmpName++;
	}
	return name;
}

xr_string& _ChangeSymbol(xr_string& name, char src, char dest)
{
	for (xr_string::iterator it = name.begin(); it != name.end(); it++)
		if (*it == src)
			*it = xr_string::value_type(dest);
	return name;
}

#ifdef M_BORLAND
AnsiString& _ReplaceItem 	( LPCSTR src, int index, LPCSTR new_item, AnsiString& dst, char separator )
{
	dst = "";
	int level = 0;
	bool bCopy = true;
	for (LPCSTR p=src; *p!=0; p++){
		if (level==index){
			if (bCopy){
				for (LPCSTR itm = new_item; *itm!=0;) dst += *itm++;
				bCopy=false;
			}
			if (*p==separator) dst += separator;
		}else{
			dst += *p;
		}
		if (*p==separator) level++;
	}
	return dst;
}

AnsiString& _ReplaceItems ( LPCSTR src, int idx_start, int idx_end, LPCSTR new_items, AnsiString& dst, char separator )
{
	dst = "";
	int level = 0;
	bool bCopy = true;
	for (LPCSTR p=src; *p!=0; p++){
		if ((level>=idx_start)&&(level<idx_end)){
			if (bCopy){
				for (LPCSTR itm = new_items; *itm!=0;) dst += *itm++;
				bCopy=false;
			}
			if (*p==separator) dst += separator;
		}else{
			dst += *p;
		}
		if (*p==separator) level++;
	}
	return dst;
}

AnsiString& _Trim( AnsiString& str )
{
	return str=str.Trim();
}

LPCSTR _CopyVal ( LPCSTR src, AnsiString& dst, char separator )
{
	LPCSTR	p;
	u32		n;
	p			= strchr	( src, separator );
	n			= (p>0) ? (p-src) : xr_strlen(src);
	dst			= src;
	dst			= dst.Delete(n+1,dst.Length());
	return		dst.c_str();
}

LPCSTR _GetItems ( LPCSTR src, int idx_start, int idx_end, AnsiString& dst, char separator )
{
	int level = 0;
	for (LPCSTR p=src; *p!=0; p++){
		if ((level>=idx_start)&&(level<idx_end))
			dst += *p;
		if (*p==separator) level++;
		if (level>=idx_end) break;
	}
	return dst.c_str();
}

LPCSTR _GetItem ( LPCSTR src, int index, AnsiString& dst, char separator, LPCSTR def, bool trim )
{
	LPCSTR	ptr;
	ptr			= _SetPos	( src, index, separator );
	if( ptr )	_CopyVal	( ptr, dst, separator );
	else	dst = def;
	if (trim)	dst			= dst.Trim();
	return		dst.c_str();
}

AnsiString _ListToSequence(const AStringVec& lst)
{
	AnsiString out;
	out = "";
	if (lst.size()){
		out			= lst.front();
		for (AStringVec::const_iterator s_it=lst.begin()+1; s_it!=lst.end(); s_it++)
			out		+= AnsiString(",")+(*s_it);
	}
	return out;
}

AnsiString _ListToSequence2(const AStringVec& lst)
{
	AnsiString out;
	out = "";
	if (lst.size()){
		out			= lst.front();
		for (AStringVec::const_iterator s_it=lst.begin()+1; s_it!=lst.end(); s_it++){
			out		+= AnsiString("\n")+(*s_it);
		}
	}
	return out;
}

void _SequenceToList(AStringVec& lst, LPCSTR in, char separator)
{
	lst.clear();
	int t_cnt=_GetItemCount(in,separator);
	AnsiString T;
	for (int i=0; i<t_cnt; i++){
		_GetItem(in,i,T,separator,0);
		_Trim(T);
		if (!T.IsEmpty()) lst.push_back(T);
	}
}

AnsiString FloatTimeToStrTime(float v, bool _h, bool _m, bool _s, bool _ms)
{
	AnsiString buf="";
	int h=0,m=0,s=0,ms;
	AnsiString t;
	if (_h){ h=iFloor(v/3600); 					t.sprintf("%02d",h); buf += t;}
	if (_m){ m=iFloor((v-h*3600)/60);			t.sprintf("%02d",m); buf += buf.IsEmpty()?t:":"+t;}
	if (_s){ s=iFloor(v-h*3600-m*60);			t.sprintf("%02d",s); buf += buf.IsEmpty()?t:":"+t;}
	if (_ms){ms=iFloor((v-h*3600-m*60-s)*1000.f);t.sprintf("%03d",ms);buf += buf.IsEmpty()?t:"."+t;}
	return buf;
}

float StrTimeToFloatTime(LPCSTR buf, bool _h, bool _m, bool _s, bool _ms)
{
	float t[4]	= {0.f,0.f,0.f,0.f};
	int   rm[4];
	int idx		= 0;
	if (_h) rm[0]=idx++;
	if (_m) rm[1]=idx++;
	if (_s) rm[2]=idx++;
	if (_ms)rm[3]=idx;
	int cnt = _GetItemCount(buf,':');
	AnsiString tmp;
	for (int k=0; k<cnt; k++){
		_GetItem(buf,k,tmp,':');
		t[rm[k]]=atof(tmp.c_str());
	}
	return t[0]*3600+t[1]*60+t[2];
}
#endif

void _SequenceToList(LPSTRVec& lst, LPCSTR in, char separator)
{
	int t_cnt = _GetItemCount(in, separator);
	string1024 buffer;
	for (int i = 0; i < t_cnt; i++)
	{
		_GetItem(in, i, buffer, separator, 0);
		_Trim(buffer);
		if (xr_strlen(buffer))
			lst.push_back(xr_strdup(buffer));
	}
}

void _SequenceToList(RStringVec& lst, LPCSTR in, char separator)
{
	lst.clear();
	int t_cnt = _GetItemCount(in, separator);
	xr_string buffer;
	for (int i = 0; i < t_cnt; i++)
	{
		_GetItem(in, i, buffer, separator, 0);
		_Trim(buffer);
		if (buffer.size())
			lst.push_back(buffer.c_str());
	}
}

void _SequenceToList(SStringVec& lst, LPCSTR in, char separator)
{
	lst.clear();
	int t_cnt = _GetItemCount(in, separator);
	xr_string buffer;
	for (int i = 0; i < t_cnt; i++)
	{
		_GetItem(in, i, buffer, separator, 0);
		_Trim(buffer);
		if (buffer.size())
			lst.push_back(buffer.c_str());
	}
}

xr_string _ListToSequence(const SStringVec& lst)
{
	static xr_string out;
	out = "";
	if (lst.size())
	{
		out = lst.front();
		for (SStringVec::const_iterator s_it = lst.begin() + 1; s_it != lst.end(); s_it++)
			out += xr_string(",") + (*s_it);
	}
	return out;
}

xr_string& _TrimLeft(xr_string& str)
{
	LPCSTR b = str.c_str();
	LPCSTR p_str = str.c_str();
	while (*p_str && (u8(*p_str) <= u8(' ')))
		p_str++;
	if (p_str != b)
		str.erase(0, p_str - b);
	return str;
}

xr_string& _TrimRight(xr_string& str)
{
	LPCSTR b = str.c_str();
	size_t l = str.length();
	if (l)
	{
		LPCSTR p_str = str.c_str() + l - 1;
		while ((p_str != b) && (u8(*p_str) <= u8(' ')))
			p_str--;
		if (p_str != (str + b))
			str.erase(p_str - b + 1, l - (p_str - b));
	}
	return str;
}

xr_string& _Trim(xr_string& str)
{
	_TrimLeft(str);
	_TrimRight(str);
	return str;
}

LPCSTR _CopyVal(LPCSTR src, xr_string& dst, char separator)
{
	LPCSTR p_str;
	ptrdiff_t n;
	p_str = strchr(src, separator);
	n = (p_str) ? (p_str - src) : xr_strlen(src);
	dst = src;
	dst = dst.erase(n, dst.length());
	return dst.c_str();
}

LPCSTR _GetItem(LPCSTR src, int index, xr_string& dst, char separator, LPCSTR def, bool trim)
{
	LPCSTR ptr;
	ptr = _SetPos(src, index, separator);
	if (ptr)
		_CopyVal(ptr, dst, separator);
	else
		dst = def;
	if (trim)
		_Trim(dst);
	return dst.c_str();
}

shared_str	_ListToSequence(const RStringVec& lst)
{
	xr_string out;
	if (lst.size())
	{
		out = *lst.front();
		for (RStringVec::const_iterator s_it = lst.begin() + 1; s_it != lst.end(); s_it++)
		{
			out += ",";
			out += **s_it;
		}
	}
	return shared_str(out.c_str());
}
