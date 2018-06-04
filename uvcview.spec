Name: uvcview
Version: 20070907
Release: alt1.0

%define LANG                    ru

Summary: A tray icon to easily launch the nvidia-settings control panel

Group: Video

License: GPL2

Url: http://freshmeat.net/projects/uvcview/

Source: %url/%name-%version.tar.bz2
Source1: uncview.po.tar.bz2
Source2: uvcviev.png

Patch: uvcview-20070607-timestap.patch
Patch1: uvcview-20070907-cam0.patch
Patch2: uvcview-20070907-stop.patch

BuildPreReq: libgtk+2-devel glibc-devel linux-libc-headers

# Automatically added by buildreq on Fri Aug 31 2007
BuildRequires: libgtk+2-devel glibc-devel linux-libc-headers

%description
UVCView
 UVCView is a simple USB Video Camera viewer. 
 This program is very simple, because it is part of another software
%prep
%setup -a1
%patch -p1
%patch1 -p1
%patch2 -p1

%build
%__autoreconf
export FLAGS="%optflags -DNDEBUG -DNO_DEBUG -D_GNU_SOURCE " \
%configure
%make_build
pushd po
make ru.mo
popd
make distdir



%install
%makeinstall
%find_lang %name

install -d -m 755 %buildroot%_desktopdir

cat > %buildroot%_desktopdir/uvcview.desktop << EOF
[Desktop Entry]
Name=uvcview
GenericName[ru]=просмотр камеры uvcview 
Comment=UVCView is a simple USB Video Camera viewer
Icon=uvcview.png
Categories=AudioVideo;Video;TV;
TryExec=/usr/bin/uvcview
Exec=/usr/bin/uvcview
Terminal=true
Type=Application
EOF


install -d -m 755 %buildroot%_datadir/locale/%LANG/LC_MESSAGES

install -m 644 po/ru.mo %buildroot%_datadir/locale/%LANG/LC_MESSAGES/uvcview.mo

install -D -m 644 %SOURCE2 %buildroot%_iconsdir/uvcview.png

			  
%post

%preun

%files -f %name.lang
%_bindir/%name
%doc ChangeLog AUTHORS
%_datadir/locale/%LANG/LC_MESSAGES/uvcview.mo
%_desktopdir/uvcview.desktop
%_iconsdir/uvcview.png

%changelog
* Mon Oct 29 2007 Hihin Ruslan <ruslandh@altlinux.ru> 20070907-alt1.0
- new version

* Fri Aug 31 2007 Hihin Ruslan <ruslandh@altlinux.ru> 20070607-alt1.0
- First build for ALT Linux.
