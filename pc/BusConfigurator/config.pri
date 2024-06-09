################################################################
# BUS configurator
################################################################

VER_MAJOR = 0
VER_MINOR = 0
VER_PATCH = 1
# build is always 0

COMPANY_WEBSITE = www.christian-marty.ch
COMPANY_NAME = Christian Marty
PRODUCT_NAME = BUS Configuration Tool

GCTUI_DESCRIPTION = Configuration tool for bus
GCTLOGIC_DESCRIPTION = Configuration tool for bus

COPYRIGHT = 2024 Christian Marty

win32:VERSION = $${VER_MAJOR}.$${VER_MINOR}.$${VER_PATCH}.0 # major.minor.patch.build
else:VERSION = $${VER_MAJOR}.$${VER_MINOR}.$${VER_PATCH}    # major.minor.patch
