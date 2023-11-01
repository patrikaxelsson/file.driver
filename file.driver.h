#ifndef __FILE_DRIVER_H__
#define __FILE_DRIVER_H__

struct InitTable {
	ULONG it_LibBaseSize;
	void **it_FuncTable;
	ULONG *it_DataTable;
	APTR it_InitRoutine;
};

extern const struct InitTable LibraryInitTable;

#endif
