# NOTE: only tested on MinGW on Windows. 
# Assumes mingw-w64 is installed on your windows PATH.

WORKDIR=$(shell echo %cd%)
MKDIR = shell "mkdir"

CC=gcc
CC64=gcc
CFLAGS_D32= -m32 -g
CFLAGS_D64= -m64 -g

OBJECTS_DLL_64 = $(WORKDIR)\obj\dll_main.o64
OBJECTS_DLL_32 = $(WORKDIR)\obj\dll_main.o32
LDFLAGS_SO_32 = -shared -Wl,--export-all-symbols -m32 -g
LDFLAGS_SO_64 = -shared -Wl,--export-all-symbols -m64 -g

check_dirs:
	cmd /c if not exist $(WORKDIR)\bin md $(WORKDIR)\bin
	cmd /c if not exist $(WORKDIR)\obj md $(WORKDIR)\obj

$(WORKDIR)\bin\eft_loader_x64.dll: check_dirs $(OBJECTS_DLL_64)
	$(CC64) $(LDFLAGS_SO_64) -o $@ $(OBJECTS_DLL_64)

$(WORKDIR)\bin\eft_loader_x86.dll: check_dirs $(OBJECTS_DLL_32)
	$(CC) $(LDFLAGS_SO_32) -o $@ $(OBJECTS_DLL_32)

$(WORKDIR)\obj\dll_main.o64: $(WORKDIR)\src\dll_main.c
	$(CC64) -std=c99 $(CFLAGS_D64) -c $< -o $@

$(WORKDIR)\obj\dll_main.o32: $(WORKDIR)\src\dll_main.c
	$(CC) -std=c99 $(CFLAGS_D32) -c $< -o $@

eft_loader_x64: $(WORKDIR)\bin\eft_loader_x64.dll
eft_loader_x86: $(WORKDIR)\bin\eft_loader_x86.dll
all: $(WORKDIR)\bin\eft_loader_x64.dll $(WORKDIR)\bin\eft_loader_x86.dll

clean:
	cmd /c del /f $(WORKDIR)\bin\eft_loader_x64.dll $(WORKDIR)\bin\eft_loader_x86.dll
	cmd /c del /f $(OBJECTS_DLL_64) $(OBJECTS_DLL_32)
	cmd /c rd $(WORKDIR)\bin
	cmd /c rd $(WORKDIR)\obj

.PHONY:	clean check_dirs