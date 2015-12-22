# GNU Makefile to create CSBase library for linux

SOLUTION_DIR = .

SUB_DIRS = ChilliSource/Core ChilliSource/Input ChilliSource/Networking ChilliSource/Rendering ChilliSource/Social ChilliSource/UI ChilliSource/Video ChilliSource/Web CSBackend/Rendering/OpenGL
PLATFORM_SUB_DIRS = SFML Input Core

MKDIR = mkdir -p

PLATFORM = Linux
PLATFORM_SRC_DIR = ../../Source/CSBackend/Platform/$(PLATFORM)/
SRC_DIR = ../../Source/
CORE_LIB_DIR = ../../Libraries/Core/$(PLATFORM)
CRICKET_LIB_DIR = ../../Libraries/CricketAudio/$(PLATFORM)
OBJ_DIR = $(SOLUTION_DIR)/csobj/$(PLATFORM)/
OUT_DIR = $(SOLUTION_DIR)/csbin/$(PLATFORM)/
TARGET_ARCHITECTURE = linux_x86_64
TARGET_VERSION = release

LIBS_LINUX = CSBase z sfml-system-s sfml-window-s

#CPPFLAGS += -fpermissive
# add compiler use c++11 flags
CPPFLAGS += -std=c++11 -std=gnu++11
# add compiler debug flag for debug versions and debug defines
CPPFLAGS += $(if $(or $(call eq,$(TARGET_VERSION),debug),$(call eq,$(TARGET_VERSION),DEBUG)), -g -D_DEBUG -DDEBUG -DCS_ENABLE_DEBUG)
# add compiler flag for each architecture
CPPFLAGS += $(if $(call eq,$(TARGET_ARCHITECTURE),linux_x86_64), -m64 )
CPPFLAGS += $(if $(call eq,$(TARGET_ARCHITECTURE),linux_x86_32), -m32 )
# add needed include path
CPPFLAGS += -I $(SRC_DIR) -I $(CORE_LIB_DIR)/Headers
CPPFLAGS += -include string -include cstring
# add platform define
CPPFLAGS += -DCS_TARGETPLATFORM_$(call to_upper,$(PLATFORM))
# add dependency defines and library flags
CPPFLAGS += -DGLEW_STATIC -DSFML_STATIC
CPPFLAGS += -static
CPPFLAGS += -L$(CORE_LIB_DIR)/Libs/$(if $(call contains,x86_64,$(TARGET_ARCHITECTURE)),x86_64,x86_32) 
CPPFLAGS += $(foreach lib,$(LIBS_$(call to_upper,$(PLATFORM))), -l$(lib))
#CPPFLAGS += -L$(CRICKET_LIB_DIR)/Libs
#CPPFLAGS += -lck



# Helper Macros
eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))
contains = $(findstring $(1),$(2))
to_upper = $(shell echo $(1) | tr '[a-z]' '[A-Z]')

OUT_PREFIX_DIR = $(OUT_DIR)$(TARGET_ARCHITECTURE)/$(TARGET_VERSION)
OBJ_PREFIX_DIR = $(OBJ_DIR)$(TARGET_ARCHITECTURE)/$(TARGET_VERSION)/libChilliSource
define make_dir_rule

$$(OBJ_PREFIX_DIR)/$1.phony : $$(shell find $2$1/ -name *.[ch]) $$(shell find $2$1/ -name *.[ch]pp) 
	rm -rf $$(OBJ_PREFIX_DIR)/$1
	$$(MKDIR) $$(OBJ_PREFIX_DIR)/$1
	$$(if $$(shell find $2$1/ -name *.c)$$(shell find $2$1/ -name *.cpp),$$(COMPILE.cpp) $$(shell find $2$1/ -name *.c) $$(shell find $2$1/ -name *.cpp),echo "No .c or .cpp files for $2$1")
	mv *.o $$(OBJ_PREFIX_DIR)/$1 || true
	touch $$@
endef



#Target rules
all : $(OUT_PREFIX_DIR)/libChilliSource.a headers

clean :
	rm -r csobj

$(OUT_PREFIX_DIR)/libChilliSource.a : $(addprefix $(OBJ_PREFIX_DIR)/,$(addsuffix .phony,$(SUB_DIRS))) $(addprefix $(OBJ_PREFIX_DIR)/,$(addsuffix .phony,$(PLATFORM_SUB_DIRS))) $(OBJ_PREFIX_DIR)/Main.o
	$(MKDIR) $(OUT_PREFIX_DIR)
	$(AR) $(ARFLAGS) $@ $(shell find $(OBJ_PREFIX_DIR)/ -name *.o)

$(OBJ_PREFIX_DIR)/Main.o : $(PLATFORM_SRC_DIR)/Main.cpp
	$(COMPILE.cpp) $(PLATFORM_SRC_DIR)/Main.cpp
	mv *.o $(OBJ_PREFIX_DIR)

.PHONY : headers
headers :
	python ../../Tools/Scripts/extract_headers.py -i "$(SRC_DIR)/" -o "$(OUT_DIR)Headers"
	python ../../Tools/Scripts/extract_headers.py -i "$(PLATFORM_SRC_DIR)/" -o "$(OUT_DIR)PlatformHeaders"
	cp -rf $(OUT_DIR)PlatformHeaders/* $(OUT_DIR)Headers/ || true
	rm -rf $(OUT_DIR)PlatformHeaders

$(foreach dir,$(SUB_DIRS),       \
   $(eval $(call make_dir_rule,$(dir),$(SRC_DIR)))\
)
$(foreach dir,$(PLATFORM_SUB_DIRS),       \
   $(eval $(call make_dir_rule,$(dir),$(PLATFORM_SRC_DIR)))\
)



