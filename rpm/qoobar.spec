%define frominstall %{_builddir}/%{name}-%{version}

%if 0%{?fedora} || 0%{?centos_version}
%define QMAKE qmake-qt4 "LIBS+=-L/lib/ -lz" qoobar.pro
%define GROUP Applications/Multimedia
%else
%if 0%{?mdkversion}
%define QMAKE qmake
%define GROUP Sound
%else
%define QMAKE qmake
%define GROUP Productivity/Multimedia/Sound/Utilities
%endif
%endif

Name: qoobar
Version: 1.6.8
Release: 1
Summary: A simple tagger for tagging classical music
Group: %{GROUP}
License: GPLv3+
URL: http://qoobar.sourceforge.net
Source0: http://sourceforge.net/projects/qoobar/files/%{name}-%{version}/%{name}-%{version}.tar.gz
%if 0%{?fedora} || 0%{?rhel_version}
BuildRequires: qt-devel >= 4.5, desktop-file-utils, gcc-c++, zlib-devel
BuildRequires: glib-devel, gstreamer-devel, gstreamer-plugins-base-devel
%endif
%if 0%{?centos_version}
%if 0%{?centos_version} <= 600
BuildRequires: qt4-devel >= 4.5
%else
BuildRequires: qt-devel >= 4.5
%endif
BuildRequires: desktop-file-utils, gcc-c++, zlib-devel
BuildRequires: glib2-devel, gstreamer-devel, gstreamer-plugins-base-devel
%endif
%if 0%{?suse_version}
BuildRequires: libqt4-devel >= 4.5, update-desktop-files, zlib-devel
BuildRequires: glib2-devel, gstreamer-0_10-devel, gstreamer-0_10-plugins-base-devel
%endif
%if 0%{?mdkversion}
BuildRequires: libqt4-devel >= 4.5, desktop-file-utils, gcc-c++, zlib1-devel
BuildRequires: libglib2.0-devel, gstreamer-devel, gstreamer-plugins-base-devel
%endif


%if 0%{?suse_version}
Requires: libdiscid1
%else
Requires: libdiscid, shntool
%endif
Requires: flac, enca

%description
Qoobar is a tagger for tagging classical music.

It understands the following tags: Composer, Album, Title, Artist, Performer,
Conductor, Subtitle, Orchestra, Key, Genre, Year, Track number,
Total tracks, & Comment.

Qoobar can also show and edit user tags.

Qoobar can edit mp3, ogg, flac, speex, musepack,
wavpack, wma, mp4, ape, opus and true audio files.

Other features: filling in tags from file names, pattern-based file renaming,
cover art support, importing tags from the Internet, undo / redo system.

%package doc
Summary: Html documentation for qoobar
%if 0%{?fedora}
Group: Documentation
%else
# suse and all other distros
Group: Documentation/HTML
%endif
BuildArch: noarch
%description doc
Qoobar is a tagger for tagging classical music.

It understands the following tags: Composer, Album, Title, Artist, Performer,
Conductor, Subtitle, Orchestra, Key, Genre, Year, Track number,
Total tracks, & Comment.

Qoobar can also show and edit user tags.

Qoobar can edit mp3, ogg, oga, flac, speex, musepack,
wavpack, wma, mp4, ape, opus and true audio files.

Other features: filling in tags from file names, pattern-based file renaming,
cover art support, importing tags from the Internet, undo / redo system.

%prep
%setup -q

%build
%{QMAKE}
make %{?_smp_mflags}
strip %{frominstall}/release/qoobar

%install
make INSTALL_ROOT=${RPM_BUILD_ROOT} install
%if 0%{?suse_version}
%suse_update_desktop_file -r -G %{name} %{name} Qt AudioVideo Audio AudioVideoEditing
%else
%if 0%{?mdkversion}
%update_desktop_database
%else
desktop-file-validate %{buildroot}%{_datadir}/applications/%{name}.desktop
%endif
%endif
install -Dpm 644 qoobar.1 %{buildroot}%{_mandir}/man1/qoobar.1

%clean
rm -rf %{buildroot}

%files
%defattr(-,root,root)
%{_bindir}/%{name}
%docdir %{_datadir}/doc/%{name}
%{_datadir}/doc/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%dir %{_datadir}/icons/hicolor
%dir %{_datadir}/icons/hicolor/16x16
%dir %{_datadir}/icons/hicolor/32x32
%dir %{_datadir}/icons/hicolor/64x64
%dir %{_datadir}/icons/hicolor/48x48
%dir %{_datadir}/icons/hicolor/128x128
%dir %{_datadir}/icons/hicolor/256x256
%dir %{_datadir}/icons/hicolor/16x16/apps
%dir %{_datadir}/icons/hicolor/32x32/apps
%dir %{_datadir}/icons/hicolor/64x64/apps
%dir %{_datadir}/icons/hicolor/48x48/apps
%dir %{_datadir}/icons/hicolor/128x128/apps
%dir %{_datadir}/icons/hicolor/256x256/apps
%{_datadir}/icons/hicolor/16x16/apps/qoobar.png
%{_datadir}/icons/hicolor/32x32/apps/qoobar.png
%{_datadir}/icons/hicolor/64x64/apps/qoobar.png
%{_datadir}/icons/hicolor/48x48/apps/qoobar.png
%{_datadir}/icons/hicolor/128x128/apps/qoobar.png
%{_datadir}/icons/hicolor/256x256/apps/qoobar.png
%{_prefix}/lib/%{name}
%doc %{_mandir}/man1/qoobar.1*


%files doc
%defattr(-,root,root)
%docdir %{_datadir}/doc/%{name}-doc
%{_datadir}/doc/%{name}-doc

