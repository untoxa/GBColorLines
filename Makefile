GBDK = ../../gbdk
GBDKLIB = $(GBDK)/lib/small/asxxxx
CC = $(GBDK)/bin/lcc

CART_SIZE = 4

ROM_BUILD_DIR = build
OBJDIR = obj
CFLAGS = -Isrc/include -Wa-Isrc/include -Wa-I$(GBDKLIB)

PROJECT_NAME = colorlines

LFLAGS_NBANKS = -Wl-yt0x1B -Wl-yo$(CART_SIZE) -Wl-ya1 -Wl-j

LFLAGS = $(LFLAGS_NBANKS) -Wl-j -Wm-yS -Wl-klib -Wl-lhUGEDriver.lib -Wm-yc -Wm-yn"$(PROJECT_NAME)"

TARGET = $(ROM_BUILD_DIR)/$(PROJECT_NAME).gb

ASRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.s))) 
CSRC = $(foreach dir,src,$(notdir $(wildcard $(dir)/*.c))) 

OBJS = $(CSRC:%.c=$(OBJDIR)/%.o) $(ASRC:%.s=$(OBJDIR)/%.o)

#all:	directories release $(TARGET)
all:	directories $(TARGET)

.PHONY: clean release debug color profile directories rom online

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

directories: $(ROM_BUILD_DIR) $(OBJDIR)

$(ROM_BUILD_DIR):
	mkdir -p $(ROM_BUILD_DIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o:	src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o:	src/%.s
	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET):	$(OBJS)
	mkdir -p $(ROM_BUILD_DIR)
	$(CC) $(LFLAGS) -o $@ $^

clean:
	@echo "CLEANUP..."
	rm -rf $(OBJDIR)
	rm -rf $(ROM_BUILD_DIR)

rom: $(TARGET)

online: directories rom
	@echo "PACKING for ITCH.IO"
	rm -f $(PROJECT_NAME).zip
	cp -f online/js-emulator.zip ./$(PROJECT_NAME).zip	
	cp -f $(TARGET) ./	
	7z a $(PROJECT_NAME).zip $(PROJECT_NAME).gb
	rm -f $(PROJECT_NAME).gb
