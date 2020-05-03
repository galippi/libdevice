##################################################################
#CFLAGS_DEBUG= -gstabs
CFLAGS_DEBUG= -gdwarf-2
#CFLAGS_OPTIM= -O6
#CFLAGS_NO_CYGWIN_DLL=-mno-cygwin
CC       = gcc
LL       = gcc
CFLAGS   = $(CFLAGS_OPTIM) -Wall $(CFLAGS_DEBUG) $(CFLAGS_NO_CYGWIN_DLL)
#CFLAGS  += -I/usr/include/SDL
CFLAGS  += -Ivcan/src -Ivcan/include -Ivcan/include/linux
CPPFLAGS   = $(CFLAGS) -fno-rtti
CPPFLAGS += -std=c++11

#LDFLAGS_STRIP_DEBUG_INFO = -s
# LDFLAGS  = -s  $(CFLAGS_NO_CYGWIN_DLL) -lcrypto -lssl #-lsocket -lnsl
LDFLAGS  = $(LDFLAGS_STRIP_DEBUG_INFO)  $(CFLAGS_NO_CYGWIN_DLL)
#LDFLAGS += -L./pc-libs
#LDLIBS = -lcrypto
#LDLIBS += -lSDL_image
#LDLIBS += -lpng12
#LDLIBS += -lz
#LDLIBS += -lSDL
#it's necessary for the new SDL library
#LDLIBS += -lwinmm
#LDLIBS += -lmsvcr70
#LDLIBS += -lmsvcrt
#LDLIBS += -lgdi32
#LDLIBS += -ldxguid
LDLIBS += -lstdc++
MAKEFILE=Makefile
##################################################################
TARGET=libdevice_test
TARGET_DIR=bin
DUMMY_DIR_FILE = $(TARGET_DIR)/dummy

CPPFILES = $(TARGET).cpp
CPPFILES+= libdevice.cpp
CPPFILES+= libdevice_adc.cpp
CPPFILES+= libdevice_dio.cpp
CPPFILES+= libdevice_timer.cpp
CPPFILES+= libdevice_can.cpp
CPPFILES+= libdevice_vcan.cpp
CPPFILES+= canlogger_libdevice.cpp
#CPPFILES+= debug.cpp
#CPPFILES+= osm_xml.cpp
#CFILES    = $(TARGET).c
#CFILES   = c_file.c
#CFILES  += SDL_WM_mini.c
VPATH += . vcan/src $(TARGET_DIR)
CFILES += vcan.c

#OBJECTS = $(addprefix $(TARGET_DIR)/,$(CPPFILES:.cpp=.o) $(CFILES:.c=.o))
OBJECTS = $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)
DEPFILES = $(addprefix $(TARGET_DIR)/,$(CPPFILES:.cpp=.d) $(CFILES:.c=.d))

TARGET_EXE = $(TARGET_DIR)/$(TARGET).exe
all : $(TARGET_EXE)

.PHONY: dummy
dummy:

$(TARGET_EXE) : $(OBJECTS)
	@echo OBJECTS=$(OBJECTS)
	@echo ^=$^
	$(LL) $(addprefix $(TARGET_DIR)/,$(notdir $^)) $(LDFLAGS) $(LDLIBS) -o $@

#%.o: %.c $(MAKEFILE)
%.o: %.c $(DUMMY_DIR_FILE)
	@echo Building $@
	$(CC) $(CFLAGS) -c -o $(TARGET_DIR)/$@ $<
	-@rm -f $(@:.o=.d)
	$(CC) -M $(CFLAGS) -c -o $(TARGET_DIR)/$(@:.o=.d) $<

#%.o: %.cpp $(MAKEFILE)
%.o: %.cpp $(DUMMY_DIR_FILE)
	@echo Building $@
	$(CC) $(CPPFLAGS) -c -o $(TARGET_DIR)/$@ $<
	-@rm -f $(@:.o=.d)
	$(CC) -M $(CPPFLAGS) -c -o $(TARGET_DIR)/$(@:.o=.d) $<

$(DUMMY_DIR_FILE):
	-mkdir $(TARGET_DIR)
	echo Dummy file >$@

##################################################################
# cleaning rule
##################################################################

clean:
	rm -f $(addprefix $(TARGET_DIR)/, *.o *.d *~ $(TARGET))

dep_test:
	@echo DEPFILES=$(DEPFILES)
	@echo wildcard=$(wildcard $(DEPFILES))

#include $(wildcard $(DEPFILES))
include $(DEPFILES)
