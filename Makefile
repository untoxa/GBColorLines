GBDK = ../../gbdk
GBDKLIB = $(GBDK)/lib/small/asxxxx
CC = $(GBDK)/bin/lcc

CART_SIZE = 4

ROM_BUILD_DIR = build
OBJDIR = obj
CFLAGS = -Isrc/include -Wa-Isrc/include -Wa-I$(GBDKLIB)

#LFLAGS_NBANKS += -Wl-yt0x1B -Wl-yo$(CART_SIZE) -Wl-ya4 -Wl-j
LFLAGS_NBANKS =

LFLAGS = $(LFLAGS_NBANKS) -Wl-j -Wl-klib -Wl-lhUGEDriver.lib -Wm-yc -Wm-yn"COLORLINES"

TARGET = $(ROM_BUILD_DIR)/colorlines.gb

ASRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.s))) 
CSRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.c))) 

ACORE = $(foreach dir,src/core,$(notdir $(wildcard $(dir)/*.s))) 
CCORE = $(foreach dir,src/core,$(notdir $(wildcard $(dir)/*.c))) 

OBJS = $(CSRC:%.c=$(OBJDIR)/%.o) $(ASRC:%.s=$(OBJDIR)/%.o) $(ACORE:%.s=$(OBJDIR)/%.o) $(CCORE:%.c=$(OBJDIR)/%.o)

all:	directories release $(TARGET) symbols

.PHONY: clean release debug color profile directories

release:
	$(eval CFLAGS += -Wf'--max-allocs-per-node 50000')
	@echo "RELEASE mode ON"
	
debug:
	$(eval CFLAGS += -Wf--debug -Wl-m -Wl-w -Wl-y)
	$(eval CFLAGS += -Wf--nolospre -Wf--nogcse)
	$(eval LFLAGS += -Wf--debug -Wl-m -Wl-w -Wl-y)
	@echo "DEBUG mode ON"

color:
	$(eval CFLAGS += -DCGB)
	$(eval LFLAGS += -Wm-yC)
	@echo "COLOR mode ON"

profile:
	$(eval CFLAGS += -Wf--profile)
	@echo "PROFILE mode ON"

.SECONDARY: $(OBJS) 

directories: $(ROM_BUILD_DIR) $(OBJDIR) $(REL_OBJDIR)

$(ROM_BUILD_DIR):
	mkdir -p $(ROM_BUILD_DIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(REL_OBJDIR):
	mkdir -p $(REL_OBJDIR)

$(OBJDIR)/%.o:	src/core/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	src/core/%.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	src/%.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(ROM_BUILD_DIR)/%.gb:	$(OBJS)
	mkdir -p $(ROM_BUILD_DIR)
	$(CC) $(LFLAGS) -o $@ $^

clean:
	@echo "CLEANUP..."
	rm -rf $(OBJDIR)
	rm -rf $(ROM_BUILD_DIR)

rom: $(TARGET)

symbols:
	python ./utils/noi2sym.py $(patsubst %.gb,%.noi,$(TARGET)) >$(patsubst %.gb,%.sym,$(TARGET))