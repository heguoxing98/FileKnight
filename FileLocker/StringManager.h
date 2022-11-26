#ifndef _STRING_MANAGER_H_
#define _STRING_MANAGER_H_

#include <iostream>
#include <Windows.h>
#include <string>
#include "Common.h"

//
//Unicode������ʵ��
//https://www.cnblogs.com/1228073191Blog/p/7375972.html
//
//


namespace common {


/*
 * Сд��ĸת��д 
 */
#define UPCASE(var) (WCHAR )(((var) >= 'a' && (var) <= 'z' ? (var) - ('a' - 'A') : (var)))

class StringManager 
{
public:
	/*
	 * @brief: wstring to string 
	 */
	static std::string WS2S(const std::wstring& input, unsigned int code_page = CP_ACP);

	/*
	 * @brief: string to wstring 
	 */
	static std::wstring S2WS(const std::string& input, unsigned int code_page = CP_ACP);

	/*
	 * @brief: string to Unicode 
	 */
	static void NTAPI InitUnicode(UnicodePtr des, PCWSTR src);

	/*
	 * @brief:  �Ƚ�Unicode�Ƿ����
	 * @param:  s1 and s2 is compared
	 * @param:  insensitive, true-��Сд������
	 * @return: if true, s1==s2; if false, s1!=s2
	 */
	static bool NTAPI EqualUnicode(const UnicodePtr s1, const UnicodePtr s2, bool insensitive);

	/*
	 *@brief: compare unicode string 
	 */
	static long NTAPI CompareUnicode(UnicodePtr s1, UnicodePtr s2, bool insensitive);

	static std::wstring UnicodeToWs(const Unicode data);

};

}


#endif 
