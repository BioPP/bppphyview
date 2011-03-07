%define name bppphyview
%define version 0.1.0
%define release 1
%define _prefix /usr

Summary: The Bio++ Phylogenetic Viewer.
Name: %{name}
Version: %{version}
Release: %{release}
Vendor: Julien Dutheil
Source: http://download.gna.org/bppsuite/%{name}-%{version}.tar.gz
License: CeCILL 2
Group: System Environment/Libraries
BuildRoot: %{_builddir}/%{name}-root
Packager: Julien Dutheil
Prefix: %{_prefix}
AutoReq: yes
AutoProv: yes

%description
Bio++ Phylogenetic Viewer, using the Qt library.

%prep
%setup -q

%build
CFLAGS="-I%{_prefix}/include $RPM_OPT_FLAGS"
CMAKE_FLAGS="-DCMAKE_INSTALL_PREFIX=%{_prefix}"
if [ %{_lib} == 'lib64' ] ; then
  CMAKE_FLAGS="$CMAKE_FLAGS -DLIB_SUFFIX=64"
fi
cmake $CMAKE_FLAGS .
make

%install
rm -rf $RPM_BUILD_ROOT
make DESTDIR=$RPM_BUILD_ROOT install

%clean
rm -rf $RPM_BUILD_ROOT

%post -p /sbin/ldconfig

%postun -p /sbin/ldconfig

%files
%defattr(-,root,root)
%doc AUTHORS.txt COPYING.txt INSTALL.txt ChangeLog
%{_prefix}/bin/phyview
%{_prefix}/share/man/man1/phyview.1.gz

%changelog
* Mon Feb 28 2011 Julien Dutheil <julien.dutheil@univ-montp2.fr>
- PhyView 0.1.0 release

