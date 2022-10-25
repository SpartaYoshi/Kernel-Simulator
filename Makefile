TARGET		:=	$(notdir kernel-sim)
BUILD		:=	build
SRCDIR		:=	source
INCLDIR	    :=	include

export INCLUDE	:=	$(foreach dir,$(INCLDIR),-I$(CURDIR)/$(dir)) # -I$(CURDIR)/$(BUILD)
export SOURCE   :=  $(foreach dir,$(SRCDIR),$(wildcard $(dir)/*))

CFLAGS	:=	-g -Wall -O2 $(INCLUDE)

$(BUILD) : $(SOURCE)
	[ -d $@ ] || mkdir -p $@
	gcc $(CFLAGS) -c $^ -o $(patsubst %.c,$(BUILD)/%.o,$(notdir $(SOURCE)))


kernel:
	gcc $(CFLAGS) $(wildcard $(BUILD)/*) -o $(TARGET)

clean:
	rm -rf $(BUILD) $(TARGET)