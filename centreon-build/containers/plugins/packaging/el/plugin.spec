Name:           @NAME@
Version:        @VERSION@
Release:        @RELEASE@%{?dist}
Summary:        Centreon plugin
Group:          Development/Libraries
License:        ASL 2.0
URL:            https://www.centreon.com/
BuildArch:      noarch
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
AutoReqProv:    no

Requires: @REQUIRES@

@CUSTOM_PKG_DATA@

%description

%prep

%build

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/usr/lib/centreon/plugins/
%{__install} -m 775 @PATH@/plugin/@PLUGIN_NAME@.rpm.current %{buildroot}@PLUGIN_PATH@/@PLUGIN_NAME@

%clean
rm -rf \%{buildroot}

%files
%defattr(-,root,root,-)
@PLUGIN_PATH@