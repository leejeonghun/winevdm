/*
 * MMSYSTEM MCI and low level mapping functions
 *
 * Copyright 1999 Eric Pouech
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stddef.h>
#include <assert.h>
#define NONAMELESSUNION
#ifdef WINE_NO_UNICODE_MACROS
# define WINELIB_NAME_AW(func) \
    func##_must_be_suffixed_with_W_or_A_in_this_context \
    func##_must_be_suffixed_with_W_or_A_in_this_context
#else  /* WINE_NO_UNICODE_MACROS */
# ifdef UNICODE
#  define WINELIB_NAME_AW(func) func##W
# else
#  define WINELIB_NAME_AW(func) func##A
# endif
#endif  /* WINE_NO_UNICODE_MACROS */
#ifdef WINE_NO_UNICODE_MACROS
# define DECL_WINELIB_TYPE_AW(type)  /* nothing */
#else
# define DECL_WINELIB_TYPE_AW(type)  typedef WINELIB_NAME_AW(type) type;
#endif
#include "wine/winbase16.h"
#include "windef.h"
#include "winbase.h"
#include "winuser.h"
#include "wownt32.h"
#include "winemm16.h"
#include "digitalv.h"
#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(winmm);

/* =================================
 *       A U X    M A P P E R S
 * ================================= */

/* =================================
 *     M I X E R  M A P P E R S
 * ================================= */

/**************************************************************************
 * 				MMSYSTDRV_Mixer_Map16To32W		[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_Mixer_Map16To32W  (UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2)
{
    return MMSYSTEM_MAP_MSGERROR;
}

/**************************************************************************
 * 				MMSYSTDRV_Mixer_UnMap16To32W	[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_Mixer_UnMap16To32W(UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2, MMRESULT fn_ret)
{
#if 0
    MIXERCAPSA	micA;
    UINT	ret = mixerGetDevCapsA(devid, &micA, sizeof(micA));

    if (ret == MMSYSERR_NOERROR) {
	mixcaps->wMid           = micA.wMid;
	mixcaps->wPid           = micA.wPid;
	mixcaps->vDriverVersion = micA.vDriverVersion;
	strcpy(mixcaps->szPname, micA.szPname);
	mixcaps->fdwSupport     = micA.fdwSupport;
	mixcaps->cDestinations  = micA.cDestinations;
    }
    return ret;
#endif
    return MMSYSTEM_MAP_MSGERROR;
}

/**************************************************************************
 * 				MMSYSTDRV_Mixer_MapCB
 */
static  void	                MMSYSTDRV_Mixer_MapCB(DWORD uMsg, DWORD_PTR* dwUser, DWORD_PTR* dwParam1, DWORD_PTR* dwParam2)
{
    FIXME("NIY\n");
}

/* =================================
 *   M I D I  I N    M A P P E R S
 * ================================= */

/**************************************************************************
 * 				MMSYSTDRV_MidiIn_Map16To32W		[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_MidiIn_Map16To32W  (UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    case MIDM_GETNUMDEVS:
    case MIDM_RESET:
	case MIDM_START:
	case MIDM_STOP:
	ret = MMSYSTEM_MAP_OK;
	break;

    case MIDM_OPEN:
    case MIDM_CLOSE:
	FIXME("Shouldn't be used: the corresponding 16 bit functions use the 32 bit interface\n");
	break;

    case MIDM_GETDEVCAPS:
	{
		LPMIDIINCAPSW	mic32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPMIDIINCAPS16) + sizeof(MIDIINCAPSW));
		LPMIDIINCAPS16	mic16 = MapSL(*lpParam1);

	    if (mic32) {
		*(LPMIDIINCAPS16*)mic32 = mic16;
		mic32 = (LPMIDIINCAPSW)((LPSTR)mic32 + sizeof(LPMIDIINCAPS16));
		*lpParam1 = (DWORD)mic32;
		*lpParam2 = sizeof(MIDIINCAPSW);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case MIDM_PREPARE:
	{
	    LPMIDIHDR		mh32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPMIDIHDR) + sizeof(MIDIHDR));
	    LPMIDIHDR16		mh16 = MapSL(*lpParam1);

	    if (mh32) {
		*(LPMIDIHDR*)mh32 = (LPMIDIHDR)*lpParam1;
		mh32 = (LPMIDIHDR)((LPSTR)mh32 + sizeof(LPMIDIHDR));
		mh32->lpData = MapSL((SEGPTR)mh16->lpData);
		mh32->dwBufferLength = mh16->dwBufferLength;
		mh32->dwBytesRecorded = mh16->dwBytesRecorded;
		mh32->dwUser = mh16->dwUser;
		mh32->dwFlags = mh16->dwFlags;
		mh16->reserved = (MIDIHDR16*)mh32; /* for reuse in unprepare and write */
		*lpParam1 = (DWORD)mh32;
		*lpParam2 = offsetof(MIDIHDR,dwOffset); /* old size, without dwOffset */

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
	case MIDM_ADDBUFFER:
    case MIDM_UNPREPARE:
	{
	    LPMIDIHDR16		mh16 = MapSL(*lpParam1);
	    LPMIDIHDR		mh32 = (MIDIHDR*)mh16->reserved;

	    *lpParam1 = (DWORD)mh32;
	    *lpParam2 = offsetof(MIDIHDR,dwOffset);
	    if (wMsg == MIDM_ADDBUFFER && mh32->dwBufferLength < mh16->dwBufferLength) {
		ERR("Size of buffer has been increased from %d to %d, keeping initial value\n",
		    mh32->dwBufferLength, mh16->dwBufferLength);
	    } else
			mh32->dwBufferLength = mh16->dwBufferLength;
	    ret = MMSYSTEM_MAP_OKMEM;
	}
	break;
    }
    return ret;
}

/**************************************************************************
 * 				MMSYSTDRV_MidiIn_UnMap16To32W	[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_MidiIn_UnMap16To32W(UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2, MMRESULT fn_ret)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    case MIDM_GETNUMDEVS:
    case MIDM_RESET:
	case MIDM_START:
	case MIDM_STOP:
	ret = MMSYSTEM_MAP_OK;
	break;

    case MIDM_OPEN:
    case MIDM_CLOSE:
	FIXME("Shouldn't be used: the corresponding 16 bit functions use the 32 bit interface\n");
	break;

    case MIDM_GETDEVCAPS:
	{
        LPMIDIINCAPSW		mic32 = (LPMIDIINCAPSW)(*lpParam1);
	    LPMIDIINCAPS16		mic16 = *(LPMIDIINCAPS16*)((LPSTR)mic32 - sizeof(LPMIDIINCAPS16));

	    mic16->wMid			= mic32->wMid;
	    mic16->wPid			= mic32->wPid;
	    mic16->vDriverVersion	= mic32->vDriverVersion;
            WideCharToMultiByte( CP_ACP, 0, mic32->szPname, -1, mic16->szPname,
                                 sizeof(mic16->szPname), NULL, NULL );
	    mic16->dwSupport		= mic32->dwSupport;
	    HeapFree(GetProcessHeap(), 0, (LPSTR)mic32 - sizeof(LPMIDIINCAPS16));
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    case MIDM_PREPARE:
    case MIDM_UNPREPARE:
    case MIDM_ADDBUFFER:
	{
	    LPMIDIHDR		mh32 = (LPMIDIHDR)(*lpParam1);
	    LPMIDIHDR16		mh16 = MapSL(*(SEGPTR*)((LPSTR)mh32 - sizeof(LPMIDIHDR)));

	    assert((MIDIHDR*)mh16->reserved == mh32);
	    mh16->dwFlags = mh32->dwFlags;

	    if (wMsg == MODM_UNPREPARE && fn_ret == MMSYSERR_NOERROR) {
		HeapFree(GetProcessHeap(), 0, (LPSTR)mh32 - sizeof(LPMIDIHDR));
		mh16->reserved = 0;
	    }
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    }
    return ret;
}

/**************************************************************************
 * 				MMSYSTDRV_MidiIn_MapCB		[internal]
 */
static  void            	MMSYSTDRV_MidiIn_MapCB(UINT uMsg, DWORD_PTR* dwUser, DWORD_PTR* dwParam1, DWORD_PTR* dwParam2)
{
    switch (uMsg) {
    case MIM_OPEN:
    case MIM_CLOSE:
	/* dwParam1 & dwParam2 are supposed to be 0, nothing to do */

    case MIM_DATA:
    case MIM_MOREDATA:
    case MIM_ERROR:
	/* dwParam1 & dwParam2 are data, nothing to do */
	break;
    case MIM_LONGDATA:
    case MIM_LONGERROR:
        {
	    LPMIDIHDR		mh32 = (LPMIDIHDR)(*dwParam1);
	    SEGPTR		segmh16 = *(SEGPTR*)((LPSTR)mh32 - sizeof(LPMIDIHDR));
	    LPMIDIHDR16		mh16 = MapSL(segmh16);

	    *dwParam1 = (DWORD)segmh16;
	    mh16->dwFlags = mh32->dwFlags;
	    mh16->dwBytesRecorded = mh32->dwBytesRecorded;
	}
	break;
    default:
	ERR("Unknown msg %u\n", uMsg);
    }
}

/* =================================
 *   M I D I  O U T  M A P P E R S
 * ================================= */

/**************************************************************************
 * 				MMSYSTDRV_MidiOut_Map16To32W	[internal]
 */
static MMSYSTEM_MapType	MMSYSTDRV_MidiOut_Map16To32W  (UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    case MODM_GETNUMDEVS:
    case MODM_DATA:
    case MODM_RESET:
    case MODM_SETVOLUME:
	ret = MMSYSTEM_MAP_OK;
	break;

    case MODM_OPEN:
    case MODM_CLOSE:
    case MODM_GETVOLUME:
	FIXME("Shouldn't be used: the corresponding 16 bit functions use the 32 bit interface\n");
	break;

    case MODM_GETDEVCAPS:
	{
            LPMIDIOUTCAPSW	moc32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPMIDIOUTCAPS16) + sizeof(MIDIOUTCAPSW));
	    LPMIDIOUTCAPS16	moc16 = MapSL(*lpParam1);

	    if (moc32) {
		*(LPMIDIOUTCAPS16*)moc32 = moc16;
		moc32 = (LPMIDIOUTCAPSW)((LPSTR)moc32 + sizeof(LPMIDIOUTCAPS16));
		*lpParam1 = (DWORD)moc32;
		*lpParam2 = sizeof(MIDIOUTCAPSW);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case MODM_PREPARE:
	{
	    LPMIDIHDR		mh32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPMIDIHDR) + sizeof(MIDIHDR));
	    LPMIDIHDR16		mh16 = MapSL(*lpParam1);

	    if (mh32) {
		*(LPMIDIHDR*)mh32 = (LPMIDIHDR)*lpParam1;
		mh32 = (LPMIDIHDR)((LPSTR)mh32 + sizeof(LPMIDIHDR));
		mh32->lpData = MapSL((SEGPTR)mh16->lpData);
		mh32->dwBufferLength = mh16->dwBufferLength;
		mh32->dwBytesRecorded = mh16->dwBytesRecorded;
		mh32->dwUser = mh16->dwUser;
		mh32->dwFlags = mh16->dwFlags;
		mh16->reserved = (MIDIHDR16*)mh32; /* for reuse in unprepare and write */
		*lpParam1 = (DWORD)mh32;
		*lpParam2 = offsetof(MIDIHDR,dwOffset); /* old size, without dwOffset */

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case MODM_UNPREPARE:
    case MODM_LONGDATA:
	{
	    LPMIDIHDR16		mh16 = MapSL(*lpParam1);
	    LPMIDIHDR		mh32 = (MIDIHDR*)mh16->reserved;

	    *lpParam1 = (DWORD)mh32;
	    *lpParam2 = offsetof(MIDIHDR,dwOffset);
	    /* dwBufferLength can be reduced between prepare & write */
	    if (wMsg == MODM_LONGDATA && mh32->dwBufferLength < mh16->dwBufferLength) {
		ERR("Size of buffer has been increased from %d to %d, keeping initial value\n",
		    mh32->dwBufferLength, mh16->dwBufferLength);
	    } else
                mh32->dwBufferLength = mh16->dwBufferLength;
	    ret = MMSYSTEM_MAP_OKMEM;
	}
	break;

    case MODM_CACHEPATCHES:
    case MODM_CACHEDRUMPATCHES:
    default:
	FIXME("NIY: no conversion yet for %u [%lx,%lx]\n", wMsg, *lpParam1, *lpParam2);
	break;
    }
    return ret;
}

/**************************************************************************
 * 				MMSYSTDRV_MidiOut_UnMap16To32W	[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_MidiOut_UnMap16To32W(UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2, MMRESULT fn_ret)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    case MODM_GETNUMDEVS:
    case MODM_DATA:
    case MODM_RESET:
    case MODM_SETVOLUME:
	ret = MMSYSTEM_MAP_OK;
	break;

    case MODM_OPEN:
    case MODM_CLOSE:
    case MODM_GETVOLUME:
	FIXME("Shouldn't be used: the corresponding 16 bit functions use the 32 bit interface\n");
	break;

    case MODM_GETDEVCAPS:
	{
            LPMIDIOUTCAPSW		moc32 = (LPMIDIOUTCAPSW)(*lpParam1);
	    LPMIDIOUTCAPS16		moc16 = *(LPMIDIOUTCAPS16*)((LPSTR)moc32 - sizeof(LPMIDIOUTCAPS16));

	    moc16->wMid			= moc32->wMid;
	    moc16->wPid			= moc32->wPid;
	    moc16->vDriverVersion	= moc32->vDriverVersion;
            WideCharToMultiByte( CP_ACP, 0, moc32->szPname, -1, moc16->szPname,
                                 sizeof(moc16->szPname), NULL, NULL );
	    moc16->wTechnology		= moc32->wTechnology;
	    moc16->wVoices		= moc32->wVoices;
	    moc16->wNotes		= moc32->wNotes;
	    moc16->wChannelMask		= moc32->wChannelMask;
	    moc16->dwSupport		= moc32->dwSupport;
	    HeapFree(GetProcessHeap(), 0, (LPSTR)moc32 - sizeof(LPMIDIOUTCAPS16));
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    case MODM_PREPARE:
    case MODM_UNPREPARE:
    case MODM_LONGDATA:
	{
	    LPMIDIHDR		mh32 = (LPMIDIHDR)(*lpParam1);
	    LPMIDIHDR16		mh16 = MapSL(*(SEGPTR*)((LPSTR)mh32 - sizeof(LPMIDIHDR)));

	    assert((MIDIHDR*)mh16->reserved == mh32);
	    mh16->dwFlags = mh32->dwFlags;

	    if (wMsg == MODM_UNPREPARE && fn_ret == MMSYSERR_NOERROR) {
		HeapFree(GetProcessHeap(), 0, (LPSTR)mh32 - sizeof(LPMIDIHDR));
		mh16->reserved = 0;
	    }
	    ret = MMSYSTEM_MAP_OK;
	}
	break;

    case MODM_CACHEPATCHES:
    case MODM_CACHEDRUMPATCHES:
    default:
	FIXME("NIY: no conversion yet for %u [%lx,%lx]\n", wMsg, *lpParam1, *lpParam2);
	break;
    }
    return ret;
}

/******************************************************************
 *		                        MMSYSTDRV_MidiOut_MapCB
 */
static  void MMSYSTDRV_MidiOut_MapCB(UINT uMsg, DWORD_PTR* dwUser, DWORD_PTR* dwParam1, DWORD_PTR* dwParam2)
{
    switch (uMsg) {
    case MOM_OPEN:
    case MOM_CLOSE:
	/* dwParam1 & dwParam2 are supposed to be 0, nothing to do */
	break;
    case MOM_POSITIONCB:
	/* MIDIHDR.dwOffset exists since Win 32 only */
	FIXME("MOM_POSITIONCB/MEVT_F_CALLBACK wants MIDIHDR.dwOffset in 16 bit code\n");
	/* fall through */
    case MOM_DONE:
        {
	    /* initial map is: 16 => 32 */
	    LPMIDIHDR		mh32 = (LPMIDIHDR)(*dwParam1);
	    SEGPTR		segmh16 = *(SEGPTR*)((LPSTR)mh32 - sizeof(LPMIDIHDR));
	    LPMIDIHDR16		mh16 = MapSL(segmh16);

	    *dwParam1 = (DWORD)segmh16;
	    mh16->dwFlags = mh32->dwFlags;
	}
	break;
    default:
	ERR("Unknown msg %u\n", uMsg);
    }
}

/* =================================
 *   W A V E  I N    M A P P E R S
 * ================================= */

/**************************************************************************
 * 				MMSYSTDRV_WaveIn_Map16To32W		[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_WaveIn_Map16To32W  (UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    case WIDM_GETNUMDEVS:
    case WIDM_RESET:
    case WIDM_START:
    case WIDM_STOP:
	ret = MMSYSTEM_MAP_OK;
	break;
    case WIDM_OPEN:
    case WIDM_CLOSE:
	FIXME("Shouldn't be used: the corresponding 16 bit functions use the 32 bit interface\n");
	break;
    case WIDM_GETDEVCAPS:
	{
            LPWAVEINCAPSW	wic32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPWAVEINCAPS16) + sizeof(WAVEINCAPSW));
	    LPWAVEINCAPS16	wic16 = MapSL(*lpParam1);

	    if (wic32) {
		*(LPWAVEINCAPS16*)wic32 = wic16;
		wic32 = (LPWAVEINCAPSW)((LPSTR)wic32 + sizeof(LPWAVEINCAPS16));
		*lpParam1 = (DWORD)wic32;
		*lpParam2 = sizeof(WAVEINCAPSW);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case WIDM_GETPOS:
	{
            LPMMTIME		mmt32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPMMTIME16) + sizeof(MMTIME));
	    LPMMTIME16		mmt16 = MapSL(*lpParam1);

	    if (mmt32) {
		*(LPMMTIME16*)mmt32 = mmt16;
		mmt32 = (LPMMTIME)((LPSTR)mmt32 + sizeof(LPMMTIME16));

		mmt32->wType = mmt16->wType;
		*lpParam1 = (DWORD)mmt32;
		*lpParam2 = sizeof(MMTIME);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case WIDM_PREPARE:
	{
	    LPWAVEHDR		wh32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPWAVEHDR) + sizeof(WAVEHDR));
	    LPWAVEHDR		wh16 = MapSL(*lpParam1);

	    if (wh32) {
		*(LPWAVEHDR*)wh32 = (LPWAVEHDR)*lpParam1;
		wh32 = (LPWAVEHDR)((LPSTR)wh32 + sizeof(LPWAVEHDR));
		wh32->lpData = MapSL((SEGPTR)wh16->lpData);
		wh32->dwBufferLength = wh16->dwBufferLength;
		wh32->dwBytesRecorded = wh16->dwBytesRecorded;
		wh32->dwUser = wh16->dwUser;
		wh32->dwFlags = wh16->dwFlags;
		wh32->dwLoops = wh16->dwLoops;
		/* FIXME: nothing on wh32->reserved */
		/* could link the wh32->reserved at this level for memory house keeping */
		wh16->reserved = wh32; /* for reuse in unprepare and write */
		*lpParam1 = (DWORD)wh32;
		*lpParam2 = sizeof(WAVEHDR);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case WIDM_ADDBUFFER:
    case WIDM_UNPREPARE:
	{
	    LPWAVEHDR		wh16 = MapSL(*lpParam1);
	    LPWAVEHDR		wh32 = wh16->reserved;

	    *lpParam1 = (DWORD)wh32;
	    *lpParam2 = sizeof(WAVEHDR);
	    /* dwBufferLength can be reduced between prepare & write */
	    if (wMsg == WIDM_ADDBUFFER && wh32->dwBufferLength < wh16->dwBufferLength) {
		ERR("Size of buffer has been increased from %d to %d, keeping initial value\n",
		    wh32->dwBufferLength, wh16->dwBufferLength);
	    } else
                wh32->dwBufferLength = wh16->dwBufferLength;
	    ret = MMSYSTEM_MAP_OKMEM;
	}
	break;
    case WIDM_MAPPER_STATUS:
	/* just a single DWORD */
	*lpParam2 = (DWORD)MapSL(*lpParam2);
	ret = MMSYSTEM_MAP_OK;
	break;
    default:
	FIXME("NIY: no conversion yet for %u [%lx,%lx]\n", wMsg, *lpParam1, *lpParam2);
	break;
    }
    return ret;
}

/**************************************************************************
 * 				MMSYSTDRV_WaveIn_UnMap16To32W	[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_WaveIn_UnMap16To32W(UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2, MMRESULT fn_ret)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    case WIDM_GETNUMDEVS:
    case WIDM_RESET:
    case WIDM_START:
    case WIDM_STOP:
    case WIDM_MAPPER_STATUS:
	ret = MMSYSTEM_MAP_OK;
	break;
    case WIDM_OPEN:
    case WIDM_CLOSE:
	FIXME("Shouldn't be used: the corresponding 16 bit functions use the 32 bit interface\n");
	break;
    case WIDM_GETDEVCAPS:
	{
            LPWAVEINCAPSW		wic32 = (LPWAVEINCAPSW)(*lpParam1);
	    LPWAVEINCAPS16		wic16 = *(LPWAVEINCAPS16*)((LPSTR)wic32 - sizeof(LPWAVEINCAPS16));

	    wic16->wMid = wic32->wMid;
	    wic16->wPid = wic32->wPid;
	    wic16->vDriverVersion = wic32->vDriverVersion;
            WideCharToMultiByte( CP_ACP, 0, wic32->szPname, -1, wic16->szPname,
                                 sizeof(wic16->szPname), NULL, NULL );
	    wic16->dwFormats = wic32->dwFormats;
	    wic16->wChannels = wic32->wChannels;
	    HeapFree(GetProcessHeap(), 0, (LPSTR)wic32 - sizeof(LPWAVEINCAPS16));
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    case WIDM_GETPOS:
	{
            LPMMTIME		mmt32 = (LPMMTIME)(*lpParam1);
	    LPMMTIME16		mmt16 = *(LPMMTIME16*)((LPSTR)mmt32 - sizeof(LPMMTIME16));

	    MMSYSTEM_MMTIME32to16(mmt16, mmt32);
	    HeapFree(GetProcessHeap(), 0, (LPSTR)mmt32 - sizeof(LPMMTIME16));
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    case WIDM_ADDBUFFER:
    case WIDM_PREPARE:
    case WIDM_UNPREPARE:
	{
	    LPWAVEHDR		wh32 = (LPWAVEHDR)(*lpParam1);
	    LPWAVEHDR		wh16 = MapSL(*(SEGPTR*)((LPSTR)wh32 - sizeof(LPWAVEHDR)));

	    assert(wh16->reserved == wh32);
	    wh16->dwBytesRecorded = wh32->dwBytesRecorded;
	    wh16->dwFlags = wh32->dwFlags;

	    if (wMsg == WIDM_UNPREPARE && fn_ret == MMSYSERR_NOERROR) {
		HeapFree(GetProcessHeap(), 0, (LPSTR)wh32 - sizeof(LPWAVEHDR));
		wh16->reserved = 0;
	    }
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    default:
	FIXME("NIY: no conversion yet for %u [%lx,%lx]\n", wMsg, *lpParam1, *lpParam2);
	break;
    }
    return ret;
}

/**************************************************************************
 * 				MMSYSTDRV_WaveIn_MapCB		[internal]
 */
static  void    MMSYSTDRV_WaveIn_MapCB(UINT uMsg, DWORD_PTR* dwUser, DWORD_PTR* dwParam1, DWORD_PTR* dwParam2)
{
    switch (uMsg) {
    case WIM_OPEN:
    case WIM_CLOSE:
	/* dwParam1 & dwParam2 are supposed to be 0, nothing to do */
	break;
    case WIM_DATA:
        {
	    /* initial map is: 16 => 32 */
	    LPWAVEHDR		wh32 = (LPWAVEHDR)(*dwParam1);
	    SEGPTR		segwh16 = *(SEGPTR*)((LPSTR)wh32 - sizeof(LPWAVEHDR));
	    LPWAVEHDR		wh16 = MapSL(segwh16);

	    *dwParam1 = (DWORD)segwh16;
	    wh16->dwFlags = wh32->dwFlags;
	    wh16->dwBytesRecorded = wh32->dwBytesRecorded;
	}
	break;
    default:
	ERR("Unknown msg %u\n", uMsg);
    }
}

/* =================================
 *   W A V E  O U T  M A P P E R S
 * ================================= */

/**************************************************************************
 * 				MMSYSTDRV_WaveOut_Map16To32W	[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_WaveOut_Map16To32W  (UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    /* nothing to do */
    case WODM_BREAKLOOP:
    case WODM_CLOSE:
    case WODM_GETNUMDEVS:
    case WODM_PAUSE:
    case WODM_RESET:
    case WODM_RESTART:
    case WODM_SETPITCH:
    case WODM_SETPLAYBACKRATE:
    case WODM_SETVOLUME:
	ret = MMSYSTEM_MAP_OK;
	break;

    case WODM_GETPITCH:
    case WODM_GETPLAYBACKRATE:
    case WODM_GETVOLUME:
    case WODM_OPEN:
	FIXME("Shouldn't be used: the corresponding 16 bit functions use the 32 bit interface\n");
	break;

    case WODM_GETDEVCAPS:
	{
            LPWAVEOUTCAPSW		woc32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPWAVEOUTCAPS16) + sizeof(WAVEOUTCAPSW));
	    LPWAVEOUTCAPS16		woc16 = MapSL(*lpParam1);

	    if (woc32) {
		*(LPWAVEOUTCAPS16*)woc32 = woc16;
		woc32 = (LPWAVEOUTCAPSW)((LPSTR)woc32 + sizeof(LPWAVEOUTCAPS16));
		*lpParam1 = (DWORD)woc32;
		*lpParam2 = sizeof(WAVEOUTCAPSW);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case WODM_GETPOS:
	{
            LPMMTIME		mmt32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPMMTIME16) + sizeof(MMTIME));
	    LPMMTIME16		mmt16 = MapSL(*lpParam1);

	    if (mmt32) {
		*(LPMMTIME16*)mmt32 = mmt16;
		mmt32 = (LPMMTIME)((LPSTR)mmt32 + sizeof(LPMMTIME16));

		mmt32->wType = mmt16->wType;
		*lpParam1 = (DWORD)mmt32;
		*lpParam2 = sizeof(MMTIME);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case WODM_PREPARE:
	{
	    LPWAVEHDR		wh32 = HeapAlloc(GetProcessHeap(), 0, sizeof(LPWAVEHDR) + sizeof(DWORD) + sizeof(WAVEHDR));
	    LPWAVEHDR		wh16 = MapSL(*lpParam1);

	    if (wh32) {
		*(LPWAVEHDR*)wh32 = (LPWAVEHDR)*lpParam1;
		*(DWORD *)((LPSTR)wh32 + sizeof(LPWAVEHDR)) = wh16->dwBufferLength; // preserve the prepared length
		wh32 = (LPWAVEHDR)((LPSTR)wh32 + sizeof(DWORD) + sizeof(LPWAVEHDR));
		wh32->lpData = MapSL((SEGPTR)wh16->lpData);
		wh32->dwBufferLength = wh16->dwBufferLength;
		wh32->dwBytesRecorded = wh16->dwBytesRecorded;
		wh32->dwUser = wh16->dwUser;
		wh32->dwFlags = wh16->dwFlags;
		wh32->dwLoops = wh16->dwLoops;
		/* FIXME: nothing on wh32->reserved */
		/* could link the wh32->reserved at this level for memory house keeping */
		wh16->reserved = wh32; /* for reuse in unprepare and write */
		*lpParam1 = (DWORD)wh32;
		*lpParam2 = sizeof(WAVEHDR);

		ret = MMSYSTEM_MAP_OKMEM;
	    } else {
		ret = MMSYSTEM_MAP_NOMEM;
	    }
	}
	break;
    case WODM_UNPREPARE:
    case WODM_WRITE:
	{
	    LPWAVEHDR		wh16 = MapSL(*lpParam1);
	    LPWAVEHDR		wh32 = wh16->reserved;
	    if (wMsg == WODM_UNPREPARE)
		    wh32->dwFlags = wh16->dwFlags;

	    *lpParam1 = (DWORD)wh32;
	    *lpParam2 = sizeof(WAVEHDR);
	    /* dwBufferLength can be reduced between prepare & write */
	    if (wMsg == WODM_WRITE && *(DWORD *)((LPSTR)wh32 - sizeof(DWORD)) < wh16->dwBufferLength) {
		ERR("Size of buffer has been increased from %d to %d, keeping initial value\n",
		    *(DWORD *)((LPSTR)wh32 - sizeof(DWORD)), wh16->dwBufferLength);
	    } else
                wh32->dwBufferLength = wh16->dwBufferLength;
	    ret = MMSYSTEM_MAP_OKMEM;
	}
	break;
    case WODM_MAPPER_STATUS:
	*lpParam2 = (DWORD)MapSL(*lpParam2);
	ret = MMSYSTEM_MAP_OK;
	break;
    default:
	FIXME("NIY: no conversion yet for %u [%lx,%lx]\n", wMsg, *lpParam1, *lpParam2);
	break;
    }
    return ret;
}

/**************************************************************************
 * 				MMSYSTDRV_WaveOut_UnMap16To32W	[internal]
 */
static  MMSYSTEM_MapType	MMSYSTDRV_WaveOut_UnMap16To32W(UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2, MMRESULT fn_ret)
{
    MMSYSTEM_MapType	ret = MMSYSTEM_MAP_MSGERROR;

    switch (wMsg) {
    /* nothing to do */
    case WODM_BREAKLOOP:
    case WODM_CLOSE:
    case WODM_GETNUMDEVS:
    case WODM_PAUSE:
    case WODM_RESET:
    case WODM_RESTART:
    case WODM_SETPITCH:
    case WODM_SETPLAYBACKRATE:
    case WODM_SETVOLUME:
    case WODM_MAPPER_STATUS:
	ret = MMSYSTEM_MAP_OK;
	break;

    case WODM_GETPITCH:
    case WODM_GETPLAYBACKRATE:
    case WODM_GETVOLUME:
    case WODM_OPEN:
	FIXME("Shouldn't be used: those 16 bit functions use the 32 bit interface\n");
	break;

    case WODM_GETDEVCAPS:
	{
            LPWAVEOUTCAPSW		woc32 = (LPWAVEOUTCAPSW)(*lpParam1);
	    LPWAVEOUTCAPS16		woc16 = *(LPWAVEOUTCAPS16*)((LPSTR)woc32 - sizeof(LPWAVEOUTCAPS16));

	    woc16->wMid = woc32->wMid;
	    woc16->wPid = woc32->wPid;
	    woc16->vDriverVersion = woc32->vDriverVersion;
            WideCharToMultiByte( CP_ACP, 0, woc32->szPname, -1, woc16->szPname,
                                 sizeof(woc16->szPname), NULL, NULL );
	    woc16->dwFormats = woc32->dwFormats;
	    woc16->wChannels = woc32->wChannels;
	    woc16->dwSupport = woc32->dwSupport;
	    HeapFree(GetProcessHeap(), 0, (LPSTR)woc32 - sizeof(LPWAVEOUTCAPS16));
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    case WODM_GETPOS:
	{
            LPMMTIME		mmt32 = (LPMMTIME)(*lpParam1);
	    LPMMTIME16		mmt16 = *(LPMMTIME16*)((LPSTR)mmt32 - sizeof(LPMMTIME16));

	    MMSYSTEM_MMTIME32to16(mmt16, mmt32);
	    HeapFree(GetProcessHeap(), 0, (LPSTR)mmt32 - sizeof(LPMMTIME16));
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    case WODM_PREPARE:
    case WODM_UNPREPARE:
    case WODM_WRITE:
	{
	    LPWAVEHDR		wh32 = (LPWAVEHDR)(*lpParam1);
	    LPWAVEHDR		wh16 = MapSL(*(SEGPTR*)((LPSTR)wh32 - sizeof(DWORD) - sizeof(LPWAVEHDR)));

	    assert(wh16->reserved == wh32);
	    wh16->dwFlags = wh32->dwFlags;

	    if (wMsg == WODM_UNPREPARE && fn_ret == MMSYSERR_NOERROR) {
		HeapFree(GetProcessHeap(), 0, (LPSTR)wh32 - sizeof(DWORD) - sizeof(LPWAVEHDR));
		wh16->reserved = 0;
	    }
	    ret = MMSYSTEM_MAP_OK;
	}
	break;
    default:
	FIXME("NIY: no conversion yet for %u [%lx,%lx]\n", wMsg, *lpParam1, *lpParam2);
	break;
    }
    return ret;
}

/**************************************************************************
 * 				MMDRV_WaveOut_Callback		[internal]
 */
static  void	MMSYSTDRV_WaveOut_MapCB(UINT uMsg, DWORD_PTR* dwUser, DWORD_PTR* dwParam1, DWORD_PTR* dwParam2)
{
    switch (uMsg) {
    case WOM_OPEN:
    case WOM_CLOSE:
	/* dwParam1 & dwParam2 are supposed to be 0, nothing to do */
	break;
    case WOM_DONE:
        {
	    /* initial map is: 16 => 32 */
	    LPWAVEHDR		wh32 = (LPWAVEHDR)(*dwParam1);
	    SEGPTR		segwh16 = *(SEGPTR*)((LPSTR)wh32 - sizeof(DWORD) - sizeof(LPWAVEHDR));
	    LPWAVEHDR		wh16 = MapSL(segwh16);

	    *dwParam1 = (DWORD)segwh16;
	    wh16->dwFlags = wh32->dwFlags;
	}
	break;
    default:
	ERR("Unknown msg %u\n", uMsg);
    }
}

/* ###################################################
 * #                DRIVER THUNKING                  #
 * ###################################################
 */
typedef	MMSYSTEM_MapType        (*MMSYSTDRV_MAPMSG)(UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2);
typedef	MMSYSTEM_MapType        (*MMSYSTDRV_UNMAPMSG)(UINT wMsg, DWORD_PTR* lpParam1, DWORD_PTR* lpParam2, MMRESULT ret);
typedef void                    (*MMSYSTDRV_MAPCB)(DWORD wMsg, DWORD_PTR* dwUser, DWORD_PTR* dwParam1, DWORD_PTR* dwParam2);

#include <pshpack1.h>
#define MMSYSTDRV_MAX_THUNKS      32

static struct mmsystdrv_thunk
{
    BYTE                        popl_eax;       /* popl  %eax (return address) */
    BYTE                        pushl_this;     /* pushl this (this very thunk) */
    struct mmsystdrv_thunk*     this;
    BYTE                        pushl_eax;      /* pushl %eax */
    BYTE                        jmp;            /* ljmp MMDRV_Callback3216 */
    DWORD                       callback3216;
    DWORD                       callback;       /* callback value (function, window, event...) */
    DWORD                       flags;          /* flags to control callback value (CALLBACK_???) */
    void*                       hMmdrv;         /* Handle to 32bit mmdrv object */
    enum MMSYSTEM_DriverType    kind;
} *MMSYSTDRV_Thunks;

#include <poppack.h>

static struct MMSYSTDRV_Type
{
    MMSYSTDRV_MAPMSG    mapmsg16to32W;
    MMSYSTDRV_UNMAPMSG  unmapmsg16to32W;
    MMSYSTDRV_MAPCB     mapcb;
} MMSYSTEM_DriversType[MMSYSTDRV_MAX] =
{
    {MMSYSTDRV_Mixer_Map16To32W,   MMSYSTDRV_Mixer_UnMap16To32W,   MMSYSTDRV_Mixer_MapCB},
    {MMSYSTDRV_MidiIn_Map16To32W,  MMSYSTDRV_MidiIn_UnMap16To32W,  MMSYSTDRV_MidiIn_MapCB},
    {MMSYSTDRV_MidiOut_Map16To32W, MMSYSTDRV_MidiOut_UnMap16To32W, MMSYSTDRV_MidiOut_MapCB},
    {MMSYSTDRV_WaveIn_Map16To32W,  MMSYSTDRV_WaveIn_UnMap16To32W,  MMSYSTDRV_WaveIn_MapCB},
    {MMSYSTDRV_WaveOut_Map16To32W, MMSYSTDRV_WaveOut_UnMap16To32W, MMSYSTDRV_WaveOut_MapCB},
};

typedef BOOL (WINAPI *vm_inject_t)(DWORD vpfn16, DWORD dwFlags,
    DWORD cbArgs, LPVOID pArgs, LPDWORD pdwRetCode);
extern vm_inject_t vm_inject;

/******************************************************************
 *		MMSYSTDRV_Callback3216
 *
 */
static LRESULT CALLBACK MMSYSTDRV_Callback3216(struct mmsystdrv_thunk* thunk, HDRVR hDev,
                                               DWORD wMsg, DWORD_PTR dwUser, DWORD_PTR dwParam1,
                                               DWORD_PTR dwParam2)
{
    WORD args[8];

    assert(thunk->kind < MMSYSTDRV_MAX);
    assert(MMSYSTEM_DriversType[thunk->kind].mapcb);

    MMSYSTEM_DriversType[thunk->kind].mapcb(wMsg, &dwUser, &dwParam1, &dwParam2);

    switch (thunk->flags & CALLBACK_TYPEMASK) {
    case CALLBACK_NULL:
        TRACE("Null !\n");
        break;
    case CALLBACK_WINDOW:
        TRACE("Window(%04X) handle=%p!\n", thunk->callback, hDev);
        PostMessageA(HWND_32(thunk->callback), wMsg, (WPARAM)hDev, dwParam1);
        break;
    case CALLBACK_TASK: /* aka CALLBACK_THREAD */
        TRACE("Task(%04x) !\n", thunk->callback);
        PostThreadMessageA(thunk->callback, wMsg, (WPARAM)hDev, dwParam1);
        break;
    case CALLBACK_FUNCTION:
        /* 16 bit func, call it */
        TRACE("Function (16 bit) %x!\n", thunk->callback);

        args[7] = HDRVR_16(hDev);
        args[6] = wMsg;
        args[5] = HIWORD(dwUser);
        args[4] = LOWORD(dwUser);
        args[3] = HIWORD(dwParam1);
        args[2] = LOWORD(dwParam1);
        args[1] = HIWORD(dwParam2);
        args[0] = LOWORD(dwParam2);

        if (!vm_inject)
        {
            char dllname[MAX_PATH];
            krnl386_get_config_string("otvdm", "vm", "vm86.dll", dllname, sizeof(dllname));
            HMODULE vm = LoadLibraryA(dllname);
            vm_inject = (vm_inject_t)GetProcAddress(vm, "vm_inject");
            if (!vm_inject)
            {
               vm_inject = WOWCallback16Ex;
            }
        }
        vm_inject((DWORD)thunk->callback, WCB16_PASCAL, sizeof(args), args, NULL);
        break;
    case CALLBACK_EVENT:
        TRACE("Event(%08x) !\n", thunk->callback);
        SetEvent((HANDLE)thunk->callback);
        break;
    default:
        WARN("Unknown callback type %x\n", thunk->flags & CALLBACK_TYPEMASK);
        return FALSE;
    }
    TRACE("Done\n");
    return TRUE;
}

/******************************************************************
 *		MMSYSTDRV_AddThunk
 *
 */
struct mmsystdrv_thunk*       MMSYSTDRV_AddThunk(DWORD callback, DWORD flags, enum MMSYSTEM_DriverType kind)
{
    struct mmsystdrv_thunk* thunk;

    EnterCriticalSection(&mmdrv_cs);
    if (!MMSYSTDRV_Thunks)
    {
        MMSYSTDRV_Thunks = VirtualAlloc(NULL, MMSYSTDRV_MAX_THUNKS * sizeof(*MMSYSTDRV_Thunks),
                                        MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        if (!MMSYSTDRV_Thunks)
        {
            LeaveCriticalSection(&mmdrv_cs);
            return NULL;
        }
        for (thunk = MMSYSTDRV_Thunks; thunk < &MMSYSTDRV_Thunks[MMSYSTDRV_MAX_THUNKS]; thunk++)
        {
            thunk->popl_eax     = 0x58;   /* popl  %eax */
            thunk->pushl_this   = 0x68;   /* pushl this */
            thunk->this         = thunk;
            thunk->pushl_eax    = 0x50;   /* pushl %eax */
            thunk->jmp          = 0xe9;   /* jmp MMDRV_Callback3216 */
            thunk->callback3216 = (char *)MMSYSTDRV_Callback3216 - (char *)(&thunk->callback3216 + 1);
            thunk->callback     = 0;
            thunk->flags        = CALLBACK_NULL;
            thunk->hMmdrv       = NULL;
            thunk->kind         = MMSYSTDRV_MAX;
        }
    }
    for (thunk = MMSYSTDRV_Thunks; thunk < &MMSYSTDRV_Thunks[MMSYSTDRV_MAX_THUNKS]; thunk++)
    {
        if (thunk->callback == 0 && thunk->hMmdrv == NULL)
        {
            thunk->callback = callback;
            thunk->flags = flags;
            thunk->hMmdrv = NULL;
            thunk->kind = kind;
            LeaveCriticalSection(&mmdrv_cs);
            return thunk;
        }
    }
    LeaveCriticalSection(&mmdrv_cs);
    FIXME("Out of mmdrv-thunks. Bump MMDRV_MAX_THUNKS\n");
    return NULL;
}

/******************************************************************
 *		MMSYSTDRV_FindHandle
 *
 * Must be called with lock set
 */
static void*    MMSYSTDRV_FindHandle(void* h)
{
    struct mmsystdrv_thunk* thunk;

    for (thunk = MMSYSTDRV_Thunks; thunk < &MMSYSTDRV_Thunks[MMSYSTDRV_MAX_THUNKS]; thunk++)
    {
        if (thunk->hMmdrv == h)
        {
            if (thunk->kind >= MMSYSTDRV_MAX) FIXME("Kind isn't properly initialized %x\n", thunk->kind);
            return thunk;
        }
    }
    return NULL;
}

/******************************************************************
 *		MMSYSTDRV_SetHandle
 *
 */
void    MMSYSTDRV_SetHandle(struct mmsystdrv_thunk* thunk, void* h)
{
    if (MMSYSTDRV_FindHandle(h)) FIXME("Already has a thunk for this handle %p!!!\n", h);
    thunk->hMmdrv = h;
}

/******************************************************************
 *		MMSYSTDRV_DeleteThunk
 */
void    MMSYSTDRV_DeleteThunk(struct mmsystdrv_thunk* thunk)
{
    thunk->callback = 0;
    thunk->flags = CALLBACK_NULL;
    thunk->hMmdrv = NULL;
    thunk->kind = MMSYSTDRV_MAX;
}

/******************************************************************
 *		MMSYSTDRV_CloseHandle
 */
void    MMSYSTDRV_CloseHandle(void* h)
{
    struct mmsystdrv_thunk* thunk;

    EnterCriticalSection(&mmdrv_cs);
    if ((thunk = MMSYSTDRV_FindHandle(h)))
    {
        MMSYSTDRV_DeleteThunk(thunk);
    }
    LeaveCriticalSection(&mmdrv_cs);
}

/******************************************************************
 *		MMSYSTDRV_Message
 */
DWORD   MMSYSTDRV_Message(void* h, UINT msg, DWORD_PTR param1, DWORD_PTR param2)
{
    struct mmsystdrv_thunk*     thunk = MMSYSTDRV_FindHandle(h);
    struct MMSYSTDRV_Type*      drvtype;
    MMSYSTEM_MapType            map;
    DWORD                       ret;

    if (!thunk) return MMSYSERR_INVALHANDLE;
    drvtype = &MMSYSTEM_DriversType[thunk->kind];

    map = drvtype->mapmsg16to32W(msg, &param1, &param2);
    switch (map) {
    case MMSYSTEM_MAP_NOMEM:
        ret = MMSYSERR_NOMEM;
        break;
    case MMSYSTEM_MAP_MSGERROR:
        FIXME("NIY: no conversion yet 16->32 kind=%u msg=%u\n", thunk->kind, msg);
        ret = MMSYSERR_ERROR;
        break;
    case MMSYSTEM_MAP_OK:
    case MMSYSTEM_MAP_OKMEM:
        TRACE("Calling message(msg=%u p1=0x%08lx p2=0x%08lx)\n",
              msg, param1, param2);
        switch (thunk->kind)
        {
        case MMSYSTDRV_MIXER:   ret = mixerMessage  (h, msg, param1, param2); break;
        case MMSYSTDRV_MIDIIN:
            switch (msg)
            {
            case MIDM_ADDBUFFER: ret = midiInAddBuffer(h, (LPMIDIHDR)param1, param2); break;
            case MIDM_PREPARE:   ret = midiInPrepareHeader(h, (LPMIDIHDR)param1, param2); break;
            case MIDM_UNPREPARE: ret = midiInUnprepareHeader(h, (LPMIDIHDR)param1, param2); break;
            default:             ret = midiInMessage(h, msg, param1, param2); break;
            }
            break;
        case MMSYSTDRV_MIDIOUT:
            switch (msg)
            {
            case MODM_PREPARE:   ret = midiOutPrepareHeader(h, (LPMIDIHDR)param1, param2); break;
            case MODM_UNPREPARE: ret = midiOutUnprepareHeader(h, (LPMIDIHDR)param1, param2); break;
            case MODM_LONGDATA:  ret = midiOutLongMsg(h, (LPMIDIHDR)param1, param2); break;
            default:             ret = midiOutMessage(h, msg, param1, param2); break;
            }
            break;
        case MMSYSTDRV_WAVEIN:
            switch (msg)
            {
            case WIDM_ADDBUFFER: ret = waveInAddBuffer(h, (LPWAVEHDR)param1, param2); break;
            case WIDM_PREPARE:   ret = waveInPrepareHeader(h, (LPWAVEHDR)param1, param2); break;
            case WIDM_UNPREPARE: ret = waveInUnprepareHeader(h, (LPWAVEHDR)param1, param2); break;
            default:             ret = waveInMessage(h, msg, param1, param2); break;
            }
            break;
        case MMSYSTDRV_WAVEOUT:
            switch (msg)
            {
            case WODM_PREPARE:   ret = waveOutPrepareHeader(h, (LPWAVEHDR)param1, param2); break;
            case WODM_UNPREPARE: ret = waveOutUnprepareHeader(h, (LPWAVEHDR)param1, param2); break;
            case WODM_WRITE:     ret = waveOutWrite(h, (LPWAVEHDR)param1, param2); break;
            default:             ret = waveOutMessage(h, msg, param1, param2); break;
            }
            break;
        default: ret = MMSYSERR_INVALHANDLE; break; /* should never be reached */
        }
        if (map == MMSYSTEM_MAP_OKMEM)
            drvtype->unmapmsg16to32W(msg, &param1, &param2, ret);
        break;
    default:
        FIXME("NIY\n");
        ret = MMSYSERR_NOTSUPPORTED;
        break;
    }
    return ret;
}
