Summary: Enlightened Thumbnail Generator
Name: epsilon
Version: 0.0.2
Release: 1
Copyright: BSD
Group: User Interface/X
Source: ftp://ftp.enlightenment.org/enlightenment/%{name}-%{version}.tar.gz
Packager: Michael Jennings <mej@eterm.org>
Vendor: The Enlightenment Development Team <e-develop@enlightenment.org>
Requires: epeg >= 0.9.0
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description

Epsilon is a small, display independent, and quick thumbnailing
library.  The lib itself conforms to the standard put forth by
freedesktop.org You can find out more information about it at
http://triq.net/~jens/thumbnail-spec/index.html

Epeg offers very noticeable speed increases to this standard, but it
is only available if the input image is a jpeg file.  If the file is
anything other than jpg, the traditional freedesktop.org thumbnailing
will occur.  To show the speed increase epeg offers, Epsilon can be
built with and without epeg.

%package devel
Summary: Epsilon headers and development libraries.
Group: Development/Libraries
Requires: %{name} = %{version}

%description devel
Epsilon thumbnailer development headers and libraries.

%prep
%setup -q

%build
%{configure} --prefix=%{_prefix}
%{__make} %{?_smp_mflags} %{?mflags}

%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install
test -x `which doxygen` && sh gendoc || :

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc AUTHORS COPYING* README*
%{_libdir}/libepsilon.so.*
%{_libdir}/libepsilon.la
%{_bindir}/epsilon

%files devel
%defattr(-, root, root)
%doc doc/html
%{_libdir}/libepsilon.so
%{_libdir}/libepsilon.a
%{_includedir}/Epsilon.h
%{_bindir}/epsilon-config

%changelog
