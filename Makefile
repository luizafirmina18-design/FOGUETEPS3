TITLE := Spaceflight Simulator
TARGET := SpaceflightSimulator
CONTENTID := SFS00000-XXXX00000_00-0000000000000000

PSL1GHT := $(PSL1GHT)
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment.")
endif

include $(PSL1GHT)/ppu_rules

SOURCES := source/
INCLUDES := -I$(PORTLIBS)/include -I$(PSL1GHT)/ppu/include
CFLAGS := -O2 -Wall -mcpu=cell
CXXFLAGS := $(CFLAGS)
LDFLAGS := -L$(PORTLIBS)/lib -L$(PSL1GHT)/ppu/lib
LIBS := -lSDL -lio -lsysutil -lm -lrt -llv2

CFILES := $(foreach dir,$(SOURCES),$(wildcard $(dir)*.c))
CPPFILES := $(foreach dir,$(SOURCES),$(wildcard $(dir)*.cpp))
OFILES := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

all: $(TARGET).elf $(TARGET).self

$(TARGET).elf: $(OFILES)
	$(CXX) $(CFLAGS) $(OFILES) -o $@ $(LDFLAGS) $(LIBS)

$(TARGET).self: $(TARGET).elf
	$(SELF) $< $@

clean:
	rm -f $(OFILES) $(TARGET).elf $(TARGET).self