include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=openflowAP
PKG_VERSION:=1.0
PKG_RELEASE:=1

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/home/kkyoon/openwrt-iptime/openwrt/mypackages/sdnProject/openflowAP

include $(INCLUDE_DIR)/package.mk
TARGET_CC += -g
TARGET_CFLAGS += \
	-ggdb3 -I$(STAGING_DIR)/usr/include -I$(STAGING_DIR)/usr/include/libnl3 -I$(STAGING_DIR)/usr/include/lib -L$(STAGING_DIR)/usr/lib -lubox -lubus -ljson-c -lblobmsg_json -luci -lnl3

TARGET_LDFLAGS += \
	-g -Wl,-rpath-link=$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/usr/lib -lubox -lubus -ljson-c -lblobmsg_json -luci -lnl-3 -lnl-genl-3

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
# make menuconfig에 Examples 카테고리가 생성되고 안에 helloworld 있음
define Package/openflowAP
	SECTION:=sdnProject
	CATEGORY:=SdnProject
	TITLE:=Hello, World!@
        DEPENDS:= +libubus +libubox +ubusd +libuci +libjson-c +liblua +libnl
endef

# Package description; a more verbose description on what our package does
define Package/openflowAP/description
	A simple "Hello, world!" -application.
endef


# Package preparation instructions; create the build directory and copy the source code.
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/openflowAP.o -c $(PKG_BUILD_DIR)/openflowAP.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/test.o -c $(PKG_BUILD_DIR)/test.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/tools/process.o -c $(PKG_BUILD_DIR)/tools/process.c
	$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/tools/stringEx.o -c $(PKG_BUILD_DIR)/tools/stringEx.c
	$(TARGET_CC) $(TARGET_LDFLAGS) -o $(PKG_BUILD_DIR)/$1 $(PKG_BUILD_DIR)/openflowAP.o $(PKG_BUILD_DIR)/tools/stringEx.o $(PKG_BUILD_DIR)/tools/process.o $(PKG_BUILD_DIR)/test.o
endef

# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/openflowAP/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openflowAP $(1)/usr/bin
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,openflowAP))
