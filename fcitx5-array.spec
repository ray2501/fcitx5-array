#
# spec file for package fcitx5-array
#

Name:           fcitx5-array
Version:        0.6.0
Release:        0
Summary:        Array 30 input method engine for Fcitx5
License:        GPL-2.0-or-later
URL:            https://github.com/ray2501/fcitx5-ray2501
Source:         %{name}-%{version}.tar.gz
BuildRequires:  cmake
BuildRequires:  extra-cmake-modules
BuildRequires:  fcitx5-devel
BuildRequires:  fcitx5-chinese-addons-devel
BuildRequires:  gcc-c++
BuildRequires:  hicolor-icon-theme
BuildRequires:  sqlite3-devel
BuildRequires:  fmt-devel
Requires:       fcitx5
Provides:       fcitx-array = %{version}
Obsoletes:      fcitx-array < %{version}

%description
Array 30 input method engine for Fcitx5 project.

%prep
%setup -q

%build
%cmake ..
%make_build

%install
%cmake_install
%find_lang %{name}

%files -f %{name}.lang
%license LICENSES
%doc README.md
%{_fcitx5_libdir}/array.so
%{_fcitx5_addondir}/array.conf
%{_fcitx5_imconfdir}/array.conf
%{_fcitx5_imconfdir}/array.conf
%{_fcitx5_datadir}/array/array.db
%{_datadir}/icons/hicolor/*/apps/org.fcitx.Fcitx5.fcitx-ibusarray*
%{_datadir}/icons/hicolor/*/apps/fcitx-ibusarray*

%changelog

