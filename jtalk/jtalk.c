﻿#ifndef JTALK_C
#define JTALK_C
#ifdef __cplusplus
#define JTALK_C_START \
	extern "C"        \
	{
#define JTALK_C_END }
#else
#define JTALK_C_START
#define JTALK_C_END
#endif

JTALK_C_START;

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
//#if defined(_WIN32) && !defined(__CYGWIN__)
//#define ICONV_ENABLE
#define WINDOWS_PORTAUDIO
#elif defined(__MINGW32__)
#include <comip.h>
#else
#define ICONV_ENABLE
#endif

/*****************************************************************
** インクルード
*/
#include "openjtalk.h"
#include "jtalk.h"
#include <locale.h>
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#include <malloc.h>
#include <direct.h>
#include <stdarg.h>
#else
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <wchar.h>
#endif

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#include "shlwapi.h"
#pragma comment(lib, "shlwapi.lib")
#endif

#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#ifdef WINDOWS_PORTAUDIO
#include <portaudio.h>
#else
#error "WinOut関数による処理はまだ実装していません。"
#pragma comment(lib, "winmm.lib")
#endif
#else
#include <portaudio.h>
#endif

#if defined(ICONV_ENABLE)
#include <iconv.h>
#endif

/*****************************************************************
** 型
*/

// エンコード型
typedef enum
{
	OPENJTALKCHARSET_UTF_8,
	OPENJTALKCHARSET_SHIFT_JIS,
	OPENJTALKCHARSET_UTF_16,
} OpenjtalkCharsets;

// 音声データ
typedef struct speakData_t
{
	short *data;
	size_t length;
	size_t sampling_frequency;
	bool stop;
	bool pause;
	bool paused;
	void (*onFinished)(void);
#if (!defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)) || defined(WINDOWS_PORTAUDIO)
	PaStream *stream;
#else
#endif
} SpeakData;

// 主データ
typedef struct OpenJTalk_tag
{
	Open_JTalk *open_jtalk;

	// 現在の辞書ディレクトリのパス
	char dn_dic_path[MAX_PATH];

	// 現在の音響モデルファイルディレクトリのパス
	char dn_voice_dir_path[MAX_PATH];

	// 現在の音響モデルファイルのパス
	char fn_voice_path[MAX_PATH];

	// 初期値辞書ディレクトリのパス
	char dn_dic_path_init[MAX_PATH];

	// 初期値音響モデルファイルのパス
	char fn_voice_path_init[MAX_PATH];

	// 初期音響モデルファイルディレクトリのパス
	char dn_voice_dir_path_init[MAX_PATH];

	// コマンドライン・オプション
	double gv_weight0;
	double gv_weight1;
	double msd_threshold;
	size_t sampling_frequency;
	size_t fperiod;
	double alpha;
	double beta;
	double speed;
	double additional_half_tone;
	double volume;

	// エラー内容を表す番号
	OpenjtalkErrors errorCode;

} OpenJTalk;

/*****************************************************************
** 文字列定数
*/
#if defined(_WIN32)
const char *G_SLASH_CHAR = "\\";
#else
const char *G_SLASH_CHAR = "/";
#endif
const char *G_FORWARD_SLASH_CHAR = "/";
const char *G_BACKSLASH_CHAR = "\\";

const char *G_VOICE_DEFAULT = "mei_normal.htsvoice";
const char *G_VOICE_EXT = ".htsvoice";
const char *G_VOICE_WILDCARD = "*.htsvoice";
const char *G_INI_NAME = "config.ini";
const char *G_SECTION_NAME = u8"open_jtalk_config";
const char *G_DEFAULT_DIC_DIR_NAMES[] = {"dic_utf_8*", "dic*", "open_jtalk_dic_utf_8-*", NULL};
const char *G_DEFAULT_VOICE_DIR_NAMES[] = {"voice", "voice*", "hts_voice*", NULL};

/*****************************************************************
** 大域変数
*/

// 出力冗長
bool g_verbose = false;

// データのインストール先
char g_data_install_path[MAX_PATH];

// 実行ファイルのパス
char g_current_path[MAX_PATH];

// 設定ファイルのパス
char g_ini_path[MAX_PATH];

// 設定ファイルの存在するディレクトリ
char g_ini_dir[MAX_PATH];

// 初期値もしくは設定ファイルによる音響モデルファイル名
//char g_voice_ini[MAX_PATH];

// 初期値もしくは設定ファイルによる音響モデルファイルディレクトリ名
//char g_voice_dir_ini[MAX_PATH];

// 初期値もしくは設定ファイルによる辞書ディレクトリ名
//char g_dic_dir_ini[MAX_PATH];

// 音声データ

// 最近のエラー（OpenJTalkオブジェクトが利用できないときのため）
OpenjtalkErrors g_lastError = OPENJTALKERROR_NO_ERROR;

#if defined(_WIN32)
// このdll自身の場所
char g_dll_path[MAX_PATH];
#endif

/*****************************************************************
** 前方宣言
*/

bool check_dic_utf_8(const char *path);
bool get_ini_data(OpenJTalk *oj);
bool normalize_back_slash(const char *path, char *dest);
bool normalize_forward_slash(const char *path, char *dest);
bool set_default_dic_path(OpenJTalk *oj);
bool set_default_voice(OpenJTalk *oj);
bool set_default_voice_dir_path(OpenJTalk *oj);
bool set_dic_path(OpenJTalk *oj, const char *path);
bool set_ini_path(OpenJTalk *oj, const char *dir);
bool set_nearby_dic_path(OpenJTalk *oj, const char *path);
bool set_nearby_voice_dir_path(OpenJTalk *oj, const char *path);
bool set_voice(OpenJTalk *oj, const char *path);
bool set_voice_dir_path(OpenJTalk *oj, const char *path);
char *sjistou8_path(const char *source, char *dest);
char *u8tosjis(const char *source);

/*****************************************************************
** デバッグ用関数
*/

void writelog(char *str)
{
	FILE *log;
	char *LOG_FILE = "./log.txt";
	if ((log = fopen(LOG_FILE, "a")) == NULL)
	{
		exit(EXIT_FAILURE);
	}
	fputs(str, log);
	fclose(log);
}

void console_message_integer(const char *mes, int num)
{
#if defined(_WIN32)
	char *temp = u8tosjis(mes);
	if (temp == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return;
	}
	fprintf(stderr, temp, num);
	free(temp);
#else
	fprintf(stderr, mes, num);
#endif
}

void console_message_float(const char *mes, double num)
{
#if defined(_WIN32)
	char *temp = u8tosjis(mes);
	if (temp == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return;
	}
	fprintf(stderr, temp, num);
	free(temp);
#else
	fprintf(stderr, mes, num);
#endif
}

void console_message_string(const char *mes, char *str)
{
#if defined(_WIN32)
	char *temp1 = u8tosjis(mes);
	if (temp1 == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return;
	}
	char *temp2 = u8tosjis(str);
	if (temp2 == NULL)
	{
		free(temp1);
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return;
	}
	fprintf(stderr, temp1, temp2);
	free(temp2);
	free(temp1);
#else
	fprintf(stderr, mes, str);
#endif
}

void console_message(const char *mes)
{
#if defined(_WIN32)
	char *temp = u8tosjis(mes);
	if (temp == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return;
	}
	fprintf(stderr, temp);
	free(temp);
#else
	fprintf(stderr, "%s\n", mes);
#endif
}

/*****************************************************************
** 単純補助関数
*/

void clear_path_string(char *str, size_t len)
{
	memset(str, '\0', sizeof(char) * len);
}

void clear_path_stringU16(char16_t *str, size_t len)
{
	memset(str, L'\0', sizeof(char16_t) * len);
}

size_t strlenU16(const char16_t *s)
{
	char16_t *p = (char16_t *)s;
	size_t c = 0;
	while (*p++)
	{
		c++;
	}
	return c;
}

char16_t *strcpyU16(char16_t *s1, const char16_t *s2)
{
	if (s1 == NULL || s2 == NULL)
	{
		return s1;
	}
	char16_t *p1 = s1;
	char16_t *p2 = (char16_t *)s2;
	do
	{
		*p1++ = (*p2) ? *p2++ : u'\0';
	} while (*p2);
	return s1;
}

char16_t *strncpyU16(char16_t *s1, const char16_t *s2, size_t n)
{
	char16_t *result = s1;
	if (s1 == NULL || s2 == NULL || n == 0)
	{
		return result;
	}
	char16_t *p1 = s1;
	char16_t *p2 = (char16_t *)s2;
	while (n--)
	{
		*p1++ = (*p2) ? *p2++ : u'\0';
	}
	return result;
}

size_t GetUTF8Length(char firstbyte)
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

size_t GetSJISLength(char firstbyte)
{
	if (((unsigned char)firstbyte >= 0x81 && (unsigned char)firstbyte <= 0x9f) || ((unsigned char)firstbyte >= 0xe0 && (unsigned char)firstbyte <= 0xfc))
	{
		return 2;
	}
	else
	{
		return 1;
	}
}

/******************************************************************
** オブジェクト管理関数
*/

// 初期化本体
bool JTalkData_initialize(OpenJTalk *oj)
{
	if (!oj)
	{
		return false;
	}

	// 文字列クリア
	clear_path_string(oj->fn_voice_path, MAX_PATH);
	clear_path_string(oj->dn_dic_path, MAX_PATH);
	clear_path_string(oj->dn_voice_dir_path, MAX_PATH);

	//  引数の辞書ディレクトリを設定する
	if (strlen(oj->dn_dic_path_init) > 0)
	{
		set_dic_path(oj, oj->dn_dic_path_init);
	}

	// 引数の音響モデルディレクトリを設定する
	if (strlen(oj->dn_voice_dir_path_init) > 0)
	{
		set_voice_dir_path(oj, oj->dn_voice_dir_path_init);
	}

	//  引数の音響モデルファイルを設定する
	if (strlen(oj->fn_voice_path_init) > 0)
	{
		set_voice(oj, oj->fn_voice_path_init);
	}

	// カレントフォルダに設定ファイルがあれば、設定を解釈する
	bool current_ini = set_ini_path(oj, g_current_path);
	if (current_ini)
	{
		get_ini_data(oj);
	}
	else
	{
		if (g_verbose)
		{
			console_message(u8"カレントフォルダに設定ファイルはありません。\n");
		}
	}

	// 未確定ならば、カレントディレクトリの周囲の辞書を探す
	if (strlen(oj->dn_dic_path) == 0)
	{
		set_nearby_dic_path(oj, g_current_path);
	}

	// 未確定ならば、カレントディレクトリの周囲の音響モデルフォルダを探す
	if (strlen(oj->dn_voice_dir_path) == 0)
	{
		set_nearby_voice_dir_path(oj, g_current_path);
	}

	//  未確定ならば、改めて引数の音響モデルファイルで設定する
	if (strlen(oj->fn_voice_path) == 0)
	{
		if (strlen(oj->fn_voice_path_init) > 0)
		{
			set_voice(oj, oj->fn_voice_path_init);
		}
	}

// Windowsだけの特別の処理
#if defined(_WIN32)
	// カレントフォルダに設定ファイルがなかったときだけ解釈する
	if (!current_ini)
	{
		// 共有ライブラリのあるフォルダに設定ファイルがあれば、設定を解釈する
		if (set_ini_path(oj, g_dll_path))
		{
			get_ini_data(oj);
		}
		else
		{
			if (g_verbose)
			{
				console_message(u8"共有ライブラリのあるフォルダに設定ファイルはありません。\n");
			}
		}

		// 未確定ならば、カレントディレクトリの周囲の辞書を探す
		if (strlen(oj->dn_dic_path) == 0)
		{
			set_nearby_dic_path(oj, g_dll_path);
		}

		// 未確定ならば、カレントディレクトリの周囲の音響モデルフォルダを探す
		if (strlen(oj->dn_voice_dir_path) == 0)
		{
			set_nearby_voice_dir_path(oj, g_dll_path);
		}

		//  未確定ならば、改めて引数の音響モデルファイルで設定する
		if (strlen(oj->fn_voice_path) == 0)
		{
			if (strlen(oj->fn_voice_path_init) > 0)
			{
				set_voice(oj, oj->fn_voice_path_init);
			}
		}
	}
#endif

	// データフォルダマクロ変数INSTALL_PATH
	// スラッシュをバックスラッシュに整える
	clear_path_string(g_data_install_path, MAX_PATH);
	if (strlen(INSTALL_PATH) < MAX_PATH)
	{
#if defined(_WIN32) && !defined(__MINGW32__)
		char str_temp[MAX_PATH];
		sjistou8_path(INSTALL_PATH, str_temp);
		normalize_back_slash(str_temp, g_data_install_path);
#elif defined(__MINGW32__)
		char str_temp[MAX_PATH];
		sjistou8_path(INSTALL_PATH, str_temp);
		//u8tou8_path(INSTALL_PATH, str_temp);
		normalize_back_slash(str_temp, g_data_install_path);
#else
		strcpy(g_data_install_path, INSTALL_PATH);
#endif
		if (g_verbose)
		{
			console_message_string(u8"ディレクトリ区切り文字の正規化： %s\n", g_data_install_path);
		}
	}

	// まだ未確定ならば、省略値にする
	if (strlen(oj->dn_dic_path) == 0)
	{
		set_default_dic_path(oj);
	}

	// まだ未確定ならば、省略値にする
	if (strlen(oj->dn_voice_dir_path) == 0)
	{
		set_default_voice_dir_path(oj);
	}

	// まだ未確定ならば、省略値にする
	if (strlen(oj->fn_voice_path) == 0)
	{
		set_default_voice(oj);
	}

	// 最終的に、辞書ディレクトリが確定しなかったら
	if (strlen(oj->dn_dic_path) == 0)
	{
		if (g_verbose)
		{
			console_message(u8"設定ファイルの辞書ディレクトリが見つかりません。\n");
		}
		return false;
	}

	// 最終的に、音響モデルファイルが確定しなかったら
	if (strlen(oj->fn_voice_path) == 0)
	{
		if (g_verbose)
		{
			console_message(u8"音響モデルファイルが見つかりません。\n");
		}
		return false;
	}

	if (g_verbose)
	{
		console_message("\n");
	}
	return true;
}

void JTalkData_Clear(OpenJTalk *oj)
{
	if (!oj)
	{
		return;
	}
	free(oj);
}

/******************************************************************
** 文字列変換関数
*/

#if defined(ICONV_ENABLE)
typedef enum
{
	CS_SHIFT_JIS,
	CS_UTF_8,
	CS_UTF_16LE
} Charset;

char *charset_string(Charset charset)
{
	switch (charset)
	{
	case CS_SHIFT_JIS:
		return "SHIFT_JIS";
	case CS_UTF_8:
		return "UTF-8";
	case CS_UTF_16LE:
		return "UTF-16LE";
	default:
		return "";
	}
}

char *convert_charset(const char *in_str, Charset from, Charset to)
{
	char *cs_from = charset_string(from);
	char *cs_to = charset_string(to);
	iconv_t conv = iconv_open(cs_to, cs_from);
	if (conv == (iconv_t)-1)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	size_t n_in = (from == CS_UTF_16LE) ? strlenU16((char16_t *)in_str) * sizeof(char16_t) : strlen(in_str) * sizeof(char);
	size_t length = n_in * 6;
	size_t type_size = (to == CS_UTF_16LE) ? sizeof(char16_t) : sizeof(char);
	length *= type_size;
	size_t n_out = length;
	char *in = (char *)in_str;
	char *str_out = (char *)malloc(length + type_size);
	char *out = (char *)str_out;
	if (iconv(conv, (char **)&in, &n_in, &out, &n_out) == (size_t)-1)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	str_out[length - n_out] = 0;
	if (type_size == sizeof(char16_t))
	{
		str_out[length - n_out + 1] = 0;
	}
	return str_out;
}

char *convert_charset_path(const char *in_str, Charset from, char *out_str, Charset to)
{
	if (out_str == NULL || in_str == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char *temp = convert_charset(in_str, from, to);
	if (temp == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	if (to == CS_UTF_16LE)
	{
		strncpyU16((char16_t *)out_str, (char16_t *)temp, MAX_PATH - 1);
	}
	else
	{
		strncpy(out_str, temp, MAX_PATH - 1);
	}
	free(temp);
	return out_str;
}

char16_t *u8tou16(const char *in_str)
{
	return (char16_t *)convert_charset(in_str, CS_UTF_8, CS_UTF_16LE);
}

char16_t *sjistou16(const char *in_str)
{
	return (char16_t *)convert_charset(in_str, CS_SHIFT_JIS, CS_UTF_16LE);
}

char *sjistou8(const char *in_str)
{
	return (char *)convert_charset(in_str, CS_SHIFT_JIS, CS_UTF_8);
}

char *u16tou8(const char16_t *in_str)
{
	return (char *)convert_charset((char *)in_str, CS_UTF_16LE, CS_UTF_8);
}

char *u16tosjis(const char16_t *in_str)
{
	return (char *)convert_charset((char *)in_str, CS_UTF_16LE, CS_SHIFT_JIS);
}

char *u8tosjis(const char *in_str)
{
	return (char *)convert_charset((char *)in_str, CS_UTF_8, CS_SHIFT_JIS);
}

char16_t *u8tou16_path(const char *in_str, char16_t *dest)
{
	return (char16_t *)convert_charset_path((char *)in_str, CS_UTF_8, (char *)dest, CS_UTF_16LE);
}

char16_t *sjistou16_path(const char *in_str, char16_t *dest)
{
	return (char16_t *)convert_charset_path((char *)in_str, CS_SHIFT_JIS, (char *)dest, CS_UTF_16LE);
}

char *u16tou8_path(const char16_t *in_str, char *dest)
{
	return (char *)convert_charset_path((char *)in_str, CS_UTF_16LE, (char *)dest, CS_UTF_8);
}

char *sjistou8_path(const char *in_str, char *dest)
{
	return (char *)convert_charset_path((char *)in_str, CS_SHIFT_JIS, (char *)dest, CS_UTF_8);
}

char *u16tosjis_path(const char16_t *in_str, char *dest)
{
	return (char *)convert_charset_path((char *)in_str, CS_UTF_16LE, (char *)dest, CS_SHIFT_JIS);
}

char *u8tosjis_path(const char *in_str, char *dest)
{
	return (char *)convert_charset_path((char *)in_str, CS_UTF_8, (char *)dest, CS_SHIFT_JIS);
}

#else
// Windows 専用

char *u16tou8(const char16_t *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)source, -1, (char *)NULL, 0, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char *buff = (char *)malloc(size + 1);
	if (!buff)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	size = WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)source, -1, buff, size, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		if (buff)
		{
			free(buff);
		}
		return NULL;
	}
	return buff;
}

char *u16tou8_path(const char16_t *source, char *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)source, -1, dest, 0, NULL, NULL);
	if (size == 0 || size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	size = WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)source, -1, dest, MAX_PATH - 1, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return dest;
}

char *u16tosjis(const char16_t *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)source, -1, (char *)NULL, 0, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char *buff = (char *)malloc(size + 1);
	if (!buff)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)source, -1, buff, size, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		if (buff)
		{
			free(buff);
		}
		return NULL;
	}
	return buff;
}

char *u16tosjis_path(const char16_t *source, char *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)source, -1, dest, 0, NULL, NULL);
	if (size == 0 || size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)source, -1, dest, MAX_PATH - 1, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return dest;
}

char16_t *u8tou16(const char *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)NULL, 0);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char16_t *buff = (char16_t *)malloc(size * sizeof(char16_t) + 1);
	if (!buff)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)buff, size);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		if (buff)
		{
			free(buff);
		}
		return NULL;
	}
	return buff;
}

char16_t *u8tou16_path(const char *source, char16_t *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)dest, 0);
	if (size == 0 || size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)dest, MAX_PATH - 1);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return dest;
}

char16_t *sjistou16(const char *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = MultiByteToWideChar(CP_ACP, 0, source, -1, (LPWSTR)NULL, 0);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char16_t *buff = (char16_t *)malloc(size * sizeof(char16_t) + 1);
	if (!buff)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	MultiByteToWideChar(CP_ACP, 0, source, -1, (LPWSTR)buff, size);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		if (buff)
		{
			free(buff);
		}
		return NULL;
	}
	return buff;
}

char16_t *sjistou16_path(const char *source, char16_t *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = MultiByteToWideChar(CP_ACP, 0, source, -1, (LPWSTR)dest, 0);
	if (size == 0 || size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	size = MultiByteToWideChar(CP_ACP, 0, source, -1, (LPWSTR)dest, MAX_PATH - 1);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return dest;
}

char *u8tosjis(const char *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)NULL, 0);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char16_t *buff = (char16_t *)malloc(size * sizeof(char16_t) + 1);
	if (!buff)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)buff, size);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		free(buff);
		return NULL;
	}
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)buff, -1, NULL, 0, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		free(buff);
		return NULL;
	}
	char *temp = (char *)malloc(size * sizeof(char) + 1);
	if (!temp)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		free(buff);
		return NULL;
	}
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)buff, -1, temp, size, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		free(temp);
		free(buff);
		return NULL;
	}
	free(buff);
	return temp;
}

char *u8tosjis_path(const char *source, char *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)NULL, 0);
	if (size == 0 || size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char16_t temp[MAX_PATH];
	size = MultiByteToWideChar(CP_UTF8, 0, source, -1, (LPWSTR)temp, MAX_PATH - 1);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)temp, -1, NULL, 0, NULL, NULL);
	if (size == 0 || size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	size = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)temp, -1, dest, MAX_PATH - 1, NULL, NULL);
	if (size == 0)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return dest;
}

char *sjistou8(const char *source)
{
	char16_t *temp = sjistou16(source);
	if (!temp)
	{
		return NULL;
	}
	char *result = u16tou8(temp);
	free(temp);
	return result;
}

char *sjistou8_path(const char *source, char *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char *temp = sjistou8(source);
	if (!temp)
	{
		return NULL;
	}
	if (strlen(temp) >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		free(temp);
		return NULL;
	}
	strcpy(dest, temp);
	free(temp);
	return dest;
}
#endif /* ELSE ICONV_ENABLE */

char *sjistosjis(const char *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char *temp = (char *)malloc(strlen(source) + 1);
	if (!temp)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	return strcpy(temp, source);
}

char *sjistosjis_path(const char *source, char *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = strlen(source);
	if (size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return strcpy(dest, source);
}

char *u8tou8(const char *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char *temp = (char *)malloc(strlen(source) + 1);
	if (!temp)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	return strcpy(temp, source);
}

char *u8tou8_path(const char *source, char *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = strlen(source);
	if (size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return strcpy(dest, source);
}

char16_t *u16tou16(const char16_t *source)
{
	if (source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	char16_t *temp = (char16_t *)malloc(strlenU16(source) * sizeof(char16_t) + 1);
	if (!temp)
	{
		g_lastError = OPENJTALKERROR_MALLOC_ERROR;
		return NULL;
	}
	return strcpyU16(temp, source);
}

char16_t *u16tou16_path(const char16_t *source, char16_t *dest)
{
	if (dest == NULL || source == NULL)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	int size = strlenU16(source);
	if (size >= MAX_PATH)
	{
		g_lastError = OPENJTALKERROR_CHARSET_CONVERTING_ERROR;
		return NULL;
	}
	return strcpyU16(dest, source);
}

/*****************************************************************
** split_path の実装
*/

char *point_basename(const char *path)
{
	const char *b = path;
	const char *p;
	for (p = b; *p; p++)
	{
		size_t step = GetUTF8Length(*p);
		if (step > 1)
		{
			p += step - 1;
			continue;
		}
		if ((*p == *G_BACKSLASH_CHAR) || (*p == *G_FORWARD_SLASH_CHAR))
		{
			b = p + 1;
		}
	}
	return (char *)b;
}

bool has_driveletter(const char *path)
{
#if defined(_WIN32)
	char ch = path[0];
	return path[1] == ':' && ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'));
#else
	return false;
#endif
}

char *get_drive(const char *path, char *dest)
{
	if (dest == NULL)
	{
		return NULL;
	}
	clear_path_string(dest, MAX_PATH);
#if defined(_WIN32)
	char ch = path[0];
	if (path[1] == ':' && ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')))
	{
		dest[0] = ch;
		dest[1] = ':';
		dest[2] = '\0';
	}
#endif
	return dest;
}

char *get_dir(const char *path, char *dest)
{
	if (dest == NULL)
	{
		return NULL;
	}
	char temp[MAX_PATH];
	if (has_driveletter(path))
	{
		strcpy(temp, path + 2);
	}
	else
	{
		strcpy(temp, path);
	}
	char *p = point_basename(temp);
	if (p != NULL)
	{
		*p = '\0';
	}
	clear_path_string(dest, MAX_PATH);
	strcpy(dest, temp);
	return dest;
}

char *get_fname(const char *path, char *dest)
{
	if (dest == NULL)
	{
		return NULL;
	}
	char temp[MAX_PATH];
	char *p = temp;
	strcpy(temp, point_basename(path));
	while (strchr(temp, '.') != NULL)
	{
		if (strlen(p) < 2)
		{
			break;
		}
		if (*p)
		{
			p += strlen(p);
			while (*p != '.')
			{
				--p;
			}
			if (!*p)
			{
				--p;
				break;
			}
			else
			{
				*p = '\0';
			}
		}
	}
	clear_path_string(dest, MAX_PATH);
	strcpy(dest, temp);
	return dest;
}

char *get_ext(const char *path, char *dest)
{
	if (dest == NULL)
	{
		return NULL;
	}
	char temp[MAX_PATH];
	strcpy(temp, point_basename(path));
	char *e = strrchr(temp, '.');
	clear_path_string(dest, MAX_PATH);
	if (e != NULL)
	{
		strcpy(dest, e);
	}
	return dest;
}

void split_path(const char *path, char *drive, char *dir, char *fname, char *ext)
{
	if (path == NULL)
	{
		return;
	}
	if (drive != NULL)
	{
		get_drive(path, drive);
	}
	if (dir != NULL)
	{
		get_dir(path, dir);
	}
	if (fname != NULL)
	{
		get_fname(path, fname);
	}
	if (ext != NULL)
	{
		get_ext(path, ext);
	}
}

/*****************************************************************
** ファイル・ディレクトリ関連汎用関数
*/

char *add_slash(char *path)
{
	size_t len = strlen(path);
	if (len == 0)
	{
		return NULL;
	}
	char *p = point_basename((const char *)path);
	if (*p != '\0')
	{
		strcat(path, G_SLASH_CHAR);
	}
	return path;
}

char *get_dir_path(const char *path, char *dist)
{
#if defined(_WIN32)
	char drive[MAX_PATH];
	if (!get_drive(path, drive))
	{
		return NULL;
	}
	char dir[MAX_PATH];
	if (!get_dir(path, dir))
	{
		return NULL;
	}
	if (strlen(drive) + strlen(dir) + 1 > MAX_PATH)
	{
		return NULL;
	}
	return strcat(strcpy(dist, drive), dir);
#else
	return get_dir(path, dist);
#endif
}

bool exists_file(const char *name)
{
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	char16_t temp[MAX_PATH];
	char16_t *res = u8tou16_path(name, temp);
	if (res == NULL)
	{
		return false;
	}
	if (PathFileExistsW(temp) == false)
	{
		return false;
	}
	return !PathIsDirectoryW(temp);
#elif defined(__MINGW32__)
	char temp2[MAX_PATH];
	char *res = u8tosjis_path(name, temp2);
	if (res == NULL)
	{
		return false;
	}
	int fd = open(temp2, O_RDONLY);
	if (fd <= 0)
	{
		return false;
	}
	close(fd);
	if (fd <= 0)
	{
		return false;
	}
	return true;
#else
	int fd = open(name, O_RDONLY);
	if (fd <= 0)
	{
		return false;
	}
	close(fd);
	if (fd <= 0)
	{
		return false;
	}
	return true;
#endif
}

bool exists_dir(const char *path)
{
	size_t len = strlen(path);
	if (len == 0)
	{
		return false;
	}
	if ((path[len - 1] == *G_BACKSLASH_CHAR) || (path[len - 1] == *G_FORWARD_SLASH_CHAR))
	{
		char temp[MAX_PATH];
		strcpy(temp, path);
		char *base = point_basename(temp);
		if (*base == '\0')
		{
			temp[len - 1] = '\0';
			path = temp;
		}
	}
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	char16_t temp2[MAX_PATH];
	if (u8tou16_path(path, temp2) == NULL)
	{
		return false;
	}
	if (PathFileExistsW(temp2) == false)
	{
		return false;
	}
	return PathIsDirectoryW(temp2);
#elif defined(__MINGW32__)
	char temp4[MAX_PATH];
	char *res = u8tosjis_path(path, temp4);
	if (res == NULL)
	{
		return false;
	}
	struct stat sb;
	if (stat(temp4, &sb) == -1)
	{
		return false;
	}
	if (S_ISDIR(sb.st_mode))
	{
		return true;
	}
	return false;
#else
	char temp3[MAX_PATH];
	normalize_forward_slash(path, temp3);
	struct stat sb;
	if (stat(temp3, &sb) == -1)
	{
		return false;
	}
	if (S_ISDIR(sb.st_mode))
	{
		return true;
	}
	return false;
#endif
}

#if (defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__))
typedef unsigned int ino_t;

struct dirent
{
	ino_t d_ino;
	char d_name[MAX_PATH];
};

typedef struct
{
	HANDLE h;
	WIN32_FIND_DATAA *fd;
	bool has_next;
	struct dirent entry;
} DIR;

DIR *opendir(const char *name)
{
	char *path;
	HANDLE h;
	WIN32_FIND_DATAA *fd;
	DIR *dir;
	size_t namlen;
	if (name == NULL)
	{
		return NULL;
	}
	if ((namlen = strlen(name)) == -1)
	{
		return NULL;
	}
	if ((name[namlen - 1] == *G_BACKSLASH_CHAR) || (name[namlen - 1] == *G_FORWARD_SLASH_CHAR))
	{
		path = (char *)_malloca(namlen + 2);
		strcpy_s(path, namlen + 2, name);
		path[namlen] = '*';
		path[namlen + 1] = '\0';
	}
	else
	{
		path = (char *)_malloca(namlen + 3);
		strcpy_s(path, namlen + 3, name);
		path[namlen] = *G_SLASH_CHAR;
		path[namlen + 1] = '*';
		path[namlen + 2] = '\0';
	}
	if ((fd = (WIN32_FIND_DATAA *)malloc(sizeof(WIN32_FIND_DATA))) == NULL)
	{
		return NULL;
	}
	if ((h = FindFirstFileA(path, fd)) == INVALID_HANDLE_VALUE)
	{
		free(fd);
		return NULL;
	}
	if ((dir = (DIR *)malloc(sizeof(DIR))) == NULL)
	{
		FindClose(h);
		free(fd);
		return NULL;
	}
	dir->h = h;
	dir->fd = fd;
	dir->has_next = true;
	return dir;
}

struct dirent *readdir(DIR *dir)
{
	char *cFileName;
	char *d_name;
	if (dir == NULL)
	{
		return NULL;
	}
	if (dir->fd == NULL)
	{
		return NULL;
	}
	if (!dir->has_next)
	{
		return NULL;
	}
	cFileName = dir->fd->cFileName;
	d_name = dir->entry.d_name;
	strcpy_s(d_name, _MAX_PATH, cFileName);
	dir->has_next = FindNextFileA(dir->h, dir->fd);
	return &dir->entry;
}

int closedir(DIR *dir)
{
	if (dir == NULL)
	{
		return -1;
	}
	if (dir->h && dir->h != INVALID_HANDLE_VALUE)
	{
		FindClose(dir->h);
	}
	if (dir->fd)
	{
		free(dir->fd);
	}
	free(dir);
	return 0;
}
#endif

size_t get_step_filechar(const unsigned char *s)
{
#if (defined(_WIN32))
	unsigned char c = s[0];
	if ((c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc))
	{
		c = s[1];
		if (c >= 0x40 && c <= 0xfc)
		{
			return 2;
		}
	}
	return 1;
#else
	return GetUTF8Length(*s);
#endif
}

bool wildcard_match_rec(const char *wildcard, const char *target)
{
	const char *pw = wildcard;
	const char *pt = target;
	while (true)
	{
		if (*pt == '\0')
		{
			while (*pw != '\0')
			{
				if (*pw++ != '*')
				{
					return false;
				}
			}
			return true;
		}
		else if (*pw == '\0')
		{
			return false;
		}
		size_t w_skip = get_step_filechar((unsigned char *)pw);
		size_t t_skip = get_step_filechar((unsigned char *)pt);
		if (*pw == '*')
		{
			if (*(pw + w_skip) == '\0')
			{
				return true;
			}
			else if (wildcard_match_rec(pw, pt + t_skip))
			{
				return true;
			}
			else
			{
				return wildcard_match_rec(pw + w_skip, pt);
			}
		}
		else if (*pw == '?' || (*pw == *pt))
		{
			pw += w_skip;
			pt += t_skip;
			continue;
		}
		else
		{
			return false;
		}
	}
}

bool wildcard_match(const char *wildcard, const char *target)
{
#if (defined(_WIN32) && !defined(__CYGWIN__))
	if (strcmp(wildcard, "*.*") == 0)
	{
		return true;
	}
#endif
	if (strcmp(wildcard, "*") == 0)
	{
		return true;
	}
	else
	{
		return wildcard_match_rec(wildcard, target);
	}
}

// 指定のパスから、指定のワイルドカード文字列にマッチするファイルもしくはディレクトリを探し、
// あれば結果をresultにかえす。
// resultにはパス文字列に十分な領域が確保されているとする
// path の最後にはスラッシュはない
// なお再帰的には探さない、
// 候補が複数あっても最初に見つけたもののみを返す
// このとき何を最初に見つけるかはreaddirの動作に依存する
bool search_directory_or_file(const char *path, const char *wildcard, bool isDirectory, char *result)
{
	if (path == NULL || strlen(path) == 0)
	{
		return false;
	}
	if (wildcard == NULL || strlen(wildcard) == 0)
	{
		return false;
	}
	if (strlen(path) + 1 > MAX_PATH)
	{
		return false;
	}
	DIR *dir;
	char opendirpath[MAX_PATH];
	clear_path_string(opendirpath, MAX_PATH);
#if defined(_WIN32)
	char *res = u8tosjis_path(path, opendirpath);
	if (res == NULL)
	{
		return false;
	}	
	if ((dir = opendir(opendirpath)) == NULL)
#else
	if ((dir = opendir(path)) == NULL)
#endif
	{
		return false;
	}
	char temp[MAX_PATH];
	struct dirent *entry;
	bool found = false;
	//size_t len = strlen(wildcard);
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}
		if (strlen(path) + 1 + strlen(entry->d_name) + 2 > MAX_PATH)
		{
			continue;
		}
		strcpy(temp, path);
		add_slash(temp);
		strcat(temp, entry->d_name);
		if ((isDirectory ? exists_dir : exists_file)(temp))
		{
			if (!wildcard_match(wildcard, entry->d_name))
			{
				continue;
			}
			strcpy(result, temp);
			found = true;
			break;
		}
	}
	closedir(dir);
	return found;
}

bool search_file(const char *path, const char *wildcard, char *result)
{
	return search_directory_or_file(path, wildcard, false, result);
}

bool search_directory(const char *path, const char *wildcard, char *result)
{
	return search_directory_or_file(path, wildcard, true, result);
}

bool search_file_recursive(const char *path, const char *wildcard, char *result, unsigned int *pc, const unsigned int max)
{
	if (strlen(path) + 1 > MAX_PATH)
	{
		return false;
	}
	DIR *dir;
	char opendirpath[MAX_PATH];
	clear_path_string(opendirpath, MAX_PATH);
#if defined(_WIN32)
	char *res = u8tosjis_path(path, opendirpath);
	if (res == NULL)
	{
		return false;
	}
	if ((dir = opendir(opendirpath)) == NULL)
#else
	if ((dir = opendir(path)) == NULL)
#endif
	{
		return false;
	}
	char temp[MAX_PATH];
	struct dirent *entry;
	bool found = false;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}
		if (strlen(path) + 1 + strlen(entry->d_name) + 2 > MAX_PATH)
		{
			continue;
		}
		strcpy(temp, path);
		strcat(temp, G_SLASH_CHAR);
		strcat(temp, entry->d_name);
		if (exists_dir(temp))
		{
			if (search_file_recursive(temp, wildcard, result, pc, max))
			{
				found = true;
				break;
			}
			if (*pc > max)
			{
				break;
			}
		}
		else
		{
			*pc += 1;
			if (*pc > max)
			{
				break;
			}

			if (!wildcard_match(wildcard, entry->d_name))
			{
				continue;
			}
			strcpy(result, temp);
			found = true;
			break;
		}
	}
	closedir(dir);
	return found;
}

void remove_filelist(HtsVoiceFilelist *list)
{
	HtsVoiceFilelist *prev = NULL;
	while (list != NULL)
	{
		if (prev != NULL)
		{
			free(prev);
		}
		if (list->path)
		{
			free(list->path);
		}
		if (list->name)
		{
			free(list->name);
		}
		prev = list;
		list = list->succ;
	}
	if (prev != NULL)
	{
		free(prev);
	}
}

unsigned int count_filelist(HtsVoiceFilelist *list)
{
	unsigned int result = 0;
	for (; list != NULL; list = list->succ)
	{
		result++;
	}
	return result;
}

unsigned int counter_of_file_or_directory_list;

void counter_reset_file_or_directory_list()
{
	counter_of_file_or_directory_list = 0;
}

HtsVoiceFilelist *get_file_or_directory_list(const char *path, const char *wildcard, bool isDirectory, OpenjtalkCharsets charset)
{
	HtsVoiceFilelist *top = NULL;
	HtsVoiceFilelist *list = NULL;
	if (strlen(path) + 1 > MAX_PATH)
	{
		return top;
	}
	DIR *dir;
	char opendirpath[MAX_PATH];
	clear_path_string(opendirpath, MAX_PATH);
#if defined(_WIN32)
	char *res = u8tosjis_path(path, opendirpath);
	if (res == NULL)
	{
		return NULL;
	}
	if ((dir = opendir(opendirpath)) == NULL)
#else
	if ((dir = opendir(path)) == NULL)
#endif
	{
		return top;
	}
	char temp[MAX_PATH];
	struct dirent *entry;
	while ((entry = readdir(dir)) != NULL)
	{
		if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
		{
			continue;
		}

		if (counter_of_file_or_directory_list > VOICESEARCHMAX)
		{
			continue;
		}
		counter_of_file_or_directory_list++;

		if (strlen(path) + 1 + strlen(entry->d_name) + 2 > MAX_PATH)
		{
			continue;
		}
		strcpy(temp, path);
		strcat(temp, G_SLASH_CHAR);
		strcat(temp, entry->d_name);
		if (exists_dir(temp))
		{
			HtsVoiceFilelist *res = get_file_or_directory_list((const char *)temp, wildcard, isDirectory, charset);
			if (res != NULL)
			{
				for (HtsVoiceFilelist *item = res; item != NULL; item = item->succ)
				{
					if (list != NULL)
					{
						list->succ = item;
					}
					else
					{
						top = item;
					}
					list = item;
				}
			}
		}
		if ((isDirectory ? exists_dir : exists_file)(temp))
		{
			if (!wildcard_match(wildcard, entry->d_name))
			{
				continue;
			}
			HtsVoiceFilelist *data = (HtsVoiceFilelist *)malloc(sizeof(HtsVoiceFilelist));
			data->succ = NULL;
			switch (charset)
			{
			case OPENJTALKCHARSET_SHIFT_JIS:
//data->charset = OPENJTALKCHARSET_SHIFT_JIS;
#if defined(_WIN32)
				data->path = u8tosjis(temp);
				data->name = u8tosjis(entry->d_name);
#else
				data->path = u8tosjis(temp);
				data->name = u8tosjis(entry->d_name);
#endif
				break;
			case OPENJTALKCHARSET_UTF_8:
//data->charset = OPENJTALKCHARSET_UTF_8;
#if defined(_WIN32)
				data->path = u8tou8(temp);
				data->name = u8tou8(entry->d_name);
#else
				data->path = u8tou8(temp);
				data->name = u8tou8(entry->d_name);
#endif
				break;
			case OPENJTALKCHARSET_UTF_16:
//data->charset = OPENJTALKCHARSET_UTF_16;
#if defined(_WIN32)
				data->pathU16 = u8tou16(temp);
				data->nameU16 = u8tou16(entry->d_name);
#else
				data->pathU16 = u8tou16(temp);
				data->nameU16 = u8tou16(entry->d_name);
#endif
				break;
			default:
				data->path = NULL;
				data->name = NULL;
			}
			if (top != NULL)
			{
				list->succ = data;
			}
			else
			{
				top = data;
			}
			list = data;
		}
	}
	closedir(dir);
	return top;
}

bool get_fullpath(const char *path, char *dest)
{
#if defined(_WIN32)
	if (_fullpath(dest, path, MAX_PATH) == NULL)
#else
	if (realpath(path, dest) == NULL)
#endif
	{
		clear_path_string(dest, MAX_PATH);
		return false;
	}
	return true;
}

bool is_name_only(const char *path)
{
	if (strlen(path) == 0)
	{
		return false;
	}
	char drive[MAX_PATH];
	char current_dir[MAX_PATH];
	char fname[MAX_PATH];
	char ext[MAX_PATH];
	split_path(path, drive, current_dir, fname, ext);
#if defined(_WIN32)
	if (strlen(drive) != 0)
	{
		return false;
	}
#endif
	if (strlen(current_dir) != 0)
	{
		return false;
	}
	if (strlen(fname) == 0)
	{
		return false;
	}
	if (strlen(ext) != 0)
	{
		char temp[MAX_PATH];
		strcpy(temp, ext);
		char *p;
		for (p = temp; *p; p++)
		{
			*p = tolower(*p);
		}
		if (strcmp(temp, G_VOICE_EXT) == 0)
		{
			return false;
		}
	}
	return true;
}

bool is_relative(const char *path)
{
	if (strlen(path) == 0)
	{
		return true;
	}
	char drive[MAX_PATH];
	char current_dir[MAX_PATH];
	split_path(path, drive, current_dir, NULL, NULL);
#if defined(_WIN32)
	if (strlen(drive) != 0)
	{
		return false;
	}
#endif
	if (strlen(current_dir) == 0)
	{
		return true;
	}
	if ((current_dir[0] != *G_BACKSLASH_CHAR) && (current_dir[0] != *G_FORWARD_SLASH_CHAR))
	{
		return true;
	}
	return false;
}

bool change_extension_u8(const char *path, const char *ext, char *dest)
{
	clear_path_string(dest, MAX_PATH);
	if (path == NULL || ext == NULL || dest == NULL)
	{
		return false;
	}
	char drive[MAX_PATH];
	char dir[MAX_PATH];
	char fname[MAX_PATH];
	split_path(path, drive, dir, fname, NULL);
	if (strlen(drive) + strlen(dir) + strlen(fname) + strlen(ext) + 1 > MAX_PATH)
	{
		return false;
	}
	strcat(strcat(strcat(strcpy(dest, drive), dir), fname), ext);
	return true;
}

bool normalize_back_slash(const char *path, char *dest)
{
	if (path == NULL || dest == NULL)
	{
		return false;
	}

	const char *p = path;
	char *q = dest;
	if (has_driveletter(path))
	{
		*q++ = *p++;
		*q++ = *p++;
	}
	if ((*p == *G_BACKSLASH_CHAR) || (*p == *G_FORWARD_SLASH_CHAR))
	{
		p++;
		*q++ = *G_BACKSLASH_CHAR;
	}

	while (*p)
	{
		size_t step = GetUTF8Length(*p);
		if (step > 1)
		{
			while (step--)
			{
				*q++ = *p++;
			}
		}
		else if ((*p == *G_BACKSLASH_CHAR) || (*p == *G_FORWARD_SLASH_CHAR))
		{
			p++;
			*q++ = *G_BACKSLASH_CHAR;
		}
		else
		{
			*q++ = *p++;
		}
	}
	*q = '\0';

	return true;
}

bool normalize_forward_slash(const char *path, char *dest)
{
	if (path == NULL || dest == NULL)
	{
		return false;
	}

	const char *p = path;
	char *q = dest;
	if (has_driveletter(path))
	{
		*q++ = *p++;
		*q++ = *p++;
	}
	if ((*p == *G_BACKSLASH_CHAR) || (*p == *G_FORWARD_SLASH_CHAR))
	{
		p++;
		*q++ = *G_FORWARD_SLASH_CHAR;
	}

	while (*p)
	{
		size_t step = GetUTF8Length(*p);
		if (step > 1)
		{
			while (step--)
			{
				*q++ = *p++;
			}
		}
		else if ((*p == *G_BACKSLASH_CHAR) || (*p == *G_FORWARD_SLASH_CHAR))
		{
			p++;
			*q++ = *G_FORWARD_SLASH_CHAR;
		}
		else
		{
			*q++ = *p++;
		}
	}
	*q = '\0';

	return true;
}


bool get_current_path(char *path)
{
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	char *p = _getcwd(path, MAX_PATH);
	return p != NULL;
#else
	char *p = getcwd(path, MAX_PATH);
	return p != NULL;
#endif
}

bool set_current_path()
{
	bool res = get_current_path(g_current_path);
	if (g_verbose)
	{
		console_message_string(u8"Current Path: %s\n", g_current_path);
	}
	if (res)
	{
		return true;
	}
	clear_path_string(g_current_path, MAX_PATH);
	return false;
}

/******************************************************************
** 非ファイル関連補助関数
*/

bool sleep_internal(unsigned long time)
{
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
	Sleep(time);
#else
	clock_t start = clock();
	clock_t end;
	do
	{
		if ((end = clock()) == (clock_t)-1)
		{
			return false;
		}
	} while (1000.0 * (end - start) / CLOCKS_PER_SEC < time);
#endif
	return true;
}

/*****************************************************************
** 音声設定関数
*/

char *make_voice_path(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		return NULL;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	// パスが指定されていないとき、
	if (path == NULL || strlen(path) == 0)
	{
		// 音響モデルファイルフォルダが確定されていれば
		if (oj->dn_voice_dir_path != NULL && strlen(oj->dn_voice_dir_path) != 0)
		{
			// 標準名の音響モデルファイルを探す
			unsigned int c = 0;
			if (search_file_recursive(oj->dn_voice_dir_path, G_VOICE_DEFAULT, temp, &c, VOICESEARCHMAX))
			{
				goto return_true;
			}

			// 無ければ、何か音響モデルファイルを探す
			c = 0;
			if (search_file_recursive(oj->dn_voice_dir_path, G_VOICE_WILDCARD, temp, &c, VOICESEARCHMAX))
			{
				goto return_true;
			}
		}

		// パスが指定されていないとき、決まらなければ、偽を返す
		return NULL;
	}
	// パスが指定されているとき
	else
	{
		// 名前のみの表記の場合、
		if (is_name_only(path))
		{
			if (strlen(path) + strlen(G_VOICE_EXT) + 1 > MAX_PATH)
			{
				return NULL;
			}

			char name_extended[MAX_PATH];
			strcat(strcpy(name_extended, path), G_VOICE_EXT);
			unsigned int c = 0;
			if (search_file_recursive(oj->dn_voice_dir_path, name_extended, temp, &c, VOICESEARCHMAX))
			{
				goto return_true;
			}
			return NULL;
		}

		// 相対指定の場合、
		if (is_relative(path))
		{
			if (oj->dn_voice_dir_path != NULL && strlen(oj->dn_voice_dir_path) != 0)
			{
				if (strlen(oj->dn_voice_dir_path) + 1 + strlen(path) + 1 > MAX_PATH)
				{
					return false;
				}

				strcpy(temp, oj->dn_voice_dir_path);
				strcat(temp, G_SLASH_CHAR);
				strcat(temp, path);
				if (exists_file(temp))
				{
					goto return_true;
				}
			}
		}
		// 絶対指定の場合、
		else
		{
			strcpy(temp, path);
			if (exists_file(temp))
			{
				goto return_true;
			}
		}
	}
	return NULL;

return_true:
	if (temp == NULL)
	{
		return NULL;
	}
	char *r = malloc(strlen(temp) + 1);
	if (r == NULL)
	{
		return NULL;
	}
	char *s = temp;
	char *d = r;
	while ((*d++ = *s++) != '\0')
	{
	}
	return r;
}

// 設定ファイルの情報から辞書ディレクトリを決定
bool set_config_dic_dir(OpenJTalk *oj, const char *path_original)
{
	if (g_verbose)
	{
		console_message(u8"設定ファイルの情報から辞書ディレクトリを決定\n");
	}

	char path[MAX_PATH];
	clear_path_string(path, MAX_PATH);
#if defined(_WIN32)
	normalize_back_slash(path_original, path);
#else
	strcpy(path, path_original);
#endif

	if (!oj)
	{
		return false;
	}

	if (path != NULL && strlen(path) == 0)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	if (is_relative(path))
	{
		if (g_verbose)
		{
			console_message_string(u8"設定ファイルの辞書ディレクトリ指定は相対指定です。: %s\n", path);
		}

		if (g_ini_dir == NULL || strlen(g_ini_dir) == 0)
		{
			return false;
		}

		if (strlen(g_ini_dir) + 1 + strlen(path) + 1 > MAX_PATH)
		{
			return false;
		}

		strcpy(temp, g_ini_dir);
		strcat(temp, G_SLASH_CHAR);
		strcat(temp, path);
	}
	else
	{
		if (g_verbose)
		{
			console_message_string(u8"設定ファイルの辞書ディレクトリ指定は相対指定ではありません。: %s\n", path);
		}
		strcpy(temp, path);
	}

	if (!exists_dir(temp))
	{
		if (g_verbose)
		{
			console_message_string(u8"設定ファイルで指定された辞書ディレクトリは存在しません。:  %s\n", path);
		}
		return false;
	}
	else
	{
		if (g_verbose)
		{
			console_message_string(u8"設定ファイルで指定された辞書ディレクトリは実在します。: %s\n", path);
		}
	}

	char full[MAX_PATH];
	char full2[MAX_PATH];
	clear_path_string(full, MAX_PATH);
	clear_path_string(full2, MAX_PATH);
	get_fullpath(temp, full);
	if (g_verbose)
	{
		console_message_string(u8"辞書ディレクトリのフルパス: %s\n", full);
	}

	if (!check_dic_utf_8(full))
	{
		console_message(u8"辞書ファイルの設定に失敗しました。\n");
		return false;
	}

#if defined(_WIN32)
#else
#endif
	if (g_verbose)
	{
		console_message_string(u8"辞書ディレクトリのフルパス： %s\n", full);
	}
	strcpy(oj->dn_dic_path, full);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(full, temp2);
	Open_JTalk_load_dic(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_dic(oj->open_jtalk, full);
#endif
	return true;
}

// 設定ファイルの情報から音響モデルディレクトリを決定
bool set_config_voice_dir(OpenJTalk *oj, const char *path_original)
{
	if (g_verbose)
	{
		console_message(u8"設定ファイルの情報から音響モデルディレクトリを決定\n");
	}

	char path[MAX_PATH];
	clear_path_string(path, MAX_PATH);
#if defined(_WIN32)
	normalize_back_slash(path_original, path);
#else
	strcpy(path, path_original);
#endif

	if (!oj)
	{
		return false;
	}

	if (path != NULL && strlen(path) == 0)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	if (is_relative(path))
	{
		if (g_verbose)
		{
			console_message_string(u8"音響モデルディレクトリ指定は相対指定です。： %s\n", path);
		}

		if (g_ini_dir == NULL || strlen(g_ini_dir) == 0)
		{
			return false;
		}

		if (strlen(g_ini_dir) + 1 + strlen(path) + 1 > MAX_PATH)
		{
			return false;
		}

		strcpy(temp, g_ini_dir);
		strcat(temp, G_SLASH_CHAR);
		strcat(temp, path);
	}
	else
	{
		if (g_verbose)
		{
			console_message_string(u8"音響モデルディレクトリ指定は相対指定ではありません。： %s\n", path);
		}
		strcpy(temp, path);
	}

	if (!exists_dir(temp))
	{
		if (g_verbose)
		{
			console_message_string(u8"指定された音響モデルディレクトリは存在しません。： %s\n", temp);
		}
		return false;
	}

	char full[MAX_PATH];
	clear_path_string(full, MAX_PATH);
	get_fullpath(temp, full);
	if (g_verbose)
	{
		console_message_string(u8"指定された音響モデルディレクトリのフルパス： %s\n", full);
	}

	strcpy(oj->dn_voice_dir_path, full);
	return true;
}

// 設定ファイルからの音響モデルファイル設定
bool set_config_voice(OpenJTalk *oj, const char *path_original)
{
	if (g_verbose)
	{
		console_message(u8"設定ファイルの情報から音響モデルファイルを決定\n");
	}

	char path[MAX_PATH];
	clear_path_string(path, MAX_PATH);
#if defined(_WIN32)
	normalize_back_slash(path_original, path);
#else
	strcpy(path, path_original);
#endif

	if (!oj)
	{
		return false;
	}

	if (path == NULL || strlen(path) == 0)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	if (is_name_only(path))
	{
		if (g_verbose)
		{
			console_message_string(u8"音響モデルファイルは名前のみの指定です。： %s\n", path);
		}

		if (strlen(path) + strlen(G_VOICE_EXT) + 1 > MAX_PATH)
		{
			return false;
		}

		char name_extended[MAX_PATH];
		strcat(strcpy(name_extended, path), G_VOICE_EXT);
		unsigned int c = 0;
		if (search_file_recursive(oj->dn_voice_dir_path, name_extended, temp, &c, VOICESEARCHMAX))
		{
			goto return_true;
		}
		return false;
	}

	if (is_relative(path))
	{
		if (g_verbose)
		{
			console_message_string(u8"音響モデルファイル指定は相対指定です： %s\n", path);
		}

		if (g_ini_dir == NULL || strlen(g_ini_dir) == 0)
		{
			return false;
		}

		if (strlen(g_ini_dir) + 1 + strlen(path) + 1 > MAX_PATH)
		{
			return false;
		}

		strcpy(temp, g_ini_dir);
		strcat(temp, G_SLASH_CHAR);
		strcat(temp, path);
	}
	else
	{
		if (g_verbose)
		{
			console_message_string(u8"音響モデルファイル指定は相対指定ではありません： %s\n", path);
		}
		strcpy(temp, path);
	}

	if (!exists_file(temp))
	{
		if (g_verbose)
		{
			console_message_string(u8"指定された音響モデルファイルは存在しません： %s\n", temp);
		}
		return false;
	}

	char full[MAX_PATH];
return_true:
	clear_path_string(full, MAX_PATH);
	get_fullpath(temp, full);
	strcpy(oj->fn_voice_path, full);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(full, temp2);
	Open_JTalk_load_voice(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_voice(oj->open_jtalk, full);
#endif
	return true;
}

// 指定位置の近くの辞書ディレクトリを決定
bool set_nearby_dic_path(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		return false;
	}

	if (path == NULL || strlen(path) == 0)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	// 指定位置から可能性のある名前を探す
	for (const char **d = G_DEFAULT_DIC_DIR_NAMES; *d != NULL; d++)
	{
		if (search_directory(path, *d, temp))
		{
			goto check_charset;
		}
	}

	// その親ディレクトリから可能性のあるフォルダを探す
	char parent[MAX_PATH];
	get_dir_path(path, parent);
	if (parent != NULL && strlen(parent) != 0)
	{
		for (const char **d = G_DEFAULT_DIC_DIR_NAMES; *d != NULL; d++)
		{
			if (search_directory(parent, *d, temp))
			{
				goto check_charset;
			}
		}
	}
	return false;

check_charset:
	if (!check_dic_utf_8(temp))
	{
		return false;
	}
	strcpy(oj->dn_dic_path, temp);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(temp, temp2);
	Open_JTalk_load_dic(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_dic(oj->open_jtalk, temp);
#endif

	return true;
}

// 指定位置の近くの音響モデルディレクトリを決定
bool set_nearby_voice_dir_path(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		return false;
	}

	if (path == NULL || strlen(path) == 0)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	// 指定位置から可能性のあるフォルダを探す
	for (const char **d = G_DEFAULT_VOICE_DIR_NAMES; *d != NULL; d++)
	{
		if (search_directory(path, *d, temp))
		{
			goto return_true;
		}
	}

	// その親ディレクトリから可能性のあるフォルダを探す
	char parent[MAX_PATH];
	get_dir_path(path, parent);
	if (parent != NULL && strlen(parent) != 0)
	{
		for (const char **d = G_DEFAULT_VOICE_DIR_NAMES; *d != NULL; d++)
		{
			if (search_directory(parent, *d, temp))
			{
				goto return_true;
			}
		}
	}
	return false;

return_true:
	strcpy(oj->dn_voice_dir_path, temp);
	return true;
}

// 音響モデルファイル設定
bool set_voice(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	// パスが指定されていないとき、
	if (path == NULL || strlen(path) == 0)
	{
		// 音響モデルファイルフォルダが確定されていれば
		if (oj->dn_voice_dir_path != NULL && strlen(oj->dn_voice_dir_path) != 0)
		{
			// 標準名の音響モデルファイルを探す
			unsigned int c = 0;
			if (search_file_recursive(oj->dn_voice_dir_path, G_VOICE_DEFAULT, temp, &c, VOICESEARCHMAX))
			{
				goto return_true;
			}

			// 無ければ、何か音響モデルファイルを探す
			c = 0;
			if (search_file_recursive(oj->dn_voice_dir_path, G_VOICE_WILDCARD, temp, &c, VOICESEARCHMAX))
			{
				goto return_true;
			}
		}

		// パスが指定されていないとき、決まらなければ、偽を返す
		return false;
	}
	// パスが指定されているとき
	else
	{
		// 名前のみの表記の場合、
		if (is_name_only(path))
		{
			if (strlen(path) + strlen(G_VOICE_EXT) + 1 > MAX_PATH)
			{
				return false;
			}

			// dirが確定していれば
			char name_extended[MAX_PATH];
			strcat(strcpy(name_extended, path), G_VOICE_EXT);
			unsigned int c = 0;
			if (search_file_recursive(oj->dn_voice_dir_path, name_extended, temp, &c, VOICESEARCHMAX))
			{
				goto return_true;
			}
			return false;
		}

		// 相対指定の場合、
		if (is_relative(path))
		{
			if (g_current_path != NULL && strlen(g_current_path) != 0)
			{
				if (strlen(g_current_path) + 1 + strlen(path) + 1 > MAX_PATH)
				{
					return false;
				}

				strcpy(temp, g_current_path);
				strcat(temp, G_SLASH_CHAR);
				strcat(temp, path);

				if (exists_file(temp))
				{
					goto return_true;
				}
			}
		}
		// 絶対指定の場合、
		else
		{
			strcpy(temp, path);
			if (exists_file(temp))
			{
				goto return_true;
			}
		}
	}
	return false;

return_true:
	strcpy(oj->fn_voice_path, temp);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(temp, temp2);
	Open_JTalk_load_voice(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_voice(oj->open_jtalk, temp);
#endif
	return true;
}

// 指定の名前の音響モデルを登録する
bool set_voice_name(OpenJTalk *oj, const char *name)
{
	if (!oj)
	{
		return false;
	}

	if (name == NULL || strlen(name) == 0)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	if (is_name_only(name))
	{
		if (strlen(name) + strlen(G_VOICE_EXT) + 1 > MAX_PATH)
		{
			return false;
		}

		char name_extended[MAX_PATH];
		strcat(strcpy(name_extended, name), G_VOICE_EXT);
		unsigned int c = 0;
		if (search_file_recursive(oj->dn_voice_dir_path, name_extended, temp, &c, VOICESEARCHMAX))
		{
			goto return_true;
		}
	}

	return false;

return_true:
	strcpy(oj->fn_voice_path, temp);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(temp, temp2);
	Open_JTalk_load_voice(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_voice(oj->open_jtalk, temp);
#endif
	return true;
}

bool set_voice_path(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		return false;
	}

	if (path == NULL || strlen(path) == 0)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	// 相対指定の場合、
	if (is_relative(path))
	{
		if (oj->dn_voice_dir_path != NULL && strlen(oj->dn_voice_dir_path) != 0)
		{
			if (strlen(oj->dn_voice_dir_path) + 1 + strlen(path) + 1 > MAX_PATH)
			{
				return false;
			}

			strcpy(temp, oj->dn_voice_dir_path);
			strcat(temp, G_SLASH_CHAR);
			strcat(temp, path);

			if (exists_file(temp))
			{
				goto return_true;
			}
		}
	}

	// 絶対指定の場合、
	else
	{
		strcpy(temp, path);
		if (exists_file(temp))
		{
			goto return_true;
		}
	}

	return false;

return_true:
	strcpy(oj->fn_voice_path, temp);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(temp, temp2);
	Open_JTalk_load_voice(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_voice(oj->open_jtalk, temp);
#endif
	return true;
}

bool set_voice_dir_path(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);
	if (path != NULL && strlen(path) != 0)
	{
		// 絶対パスならば
		if (!is_relative(path))
		{
			strcpy(temp, path);
			if (exists_dir(temp))
			{
				goto return_true;
			}
			else
			{
				return false;
			}
		}
		// 相対パスならば
		else
		{
			// カレントパスを基準にして探す
			if (g_current_path != NULL && strlen(g_current_path) != 0)
			{
				if (strlen(g_current_path) + 1 + strlen(path) + 1 > MAX_PATH)
				{
					return false;
				}

				char temp2[MAX_PATH];
				strcpy(temp2, g_current_path);
				strcat(temp2, G_SLASH_CHAR);
				strcat(temp2, path);

				if (exists_dir(temp2))
				{
					get_fullpath(temp2, temp);
					goto return_true;
				}
			}
		}
	}

	return false;

return_true:
	strcpy(oj->dn_voice_dir_path, temp);
	return true;
}

bool set_default_voice_dir_path(OpenJTalk *oj)
{
	if (!oj)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	// 標準のインストールフォルダから可能性のある名前を探す
	if (g_data_install_path != NULL && strlen(g_data_install_path) != 0)
	{
		for (const char **d = G_DEFAULT_VOICE_DIR_NAMES; *d != NULL; d++)
		{
			if (search_directory(g_data_install_path, *d, temp))
			{
				goto return_true;
			}
		}
	}
	return false;

return_true:
	strcpy(oj->dn_voice_dir_path, temp);
	return true;
}

// 特に指定が無い場合の音響モデルファイルの登録
// 標準名を優先するが、無ければどれでもいい
// 何も登録できなければ、falseを返す
bool set_default_voice(OpenJTalk *oj)
{
	if (!oj)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);
	unsigned int c = 0;

	// 音響モデルフォルダが確定していれば、
	if (oj->dn_voice_dir_path != NULL && strlen(oj->dn_voice_dir_path) != 0)
	{
		// 標準の音響モデルファイルを探す。
		c = 0;
		if (search_file_recursive(oj->dn_voice_dir_path, G_VOICE_DEFAULT, temp, &c, VOICESEARCHMAX))
		{
			goto return_true;
		}

		// 何か音響モデルファイルを探す
		c = 0;
		if (search_file_recursive(oj->dn_voice_dir_path, G_VOICE_WILDCARD, temp, &c, VOICESEARCHMAX))
		{
			goto return_true;
		}
	}
	return false;

return_true:
	strcpy(oj->fn_voice_path, temp);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(temp, temp2);
	Open_JTalk_load_voice(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_voice(oj->open_jtalk, temp);
#endif
	return true;
}

// 音響モデルディレクトリを変更し、ついでに音響モデルも登録する
// 優先されるのは、現在と同じ名前、その次に標準名
bool set_voice_dir_and_voice(OpenJTalk *oj, const char *path)
{
	char temp[MAX_PATH];

	if (!set_voice_dir_path(oj, path))
	{
		return false;
	}

	if (!oj->fn_voice_path || strlen(oj->fn_voice_path) == 0)
	{
		return set_default_voice(oj);
	}

	if (get_fname(oj->fn_voice_path, temp) == NULL)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}

	if (set_voice_name(oj, temp))
	{
		return true;
	}

	return set_default_voice(oj);
}

/******************************************************************
** 辞書ディレクトリ設定関数
*/

// 辞書がUTF-8向けかどうか
// unk.dic の 0x28からの文字列を調べる
bool check_dic_utf_8(const char *path)
{
	if (path == NULL || strlen(path) == 0)
	{
		return false;
	}
	char file[MAX_PATH];
	clear_path_string(file, MAX_PATH);
#if defined(_WIN32)
	u8tosjis_path(path, file);
#else
	u8tou8_path(path, file);
#endif
	if (7 + 1 + strlen(file) + 1 > MAX_PATH)
	{
		return false;
	}

	strcat(file, G_SLASH_CHAR);
	strcat(file, "unk.dic");

	FILE *fp = fopen(file, "rb");
	if (!fp)
	{
		return false;
	}
	if (fseek(fp, 0x28, SEEK_SET) != 0)
	{
		fclose(fp);
		return false;
	}
	char str[6];
	char *res = fgets(str, 6, fp);
	if( res == NULL )
	{
		fclose(fp);
		return false;
	}
	fclose(fp);
	char *p = str;
	if (*p != 'U' && *p != 'u')
		goto exit_false;
	p++;
	if (*p != 'T' && *p != 't')
		goto exit_false;
	p++;
	if (*p != 'F' && *p != 'f')
		goto exit_false;
	p++;
	if (*p != '-' && *p != '_')
		goto exit_false;
	p++;
	if (*p != '8')
		goto exit_false;
	return true;

exit_false:
	if (g_verbose)
	{
		console_message_string(u8"this dic is not in utf-8: %s\n", (char*)path);
	}
	return false;
}

bool set_dic_path(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	if (path != NULL && strlen(path) != 0)
	{
		// パスが絶対指定ならば
		if (!is_relative(path))
		{
			strcpy(temp, path);
			if (exists_dir(temp))
			{
				goto check_charset;
			}
		}
		// パスが相対指定ならば
		else
		{
			// カレントフォルダを探す
			if (g_current_path != NULL && strlen(g_current_path) != 0)
			{

				if (strlen(g_current_path) + 1 + strlen(path) + 1 > MAX_PATH)
				{
					return false;
				}

				char temp2[MAX_PATH];
				strcpy(temp2, g_current_path);
				strcat(temp2, G_SLASH_CHAR);
				strcat(temp2, path);

				if (exists_dir(temp2))
				{
					get_fullpath(temp2, temp);
					goto check_charset;
				}
			}
		}
	}
	return false;

check_charset:
	if (!*temp)
	{
		return false;
	}
	strcpy(oj->dn_dic_path, temp);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(temp, temp2);
	Open_JTalk_load_dic(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_dic(oj->open_jtalk, temp);
#endif
	return true;
}

/// <summary>
/// 省略時の辞書フォルダを設定する
/// </summary>
/// <param name="oj">構造体データ</param>
/// <returns>設定の成否</returns>
bool set_default_dic_path(OpenJTalk *oj)
{
	if (!oj)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);

	// 標準のインストールフォルダから可能性のある名前を探す。
	if (g_data_install_path != NULL && strlen(g_data_install_path) != 0)
	{
		for (const char **d = G_DEFAULT_DIC_DIR_NAMES; *d != NULL; d++)
		{
			if (search_directory(g_data_install_path, *d, temp))
			{
				goto check_charset;
			}
		}
	}
	return false;

check_charset:
	if (!check_dic_utf_8(temp))
	{
		return false;
	}
	strcpy(oj->dn_dic_path, temp);
#if defined(_WIN32)
	char temp2[MAX_PATH];
	clear_path_string(temp2, MAX_PATH);
	u8tosjis_path(temp, temp2);
	Open_JTalk_load_dic(oj->open_jtalk, temp2);
#else
	Open_JTalk_load_dic(oj->open_jtalk, temp);
#endif
	return true;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_generatePCM(OpenJTalk *oj, const char *txt, short **data, size_t *size)
{
    if (!oj)
    {
        return false;
    }

    if (txt == NULL || strlen(txt) == 0)
    {
        return false;
    }

    size_t sampling_frequency;
    if (Open_JTalk_generate_sounddata(
            oj->open_jtalk,
            txt,
            data,
            size,
            &sampling_frequency))
    {
        return true;
    }
    return false;
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_clearData(short *data)
{
    free(data);
}

/*****************************************************************
** 設定ファイル解析関数
*/

typedef struct SectionList_t
{
	struct SectionList_t *succ;
	char *name;
} SectionList;

typedef enum
{
	VTYPE_NONE,
	VTYPE_STRING,
	VTYPE_BOOLEAN,
	VTYPE_INTEGER,
	VTYPE_DOUBLE,
} ValueType;

typedef union _Value {
	double v_double;
	long v_integer;
	bool v_boolean;
	char *v_string;
} ItemValue;

typedef struct ConfigItem_t
{
	struct ConfigItem_t *succ;
	SectionList *section;
	char *key;
	ValueType vtype;
	ItemValue value;
} ConfigItem;

typedef struct ConfigScanner_t
{
	char *base;
	SectionList *first_section;
	ConfigItem *first_item;
	char *ptr;
	SectionList *current_section;
	ConfigItem *current_item;
	ConfigItem *item;
	ValueType vtype;
	char *current_section_name;
	char *current_key;
	bool current_boolean;
	char *current_string;
	long current_integer;
	double current_double;
	char *current_rawdata;
} ConfigScanner;

void SkipSpace(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return;
	}
	while (true)
	{
		switch (*sc->ptr)
		{
		case ' ':
		case '\t':
			sc->ptr++;
			continue;
		}
		return;
	}
}

bool isBeginningComment(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return false;
	}
	return *sc->ptr == '#' || *sc->ptr == ';';
}

bool GetEQ(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return false;
	}
	bool result = *sc->ptr == '=';
	if (result)
	{
		sc->ptr++;
	}
	return result;
}

char *GetQuote(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return NULL;
	}
	char *start = sc->ptr;
	while (true)
	{
		char ch = *sc->ptr;
		if ((ch == '\r') || (ch == '\n') || (ch == '\0'))
		{
			return NULL;
		}
		else if (ch == '"')
		{
			break;
		}
		else if (ch == '\\')
		{
			ch = *++sc->ptr;
		}
		size_t step = GetUTF8Length(ch);
		if (step == 0)
		{
			return NULL;
		}
		sc->ptr += step;
	}
	size_t length = sc->ptr - start;
	char *temp = (char *)malloc(length + 1);
	memset(temp, 0, length + 1);
	char *s = start;
	char *d = temp;
	while (true)
	{
		char ch = *s;
		if (ch == '"')
		{
			break;
		}
		else if (ch == '\\')
		{
			ch = *++s;
		}
		size_t step = GetUTF8Length(ch);
		memcpy(d, s, step);
		s += step;
		d += step;
	}
	if (*sc->ptr == '"')
	{
		sc->ptr++;
	}
	return temp;
}

char *GetString(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return NULL;
	}
	char *start = sc->ptr;
	while (true)
	{
		char ch = *sc->ptr;
		if ((ch == ' ') || (ch == '\t'))
		{
			break;
		}
		if ((ch == '\r') || (ch == '\n') || (ch == '\0'))
		{
			break;
		}
		if ((ch == ']') | (ch == '=') || (ch == '"'))
		{
			break;
		}
		if ((ch == '#') || (ch == ';'))
		{
			break;
		}
		size_t step = GetUTF8Length(ch);
		if (step == 0)
		{
			return NULL;
		}
		sc->ptr += step;
	}
	size_t length = sc->ptr - start;
	if (length == 0)
	{
		return NULL;
	}
	char *temp = (char *)malloc(length + 1);
	memset(temp, '\0', length + 1);
	strncpy(temp, start, length);
	return temp;
}

bool ScanBoolean(ConfigScanner *sc)
{
	bool result = false;
	if (sc == NULL)
	{
		return result;
	}
	char *str = GetString(sc);
	if (str == NULL)
	{
		return result;
	}
	if (strcmp(str, "true") == 0)
	{
		sc->current_boolean = true;
		sc->vtype = VTYPE_BOOLEAN;
		result = true;
	}
	else if (strcmp(str, "false") == 0)
	{
		sc->current_boolean = false;
		sc->vtype = VTYPE_BOOLEAN;
		result = true;
	}
	free(str);
	return result;
}

bool ScanNumber(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return false;
	}
	char *start = sc->ptr;
	char ch = *sc->ptr;
	if (ch == '+' || ch == '-')
	{
		sc->ptr++;
		ch = *sc->ptr;
	}
	bool point = false;
	if (ch >= '0' && ch <= '9')
	{
		while (true)
		{
			if (ch == '.')
			{
				if (point)
				{
					return false;
				}
				else
				{
					point = true;
				}
				sc->ptr++;
				ch = *sc->ptr;
			}
			else if (ch >= '0' && ch <= '9')
			{
				sc->ptr++;
				ch = *sc->ptr;
			}
			else
			{
				break;
			}
		}
	}
	size_t length = sc->ptr - start;
	char *temp = (char *)malloc(length + 1);
	memset(temp, 0, length + 1);
	strncpy(temp, start, length);
	if (point)
	{
		sc->current_double = atof(temp);
		sc->vtype = VTYPE_DOUBLE;
	}
	else
	{
		sc->current_integer = atoi(temp);
		sc->vtype = VTYPE_INTEGER;
	}
	free(temp);
	return true;
}

bool ScanValueString(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return false;
	}
	char ch = *sc->ptr;
	if (ch == '"')
	{
		sc->ptr++;
		char *value = GetQuote(sc);
		if (value == NULL)
		{
			return false;
		}
		sc->current_string = value;
		sc->vtype = VTYPE_STRING;
	}
	else if (ch == 't' || ch == 'f')
	{
		if (!ScanBoolean(sc))
		{
			return false;
		}
	}
	else if (ch == '+' || ch == '-' || (ch >= '0' && ch <= '9'))
	{
		if (!ScanNumber(sc))
		{
			return false;
		}
	}
	return true;
}

bool ScanValue(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return false;
	}
	char ch = *sc->ptr;
	if (ch == '"')
	{
		sc->ptr++;
		char *value = GetQuote(sc);
		if (value == NULL)
		{
			return false;
		}
		sc->current_string = value;
		sc->vtype = VTYPE_STRING;
	}
	else if (ch == 't' || ch == 'f')
	{
		if (!ScanBoolean(sc))
		{
			return false;
		}
	}
	else if (ch == '+' || ch == '-' || (ch >= '0' && ch <= '9'))
	{
		if (!ScanNumber(sc))
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

int ScanEol(ConfigScanner *sc)
{
	switch (*sc->ptr)
	{
	case '\0':
		return true;
	case '\n':
		sc->ptr++;
		return true;
	case '\r':
		sc->ptr++;
		if (*sc->ptr == '\n')
		{
			sc->ptr++;
		}
		return true;
	default:
		return false;
	}
}

void SkipLine(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return;
	}
	while (true)
	{
		size_t step = GetUTF8Length(*sc->ptr);
		switch (*sc->ptr)
		{
		case '\0':
			return;
		case '\n':
			sc->ptr++;
			return;
		case '\r':
			sc->ptr++;
			if (*sc->ptr == '\n')
			{
				sc->ptr++;
			}
			return;
		default:
			if (step == 0)
			{
				return;
			}
			sc->ptr += step;
		}
	}
	return;
}

bool GetKeyWord(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return false;
	}
	char ch = *sc->ptr;
	char *key;
	if (ch == '"')
	{
		sc->ptr++;
		key = GetQuote(sc);
		if (key == NULL)
		{
			return false;
		}
	}
	else
	{
		key = GetString(sc);
		if (key == NULL)
		{
			return false;
		}
	}
	sc->current_key = key;
	return true;
}

void ScanPair(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return;
	}
	SkipSpace(sc);
	if (!GetKeyWord(sc))
	{
		return;
	}
	SkipSpace(sc);
	if (!GetEQ(sc))
	{
		return;
	}
	SkipSpace(sc);
	if (!ScanValue(sc))
	{
		return;
	}
	return;
}

bool isSectionTop(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return false;
	}
	return *sc->ptr == '[';
}

char *ScanSection(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return NULL;
	}
	char ch = *sc->ptr;
	if (ch != '[')
	{
		return NULL;
	}
	sc->ptr++;
	SkipSpace(sc);
	char *section = GetString(sc);
	if (section == NULL)
	{
		return NULL;
	}
	SkipSpace(sc);
	if (*sc->ptr != ']')
	{
		free(section);
		return NULL;
	}
	sc->ptr++;
	return section;
}

void ScanFile(ConfigScanner *sc)
{
	if (g_verbose)
	{
		console_message(u8"設定解析開始\n");
	}

	if (sc == NULL)
	{
		return;
	}
	sc->ptr = sc->base;
	sc->current_section_name = NULL;
	while (*sc->ptr != '\0')
	{
		sc->vtype = VTYPE_NONE;
		sc->current_key = NULL;
		SkipSpace(sc);
		if (isSectionTop(sc))
		{
			char *section = ScanSection(sc);
			if (section)
			{
				if (g_verbose)
				{
					console_message_string(u8"config section： %s\n", section);
				}
				SectionList *sec = (SectionList *)malloc(sizeof(SectionList));
				sec->name = section;
				sec->succ = NULL;
				if (!sc->first_section)
				{
					sc->first_section = sec;
				}
				else
				{
					sc->current_section->succ = sec;
				}
				sc->current_section = sec;
			}
		}
		else
		{
			ScanPair(sc);
		}
		SkipSpace(sc);
		if (isBeginningComment(sc))
		{
			SkipLine(sc);
		}
		else if (!ScanEol(sc))
		{
			break;
		}

		if (sc->current_key != NULL)
		{
			ConfigItem *temp = (ConfigItem *)malloc(sizeof(ConfigItem));
			temp->key = sc->current_key;
			temp->vtype = sc->vtype;
			temp->section = sc->current_section;
			temp->succ = NULL;
			switch (temp->vtype)
			{
			case VTYPE_NONE:
				break;
			case VTYPE_STRING:
				temp->value.v_string = sc->current_string;
				break;
			case VTYPE_BOOLEAN:
				temp->value.v_boolean = sc->current_boolean;
				break;
			case VTYPE_INTEGER:
				temp->value.v_integer = sc->current_integer;
				break;
			case VTYPE_DOUBLE:
				temp->value.v_double = sc->current_double;
				break;
			}

			if (!sc->current_item)
			{
				sc->first_item = temp;
			}
			else
			{
				sc->current_item->succ = temp;
			}
			sc->current_item = temp;
		}
	}
}

bool get_filesize(const char *name, size_t *size)
{
	FILE *fp = fopen(name, "rb");
	if (!fp)
	{
		return false;
	}
	if (fseek(fp, 0, SEEK_END) != 0)
	{
		return false;
	}
	*size = ftell(fp);
	if (size < 0)
	{
		return false;
	}
	if (fseek(fp, 0, SEEK_SET) != 0)
	{
		return false;
	}
	fclose(fp);
	return true;
}

char *get_contents(const char *name)
{
	if (name == NULL || strlen(name) == 0)
	{
		return NULL;
	}
	size_t size = 0;
	if (get_filesize(name, &size) == false)
	{
		return NULL;
	}
	if (size == 0)
	{
		return NULL;
	}
	char *buf = (char *)malloc(sizeof(char) * size + 1);
	memset(buf, 0x00, sizeof(char) * size + 1);
	FILE *f = fopen(name, "rb");
	if (f == NULL)
	{
		free(buf);
		return NULL;
	}
	if ((getc(f) & 0xFF) != 0xEF || (getc(f) & 0xFF) != 0xBB || (getc(f) & 0xFF) != 0xBF)
	{
		fseek(f, 0, SEEK_SET);
	}
	size_t  res = fread(buf, sizeof(char), size, f);
	if ( res < size)
	{
		fclose(f);
		return buf;
	}
	fclose(f);
	return buf;
}

ConfigScanner *config_load(char *filename)
{
	if (g_verbose)
	{
		console_message(u8"設定ファイル読み込み\n");
	}

	char *text = get_contents(filename);
	if (text == NULL)
	{
		return NULL;
	}
	ConfigScanner *result = (ConfigScanner *)malloc(sizeof(ConfigScanner));
	result->base = text;
	result->first_section = NULL;
	result->current_section = NULL;
	result->first_item = NULL;
	result->current_item = NULL;
	ScanFile(result);
	return result;
}

ConfigItem *config_find(ConfigScanner *sc, const char *section, char *name)
{
	if (sc == NULL)
	{
		return NULL;
	}
	for (ConfigItem *item = sc->first_item; item != NULL; item = item->succ)
	{
		if (item->section != NULL && item->section->name != NULL)
		{
			char *item_section = item->section->name;
			if (strcmp(item_section, section) != 0)
			{
				return NULL;
			}
		}
		if (item->key != NULL)
		{
			if (strcmp(item->key, name) == 0)
			{
				return item;
			}
		}
	}
	return NULL;
}

bool config_find_integer(ConfigScanner *sc, const char *section, char *name, int *result)
{
	ConfigItem *temp = config_find(sc, section, name);
	if (temp == NULL)
	{
		return false;
	}
	if (temp->vtype == VTYPE_INTEGER)
	{
		*result = temp->value.v_integer;
		return true;
	}
	else if (temp->vtype == VTYPE_DOUBLE)
	{
		*result = (int)temp->value.v_double;
		return true;
	}
	return false;
}

bool config_find_double(ConfigScanner *sc, const char *section, char *name, double *result)
{
	ConfigItem *temp = config_find(sc, section, name);
	if (temp == NULL)
	{
		return false;
	}
	if (temp->vtype == VTYPE_DOUBLE)
	{
		*result = temp->value.v_double;
		return true;
	}
	else if (temp->vtype == VTYPE_INTEGER)
	{
		*result = (double)temp->value.v_integer;
		return true;
	}
	return false;
}

bool config_find_boolean(ConfigScanner *sc, const char *section, char *name, bool *result)
{
	ConfigItem *temp = config_find(sc, section, name);
	if (temp == NULL)
	{
		return false;
	}
	if (temp->vtype == VTYPE_BOOLEAN)
	{
		*result = temp->value.v_boolean;
		return true;
	}
	return false;
}

bool config_find_string(ConfigScanner *sc, const char *section, char *name, char **result)
{
	ConfigItem *temp = config_find(sc, section, name);
	if (temp == NULL)
	{
		return false;
	}
	if (temp->vtype == VTYPE_STRING)
	{
		*result = temp->value.v_string;
		return true;
	}
	return false;
}

void config_free(ConfigScanner *sc)
{
	if (sc == NULL)
	{
		return;
	}
	ConfigItem *c_item = sc->first_item;
	ConfigItem *c_prev = NULL;
	while (c_item != NULL)
	{
		if (c_prev)
		{
			free(c_prev);
		}
		if (c_item->key)
		{
			free(c_item->key);
		}
		if (c_item->vtype == VTYPE_STRING && c_item->value.v_string)
		{
			free(c_item->value.v_string);
		}
		c_prev = c_item;
		c_item = c_item->succ;
	}
	if (c_prev)
	{
		free(c_prev);
	}

	SectionList *s_item = sc->first_section;
	SectionList *s_prev = NULL;
	while (s_item != NULL)
	{
		if (s_prev)
		{
			free(s_prev);
		}
		if (s_item->name)
		{
			free(s_item->name);
		}
		s_prev = s_item;
		s_item = s_item->succ;
	}
	if (s_prev)
	{
		free(s_prev);
	}

	if (sc != NULL && sc->base != NULL)
	{
		free(sc->base);
	}

	if (sc != NULL)
	{
		free(sc);
	}
}

bool get_ini_data(OpenJTalk *oj)
{
	if (strlen(g_ini_path) == 0)
	{
		return false;
	}

	if (g_verbose)
	{
		console_message(u8"***** 設定ファイル解釈　開始 *****\n");
	}

	ConfigScanner *sc = config_load(g_ini_path);
	if (sc == NULL)
	{
		return false;
	}

	char *temp;
	if (config_find_string(sc, G_SECTION_NAME, u8"voice_dir", &temp))
	{
		if (strlen(oj->dn_voice_dir_path) == 0)
		{
			if (g_verbose)
			{
				console_message_string(u8"config voice_dir： %s\n", temp);
			}
			set_config_voice_dir(oj, temp);
		}
		else
		{
			if (g_verbose)
			{
				console_message(u8"config voice_dir: 無視\n");
			}
		}
	}

	if (config_find_string(sc, G_SECTION_NAME, u8"dic_dir", &temp))
	{
		if (strlen(oj->dn_dic_path) == 0)
		{
			if (!set_config_dic_dir(oj, temp))
			{
				if (g_verbose)
				{
					console_message(u8"warning: 設定ファイルからの辞書ファイルの設定に失敗しました。\n");
				}
			}
		}
		else
		{
			if (g_verbose)
			{
				console_message(u8"config dic_dir: 無視\n");
			}
		}
	}

	if (config_find_string(sc, G_SECTION_NAME, u8"voice", &temp))
	{
		if (strlen(oj->fn_voice_path) == 0)
		{
			if (g_verbose)
			{
				console_message_string(u8"config voice： %s\n", temp);
			}
			set_config_voice(oj, temp);
		}
		else
		{
			if (g_verbose)
			{
				console_message(u8"config voice: 無視\n");
			}
		}
	}

	int temp_int;
	if (config_find_integer(sc, G_SECTION_NAME, u8"sampling_frequency", &temp_int))
	{
		if (g_verbose)
		{
			console_message_integer(u8"config sampling_frequency: %d\n", temp_int);
		}
		openjtalk_setSamplingFrequency(oj, temp_int);
	}
	if (config_find_integer(sc, G_SECTION_NAME, u8"s", &temp_int))
	{
		if (g_verbose)
		{
			console_message_integer(u8"config s: %d\n", temp_int);
		}
		openjtalk_setSamplingFrequency(oj, temp_int);
	}
	if (config_find_integer(sc, G_SECTION_NAME, u8"fperiod", &temp_int))
	{
		if (g_verbose)
		{
			console_message_integer(u8"config fperiod: %d\n", temp_int);
		}
		openjtalk_setFperiod(oj, temp_int);
	}
	if (config_find_integer(sc, G_SECTION_NAME, u8"p", &temp_int))
	{
		if (g_verbose)
		{
			console_message_integer(u8"config p: %d\n", temp_int);
		}
		openjtalk_setFperiod(oj, temp_int);
	}

	double temp_double;
	if (config_find_double(sc, G_SECTION_NAME, u8"alpha", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config alpha: %f\n", temp_double);
		}
		openjtalk_setAlpha(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"a", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config a: %f\n", temp_double);
		}
		openjtalk_setAlpha(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"beta", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config beta: %f\n", temp_double);
		}
		openjtalk_setBeta(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"b", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config b: %f\n", temp_double);
		}
		openjtalk_setBeta(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"speed", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config speed: %f\n", temp_double);
		}
		openjtalk_setSpeed(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"r", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config r: %f\n", temp_double);
		}
		openjtalk_setSpeed(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"additional_half_tone", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config additional_half_tone: %f\n", temp_double);
		}
		openjtalk_setAdditionalHalfTone(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"fm", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config fm: %f\n", temp_double);
		}
		openjtalk_setAdditionalHalfTone(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"msd_threshold", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config msd_threshold: %f\n", temp_double);
		}
		openjtalk_setMsdThreshold(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"u", &temp_double))
	{
		if (g_verbose)
		{
			console_message_integer(u8"config u: %f\n", temp_double);
		}
		openjtalk_setMsdThreshold(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"gv_weight_for_spectrum", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config gv_weight_for_spectrum: %f\n", temp_double);
		}
		openjtalk_setGvWeightForSpectrum(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"jm", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config jm： %f\n", temp_double);
		}
		openjtalk_setGvWeightForSpectrum(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"gv_weight_for_log_f0", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config gv_weight_for_log_f0: %f\n", temp_double);
		}
		openjtalk_setGvWeightForLogF0(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"jf", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config jf: %f\n", temp_double);
		}
		openjtalk_setGvWeightForLogF0(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"volume", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config volume: %f\n", temp_double);
		}
		openjtalk_setVolume(oj, temp_double);
	}
	if (config_find_double(sc, G_SECTION_NAME, u8"g", &temp_double))
	{
		if (g_verbose)
		{
			console_message_float(u8"config g: %f\n", temp_double);
		}
		openjtalk_setVolume(oj, temp_double);
	}
	config_free(sc);
	if (g_verbose)
	{
		console_message(u8"***** 設定ファイル解釈　終了 *****\n");
	}
	return true;
}

// 指定フォルダに設定ファイルがあるか調べ、その位置を記録する。
bool set_ini_path(OpenJTalk *oj, const char *dir)
{
	if (dir == NULL || strlen(dir) == 0)
	{
		return false;
	}

	if (strlen(dir) + 1 + strlen(G_INI_NAME) + 1 > MAX_PATH)
	{
		return false;
	}

	char temp[MAX_PATH];
	clear_path_string(temp, MAX_PATH);
	strcpy(temp, dir);
	strcat(temp, G_SLASH_CHAR);
	strcat(temp, G_INI_NAME);

	if (!exists_file(temp))
	{
		return false;
	}

	strcpy(g_ini_path, temp);
	strcpy(g_ini_dir, dir);

	if (g_verbose)
	{
		console_message_string(u8"設定ファイル： %s\n", g_ini_path);
	}
	return true;
}

/******************************************************************
** EXPORT関数定義（音響モデルファイル関連）
*/

OPENJTALK_DLL_API HtsVoiceFilelist *OPENJTALK_CONVENTION openjtalk_getHTSVoiceListSjis(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	counter_reset_file_or_directory_list();
	HtsVoiceFilelist *result = (HtsVoiceFilelist *)get_file_or_directory_list(oj->dn_voice_dir_path, G_VOICE_WILDCARD, false, OPENJTALKCHARSET_SHIFT_JIS);
	for (HtsVoiceFilelist *list = result; list != NULL; list = list->succ)
	{
		if (list->name != NULL && strlen(list->name) > strlen(G_VOICE_EXT))
		{
			((char *)list->name)[strlen(list->name) - strlen(G_VOICE_EXT)] = '\0';
		}
	}
	return result;
}

OPENJTALK_DLL_API HtsVoiceFilelist *OPENJTALK_CONVENTION openjtalk_getHTSVoiceList(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}
	counter_reset_file_or_directory_list();
	HtsVoiceFilelist *result = (HtsVoiceFilelist *)get_file_or_directory_list(oj->dn_voice_dir_path, G_VOICE_WILDCARD, false, OPENJTALKCHARSET_UTF_8);
	for (HtsVoiceFilelist *list = result; list != NULL; list = list->succ)
	{
		if (list->name != NULL && strlen(list->name) > strlen(G_VOICE_EXT))
		{
			((char *)list->name)[strlen(list->name) - strlen(G_VOICE_EXT)] = '\0';
		}
	}
	return result;
}

OPENJTALK_DLL_API HtsVoiceFilelist *OPENJTALK_CONVENTION openjtalk_getHTSVoiceListU16(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}
	counter_reset_file_or_directory_list();
	HtsVoiceFilelist *result = (HtsVoiceFilelist *)get_file_or_directory_list(oj->dn_voice_dir_path, G_VOICE_WILDCARD, false, OPENJTALKCHARSET_UTF_16);
	for (HtsVoiceFilelist *list = result; list != NULL; list = list->succ)
	{
		size_t len = strlenU16(list->nameU16);
		if (list->nameU16 != NULL && len > strlen(G_VOICE_EXT))
		{
			list->nameU16[len - strlen(G_VOICE_EXT)] = L'\0';
		}
	}
	return result;
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_clearHTSVoiceList(OpenJTalk *oj, HtsVoiceFilelist *list)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	HtsVoiceFilelist *prev = NULL;

	while (list != NULL)
	{
		if (prev != NULL)
		{
			free(prev);
		}

		if (list->path)
		{
			list->path[0] = '\0';
		}
		if (list->name)
		{
			list->name[0] = '\0';
		}

		if (list->path)
		{
			free(list->path);
		}
		if (list->name)
		{
			free(list->name);
		}

		prev = list;
		list = list->succ;
	}

	if (prev != NULL)
	{
		free(prev);
	}
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getHTSVoiceName(OpenJTalk *oj, HtsVoiceFilelist *list, unsigned int i, char *name)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (list == NULL)
	{
		return NULL;
	}

	//HtsVoiceFilelist* temp = list;
	unsigned int counter = 0;
	for (HtsVoiceFilelist *temp = list; temp != NULL; temp = temp->succ)
	{
		if (counter++ == i)
		{
			if (temp)
			{
				clear_path_string(name, MAX_PATH);
				strcpy(name, temp->name);
				return name;
			}
			else
			{
				return NULL;
			}
		}
	}
	return NULL;
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getHTSVoiceNameSjis(OpenJTalk *oj, HtsVoiceFilelist *list, unsigned int i, char *name)
{
	return openjtalk_getHTSVoiceName(oj, list, i, name);
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getHTSVoiceNameU16(OpenJTalk *oj, HtsVoiceFilelist *list, unsigned int i, char16_t *name)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (list == NULL)
	{
		return NULL;
	}

	//HtsVoiceFilelist* temp = list;
	unsigned int counter = 0;
	for (HtsVoiceFilelist *temp = list; temp != NULL; temp = temp->succ)
	{
		if (counter++ == i)
		{
			if (temp)
			{
				clear_path_stringU16(name, MAX_PATH);
				strcpyU16(name, temp->nameU16);
				return name;
			}
			else
			{
				return NULL;
			}
		}
	}
	return NULL;
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getHTSVoicePath(OpenJTalk *oj, HtsVoiceFilelist *list, unsigned int i, char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (list == NULL)
	{
		return NULL;
	}

	//HtsVoiceFilelist* temp = list;
	unsigned int counter = 0;
	for (HtsVoiceFilelist *temp = list; temp != NULL; temp = temp->succ)
	{
		if (counter++ == i)
		{
			if (temp)
			{
				clear_path_string(path, MAX_PATH);
				strcpy(path, temp->path);
				return path;
			}
			else
			{
				return NULL;
			}
		}
	}
	return NULL;
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getHTSVoicePathSjis(OpenJTalk *oj, HtsVoiceFilelist *list, unsigned int i, char *path)
{
	return openjtalk_getHTSVoicePath(oj, list, i, path);
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getHTSVoicePathU16(OpenJTalk *oj, HtsVoiceFilelist *list, unsigned int i, char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (list == NULL)
	{
		return NULL;
	}

	//HtsVoiceFilelist* temp = list;
	unsigned int counter = 0;
	for (HtsVoiceFilelist *temp = list; temp != NULL; temp = temp->succ)
	{
		if (counter++ == i)
		{
			if (temp)
			{
				clear_path_stringU16(path, MAX_PATH);
				strcpyU16(path, temp->pathU16);
				return path;
			}
			else
			{
				return NULL;
			}
		}
	}
	return NULL;
}

OPENJTALK_DLL_API unsigned int OPENJTALK_CONVENTION openjtalk_getHTSVoiceCount(OpenJTalk *oj, HtsVoiceFilelist *list)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0;
	}

	unsigned int counter = 0;
	while (list)
	{
		counter++;
		list = list->succ;
	}

	return counter;
}

/******************************************************************
** EXPORT関数定義（一般）
*/

// コピーライト情報を表示する
OPENJTALK_DLL_API void OPENJTALK_CONVENTION jtalkdll_copyright()
{
	fprintf(stderr, u8"%s, Open JTalk Dynamic Link Libraries \n", DLL_NAME);
	if (strlen(GIT_REV))
	{
		fprintf(stderr, u8"version %d.%d.%d revision(%s)\n", VER_MAJOR, VER_MINOR, VER_BUILD, GIT_REV);
	}
	else
	{
		fprintf(stderr, u8"version %d.%d.%d\n", VER_MAJOR, VER_MINOR, VER_BUILD);
	}
	fprintf(stderr, u8"https://github.com/rosmarinus/jtalkdll.git\n");
	fprintf(stderr, u8"Copyright (C) 2020 takayan\n");
	fprintf(stderr, u8"All rights reserved.\n");
	fprintf(stderr, u8"\n");
	Open_JTalk_COPYRIGHT();

	// portaudio
	fprintf(stderr, u8"PortAudio Portable Real-Time Audio Library\n");
	fprintf(stderr, u8"Copyright (c) 1999-2011 Ross Bencina and Phil Burk\n");
	fprintf(stderr, u8"http://www.portaudio.com/\n");
	fprintf(stderr, u8"\n");
}

OpenJTalk *openjtalk_initialize_sub(const char *voice, const char *dic, const char *voiceDir)
{

#ifdef _DEBUG
	g_verbose = true;
#endif

#if (!defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)) || defined(WINDOWS_PORTAUDIO)
	if (Pa_Initialize() != paNoError)
	{
		if (g_verbose)
		{
			console_message(u8"PortAudioの初期化に失敗しました。\n");
		}
		return NULL;
	}
#endif

	OpenJTalk *oj = (OpenJTalk *)malloc(sizeof(OpenJTalk));
	if (!oj)
	{
		return NULL;
	}

	set_current_path();

	Open_JTalk *open_jtalk = Open_JTalk_initialize();
	if (open_jtalk == NULL)
	{
		return NULL;
	}
	oj->open_jtalk = open_jtalk;

	clear_path_string(oj->dn_dic_path_init, MAX_PATH);
	if (dic != NULL)
	{
		strcpy(oj->dn_dic_path_init, dic);
	}

	clear_path_string(oj->fn_voice_path_init, MAX_PATH);
	if (voice != NULL)
	{
		strcpy(oj->fn_voice_path_init, voice);
	}

	clear_path_string(oj->dn_voice_dir_path_init, MAX_PATH);
	if (voiceDir != NULL)
	{
		strcpy(oj->dn_voice_dir_path_init, voiceDir);
	}

	if (!JTalkData_initialize(oj))
	{
		return NULL;
	}

	return oj;
}

OPENJTALK_DLL_API OpenJTalk *OPENJTALK_CONVENTION openjtalk_initializeSjis(const char *voice, const char *dic, const char *voiceDir)
{
	char voice_temp[MAX_PATH];
	char dic_temp[MAX_PATH];
	char voiceDir_temp[MAX_PATH];

	voice_temp[0] = '\0';
	if (voice != NULL)
	{
#if defined(_WIN32)
		sjistou8_path(voice, voice_temp);
#else
		sjistou8_path(voice, voice_temp);
#endif
	}

	dic_temp[0] = '\0';
	if (dic != NULL)
	{
#if defined(_WIN32)
		sjistou8_path(dic, dic_temp);
#else
		sjistou8_path(dic, dic_temp);
#endif
	}

	voiceDir_temp[0] = '\0';
	if (voiceDir != NULL)
	{
#if defined(_WIN32)
		sjistou8_path(voiceDir, voiceDir_temp);
#else
		sjistou8_path(voiceDir, voiceDir_temp);
#endif
	}

	return openjtalk_initialize_sub(voice_temp, dic_temp, voiceDir_temp);
}

OPENJTALK_DLL_API OpenJTalk *OPENJTALK_CONVENTION openjtalk_initialize(const char *voice, const char *dic, const char *voiceDir)
{
	char voice_temp[MAX_PATH];
	char dic_temp[MAX_PATH];
	char voiceDir_temp[MAX_PATH];

	voice_temp[0] = '\0';
	if (voice != NULL)
	{
#if defined(_WIN32)
		u8tou8_path(voice, voice_temp);
#else
		u8tou8_path(voice, voice_temp);
#endif
	}

	dic_temp[0] = '\0';
	if (dic != NULL)
	{
#if defined(_WIN32)
		u8tou8_path(dic, dic_temp);
#else
		u8tou8_path(dic, dic_temp);
#endif
	}

	voiceDir_temp[0] = '\0';
	if (voiceDir != NULL)
	{
#if defined(_WIN32)
		u8tou8_path(voiceDir, voiceDir_temp);
#else
		u8tou8_path(voiceDir, voiceDir_temp);
#endif
	}

	return openjtalk_initialize_sub(voice_temp, dic_temp, voiceDir_temp);
}

OPENJTALK_DLL_API OpenJTalk *OPENJTALK_CONVENTION openjtalk_initializeU16(const char16_t *voice, const char16_t *dic, const char16_t *voiceDir)
{
	char voice_temp[MAX_PATH];
	char dic_temp[MAX_PATH];
	char voiceDir_temp[MAX_PATH];

	voice_temp[0] = '\0';
	if (voice != NULL)
	{
#if defined(_WIN32)
		u16tou8_path(voice, voice_temp);
#else
		u16tou8_path(voice, voice_temp);
#endif
	}

	dic_temp[0] = '\0';
	if (dic != NULL)
	{
#if defined(_WIN32)
		u16tou8_path(dic, dic_temp);
#else
		u16tou8_path(dic, dic_temp);
#endif
	}

	voiceDir_temp[0] = '\0';
	if (voiceDir != NULL)
	{
#if defined(_WIN32)
		u16tou8_path(voiceDir, voiceDir_temp);
#else
		u16tou8_path(voiceDir, voiceDir_temp);
#endif
	}

	return openjtalk_initialize_sub(voice_temp, dic_temp, voiceDir_temp);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_clear(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}
#if (!defined(_WIN32) || defined(__CYGWIN__) || defined(__MINGW32__)) || defined(WINDOWS_PORTAUDIO)
	Pa_Terminate();
#else
#endif
	Open_JTalk_clear(oj->open_jtalk);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_refresh(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	Open_JTalk_refresh(oj->open_jtalk);

	if (!JTalkData_initialize(oj))
	{
		return;
	}
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_wait(OpenJTalk *oj, int duration)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}
	sleep_internal(duration);
}

/*****************************************************************
** EXPORT関数定義（パラメータ設定）
*/

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setSamplingFrequency(OpenJTalk *oj, unsigned int i)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (i < 1)
	{
		oj->errorCode = OPENJTALKERROR_VALUE_ERROR;
		return;
	}
	oj->sampling_frequency = i;
	Open_JTalk_set_sampling_frequency(oj->open_jtalk, oj->sampling_frequency);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_s(OpenJTalk *oj, unsigned int i)
{
	openjtalk_setSamplingFrequency(oj, i);
}

OPENJTALK_DLL_API unsigned int OPENJTALK_CONVENTION openjtalk_getSamplingFrequency(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_sampling_frequency(oj->open_jtalk, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0;
	}
	oj->sampling_frequency = res;
	return oj->sampling_frequency;
}

OPENJTALK_DLL_API unsigned int OPENJTALK_CONVENTION openjtalk_get_s(OpenJTalk *oj)
{
	return openjtalk_getSamplingFrequency(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setFperiod(OpenJTalk *oj, unsigned int i)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (i < 1)
	{
		i = 1;
	}
	oj->fperiod = i;
	Open_JTalk_set_fperiod(oj->open_jtalk, oj->fperiod);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_p(OpenJTalk *oj, unsigned int i)
{
	openjtalk_setFperiod(oj, i);
}

OPENJTALK_DLL_API unsigned int OPENJTALK_CONVENTION openjtalk_getFperiod(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_fperiod(oj->open_jtalk, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0;
	}
	oj->fperiod = res;
	return oj->fperiod;
}

OPENJTALK_DLL_API unsigned int OPENJTALK_CONVENTION openjtalk_get_p(OpenJTalk *oj)
{
	return openjtalk_getFperiod(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setAlpha(OpenJTalk *oj, double f)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (f < 0.0)
	{
		f = 0.0;
	}
	else if (f > 1.0)
	{
		f = 1.0;
	}
	oj->alpha = f;
	Open_JTalk_set_alpha(oj->open_jtalk, oj->alpha);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_a(OpenJTalk *oj, double f)
{
	openjtalk_setAlpha(oj, f);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getAlpha(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_alpha(oj->open_jtalk, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->alpha = res;
	return oj->alpha;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_a(OpenJTalk *oj)
{
	return openjtalk_getAlpha(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setBeta(OpenJTalk *oj, double f)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (f < 0.0)
	{
		f = 0.0;
	}
	else if (f > 1.0)
	{
		f = 1.0;
	}
	oj->beta = f;
	Open_JTalk_set_beta(oj->open_jtalk, oj->beta);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_b(OpenJTalk *oj, double f)
{
	openjtalk_setBeta(oj, f);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getBeta(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_beta(oj->open_jtalk, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->beta = res;
	return oj->beta;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_b(OpenJTalk *oj)
{
	return openjtalk_getBeta(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setSpeed(OpenJTalk *oj, double f)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (f < 0.0)
	{
		f = 0.0;
	}
	oj->speed = f;
	Open_JTalk_set_speed(oj->open_jtalk, oj->speed);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_r(OpenJTalk *oj, double f)
{
	openjtalk_setSpeed(oj, f);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getSpeed(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_speed(oj->open_jtalk, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->speed = res;
	return oj->speed;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_r(OpenJTalk *oj)
{
	return openjtalk_getSpeed(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setAdditionalHalfTone(OpenJTalk *oj, double f)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	oj->additional_half_tone = f;
	Open_JTalk_set_additional_half_tone(oj->open_jtalk, oj->additional_half_tone);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_fm(OpenJTalk *oj, double f)
{
	openjtalk_setAdditionalHalfTone(oj, f);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getAdditionalHalfTone(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_additional_half_tone(oj->open_jtalk, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->additional_half_tone = res;
	return oj->additional_half_tone;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_fm(OpenJTalk *oj)
{
	return openjtalk_getAdditionalHalfTone(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setMsdThreshold(OpenJTalk *oj, double f)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (f < 0.0)
	{
		f = 0.0;
	}
	else if (f > 1.0)
	{
		f = 1.0;
	}
	oj->msd_threshold = f;
	Open_JTalk_set_msd_threshold(oj->open_jtalk, 1, oj->msd_threshold);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_u(OpenJTalk *oj, double f)
{
	openjtalk_setMsdThreshold(oj, f);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getMsdThreshold(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_msd_threshold(oj->open_jtalk, 0, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->msd_threshold = res;
	return oj->msd_threshold;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_u(OpenJTalk *oj)
{
	return openjtalk_getMsdThreshold(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setGvWeight(OpenJTalk *oj, unsigned int i, double f)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (f < 0.0)
	{
		f = 0.0;
	}
	if (i == 0)
	{
		oj->gv_weight0 = f;
	}
	else if (i == 1)
	{
		oj->gv_weight1 = f;
	}
	Open_JTalk_set_gv_weight(oj->open_jtalk, i, f);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setGvWeightForSpectrum(OpenJTalk *oj, double f)
{
	openjtalk_setGvWeight(oj, 0, f);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_jm(OpenJTalk *oj, double f)
{
	openjtalk_setGvWeight(oj, 0, f);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setGvWeightForLogF0(OpenJTalk *oj, double f)
{
	openjtalk_setGvWeight(oj, 1, f);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_jf(OpenJTalk *oj, double f)
{
	openjtalk_setGvWeight(oj, 1, f);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getGvWeightForSpectrum(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_gv_weight(oj->open_jtalk, 0, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->gv_weight0 = res;
	return oj->gv_weight0;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_jm(OpenJTalk *oj)
{
	return openjtalk_getGvWeightForSpectrum(oj);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getGvWeightForLogF0(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_gv_weight(oj->open_jtalk, 1, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->gv_weight1 = res;
	return oj->gv_weight1;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_jf(OpenJTalk *oj)
{
	return openjtalk_getGvWeightForLogF0(oj);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setVolume(OpenJTalk *oj, double f)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	oj->volume = f;
	Open_JTalk_set_volume(oj->open_jtalk, oj->volume);
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_set_g(OpenJTalk *oj, double f)
{
	openjtalk_setVolume(oj, f);
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_getVolume(OpenJTalk *oj)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	bool error;
	double res = Open_JTalk_get_volume(oj->open_jtalk, &error);
	if (error)
	{
		oj->errorCode = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return 0.0;
	}
	oj->volume = res;
	return oj->volume;
}

OPENJTALK_DLL_API double OPENJTALK_CONVENTION openjtalk_get_g(OpenJTalk *oj)
{
	return openjtalk_getVolume(oj);
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceDirSjis(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}

	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = sjistou8_path(path, temp);
#else
	char *res = sjistou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_dir_and_voice(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceDirSjis2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoiceDirSjis(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoiceDirSjis(OpenJTalk *oj, char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->dn_voice_dir_path || strlen(oj->dn_voice_dir_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char *res = u8tosjis_path(oj->dn_voice_dir_path, path);
#else
	char *res = u8tosjis_path(oj->dn_voice_dir_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceDir(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u8tou8_path(path, temp);
#else
	char *res = u8tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_dir_and_voice(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceDir2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoiceDirSjis(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoiceDir(OpenJTalk *oj, char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->dn_voice_dir_path || strlen(oj->dn_voice_dir_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char *res = u8tou8_path(oj->dn_voice_dir_path, path);
#else
	char *res = u8tou8_path(oj->dn_voice_dir_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceDirU16(OpenJTalk *oj, const char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlenU16(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u16tou8_path(path, temp);
#else
	char *res = u16tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_dir_and_voice(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceDirU162(OpenJTalk *oj, const char16_t *path)
{
	return openjtalk_setVoiceDirU16(oj, path);
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getVoiceDirU16(OpenJTalk *oj, char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->dn_voice_dir_path || strlen(oj->dn_voice_dir_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char16_t *res = u8tou16_path(oj->dn_voice_dir_path, path);
#else
	char16_t *res = u8tou16_path(oj->dn_voice_dir_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setDicSjis(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = sjistou8_path(path, temp);
#else
	char *res = sjistou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_dic_path(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setDicSjis2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setDicSjis(oj, path);
}

char *getlocale(int cat)
{
	return setlocale(cat, NULL);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getDicSjis(OpenJTalk *oj, char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->dn_dic_path || strlen(oj->dn_dic_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char *res = u8tosjis_path(oj->dn_dic_path, path);
#else
	char *res = u8tosjis_path(oj->dn_dic_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setDic(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u8tou8_path(path, temp);
#else
	char *res = u8tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_dic_path(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setDic2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setDic(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getDic(OpenJTalk *oj, char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->dn_dic_path || strlen(oj->dn_dic_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char *res = u8tou8_path(oj->dn_dic_path, path);
#else
	char *res = u8tou8_path(oj->dn_dic_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setDicU16(OpenJTalk *oj, const char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlenU16(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u16tou8_path(path, temp);
#else
	char *res = u16tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_dic_path(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setDicU162(OpenJTalk *oj, const char16_t *path)
{
	return openjtalk_setDicU16(oj, path);
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getDicU16(OpenJTalk *oj, char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->dn_dic_path || strlen(oj->dn_dic_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char16_t *res = u8tou16_path(oj->dn_dic_path, path);
#else
	char16_t *res = u8tou16_path(oj->dn_dic_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoicePathSjis(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}

	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = sjistou8_path(path, temp);
#else
	char *res = sjistou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_path(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoicePathSjis2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoicePathSjis(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoicePathSjis(OpenJTalk *oj, char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->fn_voice_path || strlen(oj->fn_voice_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char *res = u8tosjis_path(oj->fn_voice_path, path);
#else
	char *res = u8tosjis_path(oj->fn_voice_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoicePath(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}
	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u8tou8_path(path, temp);
#else
	char *res = u8tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_path(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoicePath2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoicePath(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoicePath(OpenJTalk *oj, char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->fn_voice_path || strlen(oj->fn_voice_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char *res = u8tou8_path(oj->fn_voice_path, path);
#else
	char *res = u8tou8_path(oj->fn_voice_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoicePathU16(OpenJTalk *oj, const char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlenU16(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u16tou8_path(path, temp);
#else
	char *res = u16tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_path(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoicePathU162(OpenJTalk *oj, const char16_t *path)
{
	return openjtalk_setVoicePathU16(oj, path);
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getVoicePathU16(OpenJTalk *oj, char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!oj->fn_voice_path || strlen(oj->fn_voice_path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}
#if defined(_WIN32)
	char16_t *res = u8tou16_path(oj->fn_voice_path, path);
#else
	char16_t *res = u8tou16_path(oj->fn_voice_path, path);
#endif
	return res;
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceNameSjis(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}

	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = sjistou8_path(path, temp);
#else
	char *res = sjistou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}

	return set_voice_name(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceNameSjis2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoiceNameSjis(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoiceNameSjis(OpenJTalk *oj, char *path)
{
	char temp[MAX_PATH];
	char temp0[MAX_PATH];
	char *res = openjtalk_getVoicePath(oj, temp);
	if (res == NULL)
	{
		return NULL;
	}
	get_fname(temp, temp0);
	return u8tosjis_path(temp0, path);
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceName(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}
	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u8tou8_path(path, temp);
#else
	char *res = u8tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_name(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceName2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoiceName(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoiceName(OpenJTalk *oj, char *path)
{
	char temp[MAX_PATH];
	char *res = openjtalk_getVoicePath(oj, temp);
	if (res == NULL)
	{
		return NULL;
	}
	return get_fname(temp, path);
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceNameU16(OpenJTalk *oj, const char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}
	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlenU16(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u16tou8_path(path, temp);
#else
	char *res = u16tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice_name(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceNameU162(OpenJTalk *oj, const char16_t *path)
{
	return openjtalk_setVoiceNameU16(oj, path);
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getVoiceNameU16(OpenJTalk *oj, char16_t *path)
{
	char temp[MAX_PATH];
	char *res = openjtalk_getVoiceName(oj, temp);
	if (res == NULL)
	{
		return NULL;
	}
	return u8tou16_path(temp, path);
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceSjis(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}

	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = sjistou8_path(path, temp);
#else
	char *res = sjistou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceSjis2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoiceSjis(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoiceSjis(OpenJTalk *oj, char *path)
{
	return openjtalk_getVoicePathSjis(oj, path);
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoice(OpenJTalk *oj, const char *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}
	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u8tou8_path(path, temp);
#else
	char *res = u8tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoice2(OpenJTalk *oj, const char *path)
{
	return openjtalk_setVoice(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getVoice(OpenJTalk *oj, char *path)
{
	return openjtalk_getVoicePath(oj, path);
}

OPENJTALK_DLL_API bool OPENJTALK_CONVENTION openjtalk_setVoiceU16(OpenJTalk *oj, const char16_t *path)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return false;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlenU16(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return false;
	}
	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u16tou8_path(path, temp);
#else
	char *res = u16tou8_path(path, temp);
#endif
	if (!res)
	{
		return false;
	}
	return set_voice(oj, temp);
}
OPENJTALK_DLL_API int OPENJTALK_CONVENTION openjtalk_setVoiceU162(OpenJTalk *oj, const char16_t *path)
{
	return openjtalk_setVoiceU16(oj, path);
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getVoiceU16(OpenJTalk *oj, char16_t *path)
{
	return openjtalk_getVoicePathU16(oj, path);
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getFullVoicePathSjis(OpenJTalk *oj, const char *path, char *buffer)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}

	if (!buffer)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = sjistou8_path(path, temp);
#else
	char *res = sjistou8_path(path, temp);
#endif

	if (!res)
	{
		return NULL;
	}

	char *temp2 = make_voice_path(oj, temp);
	if (!temp2)
	{
		return NULL;
	}

#if defined(_WIN32)
	char *res2 = u8tosjis_path(temp2, buffer);
#else
	char *res2 = u8tosjis_path(temp2, buffer);
#endif
	free(temp2);
	return res2;
}

OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_getFullVoicePath(OpenJTalk *oj, const char *path, char *buffer)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}
	if (!path)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlen(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}

	if (!buffer)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u8tou8_path(path, buffer);
#else
	char *res = u8tou8_path(path, buffer);
#endif
	if (!res)
	{
		return NULL;
	}

	char *temp2 = make_voice_path(oj, temp);
	if (!temp2)
	{
		return NULL;
	}

#if defined(_WIN32)
	char *res2 = u8tou8_path(temp2, buffer);
#else
	char *res2 = u8tou8_path(temp2, buffer);
#endif
	free(temp2);
	return res2;
}

OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_getFullVoicePathU16(OpenJTalk *oj, const char16_t *path, char16_t *buffer)
{
	if (!oj)
	{
		g_lastError = OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
		return NULL;
	}

	oj->errorCode = OPENJTALKERROR_NO_ERROR;
	if (!path || strlenU16(path) == 0)
	{
		oj->errorCode = OPENJTALKERROR_PATH_STRING_IS_NULL_OR_EMPTY;
		return NULL;
	}

	if (!buffer)
	{
		oj->errorCode = OPENJTALKERROR_BUFFER_IS_NULL;
		return NULL;
	}

	char temp[MAX_PATH];
#if defined(_WIN32)
	char *res = u16tou8_path(path, temp);
#else
	char *res = u16tou8_path(path, temp);
#endif

	if (!res)
	{
		return NULL;
	}

	char *temp2 = make_voice_path(oj, temp);
	if (!temp2)
	{
		return NULL;
	}

#if defined(_WIN32)
	char16_t *res2 = u8tou16_path(temp2, buffer);
#else
	char16_t *res2 = u8tou16_path(temp2, buffer);
#endif
	free(temp2);
	return res2;
}

OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_setVerbose(bool sw)
{
	g_verbose = sw;
}

// 実行中のOSの取得
OPENJTALK_DLL_API OPENJTALK_OS OPENJTALK_CONVENTION openjtalk_getOS(OpenJTalk *oj)
{
#if defined(_WIN32)
	return OPENJTALK_OS_WINDOWS;
#elif defined(__APPLE__)
#if TARGET_OS_IPHONE
	return OPENJTALK_OS_IPHONE;
#else
	return OPENJTALK_OS_OSX;
#endif
#elif defined(__linux)
	return OPENJTALK_OS_LINUX;
#else
	return OPENJTALK_OS_OTHER;
#endif
}

// 実行中のアーキテクチャーの取得
OPENJTALK_DLL_API OPENJTALK_ARCH OPENJTALK_CONVENTION openjtalk_getArch(OpenJTalk *oj)
{
#if defined(_WIN32)
#if defined(_WIN64)
	return OPENJTALK_ARCH_X64;
#else
	return OPENJTALK_ARCH_X86;
#endif
#elif defined(__arm__) || defined(_M_ARM) || defined(__aarch64__)
	return OPENJTALK_ARCH_ARM;
#else
	return OPENJTALK_ARCH_UNKNOWN;
#endif
}

// 文字コードを返す
OPENJTALK_DLL_API long OPENJTALK_CONVENTION openjtalk_getCharCode(char *text)
{
	if (!text)
	{
		return OPENJTALKERROR_BUFFER_IS_NULL;
	}
	long res = 0;
	int count = 0;
	for (char *p = text; *p != '\0'; p++)
	{
		res <<= sizeof(char) * 8;
		res += ((long)*p) & 0xff;
		if (++count == 4)
		{
			break;
		}
	}
	return (long)res;
}

OPENJTALK_DLL_API long OPENJTALK_CONVENTION openjtalk_getWideCharCode(char16_t *text)
{
	if (!text)
	{
		return OPENJTALKERROR_BUFFER_IS_NULL;
	}
	long res = 0;
	int count = 0;
	for (char16_t *p = text; *p != '\0'; p++)
	{
		res <<= sizeof(char16_t) * 8;
		res += ((long)*p) & 0xffff;
		if (++count == 2)
		{
			break;
		}
	}
	return (long)res;
}

// 現在のエラーコードを返す
OPENJTALK_DLL_API OpenjtalkErrors OPENJTALK_CONVENTION openjtalk_getErrorCode(OpenJTalk *oj)
{
	if (!oj)
	{
		return OPENJTALKERROR_OBJECT_POINTER_IS_NULL;
	}
	return oj->errorCode;
}

// UTF-16 -> UTF-8 への変換。使用後openjtalk_freeで解放する
// Windowsでのコンソールへの表示のため
OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_convertUtf16ToUtf8(const char16_t *source)
{
	return u16tou8(source);
}

// ShiftJIS -> UTF-8 への変換。使用後openjtalk_freeで解放する
// Windowsでのコンソールへの表示のため
OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_convertSjisToUtf8(const char *source)
{
	return sjistou8(source);
}

// UTF-16 -> ShiftJIS への変換。使用後openjtalk_freeで解放する
// Windowsでのコンソールへの表示のため
OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_convertUtf16ToSjis(const char16_t *source)
{
	return u16tosjis(source);
}

// UTF-8 -> ShiftJIS への変換。使用後openjtalk_freeで解放する
// Windowsでのコンソールへの表示のため
OPENJTALK_DLL_API char *OPENJTALK_CONVENTION openjtalk_convertUtf8ToSjis(const char *source)
{
	return u8tosjis(source);
}

// ShiftJIS -> UTF-16 への変換。使用後openjtalk_freeで解放する
// Windowsでのコンソールへの表示のため
OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_convertSjisToUtf16(const char *source)
{
	return sjistou16(source);
}

// UTF-8 -> UTF-16 への変換。使用後openjtalk_freeで解放する
// Windowsでのコンソールへの表示のため
OPENJTALK_DLL_API char16_t *OPENJTALK_CONVENTION openjtalk_convertUtf8ToUtf16(const char *source)
{
	return u8tou16(source);
}

// ただのfree
OPENJTALK_DLL_API void OPENJTALK_CONVENTION openjtalk_free(void *mem)
{
	if (!mem)
	{
		free(mem);
	}
}

#if defined(_WIN32)
#ifndef DISABLE_JTALK_DLLMAIN
// DLLMAIN
// Windowsでdll自身の位置を取得する処理を行う
BOOL WINAPI DllMain(HINSTANCE hInstDLL, DWORD dwReason, LPVOID lpReserved)
{
	char path[MAX_PATH];
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		GetModuleFileName(hInstDLL, path, MAX_PATH);
		get_dir_path(path, g_dll_path);
		if (g_verbose)
		{
			fprintf(stderr, "dll path: %s\n", path);
		}
		break;
	}
	return TRUE;
}
#else
// WindowsでラップしたC++/CLIのコードからdll自身の位置を設定する
void set_current_dll_path(const char *path)
{
	if (path != NULL && strlen(path) <= MAX_PATH)
	{
		if (g_verbose)
		{
			fprintf(stderr, "dll path: %s\n", path);
		}
		strcpy(g_dll_path, path);
	}
}
#endif
#endif

JTALK_C_END;
#endif /* JTALK_C */