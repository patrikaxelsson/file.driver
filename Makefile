all: file.driver file.driver_debug

# The order of the files given to the compiler is important as this order
# will be kept in the resulting library.
# The file containing the startup function must be first as AmigaOS simply
# starts executing from the start of the first hunk. Then to make it as
# efficient as possible to load this as a resident, the Resident structure
# is placed next, followed by the actual code for the library and then
# ending it with an empty function used to point/skip to the end in Resident
# structure.
file.driver: NoStart.c Resident.c Version.c file.driver.c ResidentEnd.c Version.h file.driver.h Debug.h ResidentEnd.h Makefile
	vc +aos68k -D__NOLIBBASE__ -nostdlib -c99 -O2 -fastcall -sc -o $@ NoStart.c Resident.c Version.c file.driver.c ResidentEnd.c

file.driver_debug: NoStart.c Resident.c Version.c file.driver.c ResidentEnd.c Version.h file.driver.h Debug.h ResidentEnd.h Makefile
	vc +aos68k -D__NOLIBBASE__ -D__DEBUG__ -nostdlib -c99 -O2 -fastcall -sc -lamiga -ldebug -o $@ NoStart.c Resident.c Version.c file.driver.c ResidentEnd.c

clean:
	$(RM) file.driver file.driver_debug

