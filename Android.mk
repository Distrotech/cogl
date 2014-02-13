LOCAL_PATH := $(call my-dir)

###########################
#
# Cogl shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_CFLAGS := -O0

LOCAL_MODULE := Cogl

include $(LOCAL_PATH)/cogl/Makefile.sources

LOCAL_C_INCLUDES := \
  $(LOCAL_PATH) \
  $(LOCAL_PATH)/deps/eglib/android \
  $(LOCAL_PATH)/deps/eglib/src \
  $(LOCAL_PATH)/cogl \
  $(LOCAL_PATH)/cogl/android \
  $(LOCAL_PATH)/cogl/android/cogl \
  $(LOCAL_PATH)/cogl/winsys \
  $(LOCAL_PATH)/cogl/driver/gl \
  $(LOCAL_PATH)/cogl/driver/gl/gles

LOCAL_EXPORT_C_INCLUDES := \
  $(LOCAL_PATH)/deps/eglib/android \
  $(LOCAL_PATH)/deps/eglib/src \
  $(LOCAL_PATH) \
  $(LOCAL_PATH)/cogl/android

LOCAL_SRC_FILES := \
  $(subst $(LOCAL_PATH)/,, \
  	$(filter-out %-win32.c, \
  		$(wildcard $(LOCAL_PATH)/deps/eglib/src/*.c) \
	 ) \
	$(wildcard $(LOCAL_PATH)/test-fixtures/*.c) \
   ) \
  $(addprefix cogl/, \
  	$(filter %.c,$(cogl_sources_c)) \
  	$(filter %.c,$(cogl_driver_gles_sources)) \
  	$(filter %.c,$(cogl_sdl2_sources_c)) \
   )

LOCAL_CFLAGS += -DHAVE_CONFIG_H -DCOGL_COMPILATION -DCOGL_GLES2_LIBNAME=\"libGLESv2.so\" -DG_LOG_DOMAIN=\"Cogl\" -DCOGL_ENABLE_DEBUG -DCOGL_OBJECT_DEBUG -DCOGL_GL_DEBUG
LOCAL_LDLIBS := -llog

LOCAL_STATIC_LIBRARIES := SDL2

LOCAL_EXPORT_LDLIBS := -Wl,--undefined=Java_org_libsdl_app_SDLActivity_nativeInit -ldl -lGLESv1_CM -lGLESv2 -llog -landroid

include $(BUILD_SHARED_LIBRARY)

