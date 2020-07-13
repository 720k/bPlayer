TEMPLATE = subdirs

SUBDIRS += \
    frontend \

unix {
SUBDIRS += \
    backend \
    tools/socket-bridge \
}
DISTFILES += \
    text/ToDo.txt \
    text/KnownIssues.txt \
    text/Notes.txt \
    text/mpv--input-cmdlist.txt \
    text/mpv--list-options.txt \
    text/mpv--list-properties.txt \
    text/mpv--list-protocols.txt

HEADERS += \
    common/BPlayer.h
