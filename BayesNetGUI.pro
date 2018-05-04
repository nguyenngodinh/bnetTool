#-------------------------------------------------
#
# Project created by QtCreator 2015-07-02T14:40:14
#
#-------------------------------------------------

QT       += \
        core gui \
        printsupport
#        qtprintsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = BayesNetGUI
TEMPLATE = app
QMAKE_CXXFLAGS += -std=c++11

SOURCES += \
    mainwindow.cpp \
    qcustomplot.cpp \
    cptmodel.cpp \
    bayesnet.cpp \
    dialogquickpredict.cpp \
    main.cpp \
    learnbayes.cpp \
    utility.cpp \
    qbayesedge.cpp \
    qbayesnode.cpp \
    bngraphicsscene.cpp \
    convertbayesdata.cpp \
    tsvmodel.cpp \
    thresholdmodel.cpp

HEADERS  += \
    qcustomplot.h \
    bayesnet.h \
    dialogquickpredict.h \
    mainwindow.h \
    learnbayes.h \
    cptmodel.h \
    utility.h \
    qbayesedge.h \
    qbayesnode.h \
    bngraphicsscene.h \
    convertbayesdata.h \
    tsvmodel.h \
    thresholdmodel.h

FORMS    += \
    mainwindow.ui \
    dialogquickpredict.ui \
    convertbayesdata.ui

