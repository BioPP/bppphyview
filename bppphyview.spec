%define _prefix /usr

URL: https://github.com/BioPP/bppphyview

Name: bppphyview
Version: 0.6.0
Release: 1%{?dist}
License: CECILL-2.0
Vendor: The Bio++ Project
Source: %{name}-%{version}.tar.gz
Summary: Bio++ Phylogenetic Viewer
Group: Productivity/Scientific/Other

Requires: libbpp-phyl12 = 2.4.0
Requires: libbpp-core4 = 2.4.0
Requires: libbpp-qt2 = 2.4.0

%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
Requires: libqt5core5 >= 5.0.0
Requires: libqt5gui5 >= 5.0.0
Requires: libqt5widgets5 >= 5.0.0
%endif
%if 0%{?suse_version}
Requires: libQt5Core5 >= 5.0.0
Requires: libQt5Gui5 >= 5.0.0
Requires: libQt5Widgets5 >= 5.0.0
%endif
%if 0%{?mageia} || 0%{?mdkversion}
%ifarch x86_64
Requires: lib64proxy-webkit >= 0.4.14
Requires: lib64qt5core5 >= 5.0.0
Requires: lib64qt5gui5 >= 5.0.0
Requires: lib64qt5widgets5 >= 5.0.0
Requires: qt5-qtdeclarative >= 5.0.0
Requires: qt5-qtbase >= 5.0.0
%else
Requires: libproxy-webkit >= 0.4.14
Requires: libqt5core5 >= 5.0.0
Requires: libqt5gui5 >= 5.0.0
Requires: libqt5widgets5 >= 5.0.0
Requires: qt5-qtdeclarative >= 5.0.0
Requires: qt5-qtbase >= 5.0.0
%endif
%endif

BuildRoot: %{_builddir}/%{name}-root
BuildRequires: cmake >= 2.8.11
BuildRequires: gcc-c++ >= 4.7.0
BuildRequires: groff
BuildRequires: libbpp-core4 = 2.4.0
BuildRequires: libbpp-core-devel = 2.4.0
BuildRequires: libbpp-phyl12 = 2.4.0
BuildRequires: libbpp-phyl-devel = 2.4.0
BuildRequires: libbpp-qt2 = 2.4.0
BuildRequires: libbpp-qt-devel = 2.4.0

%if 0%{?fedora} || 0%{?rhel_version} || 0%{?centos_version} || 0%{?scientificlinux_version}
BuildRequires: qt >= 5.0.0
BuildRequires: qt-devel >= 5.0.0
%endif
%if 0%{?suse_version}
BuildRequires: libQt5Core5 >= 5.0.0
BuildRequires: libQt5Gui5 >= 5.0.0
BuildRequires: libQt5Widgets5 >= 5.0.0
BuildRequires: libqt5-qtdeclarative-devel >= 5.0.0
BuildRequires: libqt5-qtbase-devel >= 5.0.0
%endif
%if 0%{?mageia} || 0%{?mdkversion}
%ifarch x86_64
BuildRequires: lib64proxy-webkit >= 0.4.14
BuildRequires: lib64qt5core5 >= 5.0.0
BuildRequires: lib64qt5gui5 >= 5.0.0
BuildRequires: lib64qt5widgets5 >= 5.0.0
BuildRequires: lib64qt5base5-devel >= 5.0.0
%else
BuildRequires: libproxy-webkit >= 0.4.14
BuildRequires: libqt5core5 >= 5.0.0
BuildRequires: libqt5gui5 >= 5.0.0
BuildRequires: libqt5widgets5 >= 5.0.0
BuildRequires: libqt5base5-devel >= 5.0.0
%endif
%endif

AutoReq: yes
AutoProv: yes

%if 0%{?mandriva_version}
%if %{mandriva_version} >= 2011
BuildRequires: xz
%define compress_program xz
%else
BuildRequires: lzma
%define compress_program lzma
%endif
%else
%if 0%{?mageia}
BuildRequires: xz
%define compress_program xz
%else
#For all other distributions:
BuildRequires: gzip
%define compress_program gzip
%endif
%endif

%description
Bio++ Phylogenetic Viewer, using the Qt library.

%prep
%setup -q

%build
CFLAGS="$RPM_OPT_FLAGS"
CMAKE_FLAGS="-DCMAKE_INSTALL_PREFIX=%{_prefix} -DCOMPRESS_PROGRAM=%{compress_program}"
cmake $CMAKE_FLAGS .
make

%install
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc AUTHORS.txt COPYING.txt INSTALL.txt ChangeLog
%{_prefix}/bin/phyview
%{_prefix}/share/man/man1/phyview.1*

%changelog
* Mon Mar 12 2018 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.6.0-1
- Compatibility update with Bio++ 2.4.0.
- More options in branch lengths panel.
* Thu Jun 8 2017 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.5.1-1
- Compatibility update with Bio++ 2.3.1.
* Wed May 10 2017 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.5.0-1
- Compatibility update with Bio++ 2.3.0.
* Mon Sep 28 2014 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.4.0-1
- Several bug fixed.
- New clickable panel with list of trees in memory.
- New dialog to insert subtrees.
* Fri Mar 08 2013 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.3.0-1
- Compatibility update.
- New option for header line in names translation.
* Thu Feb 09 2012 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.2.1-1
- Compatibility update.
* Thu Jun 09 2011 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.2.0-1
* Mon Feb 28 2011 Julien Dutheil <julien.dutheil@univ-montp2.fr> 0.1.0-1

