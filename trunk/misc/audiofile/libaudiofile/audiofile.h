/*
	Audio File Library
	Copyright (C) 1998-1999, Michael Pruett <michael@68k.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the 
	Free Software Foundation, Inc., 59 Temple Place - Suite 330, 
	Boston, MA  02111-1307  USA.
*/

/*
	audiofile.h

	This file contains the public interfaces to the Audio File Library.
*/

#ifndef AUDIOFILE_H
#define AUDIOFILE_H

/*
	AFvirtualfile is the preferred type to use; AF_VirtualFile will go
	away.
*/
typedef struct _AF_VirtualFile AFvirtualfile;
typedef struct _AF_VirtualFile AF_VirtualFile;

#include <sys/types.h>
#include "aupvlist.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct _AFfilesetup *AFfilesetup;
typedef struct _AFfilehandle *AFfilehandle;
typedef void (*AFerrfunc)(long, const char *);

typedef off_t AFframecount;
typedef off_t AFfileoffset;

#define AF_NULL_FILESETUP	((struct _AFfilesetup *) 0)
#define AF_NULL_FILEHANDLE	((struct _AFfilehandle *) 0)

#define AF_ERR_BASE 3000

enum
{
	AF_DEFAULT_TRACK = 1001
};

enum
{
	AF_DEFAULT_INST = 2001
};

enum
{
	AF_NUM_UNLIMITED = 99999
};

enum
{
	AF_BYTEORDER_BIGENDIAN = 501,
	AF_BYTEORDER_LITTLEENDIAN = 502
};

enum
{
	AF_FILE_UNKNOWN = -1,
	AF_FILE_RAWDATA = 0,
	AF_FILE_AIFFC = 1,
	AF_FILE_AIFF = 2,
	AF_FILE_NEXTSND = 3,
	AF_FILE_WAVE = 4,
	AF_FILE_BICSF = 5,
	AF_FILE_MPEG1BITSTREAM = 6,
	AF_FILE_SOUNDDESIGNER1 = 7,
	AF_FILE_SOUNDDESIGNER2 = 8,
	AF_FILE_AVR = 9,
	AF_FILE_IFF_8SVX = 10,
	AF_FILE_SAMPLEVISION = 11,
	AF_FILE_VOC = 12,
	AF_FILE_NIST_SPHERE = 13,
	AF_FILE_SOUNDFONT2 = 14
};

#define AF_FILE_IRCAM AF_FILE_BICSF

enum
{
	AF_LOOP_MODE_NOLOOP = 0,
	AF_LOOP_MODE_FORW = 1,
	AF_LOOP_MODE_FORWBACKW = 2
};

enum
{
	AF_SAMPFMT_TWOSCOMP = 401, /* linear two's complement */
	AF_SAMPFMT_UNSIGNED = 402, /* unsigned integer */
	AF_SAMPFMT_FLOAT = 403, /* 32-bit IEEE floating-point */
	AF_SAMPFMT_DOUBLE = 404 /* 64-bit IEEE double-precision floating-point */
};

enum
{
	AF_INST_LOOP_OFF = 0,			/* no looping */
	AF_INST_LOOP_CONTINUOUS = 1,	/* loop continuously through decay */
	AF_INST_LOOP_SUSTAIN = 3		/* loop during sustain, then continue */
};

enum
{
	AF_INST_MIDI_BASENOTE = 301,
	AF_INST_NUMCENTS_DETUNE = 302,
	AF_INST_MIDI_LONOTE = 303,
	AF_INST_MIDI_HINOTE = 304,
	AF_INST_MIDI_LOVELOCITY = 305,
	AF_INST_MIDI_HIVELOCITY = 306,
	AF_INST_NUMDBS_GAIN = 307,
	AF_INST_SUSLOOPID = 308,		/* loop id for AIFF sustain loop */
	AF_INST_RELLOOPID = 309,		/* loop id for AIFF release loop */
	AF_INST_SAMP_STARTFRAME = 310,	/* start sample for this inst */
	AF_INST_SAMP_ENDFRAME = 311,	/* end sample for this inst */
	AF_INST_SAMP_MODE = 312,		/* looping mode for this inst */
	AF_INST_TRACKID = 313,
	AF_INST_NAME = 314,				/* name of this inst */
	AF_INST_SAMP_RATE = 315,		/* sample rate of this inst's sample */
	AF_INST_PRESETID = 316,			/* ID of preset containing this inst */
	AF_INST_PRESET_NAME = 317		/* name of preset containing this inst */
};

enum
{
	AF_MISC_UNRECOGNIZED = 0,	/* unrecognized data chunk */
	AF_MISC_COPY = 201,	/* copyright string */
	AF_MISC_AUTH = 202,	/* author string */
	AF_MISC_NAME = 203,	/* name string */
	AF_MISC_ANNO = 204,	/* annotation string */
	AF_MISC_APPL = 205,	/* application-specific data */
	AF_MISC_MIDI = 206,	/* MIDI exclusive data */
	AF_MISC_PCMMAP = 207,	/* PCM mapping information (future use) */
	AF_MISC_NeXT = 208,	/* misc binary data appended to NeXT hdr */
	AF_MISC_IRCAM_PEAKAMP = 209,	/* peak amplitude information */
	AF_MISC_IRCAM_COMMENT = 210,	/* BICSF text comment */
	AF_MISC_COMMENT = 210,	/* general text comment */

	AF_MISC_ICMT = AF_MISC_COMMENT,	/* comments chunk (WAV format) */
	AF_MISC_ICRD = 211,  /* creation date (WAV format) */
	AF_MISC_ISFT = 212  /* software name (WAV format) */
};

enum
{
	/* supported compression schemes */
	AF_COMPRESSION_UNKNOWN = -1,
	AF_COMPRESSION_NONE = 0,
	AF_COMPRESSION_G722 = 501,
	AF_COMPRESSION_G711_ULAW = 502,
	AF_COMPRESSION_G711_ALAW = 503,

	/* Apple proprietary AIFF-C compression schemes (not supported) */
	AF_COMPRESSION_APPLE_ACE2 = 504,
	AF_COMPRESSION_APPLE_ACE8 = 505,
	AF_COMPRESSION_APPLE_MAC3 = 506,
	AF_COMPRESSION_APPLE_MAC6 = 507,

	AF_COMPRESSION_MPEG1 = 515,
	AF_COMPRESSION_AWARE_MULTIRATE = 516,

	AF_COMPRESSION_G726 = 517,
	AF_COMPRESSION_G728 = 518,
	AF_COMPRESSION_DVI_AUDIO = 519,
	AF_COMPRESSION_GSM = 520,
	AF_COMPRESSION_FS1016 = 521,

	AF_COMPRESSION_DEFAULT_MPEG_I = 508,
	AF_COMPRESSION_DEFAULT_MPEG1_LAYERI = AF_COMPRESSION_DEFAULT_MPEG_I,
	AF_COMPRESSION_DEFAULT_MPEG_II = 509,
	AF_COMPRESSION_DEFAULT_MPEG1_LAYERII = AF_COMPRESSION_DEFAULT_MPEG_II,
	AF_COMPRESSION_DEFAULT_MULTIRATE = 513,
	AF_COMPRESSION_DEFAULT_LOSSLESS = 514
};

/* tokens for afQuery() -- see the man page for instructions */
/* level 1 selectors */
enum
{
	AF_QUERYTYPE_INSTPARAM = 500,
	AF_QUERYTYPE_FILEFMT = 501,
	AF_QUERYTYPE_COMPRESSION = 502,
	AF_QUERYTYPE_COMPRESSIONPARAM = 503,
	AF_QUERYTYPE_MISC = 504,
	AF_QUERYTYPE_INST = 505,
	AF_QUERYTYPE_MARK = 506,
	AF_QUERYTYPE_LOOP = 507
};

/* level 2 selectors */
enum
{
	AF_QUERY_NAME = 600,	/* get name (1-3 words) */
	AF_QUERY_DESC = 601,	/* get description */
	AF_QUERY_LABEL = 602,	/* get 4- or 5-char label */
	AF_QUERY_TYPE = 603,	/* get type token */
	AF_QUERY_DEFAULT = 604,	/* dflt. value for param */
	AF_QUERY_ID_COUNT = 605,	/* get number of ids avail. */
	AF_QUERY_IDS = 606,	/* get array of id tokens */
	AF_QUERY_IMPLEMENTED = 613,	/* boolean */
	AF_QUERY_TYPE_COUNT = 607,	/* get number of types av. */
	AF_QUERY_TYPES = 608,	/* get array of types */
	AF_QUERY_NATIVE_SAMPFMT = 609,	/* for compression */
	AF_QUERY_NATIVE_SAMPWIDTH = 610,
	AF_QUERY_SQUISHFAC = 611,	/* 1.0 means variable */
	AF_QUERY_MAX_NUMBER = 612,	/* max allowed in file */
	AF_QUERY_SUPPORTED = 613	/* insts, loops, etc., supported? */
};

/* level 2 selectors which have sub-selectors */
enum
{
	AF_QUERY_TRACKS = 620,
	AF_QUERY_CHANNELS = 621,
	AF_QUERY_SAMPLE_SIZES = 622,
	AF_QUERY_SAMPLE_FORMATS = 623,
	AF_QUERY_COMPRESSION_TYPES = 624
};

/* level 3 sub-selectors */
enum
{
	AF_QUERY_VALUE_COUNT = 650,	/* number of values of the above */
	AF_QUERY_VALUES = 651	/* array of those values */
};


/*
	Old Audio File Library error codes. These are still returned by the
	AFerrorhandler calls, but are not used by the new digital media library
	error reporting routines. See the bottom of this file for the new error
	tokens.
*/

enum
{
	AF_BAD_NOT_IMPLEMENTED = 0,	/* not implemented yet */
	AF_BAD_FILEHANDLE = 1,	/* tried to use invalid filehandle */
	AF_BAD_OPEN = 3,	/* unix open failed */
	AF_BAD_CLOSE = 4,	/* unix close failed */
	AF_BAD_READ = 5,	/* unix read failed */
	AF_BAD_WRITE = 6,	/* unix write failed */
	AF_BAD_LSEEK = 7,	/* unix lseek failed */
	AF_BAD_NO_FILEHANDLE = 8,	/* failed to allocate a filehandle struct */
	AF_BAD_ACCMODE = 10,	/* unrecognized audio file access mode */
	AF_BAD_NOWRITEACC = 11,	/* file not open for writing */
	AF_BAD_NOREADACC = 12,	/* file not open for reading */
	AF_BAD_FILEFMT = 13,	/* unrecognized audio file format */
	AF_BAD_RATE = 14,	/* invalid sample rate */
	AF_BAD_CHANNELS = 15,	/* invalid number of channels*/
	AF_BAD_SAMPCNT = 16,	/* invalid sample count */
	AF_BAD_WIDTH = 17,	/* invalid sample width */
	AF_BAD_SEEKMODE = 18,	/* invalid seek mode */
	AF_BAD_NO_LOOPDATA = 19,	/* failed to allocate loop struct */
	AF_BAD_MALLOC = 20,	/* malloc failed somewhere */
	AF_BAD_LOOPID = 21,
	AF_BAD_SAMPFMT = 22,	/* bad sample format */
	AF_BAD_FILESETUP = 23,	/* bad file setup structure*/
	AF_BAD_TRACKID = 24,	/* no track corresponding to id */
	AF_BAD_NUMTRACKS = 25,	/* wrong number of tracks for file format */
	AF_BAD_NO_FILESETUP = 26,	/* failed to allocate a filesetup struct*/
	AF_BAD_LOOPMODE = 27,	/* unrecognized loop mode value */
	AF_BAD_INSTID = 28,	/* invalid instrument id */
	AF_BAD_NUMLOOPS = 29,	/* bad number of loops */
	AF_BAD_NUMMARKS = 30,	/* bad number of markers */
	AF_BAD_MARKID = 31,	/* bad marker id */
	AF_BAD_MARKPOS = 32,	/* invalid marker position value */
	AF_BAD_NUMINSTS = 33,	/* invalid number of instruments */
	AF_BAD_NOAESDATA = 34,
	AF_BAD_MISCID = 35,
	AF_BAD_NUMMISC = 36,
	AF_BAD_MISCSIZE = 37,
	AF_BAD_MISCTYPE = 38,
	AF_BAD_MISCSEEK = 39,
	AF_BAD_STRLEN = 40,	/* invalid string length */
	AF_BAD_RATECONV = 45,
	AF_BAD_SYNCFILE = 46,
	AF_BAD_CODEC_CONFIG = 47,	/* improperly configured codec */
	AF_BAD_CODEC_STATE = 48,	/* invalid codec state: can't recover */
	AF_BAD_CODEC_LICENSE = 49,	/* no license available for codec */
	AF_BAD_CODEC_TYPE = 50,	/* unsupported codec type */
	AF_BAD_COMPRESSION = AF_BAD_CODEC_CONFIG,	/* for back compat */
	AF_BAD_COMPTYPE = AF_BAD_CODEC_TYPE,	/* for back compat */

	AF_BAD_INSTPTYPE = 51,	/* invalid instrument parameter type */
	AF_BAD_INSTPID = 52,	/* invalid instrument parameter id */
	AF_BAD_BYTEORDER = 53,
	AF_BAD_FILEFMT_PARAM = 54,	/* unrecognized file format parameter */
	AF_BAD_COMP_PARAM = 55,	/* unrecognized compression parameter */
	AF_BAD_DATAOFFSET = 56,	/* bad data offset */
	AF_BAD_FRAMECNT = 57,	/* bad frame count */
	AF_BAD_QUERYTYPE = 58,	/* bad query type */
	AF_BAD_QUERY = 59,	/* bad argument to afQuery() */
	AF_WARNING_CODEC_RATE = 60,	/* using 8k instead of codec rate 8012 */
	AF_WARNING_RATECVT = 61,	/* warning about rate conversion used */

	AF_BAD_HEADER = 62,	/* failed to parse header */
	AF_BAD_FRAME = 63,	/* bad frame number */
	AF_BAD_LOOPCOUNT = 64,	/* bad loop count */
	AF_BAD_DMEDIA_CALL = 65,	/* error in dmedia subsystem call */

	/* AIFF/AIFF-C specific errors when parsing file header */
	AF_BAD_AIFF_HEADER = 108,	/* failed to parse chunk header */
	AF_BAD_AIFF_FORM = 109,	/* failed to parse FORM chunk */
	AF_BAD_AIFF_SSND = 110,	/* failed to parse SSND chunk */
	AF_BAD_AIFF_CHUNKID = 111,	/* unrecognized AIFF/AIFF-C chunk id */
	AF_BAD_AIFF_COMM = 112,	/* failed to parse COMM chunk */
	AF_BAD_AIFF_INST = 113,	/* failed to parse INST chunk */
	AF_BAD_AIFF_MARK = 114,	/* failed to parse MARK chunk */
	AF_BAD_AIFF_SKIP = 115,	/* failed to skip unsupported chunk */
	AF_BAD_AIFF_LOOPMODE = 116	/* unrecognized loop mode (forw, etc)*/
};

/* new error codes which may be retrieved via dmGetError() */
/* The old error tokens continue to be retrievable via the AFerrorhandler */
/* AF_ERR_BASE is #defined in dmedia/dmedia.h */

enum
{
	AF_ERR_NOT_IMPLEMENTED = 0+AF_ERR_BASE,	/* not implemented yet */
	AF_ERR_BAD_FILEHANDLE = 1+AF_ERR_BASE,	/* invalid filehandle */
	AF_ERR_BAD_READ = 5+AF_ERR_BASE,	/* unix read failed */
	AF_ERR_BAD_WRITE = 6+AF_ERR_BASE,	/* unix write failed */
	AF_ERR_BAD_LSEEK = 7+AF_ERR_BASE,	/* unix lseek failed */
	AF_ERR_BAD_ACCMODE = 10+AF_ERR_BASE,	/* unrecognized audio file access mode */
	AF_ERR_NO_WRITEACC = 11+AF_ERR_BASE,	/* file not open for writing */
	AF_ERR_NO_READACC = 12+AF_ERR_BASE,	/* file not open for reading */
	AF_ERR_BAD_FILEFMT = 13+AF_ERR_BASE,	/* unrecognized audio file format */
	AF_ERR_BAD_RATE = 14+AF_ERR_BASE,	/* invalid sample rate */
	AF_ERR_BAD_CHANNELS = 15+AF_ERR_BASE,	/* invalid # channels*/
	AF_ERR_BAD_SAMPCNT = 16+AF_ERR_BASE,	/* invalid sample count */
	AF_ERR_BAD_WIDTH = 17+AF_ERR_BASE,	/* invalid sample width */
	AF_ERR_BAD_SEEKMODE = 18+AF_ERR_BASE,	/* invalid seek mode */
	AF_ERR_BAD_LOOPID = 21+AF_ERR_BASE,	/* invalid loop id */
	AF_ERR_BAD_SAMPFMT = 22+AF_ERR_BASE,	/* bad sample format */
	AF_ERR_BAD_FILESETUP = 23+AF_ERR_BASE,	/* bad file setup structure*/
	AF_ERR_BAD_TRACKID = 24+AF_ERR_BASE,	/* no track corresponding to id */
	AF_ERR_BAD_NUMTRACKS = 25+AF_ERR_BASE,	/* wrong number of tracks for file format */
	AF_ERR_BAD_LOOPMODE = 27+AF_ERR_BASE,	/* unrecognized loop mode symbol */
	AF_ERR_BAD_INSTID = 28+AF_ERR_BASE,	/* invalid instrument id */
	AF_ERR_BAD_NUMLOOPS = 29+AF_ERR_BASE,	/* bad number of loops */
	AF_ERR_BAD_NUMMARKS = 30+AF_ERR_BASE,	/* bad number of markers */
	AF_ERR_BAD_MARKID = 31+AF_ERR_BASE,	/* bad marker id */
	AF_ERR_BAD_MARKPOS = 32+AF_ERR_BASE,	/* invalid marker position value */
	AF_ERR_BAD_NUMINSTS = 33+AF_ERR_BASE,	/* invalid number of instruments */
	AF_ERR_BAD_NOAESDATA = 34+AF_ERR_BASE,
	AF_ERR_BAD_MISCID = 35+AF_ERR_BASE,
	AF_ERR_BAD_NUMMISC = 36+AF_ERR_BASE,
	AF_ERR_BAD_MISCSIZE = 37+AF_ERR_BASE,
	AF_ERR_BAD_MISCTYPE = 38+AF_ERR_BASE,
	AF_ERR_BAD_MISCSEEK = 39+AF_ERR_BASE,
	AF_ERR_BAD_STRLEN = 40+AF_ERR_BASE,	/* invalid string length */
	AF_ERR_BAD_RATECONV = 45+AF_ERR_BASE,
	AF_ERR_BAD_SYNCFILE = 46+AF_ERR_BASE,
	AF_ERR_BAD_CODEC_CONFIG = 47+AF_ERR_BASE,	/* improperly configured codec */
	AF_ERR_BAD_CODEC_TYPE = 50+AF_ERR_BASE,	/* unsupported codec type */
	AF_ERR_BAD_INSTPTYPE = 51+AF_ERR_BASE,	/* invalid instrument parameter type */
	AF_ERR_BAD_INSTPID = 52+AF_ERR_BASE,	/* invalid instrument parameter id */

	AF_ERR_BAD_BYTEORDER = 53+AF_ERR_BASE,
	AF_ERR_BAD_FILEFMT_PARAM = 54+AF_ERR_BASE,	/* unrecognized file format parameter */
	AF_ERR_BAD_COMP_PARAM = 55+AF_ERR_BASE,	/* unrecognized compression parameter */
	AF_ERR_BAD_DATAOFFSET = 56+AF_ERR_BASE,	/* bad data offset */
	AF_ERR_BAD_FRAMECNT = 57+AF_ERR_BASE,	/* bad frame count */

	AF_ERR_BAD_QUERYTYPE = 58+AF_ERR_BASE,	/* bad query type */
	AF_ERR_BAD_QUERY = 59+AF_ERR_BASE,	/* bad argument to afQuery() */
	AF_ERR_BAD_HEADER = 62+AF_ERR_BASE,	/* failed to parse header */
	AF_ERR_BAD_FRAME = 63+AF_ERR_BASE,	/* bad frame number */
	AF_ERR_BAD_LOOPCOUNT = 64+AF_ERR_BASE,	/* bad loop count */

	/* AIFF/AIFF-C specific errors when parsing file header */

	AF_ERR_BAD_AIFF_HEADER = 66+AF_ERR_BASE,	/* failed to parse chunk header */
	AF_ERR_BAD_AIFF_FORM = 67+AF_ERR_BASE,	/* failed to parse FORM chunk */
	AF_ERR_BAD_AIFF_SSND = 68+AF_ERR_BASE,	/* failed to parse SSND chunk */
	AF_ERR_BAD_AIFF_CHUNKID = 69+AF_ERR_BASE,	/* unrecognized AIFF/AIFF-C chunk id */
	AF_ERR_BAD_AIFF_COMM = 70+AF_ERR_BASE,	/* failed to parse COMM chunk */
	AF_ERR_BAD_AIFF_INST = 71+AF_ERR_BASE,	/* failed to parse INST chunk */
	AF_ERR_BAD_AIFF_MARK = 72+AF_ERR_BASE,	/* failed to parse MARK chunk */
	AF_ERR_BAD_AIFF_SKIP = 73+AF_ERR_BASE,	/* failed to skip unsupported chunk */
	AF_ERR_BAD_AIFF_LOOPMODE = 74+AF_ERR_BASE	/* unrecognized loop mode (forw, etc) */
};


/* global routines */
AFerrfunc afSetErrorHandler (AFerrfunc efunc);

/* query routines */
AUpvlist afQuery (int querytype, int arg1, int arg2, int arg3, int arg4);
long afQueryLong (int querytype, int arg1, int arg2, int arg3, int arg4);
double afQueryDouble (int querytype, int arg1, int arg2, int arg3, int arg4);
void *afQueryPointer (int querytype, int arg1, int arg2, int arg3, int arg4);

/* basic operations on file handles and file setups */
AFfilesetup afNewFileSetup (void);
void afFreeFileSetup (AFfilesetup);
int afIdentifyFD (int);
int afIdentifyNamedFD (int, const char *filename, int *implemented);

AFfilehandle afOpenFile (const char *filename, const char *mode,
	AFfilesetup setup);
AFfilehandle afOpenVirtualFile(AF_VirtualFile *vfile, const char *mode, AFfilesetup setup);
AFfilehandle afOpenFD (int fd, const char *mode, AFfilesetup setup);
AFfilehandle afOpenNamedFD (int fd, const char *mode, AFfilesetup setup,
	const char *filename);

int afGetFD (AFfilehandle file);
void afSaveFilePosition (AFfilehandle file);
void afRestoreFilePosition (AFfilehandle file);
int afSyncFile (AFfilehandle file);
int afCloseFile (AFfilehandle file);

void afInitFileFormat (AFfilesetup, int format);
int afGetFileFormat (AFfilehandle, int *version);

/* track */
void afInitTrackIDs (AFfilesetup, int *trackids, int trackCount);
int afGetTrackIDs (AFfilehandle, int *trackids);

/* track data: reading, writng, seeking, sizing frames */
int afReadFrames (AFfilehandle, int track, void *buffer, int frameCount);
int afWriteFrames (AFfilehandle, int track, void *buffer, int frameCount);
AFframecount afSeekFrame (AFfilehandle, int track, AFframecount frameoffset);
AFfileoffset afTellFrame (AFfilehandle, int track);
AFfileoffset afGetTrackBytes (AFfilehandle, int track);
float afGetFrameSize (AFfilehandle, int track, int expand3to4);
#if 0
float afGetVirtualFrameSize (AFfilehandle, int track, int expand3to4);
#endif

/* track data: AES data */
/* afInitAESChannelData is obsolete -- use afInitAESChannelDataTo() */
void afInitAESChannelData (AFfilesetup, int track); /* obsolete */
void afInitAESChannelDataTo (AFfilesetup, int track, int willBeData);
int afGetAESChannelData (AFfilehandle, int track, unsigned char buf[24]);
void afSetAESChannelData (AFfilehandle, int track, unsigned char buf[24]);

#if 0
/* track setup format initialized via DMparams */
/* track format retrieved via DMparams */
DMstatus afInitFormatParams (AFfilesetup, int track, DMparams *params);
/* virtual format set via DMparams */
DMstatus afGetFormatParams (AFfilehandle, int track, DMparams *params);
/* virtual format retrieved via DMparams */
DMstatus afSetVirtualFormatParams (AFfilehandle, int track, DMparams *params);
DMstatus afGetVirtualFormatParams (AFfilehandle, int track, DMparams *params);
/* conversion/compression params set via DMparams */
DMstatus afSetConversionParams (AFfilehandle, int track, DMparams *params);
/* conversion/compression params retrieved via DMparams */
DMstatus afGetConversionParams (AFfilehandle, int track, DMparams *params);
#endif

/* track data: byte order */
void afInitByteOrder (AFfilesetup, int track, int byteOrder);
int afGetByteOrder (AFfilehandle, int track);
int afSetVirtualByteOrder (AFfilehandle, int track, int byteOrder);
int afGetVirtualByteOrder (AFfilehandle, int track);

/* track data: number of channels */
void afInitChannels (AFfilesetup, int track, int nchannels);
int afGetChannels (AFfilehandle, int track);
int afSetVirtualChannels (AFfilehandle, int track, int channelCount);
int afGetVirtualChannels (AFfilehandle, int track);
void afSetChannelMatrix (AFfilehandle, int track, double *matrix);

/* track data: sample format and sample width */
void afInitSampleFormat (AFfilesetup, int track, int sampleFormat,
	int sampleWidth);
void afGetSampleFormat (AFfilehandle file, int track, int *sampfmt,
	int *sampwidth);
void afGetVirtualSampleFormat (AFfilehandle file, int track, int *sampfmt,
	int *sampwidth);
int afSetVirtualSampleFormat (AFfilehandle, int track,
	int sampleFormat, int sampleWidth);
void afGetVirtualSampleFormat (AFfilehandle, int track,
	int *sampleFormat, int *sampleWidth);

/* track data: sampling rate */
void afInitRate (AFfilesetup, int track, double rate);
double afGetRate (AFfilehandle, int track);

#if 0
int afSetVirtualRate (AFfilehandle, int track, double rate);
double afGetVirtualRate (AFfilehandle, int track);
#endif

/* track data: compression */
void afInitCompression (AFfilesetup, int track, int compression);
#if 0
void afInitCompressionParams (AFfilesetup, int track, int compression
	AUpvlist params, int parameterCount);
#endif

int afGetCompression (AFfilehandle, int track);
#if 0
void afGetCompressionParams (AFfilehandle, int track, int *compression,
	AUpvlist params, int parameterCount);

int afSetVirtualCompression (AFfilesetup, int track, int compression);
void afSetVirtualCompressionParams (AFfilehandle, int track, int compression,
	AUpvlist params, int parameterCount);

int afGetVirtualCompression (AFfilesetup, int track, int compression);
void afGetVirtualCompressionParams (AFfilehandle, int track, int *compression,
	AUpvlist params, int parameterCount);
#endif

#if 0
/* track data: pcm mapping */
void afInitPCMMapping (AFfilesetup filesetup, int track,
	double slope, double intercept, double minClip, double maxClip);
void afGetPCMMapping (AFfilehandle file, int track,
	double *slope, double *intercept, double *minClip, double *maxClip);
/* NOTE: afSetTrackPCMMapping() is special--it does not set the virtual  */
/* format; it changes what the AF thinks the track format is! Be careful. */
int afSetTrackPCMMapping (AFfilehandle file, int track,
	double slope, double intercept, double minClip, double maxClip);
/* NOTE: afSetVirtualPCMMapping() is different from afSetTrackPCMMapping(): */
/* see comment for afSetTrackPCMMapping(). */
int afSetVirtualPCMMapping (AFfilehandle file, int track,
	double slope, double intercept, double minClip, double maxClip);
void afGetVirtualPCMMapping (AFfilehandle file, int track,
	double *slope, double *intercept, double *minClip, double *maxClip);

#endif

/* track data: data offset within the file */
/* initialize for raw reading only */
void afInitDataOffset(AFfilesetup, int track, AFfileoffset offset);
AFfileoffset afGetDataOffset (AFfilehandle, int track);

/* track data: count of frames in file */
void afInitFrameCount (AFfilesetup, int track, AFframecount frameCount);
AFframecount afGetFrameCount (AFfilehandle file, int track);

/* loop operations */
void afInitLoopIDs (AFfilesetup, int instid, int ids[], int nids);
int afGetLoopIDs (AFfilehandle, int instid, int loopids[]);
void afSetLoopMode (AFfilehandle, int instid, int loop, int mode);
int afGetLoopMode (AFfilehandle, int instid, int loopid);
int afSetLoopCount (AFfilehandle, int instid, int loop, int count);
int afGetLoopCount (AFfilehandle, int instid, int loopid);
void afSetLoopStart (AFfilehandle, int instid, int loopid, int markerid);
int afGetLoopStart (AFfilehandle, int instid, int loopid);
void afSetLoopEnd (AFfilehandle, int instid, int loopid, int markerid);
int afGetLoopEnd (AFfilehandle, int instid, int loopid);

int afSetLoopStartFrame (AFfilehandle, int instid, int loop,
	AFframecount startFrame);
AFframecount afGetLoopStartFrame (AFfilehandle, int instid, int loop);
int afSetLoopEndFrame (AFfilehandle, int instid, int loop,
	AFframecount startFrame);
AFframecount afGetLoopEndFrame (AFfilehandle, int instid, int loop);

void afSetLoopTrack (AFfilehandle, int instid, int loopid, int trackid);
int afGetLoopTrack (AFfilehandle, int instid, int loopid);

/* marker operations */
void afInitMarkIDs (AFfilesetup, int trackid, int *ids, int nids);
int afGetMarkIDs (AFfilehandle file, int trackid, int markids[]);
void afSetMarkPosition (AFfilehandle file, int trackid, int markid,
	AFframecount markpos);
AFframecount afGetMarkPosition (AFfilehandle file, int trackid, int markid);
void afInitMarkName (AFfilesetup, int trackid, int marker, const char *name);
void afInitMarkComment (AFfilesetup, int trackid, int marker,
	const char *comment);
char *afGetMarkName (AFfilehandle file, int trackid, int markid);
char *afGetMarkComment (AFfilehandle file, int trackid, int markid);

/* instrument operations */
void afInitInstIDs (AFfilesetup, int *ids, int nids);
int afGetInstIDs (AFfilehandle file, int *instids);
void afGetInstParams (AFfilehandle file, int instid, AUpvlist pvlist,
	int nparams);
void afSetInstParams (AFfilehandle file, int instid, AUpvlist pvlist,
	int nparams);
long afGetInstParamLong (AFfilehandle file, int instid, int param);
void afSetInstParamLong (AFfilehandle file, int instid, int param, long value);

/* miscellaneous data operations */
void afInitMiscIDs (AFfilesetup, int *ids, int nids);
int afGetMiscIDs (AFfilehandle, int *ids);
void afInitMiscType (AFfilesetup, int miscellaneousid, int type);
int afGetMiscType (AFfilehandle, int miscellaneousid);
void afInitMiscSize (AFfilesetup, int miscellaneousid, int size);
int afGetMiscSize (AFfilehandle, int miscellaneousid);
int afWriteMisc (AFfilehandle, int miscellaneousid, void *buf, int bytes);
int afReadMisc (AFfilehandle, int miscellaneousid, void *buf, int bytes);
int afSeekMisc (AFfilehandle, int miscellaneousid, int offset);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AUDIOFILE_H */
