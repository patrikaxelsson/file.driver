#include <exec/resident.h>
#include <exec/libraries.h>

#include "Version.h"
#include "ResidentEnd.h"
#include "file.driver.h"

const struct Resident ROMTag = {
	.rt_MatchWord = RTC_MATCHWORD,
	.rt_MatchTag  = (struct Resident *) &ROMTag,
	.rt_EndSkip   = (void *) &ResidentEnd,
	.rt_Flags     = RTF_AUTOINIT,
	.rt_Version   = 1,
	.rt_Type      = NT_LIBRARY,
	.rt_Pri       = 0,
	.rt_Name      = (char *) FileName,
	.rt_IdString  = (char *) IdString,
	.rt_Init      = (void *) &LibraryInitTable
};
