#include "jtalk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#include <io.h>
#else
//#include <sys/io.h>
#include <unistd.h>
#endif

#define MAX_PATH 260
#define MAX_LENGTH 1000

char voice[MAX_PATH];
char infile[MAX_PATH];
char outfile[MAX_PATH];
char message[MAX_PATH];

void usage_exit()
{
	fprintf(stderr, "jsay - Convert text to audible japanese speech\n");
	fprintf(stderr, "Usage: jsay [-v voice/?] [-o outfile(wav)] [-f in | message]\n\n");
	jtalkdll_copyright();
	exit(EXIT_FAILURE);
}

void error_exit(char *text)
{
	if (strlen(text)!='\0')
	{
		fprintf(stderr, "error: %s\n\n", text);
	}
	usage_exit();
}

size_t GetUTF8Step(char firstbyte)
{
	if ((firstbyte & 0x80) == 0x00)
	{
		return 1;
	}
	else if ((firstbyte & 0xe0) == 0xc0)
	{
		return 2;
	}
	else if ((firstbyte & 0xf0) == 0xe0)
	{
		return 3;
	}
	else if ((firstbyte & 0xf8) == 0xf0)
	{
		return 4;
	}
	else if ((firstbyte & 0xfc) == 0xf8)
	{
		return 5;
	}
	else if ((firstbyte & 0xfe) == 0xfc)
	{
		return 6;
	}
	return 0;
}

bool check_fullpath(char*path)
{
	if (path == NULL || *path == '\0')
	{
		return false;
	}
	char *p = path;
#if defined(_WIN32)
	char ch = *p++;
	if ((ch < 'A' || ch > 'Z') && (ch < 'a' || ch > 'z'))
	{
		return false;
	}
	if (*p++ != ':')
	{
		return false;
	}
	do
	{
		if (*p++ != '\\')
		{
			return false;
		}
		do
		{
			if ((*p >= 0x81 && *p <= 0x9f)
				|| (*p >= 0xe0 && *p <= 0xfc))
			{
				p++;
			}
			p++;
		} while (*p != '\0'&&*p != '\\');
	} while (*p != '\0');
#else
	do
	{
		if (*p != '/')
		{
			return false;
		}
		do
		{
			size_t step = GetUTF8Step(*p);
			if (step == 0)
			{
				return false;
			}
			p += step;
		} while (*p != '\0'&&*p != '/');
	} while (*p != '\0');
#endif
	return true;
}

int get_option(int argc, char *argv[])
{
	message[0] = '\0';
	for (int i = 1; i < argc; i++)
	{
		char *str = argv[i];
		if (str[0] == '-' || str[0] == '/')
		{
			switch (str[1])
			{
			case 'v':
				if (strlen(argv[i]) > 2)
				{
					strcpy(voice, &argv[i][2]);
				}
				else
				{
					if (i + 1 >= argc)
					{
						error_exit("option requires an argument -- v");
					}
					strcpy(voice, argv[++i]);
				}
				break;

			case 'o':
				if (strlen(argv[i]) > 2)
				{
					strcpy(outfile, &argv[i][2]);
				}
				else
				{
					if (i + 1 >= argc)
					{
						error_exit("option requires an argument -- o");
					}
					strcpy(outfile, argv[++i]);
				}
				break;

			case 'f':
				if (strlen(argv[i]) > 2)
				{
					strcpy(infile, &argv[i][2]);
				}
				else
				{
					if (i + 1 >= argc)
					{
						error_exit("option requires an argument -- f");
					}
					strcpy(infile, argv[++i]);
				}
				break;

			case 'h':
				usage_exit();
				break;

			default:
				fprintf(stderr, "invalid option -- %c\n", str[1]);
				usage_exit();
				break;
			}
		}
		else
		{
			size_t len = strlen(message);
			if (len + (len>0 ? 1 : 0) + strlen(str) + 1> MAX_PATH)
			{
				error_exit("too long string.");
			}
			if (check_fullpath(str))
			{
				strcpy(infile, str);
			}
			else
			{
				if (len > 0)
				{
					strcat(message, " ");
				}
				strcat(message, str);
			}
		}
	}
	return 0;
}

void print_voice_list(OpenJTalk *openjtalk)
{
	char padding[MAX_PATH];
	HtsVoiceFilelist *result = openjtalk_getHTSVoiceList(openjtalk);
	size_t length = 0;

	for (HtsVoiceFilelist*list = result; list != NULL; list = list->succ)
	{
		size_t len = strlen(list->name);
		if (length < len)
		{
			length = len;
		}
	}
	int tab_len = 4;
	length = (length / tab_len) * tab_len + tab_len;
	for (HtsVoiceFilelist*list = result; list != NULL; list = list->succ)
	{
		size_t diff = length - strlen((char*)list->name);
		strcpy(padding, " ");
		while (--diff)
		{
			strcat(padding, " ");
		}
		printf("%s%s%s\n", (char*)list->name, padding, (char*)list->path);
	}
}

char *remove_cr_lf(char*str)
{
	char *p = str;
	while (*p != '\0')
	{
		if (*p == '\x0d')
		{
			*p = '\0';
			break;
		}
		if (*p == '\x0a')
		{
			*p = '\0';
			break;
		}
		p++;
	}
	return str;
}


int main(int argc, char *argv[])
{
	get_option(argc, argv);

	OpenJTalk *openjtalk = openjtalk_initialize(NULL,NULL,NULL);

exit_success:
	openjtalk_clear(openjtalk);
	exit(EXIT_SUCCESS);
}
