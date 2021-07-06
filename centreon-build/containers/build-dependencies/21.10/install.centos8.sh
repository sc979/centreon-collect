#!/bin/sh

set -e
set -x

cp /tmp/ca-centreon-internal.pem /etc/pki/ca-trust/source/anchors/
update-ca-trust

# Clean packages
dnf clean all

# Base dnf configuration.
echo 'http_caching=none' >> /etc/yum.conf
echo 'assumeyes=1' >> /etc/yum.conf
sed -i 's/best=True/best=False/g' /etc/dnf/dnf.conf
dnf install dnf-plugins-core
dnf config-manager --set-enabled 'PowerTools'

# Install development repository.
curl -o centreon-release.rpm "http://srvi-repo.int.centreon.com/yum/standard/21.10/el8/stable/noarch/RPMS/centreon-release-21.10-1.el8.noarch.rpm"
dnf install --nogpgcheck centreon-release.rpm
sed -i -e 's#yum.centreon.com#srvi-repo.int.centreon.com/yum#g' /etc/yum.repos.d/centreon.repo
dnf config-manager --set-enabled 'centreon-testing*'

# Switch AppStream to install php73
dnf module enable php:7.3 -y

# Install required build dependencies for all Centreon projects.
xargs dnf install < /tmp/build-dependencies.txt
dnf update libarchive

# Install Node.js and related elements.
curl --silent --location https://rpm.nodesource.com/setup_16.x | bash -
dnf install --nogpgcheck -y nodejs
npm install -g redoc-cli

# Install Composer.
dnf install php php-cli php-dom php-json php-mbstring php-intl php-pdo
curl -sS https://getcomposer.org/installer | php
mv composer.phar /usr/local/bin/composer
chmod +x /usr/local/bin/composer

# Install Conan, a C++ package manager.
pip3 install --prefix=/usr conan

mkdir /tmp/conan-pkgs
cat <<EOF >/tmp/conan-pkgs/conanfile.txt
[requires]
gtest/cci.20210126
asio/1.18.1
fmt/7.1.3
spdlog/1.8.5
nlohmann_json/3.9.1
openssl/1.1.1k
grpc/1.37.0
mariadb-connector-c/3.1.12
zlib/1.2.11

[generators]
cmake_paths
cmake_find_package
EOF

conan install /tmp/conan-pkgs -s compiler.libcxx=libstdc++11 --build=missing
rm -rf /tmp/conan-pkgs

# Enable unstable repositories.
dnf config-manager --set-enabled 'centreon-unstable*'
