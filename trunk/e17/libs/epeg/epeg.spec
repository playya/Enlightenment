%define _missing_doc_files_terminate_build 0

Summary: JPEG Scaling Library
Name: epeg
Version: 0.9.0
Release: 1.%(date '+%Y%m%d')
License: BSD
Group: System Environment/Libraries
URL: http://www.enlightenment.org/
Source: ftp://ftp.enlightenment.org/pub/epeg/%{name}-%{version}.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
#BuildSuggests: xorg-x11-devel
BuildRequires: libjpeg-devel XFree86-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Epeg is a library which provides facilities for scaling JPEG images
very quickly.

%package devel
Summary: Epeg headers, static libraries, documentation and test programs
Group: System Environment/Libraries
Requires: %{name} = %{version}
Requires: libjpeg-devel XFree86-devel

%description devel
Headers, static libraries, test programs and documentation for Eet

%prep
%setup -q

%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}

%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install
test -x `which doxygen` && sh gendoc || :

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README
%{_libdir}/libepeg.so*
%{_libdir}/libepeg.la

%files devel
%defattr(-, root, root)
%doc doc/html
%{_libdir}/libepeg.a
%{_bindir}/epeg*
%{_includedir}/Epeg*

%changelog
