﻿#ifndef OPEN_JTALK_C
#define OPEN_JTALK_C

#ifdef __cplusplus
#define OPEN_JTALK_C_START extern "C" {
#define OPEN_JTALK_C_END }
#else
#define OPEN_JTALK_C_START
#define OPEN_JTALK_C_END
#endif /* __CPLUSPLUS */

OPEN_JTALK_C_START;

#define OPEN_JTALK_BLOCKSIZE 1024
#define MAXBUFLEN 1024

/* Main headers */
#include "mecab.h"
#include "njd.h"
#include "jpcommon.h"
#include "HTS_engine.h"

/* Sub headers */
#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"
#include "njd2jpcommon.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AUDIO_PLAY_PORTAUDIO
#if defined(AUDIO_PLAY_PORTAUDIO)
#if defined(_WIN32) && !defined(__CYGWIN__) && !defined(__MINGW32__)
#if defined(_WIN64)
#pragma comment(lib, "portaudio_static_x64.lib")
#else
#pragma comment(lib, "portaudio_static_x86.lib")
#endif
#endif
#endif

typedef struct Open_JTalk_tag
{
	Mecab mecab;
	NJD njd;
	JPCommon jpcommon;
	HTS_Engine engine;
} Open_JTalk;

Open_JTalk *Open_JTalk_initialize()
{
	Open_JTalk *open_jtalk = (Open_JTalk *)malloc(sizeof(Open_JTalk));
	if (open_jtalk == NULL)
	{
		return NULL;
	}
	Mecab_initialize(&open_jtalk->mecab);
	NJD_initialize(&open_jtalk->njd);
	JPCommon_initialize(&open_jtalk->jpcommon);
	HTS_Engine_initialize(&open_jtalk->engine);
	return open_jtalk;
}

void Open_JTalk_clear(Open_JTalk *open_jtalk)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	Mecab_clear(&open_jtalk->mecab);
	NJD_clear(&open_jtalk->njd);
	JPCommon_clear(&open_jtalk->jpcommon);
	HTS_Engine_clear(&open_jtalk->engine);
}

void Open_JTalk_refresh(Open_JTalk *open_jtalk)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	if (JPCommon_get_label_size(&open_jtalk->jpcommon) > 2)
	{
		HTS_Engine_refresh(&open_jtalk->engine);
	}
	JPCommon_refresh(&open_jtalk->jpcommon);
	NJD_refresh(&open_jtalk->njd);
	Mecab_refresh(&open_jtalk->mecab);
}

bool Open_JTalk_load_dic(Open_JTalk *open_jtalk, char *dn_mecab)
{
	if (open_jtalk == NULL || dn_mecab == NULL || strlen(dn_mecab) == 0)
	{
		return false;
	}

	JPCommon_refresh(&open_jtalk->jpcommon);
	NJD_refresh(&open_jtalk->njd);
	Mecab_refresh(&open_jtalk->mecab);

	if (Mecab_load(&open_jtalk->mecab, dn_mecab) != true)
	{
		Open_JTalk_clear(open_jtalk);
		return false;
	}
	return true;
}

bool Open_JTalk_load_voice(Open_JTalk *open_jtalk, char *fn_voice)
{
	if (open_jtalk == NULL || fn_voice == NULL || strlen(fn_voice) == 0)
	{
		return false;
	}

	if (JPCommon_get_label_size(&open_jtalk->jpcommon) > 2)
	{
		HTS_Engine_refresh(&open_jtalk->engine);
	}
	JPCommon_refresh(&open_jtalk->jpcommon);
	NJD_refresh(&open_jtalk->njd);

	if (HTS_Engine_load(&open_jtalk->engine, &fn_voice, 1) != true)
	{
		Open_JTalk_clear(open_jtalk);
		return false;
	}
	if (strcmp(HTS_Engine_get_fullcontext_label_format(&open_jtalk->engine), "HTS_TTS_JPN") != 0)
	{
		Open_JTalk_clear(open_jtalk);
		return false;
	}
	return true;
}

bool Open_JTalk_load(Open_JTalk *open_jtalk, char *dn_mecab, char *fn_voice)
{
	if (open_jtalk == NULL)
	{
		return false;
	}
	if (Mecab_load(&open_jtalk->mecab, dn_mecab) != true)
	{
		Open_JTalk_clear(open_jtalk);
		return false;
	}
	if (HTS_Engine_load(&open_jtalk->engine, &fn_voice, 1) != true)
	{
		Open_JTalk_clear(open_jtalk);
		return false;
	}
	if (strcmp(HTS_Engine_get_fullcontext_label_format(&open_jtalk->engine), "HTS_TTS_JPN") != 0)
	{
		Open_JTalk_clear(open_jtalk);
		return false;
	}
	return true;
}

void Open_JTalk_set_sampling_frequency(Open_JTalk *open_jtalk, size_t i)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_sampling_frequency(&open_jtalk->engine, i);
}

size_t Open_JTalk_get_sampling_frequency(Open_JTalk *open_jtalk, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return HTS_Engine_get_sampling_frequency(&open_jtalk->engine);
}

void Open_JTalk_set_fperiod(Open_JTalk *open_jtalk, size_t i)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_fperiod(&open_jtalk->engine, i);
}

size_t Open_JTalk_get_fperiod(Open_JTalk *open_jtalk, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return open_jtalk->engine.condition.fperiod;
}

void Open_JTalk_set_alpha(Open_JTalk *open_jtalk, double f)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_alpha(&open_jtalk->engine, f);
}

double Open_JTalk_get_alpha(Open_JTalk *open_jtalk, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return HTS_Engine_get_alpha(&open_jtalk->engine);
}

void Open_JTalk_set_beta(Open_JTalk *open_jtalk, double f)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_beta(&open_jtalk->engine, f);
}

double Open_JTalk_get_beta(Open_JTalk *open_jtalk, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return HTS_Engine_get_beta(&open_jtalk->engine);
}

void Open_JTalk_set_speed(Open_JTalk *open_jtalk, double f)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_speed(&open_jtalk->engine, f);
}

double Open_JTalk_get_speed(Open_JTalk *open_jtalk, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return open_jtalk->engine.condition.speed;
}

void Open_JTalk_set_additional_half_tone(Open_JTalk *open_jtalk, double f)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_add_half_tone(&open_jtalk->engine, f);
}

double Open_JTalk_get_additional_half_tone(Open_JTalk *open_jtalk, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return open_jtalk->engine.condition.additional_half_tone;
}

void Open_JTalk_set_msd_threshold(Open_JTalk *open_jtalk, size_t i, double f)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_msd_threshold(&open_jtalk->engine, i, f);
}

double Open_JTalk_get_msd_threshold(Open_JTalk *open_jtalk, size_t i, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	if (i != 0)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return HTS_Engine_get_msd_threshold(&open_jtalk->engine, i);
}

void Open_JTalk_set_gv_weight(Open_JTalk *open_jtalk, size_t i, double f)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_gv_weight(&open_jtalk->engine, i, f);
}

double Open_JTalk_get_gv_weight(Open_JTalk *open_jtalk, size_t i, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	if (i != 0 && i != 1)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return HTS_Engine_get_gv_weight(&open_jtalk->engine, i);
}

void Open_JTalk_set_volume(Open_JTalk *open_jtalk, double f)
{
	if (open_jtalk == NULL)
	{
		return;
	}
	HTS_Engine_set_volume(&open_jtalk->engine, f);
}

double Open_JTalk_get_volume(Open_JTalk *open_jtalk, bool *error)
{
	if (error)
	{
		*error = false;
	}

	if (open_jtalk == NULL)
	{
		if (error)
		{
			*error = true;
		}
		return 0;
	}
	return HTS_Engine_get_volume(&open_jtalk->engine);
}

bool Open_JTalk_generate_sounddata(Open_JTalk *open_jtalk,
								   const char *txt,
								   short **sounddata,
								   size_t *size,
								   size_t *sampling_frequency)
{
	if (open_jtalk == NULL)
	{
		return false;
	}
	char buff[MAXBUFLEN];
	text2mecab(buff, txt);
	Mecab_analysis(&open_jtalk->mecab, buff);
	mecab2njd(&open_jtalk->njd, Mecab_get_feature(&open_jtalk->mecab),
			  Mecab_get_size(&open_jtalk->mecab));
	njd_set_pronunciation(&open_jtalk->njd);
	njd_set_digit(&open_jtalk->njd);
	njd_set_accent_phrase(&open_jtalk->njd);
	njd_set_accent_type(&open_jtalk->njd);
	njd_set_unvoiced_vowel(&open_jtalk->njd);
	njd_set_long_vowel(&open_jtalk->njd);
	njd2jpcommon(&open_jtalk->jpcommon, &open_jtalk->njd);
	JPCommon_make_label(&open_jtalk->jpcommon);
	if (JPCommon_get_label_size(&open_jtalk->jpcommon) > 2)
	{
		if (HTS_Engine_synthesize_from_strings(
				&open_jtalk->engine,
				JPCommon_get_label_feature(&open_jtalk->jpcommon), JPCommon_get_label_size(&open_jtalk->jpcommon)) == TRUE)
		{
			short temp;
			HTS_GStreamSet *gss = &open_jtalk->engine.gss;
			size_t len = (gss->total_nsample / OPEN_JTALK_BLOCKSIZE + 1) * OPEN_JTALK_BLOCKSIZE;
			short *data = (short *)calloc(len, sizeof(short));
			for (unsigned int i = 0; i < gss->total_nsample; i++)
			{
				double x = gss->gspeech[i];
				if (x > 32767.0)
				{
					temp = 32767;
				}
				else if (x < -32768.0)
				{
					temp = -32768;
				}
				else
				{
					temp = (short)x;
				}
				data[i] = temp;
			}
			if (len != gss->total_nsample)
			{
				memset(data + gss->total_nsample, '\0', len - gss->total_nsample);
			}
			*sounddata = data;
			*size = len;
			*sampling_frequency = (&open_jtalk->engine)->condition.sampling_frequency;
			Open_JTalk_refresh(open_jtalk);
			return true;
		}
	}
	Open_JTalk_refresh(open_jtalk);
	return false;
}

void Open_JTalk_COPYRIGHT()
{
	fprintf(stderr, "The Japanese TTS System \"Open JTalk\"\n");
	fprintf(stderr, "Version 1.10 (http://open-jtalk.sourceforge.net/)\n");
	fprintf(stderr, "Copyright (C) 2008-2016 Nagoya Institute of Technology\n");
	fprintf(stderr, "All rights reserved.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "%s", HTS_COPYRIGHT);
	fprintf(stderr, "\n");
	fprintf(stderr, "Yet Another Part-of-Speech and Morphological Analyzer \"Mecab\"\n");
	fprintf(stderr, "Version 0.996 (http://mecab.sourceforge.net/)\n");
	fprintf(stderr, "Copyright (C) 2001-2008 Taku Kudo\n");
	fprintf(stderr, "              2004-2008 Nippon Telegraph and Telephone Corporation\n");
	fprintf(stderr, "All rights reserved.\n");
	fprintf(stderr, "\n");
	fprintf(stderr, "NAIST Japanese Dictionary\n");
	fprintf(stderr, "Version 0.6.1-20090630 (http://naist-jdic.sourceforge.jp/)\n");
	fprintf(stderr, "Copyright (C) 2009 Nara Institute of Science and Technology\n");
	fprintf(stderr, "All rights reserved.\n");
	fprintf(stderr, "\n");
}

OPEN_JTALK_C_END;
#endif /* !OPEN_JTALK_C */
