# GNU Makefile to create the ChilliSource static library for linux

SOLUTION_DIR = .

SUB_DIRS = ChilliSource/Audio ChilliSource/Core ChilliSource/Input ChilliSource/Networking ChilliSource/Rendering ChilliSource/Social ChilliSource/UI ChilliSource/Video ChilliSource/Web CSBackend/Rendering/OpenGL
PLATFORM_SUB_DIRS = SFML Input Core Networking

MKDIR = mkdir -p

PLATFORM = Linux
PLATFORM_SRC_DIR = ../../Source/CSBackend/Platform/$(PLATFORM)/
SRC_DIR = ../../Source/
CORE_LIB_DIR = ../../Libraries/Core/$(PLATFORM)
CRICKET_LIB_DIR = ../../Libraries/CricketAudio/$(PLATFORM)
OBJ_DIR = $(SOLUTION_DIR)/csobj/$(PLATFORM)/
OUT_DIR = $(SOLUTION_DIR)/csbin/$(PLATFORM)/
TARGET_ARCHITECTURE = x86_64
TARGET_VERSION = release

LIBS_LINUX = CSBase z sfml-system-s sfml-window-s crypto ssl curl

#CPPFLAGS += -fpermissive
# add compiler use c++11 flags
CPPFLAGS += -std=c++11 -std=gnu++11
# add compiler debug flag for debug versions and debug defines
CPPFLAGS += $(if $(or $(call eq,$(TARGET_VERSION),debug),$(call eq,$(TARGET_VERSION),DEBUG)), -g -D_DEBUG -DDEBUG -DCS_ENABLE_DEBUG)
# add compiler flag for each architecture
CPPFLAGS += $(if $(call eq,$(TARGET_ARCHITECTURE),x86_64), -m64 )
CPPFLAGS += $(if $(call eq,$(TARGET_ARCHITECTURE),x86_32), -m32 )
# add needed include path
CPPFLAGS += -I $(SRC_DIR) -I $(CORE_LIB_DIR)/Headers -I $(CRICKET_LIB_DIR)/Headers 
CPPFLAGS += -include string -include cstring
# add platform define
CPPFLAGS += -DCS_TARGETPLATFORM_$(call to_upper,$(PLATFORM))
# add dependency defines and library flags 
# NOTE:Always using release version of libraries even in debug build
CPPFLAGS += -DGLEW_STATIC -DSFML_STATIC
CPPFLAGS += -static
CPPFLAGS += -L$(CORE_LIB_DIR)/Libs/$(TARGET_ARCHITECTURE)/release 
CPPFLAGS += $(foreach lib,$(LIBS_$(call to_upper,$(PLATFORM))), -l$(lib))
CPPFLAGS += -L$(CRICKET_LIB_DIR)/Libs/$(TARGET_ARCHITECTURE)/release 
CPPFLAGS += -lck

DEPENDENCIES = Main.d
SOURCES_SRC_DIR += $(foreach dir,$(SUB_DIRS), $(patsubst $(SRC_DIR)%,%,$(shell find $(SRC_DIR)$(dir)/ -name *.c) $(shell find $(SRC_DIR)$(dir)/ -name *.cpp) ) )   
SOURCES_PLATFORM_SRC_DIR += $(foreach dir,$(PLATFORM_SUB_DIRS), $(patsubst $(PLATFORM_SRC_DIR)%,%,$(shell find $(PLATFORM_SRC_DIR)$(dir)/ -name *.c) $(shell find $(PLATFORM_SRC_DIR)$(dir)/ -name *.cpp) ))
SOURCES_PLATFORM_SRC_DIR += Main.cpp
OBJECTS_SRC_DIR = $(call source_to_object, $(SOURCES_SRC_DIR))
OBJECTS_PLATFORM_SRC_DIR = $(call source_to_object, $(SOURCES_PLATFORM_SRC_DIR))
DEPENDENCIES = Main.d
DEPENDENCIES += $(call object_to_depend, $(OBJECTS_SRC_DIR))
DEPENDENCIES += $(call object_to_depend, $(OBJECTS_PLATFORM_SRC_DIR))

# Helper Macros
eq = $(and $(findstring $(1),$(2)),$(findstring $(2),$(1)))
contains = $(findstring $(1),$(2))
to_upper = $(shell echo $(1) | tr '[a-z]' '[A-Z]')
# $(call source_to_object, source_file_list) 
source_to_object = $(subst .c,.o,$(filter %.c,$1)) \
                   $(subst .cpp,.o,$(filter %.cpp,$1))
# $(call source_to_depend, source_file_list) 
source_to_depend = $(subst .c,.d,$(filter %.c,$1)) \
                   $(subst .cpp,.d,$(filter %.cpp,$1))
# $(call object_to_depend, object_file_list) 
object_to_depend = $(subst .o,.d,$(filter %.o,$1)) 
# $(call make-depend,source-file,object-file,depend-file)
define make-depend
 $(COMPILE.cpp) -MM -MF $3 -MP -MT $2 $1
endef
# $(call make_object_rule,source-file,object-file,depend-file)
define make_object_rule

$2 : $1
	$$(MKDIR) $$(dir $(2))
	$$(call make-depend, $1, $2, $3)
	$$(COMPILE.cpp) $1 -o $$@
endef

OUT_PREFIX_DIR = $(OUT_DIR)$(TARGET_ARCHITECTURE)/$(TARGET_VERSION)
OBJ_PREFIX_DIR = $(OBJ_DIR)$(TARGET_ARCHITECTURE)/$(TARGET_VERSION)/libChilliSource

#Target rules
all : $(OUT_PREFIX_DIR)/libChilliSource.a headers

# include generated object dependencies
ifneq "$(MAKECMDGOALS)" "clean"
 $(foreach dependency,$(DEPENDENCIES),       \
    $(eval -include $(OBJ_PREFIX_DIR)/$(dependency))\
 ) 
endif

# cleanning need *.o for when errors interupt build midway
clean :
	rm -r csobj || true

$(OUT_PREFIX_DIR)/libChilliSource.a : $(addprefix $(OBJ_PREFIX_DIR)/, $(OBJECTS_SRC_DIR)) $(addprefix $(OBJ_PREFIX_DIR)/, $(OBJECTS_PLATFORM_SRC_DIR))
	$(MKDIR) $(OUT_PREFIX_DIR)
	$(AR) $(ARFLAGS) $@ $(shell find $(OBJ_PREFIX_DIR)/ -name *.o)

.PHONY : headers
headers :
	python ../../Tools/Scripts/extract_headers.py -i "$(SRC_DIR)/" -o "$(OUT_DIR)Headers"
	python ../../Tools/Scripts/extract_headers.py -i "$(PLATFORM_SRC_DIR)/" -o "$(OUT_DIR)PlatformHeaders"
	cp -rf $(OUT_DIR)PlatformHeaders/* $(OUT_DIR)Headers/ || true
	rm -rf $(OUT_DIR)PlatformHeaders

$(foreach source, $(SOURCES_SRC_DIR), \
   $(eval $(call make_object_rule, \
   $(addprefix $(SRC_DIR), $(source)), \
   $(addprefix $(OBJ_PREFIX_DIR)/, $(call source_to_object, $(source))), \
   $(addprefix $(OBJ_PREFIX_DIR)/, $(call source_to_depend, $(source))), \
   )) \
)

$(foreach source, $(SOURCES_PLATFORM_SRC_DIR), \
   $(eval $(call make_object_rule, \
   $(addprefix $(PLATFORM_SRC_DIR), $(source)), \
   $(addprefix $(OBJ_PREFIX_DIR)/, $(call source_to_object, $(source))), \
   $(addprefix $(OBJ_PREFIX_DIR)/, $(call source_to_depend, $(source))), \
   )) \
)
