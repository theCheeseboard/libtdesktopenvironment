Name:           libtdesktopenvironment-blueprint
Version:        beta5
Release:        2%{?dist}
Summary:        Common libraries for desktop integration for the- apps

License:        GPLv3+
URL:            https://github.com/vicr123/libtdesktopenvironment
Source0:        https://github.com/vicr123/libtdesktopenvironment/archive/%{version}.tar.gz
Conflicts:      libtdesktopenvironment
Provides:       libtdesktopenvironment

%if 0%{?fedora} == 32
BuildRequires:  make qt5-devel qt5-qtsvg-devel qt5-qtx11extras-devel libX11-devel libXScrnSaver-devel libXext-devel libXrandr-devel kf5-networkmanager-qt-devel pulseaudio-qt-devel the-libs-blueprint-devel xcb-util-keysyms-devel
Requires:       qt5 qt5-qtsvg qt5-qtx11extras libX11 libXScrnSaver libXext libXrandr kf5-networkmanager-qt pulseaudio-qt the-libs-blueprint
%endif

%if 0%{?fedora} >= 33
BuildRequires:  make qt5-qtbase-devel qt5-qtsvg-devel qt5-qtx11extras-devel libX11-devel libXScrnSaver-devel libXext-devel libXrandr-devel kf5-networkmanager-qt-devel pulseaudio-qt-devel the-libs-blueprint-devel xcb-util-keysyms-devel qt5-linguist
Requires:       qt5-qtbase qt5-qtsvg qt5-qtx11extras libX11 libXScrnSaver libXext libXrandr kf5-networkmanager-qt pulseaudio-qt the-libs-blueprint
%endif

%define debug_package %{nil}
%define _unpackaged_files_terminate_build 0

%description
Common libraries for desktop integration for the- apps

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}
Conflicts:      libtdesktopenvironment-devel
Provides:       libtdesktopenvironment-devel

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
%setup

%build
qmake-qt5
make

%install
rm -rf $RPM_BUILD_ROOT
#%make_install
make install INSTALL_ROOT=$RPM_BUILD_ROOT
cp -r $RPM_BUILD_ROOT/../lib64 $RPM_BUILD_ROOT/usr
rm -rf $RPM_BUILD_ROOT/../lib64
find $RPM_BUILD_ROOT -name '*.la' -exec rm -f {} ';'


%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig


%files
%{_libdir}/*.so.*

%files devel
%{_includedir}/*
%{_libdir}/*.so
%{_libdir}/qt5/mkspecs/modules/qt_tdesktopenvironment.pri


%changelog
* Sun May  3 2020 Victor Tran
- 
