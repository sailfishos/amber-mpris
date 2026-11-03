equals(QT_MAJOR_VERSION, 5) {
MPRISQTLIB = ambermpris
}
equals(QT_MAJOR_VERSION, 6) {
MPRISQTLIB = ambermpris6
}

!isEmpty(PREFIX):isEmpty(LIBDIR): LIBDIR = $${PREFIX}/lib
