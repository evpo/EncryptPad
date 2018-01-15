QT       += core gui widgets

CONFIG(debug, debug|release){
    CONFIG_NAME = debug
    SUBDIR_RELEASE =
}

CONFIG(release, debug|release){
    CONFIG_NAME = release
    SUBDIR_RELEASE = RELEASE=on
}

unix|macx: DEPS_SUBDIR = $$system(cd ../build && ./get_subdir.sh $$SUBDIR_RELEASE)

# For windows build sh.exe needs to be in the PATH environment variable
win32: DEPS_SUBDIR = $$system(cd ..\build && sh .\get_subdir.sh $$SUBDIR_RELEASE)
#message(DEPS_SUBDIR=$$DEPS_SUBDIR)

# install
target.path = /prj/EncryptPadLight
sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS EncryptPad.pro images
sources.path = /prj/EncryptPadLight
INSTALLS += target sources

CONFIG += c++11

QMAKE_CFLAGS += -fexceptions -o2

FORMS += \
    set_key_dialog.ui \
    file_properties_dialog.ui \
    new_key_dialog.ui \
    preferences_dialog.ui \
    find_and_replace.ui \
    get_passphrase_dialog.ui \
    confirm_passphrase_dialog.ui \
    get_passphrase_or_key_dialog.ui \
    file_encryption_dialog.ui \
    passphrase_generation_dialog.ui \
    find_dialog.ui

win32: LIBS += -luserenv
#unix: LIBS += ldl
LIBS += -L$$PWD/../deps/stlplus/portability/$$DEPS_SUBDIR \
     -L$$PWD/../back_end_src/$$DEPS_SUBDIR \
     -lback_end_src -lportability \
     -L$$PWD/../deps/libencryptmsg/src/$$DEPS_SUBDIR \
     -L$$PWD/../deps/libencryptmsg/state_machine/$$DEPS_SUBDIR \
     -L$$PWD/../deps/libencryptmsg/state_machine_client/$$DEPS_SUBDIR \
     -L$$PWD/../deps/libencryptmsg/deps/botan_1_openpgp_codec/$$DEPS_SUBDIR \
     -lsrc \
     -lstate_machine \
     -lstate_machine_client \
     -lbotan_1_openpgp_codec

win32: BOTANFILE = $$PWD/../deps/botan/botan.lib -fstack-protector
unix: BOTANFILE = -L$$PWD/../deps/botan -lbotan-2

!USE_SYSTEM_LIBS {
    LIBS += $$BOTANFILE $$PWD/../deps/zlib/libz.a -lm -ldl
} else {
    BOTANLIB = $$system(pkg-config --libs botan-2)
    LIBS += $$BOTANLIB -lz
}

!USE_SYSTEM_LIBS {
    INCLUDEPATH += $$PWD/../deps/botan/build/include
} else {
    BOTANCXX = $$system(pkg-config --cflags botan-2)
    QMAKE_CXXFLAGS += $$BOTANCXX
}
INCLUDEPATH += $$PWD/../deps/stlplus/portability
INCLUDEPATH += $$PWD/../back_end_src
INCLUDEPATH += $$PWD/../deps/libencryptmsg/include

SOURCES += \
    application.cpp \
    file_name_helper.cpp \
    main.cpp \
    mainwindow.cpp \
    set_key_dialog.cpp \
    set_passphrase_dialog.cpp \
    async_load.cpp \
    plain_text_switch.cpp \
    file_properties_dialog.cpp \
    new_key_dialog.cpp \
    preferences_dialog.cpp \
    recent_files_service.cpp \
    find_and_replace.cpp \
    get_passphrase_dialog.cpp \
    confirm_passphrase_dialog.cpp \
    get_passphrase_or_key_dialog.cpp \
    set_encryption_key.cpp \
    file_encryption_dialog.cpp \
    m_window_load_adapter.cpp \
    file_encryption_dlg_adapter.cpp \
    file_dlg_async.cpp \
    passphrase_generation_dialog.cpp \
    load_save_handler.cpp \
    file_request_service.cpp \
    plain_text_edit.cpp \
    find_dialog.cpp \
    preferences.cpp

HEADERS += \
    application.h \
    file_name_helper.h \
    mainwindow.h \
    set_key_dialog.h \
    set_passphrase_dialog.h \
    async_load.h \
    plain_text_switch.h \
    file_properties_dialog.h \
    new_key_dialog.h \
    preferences_dialog.h \
    recent_files_service.h \
    find_and_replace.h \
    get_passphrase_dialog.h \
    confirm_passphrase_dialog.h \
    get_passphrase_or_key_dialog.h \
    set_encryption_key.h \
    file_encryption_dialog.h \
    m_window_load_adapter.h \
    file_encryption_dlg_adapter.h \
    file_dlg_async.h \
    passphrase_generation_dialog.h \
    load_save_handler.h \
    file_request_service.h \
    plain_text_edit.h \
    common_definitions.h \
    find_dialog.h \
    preferences.h

QMAKE_RESOURCE_FLAGS += -no-compress
RESOURCES += \
    EncryptPad.qrc \
    culture.qrc

TRANSLATIONS = \
    encryptpad_en_gb.ts \
    encryptpad_ru_ru.ts \
    encryptpad_fr_fr.ts \
    encryptpad_zh_cn.ts

RC_FILE = encrypt_pad.rc
DESTDIR = ../../bin/$$CONFIG_NAME
OBJECTS_DIR = $$CONFIG_NAME/obj
unix|win32:TARGET = encryptpad
macx:TARGET = EncryptPad
