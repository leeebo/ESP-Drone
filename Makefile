# Project Makefile #

PROJECT_NAME := ESPlane2.0

# Path to MPU Driver
EXTRA_COMPONENT_DIRS += $(abspath ../..)
# Path to I2Cbus
#EXTRA_COMPONENT_DIRS += ${HOME}/esp/libraries/I2Cbus


include $(IDF_PATH)/make/project.mk
