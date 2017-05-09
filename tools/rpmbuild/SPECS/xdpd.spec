Name:		xdpd
Version:	0.7.8
Release:	1%{?dist}
Summary:	eXtensible DataPath daemon

Group:		System Environment/Daemons
License:        Mozilla Public License Version 2.0, http://www.mozilla.org/MPL/2.0/
URL:		http://github.com/bisdn/xdpd
Source0:	xdpd-%{version}.tar.gz
Epoch:		0

%bcond_with dpdk
%bcond_with intel_fpga

BuildRequires: autoconf
BuildRequires: automake
BuildRequires: bash
BuildRequires: binutils
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
Requires: libconfig
Requires: pkgconfig
Requires: glog
Requires: gflags
Requires: openssl-libs

# add AM_PROG_CC_C_O macro to rofl-datapath/configure.ac on centos
Patch1: xdpd-0.7.7-fix-rofl-datapath-configure-ac.patch

# enable hardware platform for Intel-FPGA
Patch2: xdpd-0.7.7-add-hw-platform-intel-fpga.patch

Buildroot: 	%{_tmppath}/%{name}-%{version}-root 

%global _missing_build_ids_terminate_build 0

%description
eXtensible DataPath daemon version %{version}


%prep

%setup 

git submodule update --init --recursive

# Apply m4 patch for centos
%patch1 -p1

# when building with Intel-FPGA support
%if %{with intel_fpga}

# Add Intel-FPGA platform to config/hw.m4
%patch2 -p1

# clone the Intel-FPGA driver
echo "Please make sure you have ssh-key based access to repository git@gitlab.bisdn.de:vBRAS/intel_fpga.git"
pushd src/xdpd/drivers
git clone git@gitlab.bisdn.de:vBRAS/intel_fpga.git
popd

%endif

export AUTOMAKE="automake --foreign -a"
autoreconf -f -i



%build
cd build/

%define configure_flags --disable-silent-rules --enable-experimental --with-pipeline-platform-funcs-inlined --with-pipeline-lockless --with-plugins="config rest" --prefix=/usr --sysconfdir=/etc

%if %{with intel_fpga}
  ../configure %{configure_flags} --with-hw-support=intel_fpga
%else
  %if %{with dpdk}
    ../configure %{configure_flags} --with-hw-support=gnu-linux-dpdk
  %else
    ../configure %{configure_flags} --with-hw-support=gnu-linux
  %endif
%endif
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
* Tue May 09 2017 Andreas Koepsel <andreas.koepsel@bisdn.de>
- build package for xdpd v0.7.8 (with Intel-FPGA driver)
* Thu May 04 2017 Andreas Koepsel <andreas.koepsel@bisdn.de>
- build package for xdpd v0.7.7 (with rofl-common v0.11)
* Sat Apr 26 2014 Andreas Koepsel <andreas.koepsel@bisdn.de>
- build package for xdpd v0.4.0 (rc2)

