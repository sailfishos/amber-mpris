Name:       amber-mpris

Summary:    Qt and QML MPRIS interface and adaptor
Version:    1.1.0
Release:    1
License:    LGPLv2+
URL:        https://github.com/sailfishos/amber-mpris
Source0:    %{name}-%{version}.tar.bz2
Requires(post): /sbin/ldconfig
Requires(postun): /sbin/ldconfig
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  sailfish-qdoc-template

%description
%{summary}.

%package devel
Summary:    Development files for %{name}
Requires:   %{name} = %{version}-%{release}

%description devel
Development files for %{name}.

%package -n amber-qml-plugin-mpris
Summary:    QML plugin for %{name}
Requires:   %{name} = %{version}-%{release}

%package -n amber-qml-plugin-mpris-doc
Summary:    QML plugin for %{name} documentation
BuildArch: noarch

%description -n amber-qml-plugin-mpris
QML plugin for %{name}.

%description -n amber-qml-plugin-mpris-doc
QML plugin for %{name} documentation.

%prep
%setup -q -n %{name}-%{version}

%build

%qmake5 VERSION=`echo %{version} | sed 's/+.*//'`

make %{?_smp_mflags}

%install
rm -rf %{buildroot}

%qmake5_install

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root,-)
%license COPYING
%{_libdir}/lib*.so.*

%files devel
%defattr(-,root,root,-)
%{_datarootdir}/qt5/mkspecs/features/ambermpris.prf
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
%{_libdir}/lib*.so
%{_libdir}/pkgconfig/*.pc

%files -n amber-qml-plugin-mpris
%defattr(-,root,root,-)
%{_libdir}/qt5/qml/Amber/Mpris/libambermprisplugin.so
%{_libdir}/qt5/qml/Amber/Mpris/plugins.qmltypes
%{_libdir}/qt5/qml/Amber/Mpris/qmldir

%files -n amber-qml-plugin-mpris-doc
%defattr(-,root,root,-)
%{_datadir}/doc/amber-mpris/ambermpris.qch
