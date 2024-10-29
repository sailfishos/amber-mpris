Name:       amber-mpris-qt6

Summary:    Qt and QML MPRIS interface and adaptor
Version:    1.1.0
Release:    1
License:    LGPLv2+
URL:        https://github.com/sailfishos/amber-mpris
Source0:    %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt6Core)
BuildRequires:  pkgconfig(Qt6DBus)
BuildRequires:  pkgconfig(Qt6Qml)

%description
%{summary}.

%package devel
Summary:    Development files for %{name}
Requires:   %{name} = %{version}-%{release}
Conflicts:  amber-mpris-devel

%description devel
Development files for %{name}.

%package -n amber-qml-plugin-mpris-qt6
Summary:    QML plugin for %{name}
Requires:   %{name} = %{version}-%{release}

%description -n amber-qml-plugin-mpris-qt6
QML plugin for %{name}.

%prep
%setup -q -n %{name}-%{version}

%build

export PATH=$PATH:%{_qt6_bindir}
%qmake_qt6 VERSION=`echo %{version} | sed 's/+.*//'`
%make_build

%install
%make_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%license COPYING
%{_libdir}/lib*6.so.*

%files devel
%{_includedir}/AmberMpris/Mpris
%{_includedir}/AmberMpris/MprisClient
%{_includedir}/AmberMpris/MprisController
%{_includedir}/AmberMpris/MprisPlayer
%{_includedir}/AmberMpris/MprisMetaData
%{_includedir}/AmberMpris/ambermpris.h
%{_includedir}/AmberMpris/mpris.h
%{_includedir}/AmberMpris/mprisclient.h
%{_includedir}/AmberMpris/mpriscontroller.h
%{_includedir}/AmberMpris/mprisplayer.h
%{_includedir}/AmberMpris/mprismetadata.h
%{_libdir}/lib*6.so
%{_libdir}/pkgconfig/*.pc

%files -n amber-qml-plugin-mpris-qt6
%{_libdir}/qt6/qml/Amber/Mpris/libambermprisplugin.so
%{_libdir}/qt6/qml/Amber/Mpris/plugins.qmltypes
%{_libdir}/qt6/qml/Amber/Mpris/qmldir
