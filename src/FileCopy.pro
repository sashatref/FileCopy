QT       += core

greaterThan(QT_MAJOR_VERSION, 5) {
    QT += core5compat
}

TARGET = FileCopy
TEMPLATE = app
PRECOMPILED_HEADER = project_pch.h

CONFIG += precompile_header console

SOURCES += \
    main.cpp

HEADERS  +=

