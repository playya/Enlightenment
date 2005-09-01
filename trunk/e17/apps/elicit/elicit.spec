Summary: EFL-based image zoom and colorpicker
Name: elicit
Version: 0.9.0
Release: 1.%(date '+%Y%m%d')
License: BSD
Group: User Interface/Desktops
URL: http://www.enlightenment.org/
Source: ftp://ftp.enlightenment.org/pub/enlightenment/%{name}-%{version}.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
BuildRequires: esmart-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
Elicit is a tool for examining images on your desktop, providing both
a global color picker and a zoom tool. Graphic artists and designers
can quickly examine graphics without needed to rely on larger tools
such as GIMP for simple examinations and color checks. Elicit is
especially useful for theme development and spot checking.

%prep
%setup -q

%build
%{configure}
%{__make} %{?_smp_mflags} %{?mflags}

%install
%{__make} %{?mflags_install} DESTDIR=$RPM_BUILD_ROOT install

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-, root, root)
%doc AUTHORS COPYING README
%{_bindir}/*
%{_datadir}/%{name}

%changelog
