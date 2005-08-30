Summary: Tools for the  Enlightenment window manager
Name: e_utils
Version: 0.0.1
Release: 1.%(date '+%Y%m%d')
License: BSD
Group: User Interface/Desktops
URL: http://www.enlightenment.org/
Source: ftp://ftp.enlightenment.org/pub/enlightenment/%{name}-%{version}.tar.gz
Packager: %{?_packager:%{_packager}}%{!?_packager:Michael Jennings <mej@eterm.org>}
Vendor: %{?_vendorinfo:%{_vendorinfo}}%{!?_vendorinfo:The Enlightenment Project (http://www.enlightenment.org/)}
Distribution: %{?_distribution:%{_distribution}}%{!?_distribution:%{_vendor}}
Prefix: %{_prefix}
BuildRoot: %{_tmppath}/%{name}-%{version}-root
BuildRequires: ecore-devel, evas-devel, esmart-devel, edje-devel, eet-devel
BuildRequires: ewl-devel, imlib2-devel, embryo-devel, emotion-devel, enscribe-devel
BuildRequires: enlightenment-devel >= 0.16.999
Requires: enlightenment >= 0.16.999
Requires: e_utils-e17setroot, e_utils-e_util_eapp_edit, e_utils-emblem
Requires: e_utils-entangle, e_utils-exige

%description
Virtual package to install all utilities for the Enlightenment window
manager.  Also includes documentation.

%package devel
Summary: Development headers for Enlightenment utilities.
Group: User Interface/Desktops
Requires: %{name} = %{version}

%description devel
Development headers for Enlightenment utilities.

%package e17setroot
Summary: Background setting utility for Enlightenment.
Group: User Interface/Desktops
Requires: enlightenment >= 0.16.999

%description e17setroot
Background setting utility for Enlightenment.

%package e_util_eapp_edit
Summary: eapp editing utility for Enlightenment.
Group: User Interface/Desktops
Requires: enlightenment >= 0.16.999
Requires: e_utils-docs

%description e_util_eapp_edit
eapp editing utility for Enlightenment.

%package emblem
Summary:  Graphical background selection utility for Enlightenment.
Group: User Interface/Desktops
Requires: enlightenment >= 0.16.999
Requires: e_utils-docs

%description emblem
Graphical background selection utility for Enlightenment.

%package entangle
Summary:  Graphical menu editing utility for Enlightenment.
Group: User Interface/Desktops
Requires: enlightenment >= 0.16.999
Requires: e_utils-docs

%description entangle
Graphical menu editing utility for Enlightenment.

%package exige
Summary:  A run dialog box for Enlightenment.
Group: User Interface/Desktops
Requires: enlightenment >= 0.16.999
Requires: e_utils-docs

%description exige
A run dialog box for Enlightenment.

%prep
%setup -q

%build
%{configure} --prefix=%{_prefix}
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
%doc AUTHORS COPYING COPYING-PLAIN README

%files devel
%defattr(-, root, root)
%{_includedir}/*

%files e17setroot
%defattr(-, root, root)
%{_bindir}/e17setroot

%files e_util_eapp_edit
%defattr(-, root, root)
%{_bindir}/e_util_eapp_edit
%{_datadir}/%{name}/data/e_utils_eapp_edit/*

%files emblem
%defattr(-, root, root)
%{_bindir}/emblem
%{_datadir}/%{name}/data/emblem/*
%{_datadir}/enlightenment/config-apps/emblem.eap

%files entangle
%defattr(-, root, root)
%{_bindir}/entangle
%{_datadir}/%{name}/data/entangle/*
%{_datadir}/enlightenment/config-apps/entangle.eap

%files exige
%defattr(-, root, root)
%{_bindir}/exige
%{_datadir}/%{name}/data/exige/*

%changelog
