Name:		xdpd
Version:	0.7.7
Release:	1%{?dist}
Summary:	eXtensible DataPath daemon

Group:		System Environment/Daemons
License:        Mozilla Public License Version 2.0, http://www.mozilla.org/MPL/2.0/
URL:		http://github.com/bisdn/xdpd
Source0:	xdpd-%{version}.tar.gz
Epoch:		0

BuildRequires: autoconf
BuildRequires: automake
BuildRequires: bash
BuildRequires: binutils
BuildRequires: boost-devel
BuildRequires: boost-system
BuildRequires: coreutils
BuildRequires: cpio
BuildRequires: diffutils
BuildRequires: dwz
BuildRequires: elfutils
BuildRequires: file
BuildRequires: findutils
BuildRequires: gawk
BuildRequires: gcc
BuildRequires: gcc-c++
BuildRequires: gdb
BuildRequires: git-core
BuildRequires: glibc
BuildRequires: glibc
BuildRequires: glibc-common
BuildRequires: glibc-devel
BuildRequires: glibc-headers
BuildRequires: grep
BuildRequires: guile
BuildRequires: gzip
BuildRequires: hostname
BuildRequires: kernel-headers
BuildRequires: libgcc
BuildRequires: libstdc++
BuildRequires: libstdc++-devel
BuildRequires: libtool
BuildRequires: m4
BuildRequires: make
BuildRequires: openssl-devel
BuildRequires: libconfig-devel
BuildRequires: glog-devel
BuildRequires: gflags-devel
Requires: boost
Requires: libconfig
Requires: pkgconfig
Requires: glog
Requires: gflags
Requires: openssl-libs


Buildroot: 	%{_tmppath}/%{name}-%{version}-root 

%global _missing_build_ids_terminate_build 0

%description
eXtensible DataPath daemon version %{version}


%prep
%autosetup 
sh autogen.sh



%build
cd build/
../configure --disable-silent-rules --enable-experimental --with-hw-support=gnu-linux-dpdk --with-pipeline-platform-funcs-inlined --with-pipeline-lockless --with-plugins="config rest" --prefix=/usr --sysconfdir=/etc
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
/usr/sbin/xdpd
/usr/bin/xcli
/etc/xdpd.conf



%clean
rm -rf $RPM_BUILD_ROOT 

%changelog
* Thu May 04 2017 Andreas Koepsel <andreas.koepsel@bisdn.de>
- build package for xdpd v0.7.7 (with rofl-common v0.11)
* Sat Apr 26 2014 Andreas Koepsel <andreas.koepsel@bisdn.de>
- build package for xdpd v0.4.0 (rc2)

