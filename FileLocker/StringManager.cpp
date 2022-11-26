#include "StringManager.h"


std::string
common::StringManager::WS2S(const std::wstring& input, unsigned int code_page)
{
	std::string error = "WS2S error.";
	std::string output;

	int input_length = static_cast<int>(input.length());
	if (input_length == 0)
		return std::string();

	//
	//Compute the length of the buffer we'll need.
	//
	int char_count = WideCharToMultiByte(code_page, 0, input.data(), input_length, NULL, 0, NULL, NULL);
	if (char_count == 0)
		return error;


	output.resize(char_count);
	WideCharToMultiByte(code_page, 0, input.data(), input_length, &output[0], char_count, NULL, NULL);

	return output;

}

std::wstring
common::StringManager::S2WS(const std::string& input, unsigned int code_page)
{
	std::wstring error = L"S2WS error.";
	std::wstring output;

	if (input.empty())
		return std::wstring();

	int input_length = static_cast<int>(input.length());

	//
	//calcute the length of the buffer.
	//
	int char_count = MultiByteToWideChar(code_page, 0, input.data(), input_length, NULL, 0);
	if (char_count == 0)
		return error;


	output.resize(char_count);
	MultiByteToWideChar(code_page, 0, input.data(), input_length, &output[0], char_count);

	return output;

}


void NTAPI 
common::StringManager::InitUnicode(UnicodePtr des, PCWSTR src)
{
    SIZE_T size = 0;
    const SIZE_T max_size = (MAXUSHORT & ~1) - sizeof(UNICODE_NULL); 

    if (src)
    {
        size = wcslen(src) * sizeof(WCHAR);
       
        if (size > max_size)
            size = max_size;
        des->length = (USHORT)size;
        des->maximumLength = (USHORT)size + sizeof(UNICODE_NULL);
    }
    else
    {
        des->length = 0;
        des->maximumLength = 0;
    }

	//
	//重要!!!
	//这里是直接赋的指针，所以注意一下内存的状况，别失效了
	//
    des->buffer = (PWCHAR)src;
}

bool NTAPI
common::StringManager::EqualUnicode(const UnicodePtr s1, const UnicodePtr s2, bool insensitive)
{
    if (s1->length != s2->length) 
        return false;
    
    return !CompareUnicode(s1, s2, insensitive );
}

long NTAPI
common::StringManager::CompareUnicode(UnicodePtr s1, UnicodePtr s2, bool insensitive)
{
    int          len = 0;
    LONG         ret = 0;
    LPCWSTR      p1 = nullptr;
    LPCWSTR      p2 = nullptr;

    len = (min(s1->length, s2->length) / sizeof(WCHAR));
    p1 = s1->buffer;
    p2 = s2->buffer;

    if (insensitive)
    {
        while (len >= 0)
        {
            WCHAR v1 = UPCASE(p1[len]);
            WCHAR v2 = UPCASE(p2[len]);
            ret = LONG(v1 - v2);
             
            if (ret != 0)
                break;
            
            len = len - 1;
        }
    }
    else
    {
        while (len > 0)
        {
            ret = *p1++ - *p2++;
            len = len - 1;

            if (ret != 0)
                break;
        }
    }

    if (!ret) 
        ret = s1->length - s2->length;

    return ret;
}

std::wstring 
common::StringManager::UnicodeToWs(const Unicode data)
{
    return std::wstring(data.buffer, data.length / sizeof(wchar_t));
}


