Name:		xdpd
Version:	0.4.0
Release:	rc2%{?dist}
Summary:	extensible data path daemon

Group:		System Environment/Daemons
License:        Mozilla Public License Version 2.0, http://www.mozilla.org/MPL/2.0/
URL:		http://codebasin.net/xdpd
Source0:	xdpd-%{version}.tar.gz
Epoch:		0

BuildRequires:	rofl-devel libconfig-devel qpid-qmf-devel boost-devel
Buildroot: 	%{_tmppath}/%{name}-%{version}-root 
Requires:	rofl libconfig qpid-qmf boost

%description
extensible data path daemon version 0.4.0 (rc2)


%prep
%setup 


%build
sh autogen.sh
cd build/
env PKG_CONFIG_PATH=/usr/local/lib/pkgconfig ../configure --prefix=/usr/local --disable-silent-rules --with-plugins="config"
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
cd build/
make install DESTDIR=$RPM_BUILD_ROOT


%post
%postun


%define _unpackaged_files_terminate_build 0 

%files
%defattr(-,root,root,-)
%doc
/usr/local/sbin/xdpd
#/usr/local/sbin/xmpclient


%clean
rm -rf $RPM_BUILD_ROOT 

%changelog
* Sat Apr 26 2014 Andreas Koepsel <andreas.koepsel@bisdn.de>
- build package for xdpd v0.4.0 (rc2)

