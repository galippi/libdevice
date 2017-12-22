##################################################################
#CFLAGS_DEBUG= -gstabs
CFLAGS_DEBUG= -gdwarf-2
#CFLAGS_OPTIM= -O6
#CFLAGS_NO_CYGWIN_DLL=-mno-cygwin
CC       = gcc
LL       = gcc
CFLAGS   = $(CFLAGS_OPTIM) -Wall $(CFLAGS_DEBUG) $(CFLAGS_NO_CYGWIN_DLL)
#CFLAGS  += -I/usr/include/SDL
CPPFLAGS   = $(CFLAGS) -fno-rtti
CPPFLAGS += -std=c++11

#LDFLAGS_STRIP_DEVUG_INFO = -s
# LDFLAGS  = -s  $(CFLAGS_NO_CYGWIN_DLL) -lcrypto -lssl #-lsocket -lnsl
LDFLAGS  = $(LDFLAGS_STRIP_DEVUG_INFO)  $(CFLAGS_NO_CYGWIN_DLL) #-lsocket -lnsl
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
TARGET_DIR=.

CPPFILES = $(TARGET).cpp
CPPFILES+= libdevice.cpp
CPPFILES+= libdevice_adc.cpp
CPPFILES+= libdevice_dio.cpp
#CPPFILES+= debug.cpp
#CPPFILES+= osm_xml.cpp
#CFILES    = $(TARGET).c
#CFILES   = c_file.c
#CFILES  += SDL_WM_mini.c

OBJECTS = $(addprefix $(TARGET_DIR)/,$(CPPFILES:.cpp=.o) $(CFILES:.c=.o))
DEPFILES = $(addprefix $(TARGET_DIR)/,$(CPPFILES:.cpp=.d) $(CFILES:.c=.d))

TARGET_EXE = $(TARGET_DIR)/$(TARGET).exe
all : $(TARGET_EXE)

.PHONY: dummy
dummy:

$(TARGET_EXE) : $(OBJECTS)
	$(LL) $^ $(LDFLAGS) $(LDLIBS) -o $@

#%.o: %.c $(MAKEFILE)
$(TARGET_DIR)/%.o: %.c
	@echo Building $@
	$(CC) $(CFLAGS) -c -o $@ $<
	-@rm -f $(@:.o=.d)
	$(CC) -M $(CFLAGS) -c -o $(@:.o=.d) $<

#%.o: %.cpp $(MAKEFILE)
$(TARGET_DIR)/%.o: %.cpp
	@echo Building $@
	$(CC) $(CPPFLAGS) -c -o $@ $<
	-@rm -f $(@:.o=.d)
	$(CC) -M $(CPPFLAGS) -c -o $(@:.o=.d) $<

##################################################################
# cleaning rule
##################################################################

clean:
	rm -f $(addprefix $(TARGET_DIR)/, *.o *.d *~ $(TARGET))

dep_test:
	@echo DEPFILES=$(DEPFILES)
	@echo wildcard=$(wildcard $(DEPFILES))

include $(wildcard $(DEPFILES))
