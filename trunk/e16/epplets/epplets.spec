Summary: Enlightenment Epplets
Name: epplets
Version: 0.10
Release: 0.01%{?_rpm_add_date:.%(date '+%y%m%d')}%{?_vendorsuffix:.%{_vendorsuffix}}
License: BSD
Group: User Interface/Desktops
URL: http://www.enlightenment.org/
Source: http://prdownloads.sourceforge.net/enlightenment/%{name}-0.10-0.01.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root
BuildRequires: imlib2-devel
#BuildSuggests: freeglut-devel xorg-x11-devel
#Requires: enlightenment >= 0.16.0
Provides: enlightenment-epplets = %{version}
Provides: e16-epplets = %{version}
Obsoletes: enlightenment-epplets

%description
Epplets are small, handy Enlightenment applets, similar to "dockapps"
or "applets" for other packages.  The epplets package contains the
base epplet API library and header files, as well as the core set of
epplets, including CPU monitors, clocks, a mail checker, mixers, a
slideshow, a URL grabber, a panel-like toolbar, and more.

%prep
%setup -n %{name}-0.10-0.01

%build
CFLAGS="$RPM_OPT_FLAGS"
export CFLAGS

%{configure} --prefix=%{_prefix} --bindir=%{_bindir} --datadir=%{_datadir} %{?acflags}
%{__make} %{?_smp_mflags} %{?mflags}

%install
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT
%{__make} install DESTDIR=$RPM_BUILD_ROOT %{?mflags_install}

%ifos linux
%post -p /sbin/ldconfig
%endif

%ifos linux
%postun -p /sbin/ldconfig
%endif

%clean
test "x$RPM_BUILD_ROOT" != "x/" && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc ChangeLog
%{_includedir}/*
%{_libdir}/*
%{_bindir}/*
%{_datadir}/e16/epplet_icons/*
%{_datadir}/e16/epplet_data/*

%changelog
