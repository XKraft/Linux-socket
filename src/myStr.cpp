#include"../head/myStr.h"
char* Str_get()
{
	char c;
	char* str = NULL;
	int size = 0;
	while((c = getchar()) != '\n')
	{
		if(size == 0)
		{
			str = (char*)malloc(sizeof(char) * (size + 1));
			if(NULL == str) exit(-1);
			str[size] = c;
		}
		else
		{
			str = (char*)realloc(str, size + 1);
			if(NULL == str) exit(-1);
			str[size] = c;
		}
		++size;
	}
	str = (char*)realloc(str, size + 1);
	if(NULL == str) exit(-1);
	str[size] = '\0';
	return str;
	
}
// char* Str_convert(char* str)
// {
// #ifdef CONVERT
// 	for(int i = 0; i < strlen(str); ++i)
// 	{
// 		if(str[i] >= 'a' && str[i] <= 'z')
// 			str[i] = str[i] + ('A' - 'a');
// 		else if(str[i] >= 'A' && str[i] <= 'Z')
// 			str[i] = str[i] + ('a' - 'A');
// 	}
// 	return str;
// #endif
// 	free(str);
// 	char* p1 = "201180096";
// 	char* p2 = (char*)malloc(sizeof(char) * 10);
// 	for(int i = 0; i < 10; ++i)
// 		p2[i] = p1[i];
// 	return p2;
// }
