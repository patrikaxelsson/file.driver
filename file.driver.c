#include <exec/exec.h>
#include <dos/dos.h>

#include <proto/exec.h>
#include <proto/dos.h>

#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "Version.h"
#include "file.driver.h"
#include "Debug.h"


struct DriverBase {
	struct Library library;
	BPTR seglist;
	struct ExecBase *sysBase;
	struct DosLibrary *dosBase;
	BPTR file;
	ULONG *buffer;
	size_t bufferSize;
	char destination[128];
};

static struct DriverBase *LibraryInit(
		__reg("d0") struct DriverBase *driverBase,
		__reg("a0") BPTR seglist,
		__reg("a6") struct ExecBase *SysBase) {
	kprintf("%s(%08lx, %08lx, %08lx)\n", __func__, driverBase, seglist, SysBase);
	// MakeLibrary() will zero the entire DriverBase struct before calling this function
	driverBase->library.lib_Node.ln_Type = NT_LIBRARY;
	driverBase->library.lib_Node.ln_Name = (char *) FileName;
	driverBase->library.lib_Flags        = LIBF_SUMUSED | LIBF_CHANGED;
	driverBase->library.lib_Version      = VERSION;
	driverBase->library.lib_Revision     = REVISION;
	driverBase->library.lib_IdString     = IdString;

	driverBase->seglist = seglist;
	driverBase->sysBase = SysBase;

	return driverBase;
}


static LONG ExtFunc(void) {
	return 0;
}

static struct DriverBase *LibraryOpen(
		__reg("a6") struct DriverBase *driverBase,
		__reg("d0") LONG version) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx, %ld)\n", __func__, driverBase, version);
	struct DosLibrary *DOSBase = NULL;
	// Make sure the memory allocator cannot expunge us on a memory allocation
	// during all of this
	driverBase->library.lib_OpenCnt++;

	// Only meant for one of these open at the same time
	if (driverBase->library.lib_OpenCnt > 1) {
		kprintf("%s - Only one open of %s allowed", __func__, FileName);
		goto error;
	}

	DOSBase = (void *) OpenLibrary("dos.library", 0);
	if (NULL == DOSBase) {
		kprintf("%s - Failed opening dos.library\n", __func__);
		goto error;
	}

	if (-1 == GetVar("CyberSound/SoundDrivers/file_Destination", driverBase->destination, sizeof(driverBase->destination), GVF_GLOBAL_ONLY)) {
		strcpy(driverBase->destination, "T:tune.pcm");
	}

	driverBase->file = Open(driverBase->destination, MODE_NEWFILE);
	if (0 == driverBase->file) {
		kprintf("%s - Failed opening %s\n", __func__, driverBase->destination);
		goto error;
	}
	kprintf("%s - Opened %s\n", __func__, driverBase->destination);
	driverBase->dosBase = DOSBase;

	// We just opened successfully and are not intended to be expunged
	driverBase->library.lib_Flags &= ~LIBF_DELEXP;

	kprintf("%s: %08lx\n", __func__, driverBase);
	return driverBase;

error:
	CloseLibrary((void *) DOSBase);
	driverBase->library.lib_OpenCnt--;
	return NULL;	
}

static BPTR LibraryExpunge(__reg("a6") struct DriverBase *driverBase) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx)\n", __func__, driverBase);
	
	kprintf("%s - lib_OpenCnt=%ld, lib_Flags=%lx\n", __func__, driverBase->library.lib_OpenCnt, driverBase->library.lib_Flags);
	if (0 == driverBase->library.lib_OpenCnt) {
		Remove((struct Node *) driverBase);

		const BPTR seglist = driverBase->seglist;
		const void *libraryStart = (uint8_t *) &driverBase->library - driverBase->library.lib_NegSize;
		const size_t librarySize = driverBase->library.lib_NegSize + driverBase->library.lib_PosSize;
		FreeMem(libraryStart, librarySize);

		kprintf("%s: %08lx\n", __func__, seglist);
		return seglist;
	}
	else {
		kprintf("%s - Setting delayed expunge!\n", __func__);
		driverBase->library.lib_Flags |= LIBF_DELEXP;
	}
	kprintf("%s: 0\n", __func__);
	return 0;
}

static BPTR LibraryClose(__reg("a6") struct DriverBase *driverBase) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx)\n", __func__, driverBase);
	struct DosLibrary *DOSBase = driverBase->dosBase;
	
	FreeVec(driverBase->buffer);
	driverBase->buffer = NULL;
	kprintf("%s - Freed buffer\n", __func__);

	if (0 != driverBase->file) {
		Close(driverBase->file);
		driverBase->file = 0;
		kprintf("%s - Closed %s\n", __func__, driverBase->destination);
	}

	CloseLibrary((void *) DOSBase);
	driverBase->dosBase = NULL;

	kprintf("%s - lib_OpenCnt=%ld, lib_Flags=%lx\n", __func__, driverBase->library.lib_OpenCnt, driverBase->library.lib_Flags);
	driverBase->library.lib_OpenCnt--;
	if(0 == driverBase->library.lib_OpenCnt && driverBase->library.lib_Flags & LIBF_DELEXP) {
		const BPTR seglist = LibraryExpunge(driverBase);
		kprintf("%s: %08lx\n", __func__, seglist);
		return seglist;
	}

	kprintf("%s: 0\n", __func__);
	return 0;
}

static BOOL SetBuffers(
		__reg("a6") struct DriverBase *driverBase,
		__reg("d0") ULONG audiosize,
		__reg("d1") ULONG queuesize
) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx, %lu, %lu)\n", __func__, driverBase, audiosize, queuesize);
	return FALSE;
}

static BOOL StreamFormat(__reg("a6") struct DriverBase *driverBase, __reg("d0") ULONG format) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx, %08lx)\n", __func__, driverBase, format);
	return FALSE;
}

static ULONG AskFrequency(__reg("a6") struct DriverBase *driverBase, __reg("d0") ULONG frequency) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx, %lu)\n", __func__, driverBase, frequency);
	return frequency;
}

static void *AllocateBuffer(
		struct DriverBase *driverBase,
		size_t bufferSize
) {
	struct ExecBase *SysBase = driverBase->sysBase;
	
	void *buffer = driverBase->buffer;
	if (NULL != buffer && bufferSize > driverBase->bufferSize) {
		FreeVec(buffer);
		buffer = NULL;
	}

	if (NULL == buffer) {
		buffer = AllocVec(bufferSize, MEMF_ANY);	
		driverBase->buffer = buffer;
		driverBase->bufferSize = bufferSize;
	}

	return buffer;
}

/* This is simple, but it is not super important to optimize for the total
 * playback performance of GMPlay+file.driver.
 * Example on A3000 030@25MHz:
 * SetEnv CyberSound/SoundDrivers/file_Destination NIL:
 * UHC:C/time GMPlay MIDI-Files/Swamp_Thing.MID OUTPUT=file FREQUENCY=22050
 * Result:
 * - When run normally:                         64.2s
 * - Commented out call to EncodeToStereoPcm(): 59.5s
 */
static void EncodeToStereoPcm(UWORD *left, UWORD *right, ULONG samples, UWORD *dest) {
	if (0 == samples) {
		return;
	}
	do {
		*dest++ = *left++;
		*dest++ = *right++;
	} while(--samples);
}

static BOOL ProvideStream(
		__reg("a6") struct DriverBase *driverBase,
		__reg("a0") UWORD *left,
		__reg("a1") UWORD *right,
		__reg("d0") ULONG samples,
		__reg("d1") UWORD interleave,
		__reg("d2") ULONG frequency,
		__reg("a2") void (*callback)(void)
) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx, %08lx, %08lx, %lu, %lu, %lu, %08lx)\n", __func__, driverBase, left, right, samples, interleave, frequency, callback);
	struct DosLibrary *DOSBase = driverBase->dosBase;
	BOOL result = TRUE;

	if (0 != interleave) {
		kprintf("%s - Interleave %lu not supported\n", __func__, interleave); 
		result = FALSE;
		goto cleanup;
	}

	// Mono - is already same format as PCM
	size_t bufferSize = sizeof(UWORD) * samples;
	void *buffer = (void *) left;

	if (left != right) {
		// Stereo - now we have to create PCM
		bufferSize *= 2;
		buffer = AllocateBuffer(driverBase, bufferSize);
		if (NULL == buffer){
			kprintf("%s - Failed allocating buffer of size %lu\n", __func__, bufferSize); 
			result = FALSE;
			goto cleanup;
		}

		EncodeToStereoPcm(left, right, samples, buffer);
	}

	if (-1 == Write(driverBase->file, buffer, bufferSize)) {
		kprintf("%s - Failed writing to %s\n", __func__, driverBase->destination); 
		result = FALSE;
		goto cleanup;
	}

cleanup:
	callback();
	return result;
}

static void FlushStream(__reg("a6") struct DriverBase *driverBase) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx)\n", __func__, driverBase);
}

static void PauseStream(__reg("a6") struct DriverBase *driverBase) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx)\n", __func__, driverBase);
}

static void ResumeStream(__reg("a6") struct DriverBase *driverBase) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx)\n", __func__, driverBase);
}

static void SD_Lock(__reg("a6") struct DriverBase *driverBase) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx)\n", __func__, driverBase);
}

static void SD_Unlock(__reg("a6") struct DriverBase *driverBase) {
	struct ExecBase *SysBase = driverBase->sysBase;
	kprintf("%s(%08lx)\n", __func__, driverBase);
}

static const APTR LibraryFunctions[] = {
	(void *) &LibraryOpen,
	(void *) &LibraryClose,
	(void *) &LibraryExpunge,
	(void *) &ExtFunc,
	(void *) &SetBuffers,
	(void *) &StreamFormat,
	(void *) &AskFrequency,
	(void *) &ProvideStream,
	(void *) &FlushStream,
	(void *) &PauseStream,
	(void *) &ResumeStream,
	(void *) &SD_Lock,
	(void *) &SD_Unlock,
	(void *) -1
};


const struct InitTable LibraryInitTable = {
	.it_LibBaseSize = sizeof(struct DriverBase),
	.it_FuncTable   = (void *) LibraryFunctions,
	// If DataTable is non-null, MakeLibrary() will init the device/library base using InitStruct(),
	// instead we do it ourselves - see LibraryBaseInit() called from InitMain()/InitChild().
	.it_DataTable   = NULL,                         
	.it_InitRoutine = (void *) &LibraryInit
};

