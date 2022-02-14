Name:           libtdesktopenvironment
Version:        {{{ git_calculate_version }}}
Release:        1%{?dist}
Summary:        Common libraries for desktop integration for the- apps

License:        GPLv3+
URL:            https://github.com/vicr123/libtdesktopenvironment
Source:         {{{ git_archive_force path=$GIT_ROOT root_dir_name=libtdesktopenvironment }}}
VCS:            {{{ git_dir_vcs }}}

%if 0%{?fedora} >= 33
BuildRequires:  make git qt5-qtbase-devel qt5-qtsvg-devel qt5-qtx11extras-devel libX11-devel libXScrnSaver-devel libXext-devel libXrandr-devel kf5-networkmanager-qt-devel pulseaudio-qt-devel the-libs-devel xcb-util-keysyms-devel qt5-linguist wayland-devel qt5-qtwayland-devel qt5-qtbase-static qt5-qtbase-private-devel
Requires:       qt5-qtbase qt5-qtsvg qt5-qtx11extras libX11 libXScrnSaver libXext libXrandr kf5-networkmanager-qt pulseaudio-qt the-libs qt5-qtwayland
%endif

%define debug_package %{nil}
%define _unpackaged_files_terminate_build 0

%description
Common libraries for desktop integration for the- apps

%package        devel
Summary:        Development files for %{name}
Requires:       %{name}%{?_isa} = %{version}-%{release}

%description    devel
The %{name}-devel package contains libraries and header files for
developing applications that use %{name}.


%prep
{{{ git_dir_setup_macro }}}
git submodule init
git submodule update

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
