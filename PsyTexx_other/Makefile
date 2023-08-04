ifeq ($(MAKECMDGOALS),)
    $(error No target selected. Please type "make palm", "make linux", or "make bsd")
    exit
endif

## ------------------------------------------ ##
## Main target options:                       ##
## ------------------------------------------ ##

ifeq ($(MAKECMDGOALS),palm)
    SDK = ../palm-os-sdk/sdk-5r3/
    PSYTEXX_DEFINES = \
	-DPALMOS \
	-DCOLOR8BITS \
	-DNOPSYNTH \
	-DSLOWMODE \
	-DNOSTORAGE
	#crashing if using storage on instrument editing
    MCC = m68k-palmos-gcc -Os
    MOBJRES = m68k-palmos-obj-res
    ####CC = ../gnutools/bin/arm-elf-gcc -Os
    #CC = ../../arm-elf-eabi-builder/gnutools/arm-elf-eabi-7.3.0/bin/arm-elf-gcc -Os -marm -mlittle-endian
    #CC = arm-palmos-gcc -Os
    CC = arm-none-eabi-gcc -Os
    ####OBJCPY = ../../arm-elf-eabi-builder/gnutools/arm-elf-eabi-7.3.0/bin/arm-elf-objcopy
    ####OBJCPY = ../gnutools/bin/arm-elf-objcopy
    #OBJCPY = arm-palmos-objcopy
    OBJCPY = arm-none-eabi-objcopy
endif
ifeq ($(MAKECMDGOALS),linux)
    # Console libs:
#    LIBS = -lpthread -lrt -lstdc++ -lgpm
#    PSYTEXX_DEFINES = -DLINUX -DUNIX -DCOLOR8BITS -DTEXTMODE
    #CC = gcc -O3
    #CXX = gcc -O3
    # X11 (OpenGL) libs:
    #LIBS = -L/usr/X11R6/lib -lpthread -lrt -lstdc++ -lX11 -lGL -lXext
    # X11 libs:
    LIBS = -L/usr/X11R6/lib -lpthread -lrt -lX11 -lgpm
    PSYTEXX_DEFINES = -DLINUX -DUNIX -DX11 -DCOLOR16BITS -DDEBUGMODE
    CC = i686-linux-gnu-gcc -O3
    CXX = i686-linux-gnu-gcc -O3
endif
ifeq ($(MAKECMDGOALS),bsd)
    # Console libs:
    #LIBS = -lpthread -lrt -lstdc++  -lossaudio -Wl,-R/usr/X11R7/lib -Wl,/usr/lib/libossaudio.a -L/usr/lib  # -lgpm 
    #PSYTEXX_DEFINES = -DBSD -DUNIX -DCOLOR8BITS -DTEXTMODE
    #CC = gcc -O3
    #CXX = gcc -O3
    # X11 (OpenGL) libs:
    #LIBS = -L/usr/X11R6/lib -lpthread -lrt -lstdc++ -lX11 -lGL -lXext
    # X11 libs:
    LIBS = -L/usr/X11R7/lib -lpthread -lrt -lX11 -lossaudio -Wl,-R/usr/X11R7/lib -Wl,/usr/lib/libossaudio.a -L/usr/lib
    PSYTEXX_DEFINES = -DBSD -DLINUX -DUNIX -DX11 -DCOLOR16BITS -DDEBUGMODE -I/usr/X11R7/include/
    CC = gcc -O3
    CXX = gcc -O3
endif

## ------------------------------------------ ##
## Directories:                               ##
## ------------------------------------------ ##

PSY = ../psytexx/
CPSY = $(PSY)code/
MEM = ../memory/
CMEM = $(MEM)code/
XM = ../xm/
CXM = $(XM)code/
SND = ../sound/
CSND = $(SND)code/
CORE = ../core/
CCORE = $(CORE)code/
FS = ../filesystem/
CFS = $(FS)code/
TM = ../time/
CTM = $(TM)code/
DEMO = ../demoengine/
CDEMO = $(DEMO)code/
JPG = $(DEMO)tiny_jpg/
WM = ../window_manager/
CWM = $(WM)code/
ARM = ARMPalmOS/
CARM = $(ARM)code/

## ------------------------------------------ ##
## Object files and includes:                 ##
## ------------------------------------------ ##

DEFINES = $(PSYTEXX_DEFINES) -I../
#-fpermissive

STD_INCS = $(MEM)memory.h $(CORE)core.h $(CORE)debug.h
XM_STD_INCS = $(XM)xm.h $(STD_INCS)
PSYNTH_STD_INCS = $(XM)psynth/psynth.h $(XM)psynth/psynth_net.h $(STD_INCS)
XM_OBJ = \
    xm_play.o \
    xm_song.o \
    xm_pattern.o \
    xm_instrum.o \
    xm_sample.o \
    xm_tables.o \
    xm_channels.o \
    xm_main.o
PSYNTH_OBJ = \
    psynths.o \
    psynth_net.o
SOUND_OBJ = sound_player.o sound_sndout.o
WINDOW_MANAGER_OBJ = wmanager.o eventloop.o
PSY_WINDOWS_OBJ = \
    win_main.o \
    win_files.o \
    win_list.o \
    win_button.o \
    win_scrollbar.o \
    win_popup.o \
    win_text.o \
    win_dialog.o \
    win_psymenu.o \
    win_psytable.o \
    win_psysamples.o \
    win_psypattern.o \
    win_psypatterntab.o \
    win_psyconfig.o \
    win_psyplaylist.o \
    win_psychannels.o \
    win_psyinsteditor.o \
    win_psysmpeditor.o \
    win_psysmpview.o \
    win_psynet.o
DEMOENGINE_OBJ = demoengine.o jpg.o
OBJECTS = \
    psytexx.o \
    memory.o \
    debug.o \
    v3nus_fs.o \
    timemanager.o \
    $(PSY_WINDOWS_OBJ) \
    $(XM_OBJ) \
    $(PSYNTH_OBJ) \
    $(SOUND_OBJ) \
    $(WINDOW_MANAGER_OBJ) \
    $(DEMOENGINE_OBJ)

ifeq ($(MAKECMDGOALS),palm)
    ARM_PALM_INCS = -IARMPalmOS/ -I$(SDK)include -I$(SDK)include/Libraries -I$(SDK)include/Extensions/ExpansionMgr -I$(SDK)include/Dynamic -I$(SDK)include/Core -I$(SDK)include/Core/Hardware -I$(SDK)include/Core/System -I$(SDK)include/Core/System/Unix -I$(SDK)include/Core/UI
    #-I../gnutools/arm-elf/include
    ARM_PALM_FLAGS = -DNATIVE -mcpu=arm9tdmi -nostartfiles -ffixed-r9 -mpic-register=r10 -msingle-pic-base -fPIC
    #-mfp=3 -mshort-load-bytes
    #-ffixed-r8 removed
    DEFINES += $(ARM_PALM_INCS) $(ARM_PALM_FLAGS) $(ARM_PALM_DEFINES)
    ARM_PALM_FILES = $(CARM)palm_functions.c $(ARM)PalmTypes.h $(ARM)DebugMgr.h $(ARM)palm_functions.h
    ARM_PALM_OBJECTS = palm_functions.o
    PALM_OBJECTS = $(OBJECTS) $(ARM_PALM_OBJECTS)
endif

## ------------------------------------------ ##
## Main rules for different targets:          ##
## ------------------------------------------ ##

linux: $(OBJECTS)
	$(CC) $(DEFINES) -o psytexx $(OBJECTS) $(LIBS)

bsd: $(OBJECTS)
	$(CC) $(DEFINES) -o psytexx $(OBJECTS) $(LIBS)

palm: $(PALM_OBJECTS)
#Link main code ( with special ARM script - armscript.ld ) :
	$(CC) $(DEFINES) -o main_palm.o $(PALM_OBJECTS) -Wl,--noinhibit-exec -Wl,--build-id=none -Xlinker --script=ARMFiles/armscript1.ld
#Create BIN files from the program_palm.o :
	$(OBJCPY) -j .text -O binary main_palm.o armcode
	split -b 64000 armcode armc
	mv armcaa armc0000.bin
	mv armcab armc0001.bin
	mv armcac armc0002.bin
	$(OBJCPY) -j .got -O binary main_palm.o armg0000.bin
	$(OBJCPY) -j .alldata -O binary main_palm.o armd0000.bin
	$(OBJCPY) -j .rel -O binary main_palm.o armr0000.bin
#Create Palm resources ( *.BIN files ) :
	pilrc Resources/resources.rcp
#Build PRC file :
	$(MCC) ARMFiles/arm_starter.c -o arm_starter.o -I$(SDK)include
	$(MOBJRES) arm_starter.o
	build-prc -v 1.0 ARMFiles/info.def -o psytexx.prc *.grc *.bin

## ------------------------------------------ ##
## Other rules:                               ##
## ------------------------------------------ ##

psytexx.o: $(CPSY)psytexx.c $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)psytexx.c

win_main.o: $(CPSY)win_main.c $(PSY)win_main.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_main.c

win_files.o: $(CPSY)win_files.c $(PSY)win_files.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_files.c

win_list.o: $(CPSY)win_list.c $(PSY)win_list.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_list.c

win_button.o: $(CPSY)win_button.c $(PSY)win_button.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_button.c

win_popup.o: $(CPSY)win_popup.c $(PSY)win_popup.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_popup.c

win_scrollbar.o: $(CPSY)win_scrollbar.c $(PSY)win_scrollbar.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_scrollbar.c

win_psymenu.o: $(CPSY)win_psymenu.c $(PSY)win_psymenu.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psymenu.c

win_psytable.o: $(CPSY)win_psytable.c $(PSY)win_psytable.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psytable.c

win_text.o: $(CPSY)win_text.c $(PSY)win_text.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_text.c

win_psysamples.o: $(CPSY)win_psysamples.c $(PSY)win_psysamples.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psysamples.c

win_psypattern.o: $(CPSY)win_psypattern.c $(PSY)win_psypattern.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psypattern.c

win_psypatterntab.o: $(CPSY)win_psypatterntab.c $(PSY)win_psypatterntab.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psypatterntab.c

win_dialog.o: $(CPSY)win_dialog.c $(PSY)win_dialog.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_dialog.c

win_psyconfig.o: $(CPSY)win_psyconfig.c $(PSY)win_psyconfig.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psyconfig.c

win_psyplaylist.o: $(CPSY)win_psyplaylist.c $(PSY)win_psyplaylist.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psyplaylist.c

win_psychannels.o: $(CPSY)win_psychannels.c $(PSY)win_psychannels.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psychannels.c

win_psyinsteditor.o: $(CPSY)win_psyinsteditor.c $(PSY)win_psyinsteditor.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psyinsteditor.c

win_psysmpeditor.o: $(CPSY)win_psysmpeditor.c $(PSY)win_psysmpeditor.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psysmpeditor.c

win_psysmpview.o: $(CPSY)win_psysmpview.c $(PSY)win_psysmpview.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psysmpview.c

win_psynet.o: $(CPSY)win_psynet.c $(PSY)win_psynet.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CPSY)win_psynet.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

xm_play.o: $(CXM)xm_play.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_play.c

xm_song.o: $(CXM)xm_song.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_song.c

xm_pattern.o: $(CXM)xm_pattern.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_pattern.c

xm_instrum.o: $(CXM)xm_instrum.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_instrum.c

xm_sample.o: $(CXM)xm_sample.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_sample.c

xm_tables.o: $(CXM)xm_tables.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_tables.c

xm_channels.o: $(CXM)xm_channels.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_channels.c

xm_main.o: $(CXM)xm_main.c $(XM_STD_INCS)
	$(CC) $(DEFINES) -c $(CXM)xm_main.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

psynth_net.o: $(XM)psynth/psynth_net.c $(PSYNTH_STD_INCS)
	$(CC) $(DEFINES) -c $(XM)psynth/psynth_net.c

psynths.o: $(XM)psynth/psynths.c $(PSYNTH_STD_INCS)
	$(CC) $(DEFINES) -c $(XM)psynth/psynths.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

sound_player.o: $(CSND)sound_player.c $(SND)sound.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CSND)sound_player.c

sound_sndout.o: $(CSND)sound_sndout.c $(SND)sound.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CSND)sound_sndout.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

timemanager.o: $(CTM)timemanager.c $(TM)timemanager.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CTM)timemanager.c

v3nus_fs.o: $(CFS)v3nus_fs.c $(FS)v3nus_fs.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CFS)v3nus_fs.c

debug.o: $(CCORE)debug.c
	$(CC) $(DEFINES) -c $(CCORE)debug.c

memory.o: $(CMEM)memory.c $(MEM)memory.h $(CORE)core.h
	$(CC) $(DEFINES) -c $(CMEM)memory.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

demoengine.o: $(CDEMO)demoengine.c $(DEMO)demoengine.h $(CDEMO)demo_effects.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CDEMO)demoengine.c

jpg.o: $(JPG)jpg.c $(JPG)jpg.h
	$(CC) $(DEFINES) -c $(JPG)jpg.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

wmanager.o: $(CWM)wmanager.c $(WM)wmanager.h $(WM)struct.h $(CWM)wm_linux.h $(CWM)wm_linuxgraphics.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CWM)wmanager.c

eventloop.o: $(CWM)eventloop.c $(WM)wmanager.h $(WM)struct.h $(CWM)wm_linux.h $(CWM)wm_linuxgraphics.h $(STD_INCS)
	$(CC) $(DEFINES) -c $(CWM)eventloop.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

palm_functions.o: $(ARM_PALM_FILES)
	$(CC) $(DEFINES) -c $(CARM)palm_functions.c

## ------------------------------------------ ##
##                                            ##
## ------------------------------------------ ##

clean:
	#@if exist *.o rm *.o
	#@if exist *.bin rm *.bin
	#@if exist *.grc rm *.grc
	#@if exist armcode rm armcode
	rm *.o *.grc *.bin armcode

