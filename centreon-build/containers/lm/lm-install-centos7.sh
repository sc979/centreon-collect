#!/bin/sh

set -e
set -x

systemctl mysql start
systemctl httpd start
cd /usr/share/centreon/www/modules/centreon-license-manager/frontend/app
npm install
npm install -g gulp
gulp
/tmp/install-centreon-module.php -c /etc/centreon/centreon.conf.php -m centreon-license-manager
systemctl httpd stop
systemctl mysql stop
