TITLE		:= Spaceflight Simulator
TARGET		:= SpaceflightSimulator
CONTENTID	:= UP0001-SFS0000000_00-0000000000000000

PSL1GHT		:= $(PSL1GHT)
ifeq ($(strip $(PSL1GHT)),)
$(error "Please set PSL1GHT in your environment.")
endif

include $(PSL1GHT)/ppu_rules

SOURCES		:= source/
DATA		:= data/

INCLUDES	:= -I$(PORTLIBS)/include -I$(PSL1GHT)/ppu/include
CFLAGS		:= -O2 -Wall -mcpu=cell $(INCLUDES)
CXXFLAGS	:= $(CFLAGS)
LDFLAGS		:= -L$(PORTLIBS)/lib -L$(PSL1GHT)/ppu/lib
LIBS		:= -lSDL -lio -lsysutil -lm -lrt -llv2

CFILES		:= $(foreach dir,$(SOURCES),$(wildcard $(dir)*.c))
CPPFILES	:= $(foreach dir,$(SOURCES),$(wildcard $(dir)*.cpp))
OFILES		:= $(CFILES:.c=.o) $(CPPFILES:.cpp=.o)

all: $(TARGET).elf $(TARGET).self

$(TARGET).elf: $(OFILES)
	$(CXX) $(CFLAGS) $(OFILES) -o $@ $(LDFLAGS) $(LIBS)

$(TARGET).self: $(TARGET).elf
	$(SELF) $< $@

pkg: $(TARGET).self
	@echo "Gerando PARAM.SFO e PKG..."
	$(PSL1GHT)/host/bin/sfo.py -f sfo.xml -o PARAM.SFO
	mkdir -p pkg/USRDIR
	cp $(TARGET).self pkg/USRDIR/EBOOT.BIN
	cp PARAM.SFO pkg/PARAM.SFO
	cp ICON0.PNG pkg/ICON0.PNG 2>/dev/null || echo "sem ICON0, segue o jogo"
	$(PSL1GHT)/host/bin/pkg.py --contentid $(CONTENTID) pkg/ $(TARGET).pkg
	ls -lh $(TARGET).pkg

clean:
	rm -f $(OFILES) $(TARGET).elf $(TARGET).self PARAM.SFO $(TARGET).pkg
	rm -rf pkg

.PHONY: all clean pkg