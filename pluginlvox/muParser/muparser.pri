INCLUDEPATH += $$PWD/include

HEADERS += \
    $$PWD/include/muParser.h \
    $$PWD/include/muParserBase.h \
    $$PWD/include/muParserBytecode.h \
    $$PWD/include/muParserCallback.h \
    $$PWD/include/muParserDef.h \
    $$PWD/include/muParserDLL.h \
    $$PWD/include/muParserError.h \
    $$PWD/include/muParserFixes.h \
    $$PWD/include/muParserInt.h \
    $$PWD/include/muParserStack.h \
    $$PWD/include/muParserTemplateMagic.h \
    $$PWD/include/muParserTest.h \
    $$PWD/include/muParserToken.h \
    $$PWD/include/muParserTokenReader.h

SOURCES += \
    $$PWD/src/muParser.cpp \
    $$PWD/src/muParserBase.cpp \
    $$PWD/src/muParserBytecode.cpp \
    $$PWD/src/muParserCallback.cpp \
    $$PWD/src/muParserDLL.cpp \
    $$PWD/src/muParserError.cpp \
    $$PWD/src/muParserInt.cpp \
    $$PWD/src/muParserTest.cpp \
    $$PWD/src/muParserTokenReader.cpp
