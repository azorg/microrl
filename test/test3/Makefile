#============================================================================
OUT_NAME     := mrl_test3
#EXEC_EXT    := .exe
#OUT_DIR     := .
#CLEAN_DIR   := $(OUT_DIR)/tmp
#CLEAN_FILES := "$(OUT_DIR)/$(OUT_NAME).map" "$(OUT_DIR)/$(OUT_NAME).exe"
#----------------------------------------------------------------------------
#
# 1-st way to select source files
SRCS := mrl_test3.c ../../mrl.c
HDRS := ../../mrl.h ../../mrl_conf.h ../../mrl_defs.h

# 2-nd way to select source files
#SRC_DIRS := . ..
#HDR_DIRS := . ..

#----------------------------------------------------------------------------
#INC_DIRS  := ..
#INC_FLAGS := 
DEFS      := -DMRL_USE_HISTORY -DMRL_USE_COMPLETE #-DMRL_PRINT_ESC_OFF #-DMRL_DEBUG
OPTIM     := -Os -fomit-frame-pointer
WARN      := -Wall

CFLAGS    := $(WARN) $(OPTIM) $(DEFS) $(CFLAGS) -pipe
CXXFLAGS  := $(CXXFLAGS) $(CFLAGS)
LDFLAGS   := -lm $(LDFLAGS)

PREFIX    := /opt
#----------------------------------------------------------------------------
#_AS  := @as
#_CC  := @gcc
#_CXX := @g++
#_LD  := @g++
#----------------------------------------------------------------------------
include ../Makefile.skel
#============================================================================
#*** end of "Makefile" file ***#

