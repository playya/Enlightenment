%define _missing_doc_files_terminate_build 0

Summary: Evas "smart objects"
Name: esmart
Version: 0.9.0
Release: 1.%(date '+%Y%m%d')
License: BSD
Group: User Interface/X
Source: ftp://ftp.enlightenment.org/enlightenment/%{name}-%{version}.tar.gz
URL: http://www.enlightenment.org/pages/efl.html
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
Requires: evas >= 1.0.0 imlib2 libjpeg ecore epsilon 
Requires: edje freetype2 eet edb embryo
#BuildSuggests: xorg-x11-devel 
BuildRequires: evas-devel imlib2-devel libjpeg-devel XFree86-devel
BuildRequires: ecore-devel epsilon-devel edje-devel embryo-devel
BuildRequires: freetype2-devel eet-devel edb-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Esmart contains "smart" pre-built evas objects.  It currently includes
a thumbnail generator and a horizontal/vertical container.

%package devel
Summary: Eves "smart objects" headers and development libraries.
Group: Development/Libraries
Requires: %{name} = %{version}
Requires: evas-devel imlib2-devel libjpeg-devel XFree86-devel
Requires: ecore-devel epsilon-devel edje-devel embryo-devel
Requires: freetype2-devel eet-devel edb-devel

%description devel
Evas "smart objects" development headers and libraries.

%prep
%setup -q

%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}

%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install
#test -x `which doxygen` && sh gendoc || :

%post
/sbin/ldconfig || :

%postun
/sbin/ldconfig || :

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING* README
%{_libdir}/libesmart_*.so.*
%{_libdir}/libesmart_*.la
%{_libdir}/esmart/layout/*.so
%{_libdir}/esmart/layout/*.la
%{_bindir}/esmart_file_dialog_test
%{_bindir}/esmart_test
%{_datadir}/esmart/esmart.png

%files devel
%defattr(-, root, root)
#%doc doc/html
%{_libdir}/libesmart_*.so
%{_libdir}/libesmart_*.a
%{_libdir}/esmart/layout/*.a
%{_includedir}/Esmart/Esmart_*
%{_bindir}/esmart-config
%{_libdir}/pkgconfig/esmart.pc

%changelog
