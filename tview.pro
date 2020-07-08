TEMPLATE = app

QT       += core gui xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#TARGET = TView
#DESTDIR = $$PWD

#message($$TARGET)          # 生成目标的名字
#message($$DESTDIR)         # 目标输出文件路径
#message($$PWD)             # 当前文件(.pro或.pri)所在的路径
#message($$OUT_PWD)         # Makefile生成的路径
#message($$_PRO_FILE_)      # pro项目文件(带路径)
#message($$_PRO_FILE_PWD_)  # pro项目文件所在的路径

#message($$QMAKE_HOST.arch) # 计算机架构
#message($$QMAKE_HOST.os)   # 计算机系统
#message($$QMAKE_HOST.cpu_count)        # 计算机CPU核心数
#message($$QMAKE_HOST.name)             # 计算机名
#message($$QMAKE_HOST.version)          # 系统版本(数字形式)
#message($$QMAKE_HOST.version_string)   # 系统版本(字符串形式)

QMAKE_PRE_LINK += echo 'Start Build...'  # 编译链接前自动执行命令
QMAKE_POST_LINK += echo 'Build Success.' # 编译链接后自动执行命令

win32: {
    message("windows...")
    contains(QMAKE_HOST.arch, x86):{
        INCLUDEPATH +=  $$PWD/ffmpeg/windows/include
        LIBS += $$PWD/ffmpeg/windows/lib/avcodec.lib \
                $$PWD/ffmpeg/windows/lib/avfilter.lib \
                $$PWD/ffmpeg/windows/lib/avformat.lib \
                $$PWD/ffmpeg/windows/lib/swscale.lib \
                $$PWD/ffmpeg/windows/lib/avutil.lib \
                $$PWD/ffmpeg/windows/lib/avdevice.lib
    }else {

    }
    RC_FILE=main.rc

    BUILD_PATH = build_$$QT_VERSION

    CONFIG(debug, debug|release) {
        DESTDIR = $$PWD/$$BUILD_PATH/debug
        OBJECTS_DIR = $$PWD/$$BUILD_PATH/debug/.obj
        MOC_DIR = $$PWD/$$BUILD_PATH/debug/.moc
        RCC_DIR = $$PWD/$$BUILD_PATH/debug/.rcc
        UI_DIR = $$PWD/$$BUILD_PATH/debug/.ui
    } else {
        DESTDIR = $$PWD/$$BUILD_PATH/release
        OBJECTS_DIR = $$PWD/$$BUILD_PATH/release/.obj
        MOC_DIR = $$PWD/$$BUILD_PATH/release/.moc
        RCC_DIR = $$PWD/$$BUILD_PATH/release/.rcc
        UI_DIR = $$PWD/$$BUILD_PATH/release/.ui
    }
}

android: {
    message("android...")
    INCLUDEPATH += $$PWD/ffmpeg/android/x86/include

    LIBS += -L$$PWD/ffmpeg/android/x86/lib/ -lavcodec \
            -L$$PWD/ffmpeg/android/x86/lib/ -lavfilter \
            -L$$PWD/ffmpeg/android/x86/lib/ -lavformat \
            -L$$PWD/ffmpeg/android/x86/lib/ -lswscale \
            -L$$PWD/ffmpeg/android/x86/lib/ -lavutil \
            -L$$PWD/ffmpeg/android/x86/lib/ -lavdevice \
}

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    cffmpeg.cpp \
    view.cpp \
    worker.cpp

HEADERS += \
    mainwindow.h \
    cffmpeg.h \
    view.h \
    worker.h

FORMS += \
    mainwindow.ui \
    view.ui

RESOURCES += \
    res.qrc

include(treeview/treeview.pri)

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    android/AndroidManifest.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew \
    android/gradlew.bat \
    android/res/values/libs.xml \
    readme.md

contains(ANDROID_TARGET_ARCH,x86) {
    ANDROID_EXTRA_LIBS = \
    $$PWD/ffmpeg/android/x86/lib/libavcodec.so \
    $$PWD/ffmpeg/android/x86/lib/libavdevice.so \
    $$PWD/ffmpeg/android/x86/lib/libavfilter.so \
    $$PWD/ffmpeg/android/x86/lib/libavformat.so \
    $$PWD/ffmpeg/android/x86/lib/libavutil.so \
    $$PWD/ffmpeg/android/x86/lib/libswresample.so \
    $$PWD/ffmpeg/android/x86/lib/libswscale.so

    ANDROID_PACKAGE_SOURCE_DIR = \
        $$PWD/android
}
