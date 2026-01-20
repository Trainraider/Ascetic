SHELL ?= /bin/bash

#app info
VERSION    ?= 0.0.1
TARGET     ?= template_app
NAME       ?= Template App
#APP_ID can start with a website or email in reverse url format
APP_ID     ?= com.email.name.$(TARGET)
#APP_PREFIX is APP_ID converted to a path.
APP_PREFIX ?= $(shell echo $(APP_ID) | sed 's:\.:/:g;s:^:/:g')
COPYRIGHT  ?= Copyright (C) 2026
AUTHOR     ?= [Your name here]
COMMENT    ?= GTK4 template Application
CATEGORIES ?= Utility;ComputerScience;GNOME;GTK;

# Customize below to fit your system

# Install paths
PREFIX ?= /usr/local

#Project directory
PD = $(shell pwd)

#Build/Source paths
SRC     ?= $(PD)/source
DBG     ?= $(PD)/debug
RLS     ?= $(PD)/build
DATA    ?= $(SRC)/data
BLP     ?= $(SRC)/blueprints

#Files
RESOURCES    ?= $(DATA)/icon.svg

#Dependencies
PKG_CONFIG ?= pkg-config

INCS = `$(PKG_CONFIG) --cflags gtk4 libadwaita-1` \

LIBS = `$(PKG_CONFIG) --libs gtk4 libadwaita-1` \

#Optional flags
CFLAGS         ?= -march=native -pipe
RELEASE_CFLAGS  = -O2 -g -flto
RELEASE_LDFLAGS = -flto
DEBUG_CPPFLAGS  = -DDEBUG
DEBUG_CFLAGS    = -O0 -ggdb -Wpedantic -Wall -Wextra -fsanitize=undefined,address -fstack-protector-strong
DEBUG_LDFLAGS   = -fsanitize=undefined,address

