TEMPLATE = subdirs
src.depends = qtdbusextended
SUBDIRS = src qtdbusextended doc

no-qml {
    message(Building without QML dependency.)
} else {
    message(Building with QML dependency.)
    declarative.depends = src
    SUBDIRS += declarative
}
