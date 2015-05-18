# GNU Makefile to create CSBase library for linux

SUB_DIRS = aes base64 json md5 minizip png rapidxml SHA1

MKDIR = mkdir -p

PLATFORM_SRC_DIR = PlatformSource/Linux
SRC_DIR = Source/
OBJ_DIR = obj/
OUT_DIR = bin/
TARGET_ARCHITECTURE = linux_x86_64
TARGET_VERSION = release

#CPPFLAGS += -fpermissive
# add compiler debug flag for debug versions
CPPFLAGS += $(if $(or $(call eq,$(TARGET_VERSION),debug),$(call eq,$(TARGET_VERSION),DEBUG)), -g )
# add compiler flag for each architecture
CPPFLAGS += $(if $(call eq,$(TARGET_ARCHITECTURE),linux_x86_64), -m64 )
CPPFLAGS += $(if $(call eq,$(TARGET_ARCHITECTURE),linux_x86_32), -m32 )
# add needed include path
CPPFLAGS += -I $(PLATFORM_SRC_DIR) -I $(SRC_DIR)
CPPFLAGS += -include string 

eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))

OUT_PREFIX_DIR = $(OUT_DIR)$(TARGET_ARCHITECTURE)/$(TARGET_VERSION)
OBJ_PREFIX_DIR = $(OBJ_DIR)$(TARGET_ARCHITECTURE)/$(TARGET_VERSION)/libCSBase
define make_dir_rule

$$(OBJ_PREFIX_DIR)/$1.phony : $$(shell find $$(SRC_DIR)$1/ -name *.[ch]) $$(shell find $$(SRC_DIR)$1/ -name *.[ch]pp) 
	rm -rf $$(OBJ_PREFIX_DIR)/$1
	$$(MKDIR) $$(OBJ_PREFIX_DIR)/$1
	$$(if $$(shell find $$(SRC_DIR)$1/ -name *.c)$$(shell find $$(SRC_DIR)$1/ -name *.cpp),$$(COMPILE.cpp) $$(shell find $$(SRC_DIR)$1/ -name *.c) $$(shell find $$(SRC_DIR)$1/ -name *.cpp)   $$(if $$(call eq,$1,json), -include string -include $$(SRC_DIR)$1/json.h ) $$(if $$(call eq,$1,minizip), -include $$(PLATFORM_SRC_DIR)/zlib.h ) $$(if $$(call eq,$1,png), -include $$(SRC_DIR)$1/png.h ),echo "No .c or .cpp files for $1")
	mv *.o $$(OBJ_PREFIX_DIR)/$1 || true
	touch $$@
endef


#Target rules
all : $(OUT_PREFIX_DIR)/libCSBase.a headers

clean :
	rm -r obj

$(OUT_PREFIX_DIR)/libCSBase.a : $(addprefix $(OBJ_PREFIX_DIR)/,$(addsuffix .phony,$(SUB_DIRS)))
	$(MKDIR) $(OUT_PREFIX_DIR)
	$(AR) $(ARFLAGS) $@ $(shell find $(OBJ_PREFIX_DIR)/ -name *.o)

.PHONY : headers
headers :
	python ../../../Tools/Scripts/extract_headers.py -i "$(SRC_DIR)/" -o "$(OUT_DIR)Headers"
	python ../../../Tools/Scripts/extract_headers.py -i "$(PLATFORM_SRC_DIR)/" -o "$(OUT_DIR)PlatformHeaders"
	cp -r $(OUT_DIR)PlatformHeaders/* $(OUT_DIR)Headers/
	rm -rf $(OUT_DIR)PlatformHeaders

$(foreach dir,$(SUB_DIRS),       \
   $(eval $(call make_dir_rule,$(dir)))\
)



