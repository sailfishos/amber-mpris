moc_files = qt5.compile_moc(
  headers: ['dbusextendedabstractinterface.h'],
  dependencies: qt5_dep,
  extra_args: '-DQT_DBUS_EXTENDED_LIBRARY'
)

dbusextended = static_library('dbusextended-qt5',
  'dbusextendedabstractinterface.cpp',
  moc_files,
  dependencies: qt5_dep,
  cpp_args: '-DQT_DBUS_EXTENDED_LIBRARY',
)
