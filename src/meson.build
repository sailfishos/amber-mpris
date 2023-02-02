mpris_headers = [ 
  'mprisclient.h',
  'mprisclient_p.h',
  'mpriscontroller.h',
  'mpris.h',
  'mprismetadata.h',
  'mprismetadata_p.h',
  'mprisplayeradaptor_p.h',
  'mprisplayer.h',
  'mprisplayer_p.h',
  'mprispropertiesadaptor_p.h',
  'mprisserviceadaptor_p.h',
]

moc_files = qt5.compile_moc(
  headers: mpris_headers,
  sources: [
    'mprisclient.cpp',
    'mpriscontroller.cpp',
  ],
  extra_args: '-DMPRIS_QT_LIBRARY',
  include_directories: '../qtdbusextended',
  dependencies: qt5_dep,
)

mpris_sources = [
  'mprisclient.cpp',
  'mpriscontroller.cpp',
  'mpris.cpp',
  'mprismetadata.cpp',
  'mprismetadataproxy.cpp',
  'mprisplayeradaptor.cpp',
  'mprisplayer.cpp',
  'mprisplayerinterface.cpp',
  'mprispropertiesadaptor.cpp',
  'mprisrootinterface.cpp',
  'mprisserviceadaptor.cpp',
]

ambermpris = library('ambermpris',
  sources: [mpris_sources, moc_files],
  dependencies: qt5_dep,
  cpp_args: '-DMPRIS_QT_LIBRARY',
  include_directories: '../qtdbusextended',
  link_with: dbusextended,
)

pkgconf = import('pkgconfig')
pkgconf.generate(
  libraries: ambermpris,
  name: 'ambermpris',
  requires: ['Qt5Core', 'Qt5DBus'],
  description: 'Mpris library',
)
