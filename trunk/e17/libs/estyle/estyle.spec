# this is NOT relocatable, unless you alter the patch!
%define	name	estyle
%define	ver	0.0.1
%define	rel	1
%define prefix  /usr

Summary: Enlightened Text Style Library
Name: %{name}
Version: %{ver}
Release: %{rel}
Copyright: BSD
Group: User Interface/X
URL: http://www.enlightenment.org/efm.html
Packager: Term <kempler@utdallas.edu>
Vendor: The Enlightenment Development Team <e-develop@enlightenment.org>
Source: ftp://ftp.enlightenment.org/enlightenment/%{name}-%{ver}.tar.gz
BuildRoot: /var/tmp/%{name}-root
Requires: evas >= 0.0.1

#Patch1: evas_test-fix.patch

%description
Evas is an advanced canvas library, providing three backends for
rendering: X11 (without some features like alpha-blending), imlib2, or
OpenGL (hardware accelerated). Due to its simple API, evas can be
developed with rapidly, and cleanly.

Install evas if you want to develop applications against the only
hardware-accelerated canvas library, or if you want to try out the
applications under development.

%package devel
Summary: Enlightened Canvas Library headers and development libraries.
Group: Development/Libraries
Requires: %{name} = %{ver}

%description devel
Evas development headers and libraries.

%prep
%setup -q

%build
./configure --prefix=%{prefix}
make

%install
make prefix=$RPM_BUILD_ROOT%{prefix} install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{prefix}/lib/libevas.so.*
%{prefix}/bin/evas_*
%{prefix}/share/evas/*

%files devel
%defattr(-,root,root)
%{prefix}/lib/libevas.so
%{prefix}/lib/libevas.*a
%{prefix}/include/Evas.h
%{prefix}/bin/evas-config
