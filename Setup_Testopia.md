# Setup Testopia Server

Tested on following distributions (see /etc/lsb-release)
- Ubuntu 14.04.2 LTS
- Ubuntu 16.04.2 LTS
- Linux Mint 17 Qiana

## Docs
- http://bugzilla.readthedocs.org/en/latest/installing/quick-start.html
- https://github.com/bugzilla/extensions-Testopia
- https://wiki.mozilla.org/Testopia:README

## Install on Linux

Open a terminal and enter the following commands:
    
Become root
```bash
sudo bash
```

Install packages (ensure all APT repos enabled!)
```bash
# keyserver.ubuntu.com not needed for Mint!
apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 16126D3A3E5C1192
apt-get update
apt-get -y dist-upgrade
apt-get -y install git nano apache2 mysql-server libappconfig-perl libdate-calc-perl libtemplate-perl libmime-perl build-essential libdatetime-timezone-perl libdatetime-perl libemail-sender-perl libemail-mime-perl libemail-mime-modifier-perl libdbi-perl libdbd-mysql-perl libcgi-pm-perl libmath-random-isaac-perl libmath-random-isaac-xs-perl libapache2-mod-perl2 libapache2-mod-perl2-dev libchart-perl libxml-perl libxml-twig-perl perlmagick libgd-graph-perl libtemplate-plugin-gd-perl libsoap-lite-perl libhtml-scrubber-perl libjson-rpc-perl libdaemon-generic-perl libtheschwartz-perl libtest-taint-perl libauthen-radius-perl libfile-slurp-perl libencode-detect-perl libmodule-build-perl libnet-ldap-perl libauthen-sasl-perl libtemplate-perl-doc libfile-mimeinfo-perl libhtml-formattext-withlinks-perl libfile-which-perl libgd-dev libmysqlclient-dev lynx-cur graphviz python-sphinx rst2pdf unzip apache2-utils xsltproc cpanminus cmake
# if available
apt-get -y apache2-mpm-prefork
apt-get -y install libtext-diff-perl libjson-xs-perl 
```
-> Set and remember MySQL root password!
    
Patch MySQL Configuration (set ft_min_word_len, max_allowed_packet):
```bash
cd /etc/mysql/mysql.conf.d
patch -p0 <<EOF 
--- mysqld.cnf.orig	2017-05-06 21:26:59.555697247 +0200
+++ mysqld.cnf	2017-05-06 21:28:25.264800371 +0200
@@ -25,6 +25,9 @@
 nice		= 0
 
 [mysqld]
+# Testopia settings
+ft_min_word_len=2
+
 #
 # * Basic Settings
 #
@@ -45,7 +48,8 @@
 # * Fine Tuning
 #
 key_buffer_size		= 16M
-max_allowed_packet	= 16M
+# max_allowed_packet	= 16M
+max_allowed_packet	= 100M
 thread_stack		= 192K
 thread_cache_size       = 8
 # This replaces the startup script and checks MyISAM tables if needed
EOF
```

Restart MySQL
```bash
service mysql restart
```

Install DB
```bash
export db_pass=<your-MySQL-bugs-user-password>
mysql -u root -p -e "GRANT ALL PRIVILEGES ON bugs.* TO bugs@localhost IDENTIFIED BY '$db_pass'"
```

Get Bugzilla and Testopia
```bash
cd /var/www
mv html html.bak
git clone --branch release-4.4-stable https://github.com/bugzilla/bugzilla
git clone https://github.com/bugzilla/extensions-Testopia.git
wget -O ~/Downloads/testopia-2.4-BUGZILLA-3.6.tar.gz https://ftp.mozilla.org/pub/mozilla.org/webtools/testopia/testopia-2.4-BUGZILLA-3.6.tar.gz
git clone https://github.com/jpoehls/extjs-3.2.1.git
cp -r bugzilla html
tar -C html -x -f ~/Downloads/testopia-2.4-BUGZILLA-3.6.tar.gz ./tr_builds.cgi ./tr_list_plans.cgi ./tr_show_plan.cgi ./tr_list_runs.cgi ./tr_list_cases.cgi ./tr_categories.cgi ./tr_list_environments.cgi ./tr_show_product.cgi
cp -r extensions-Testopia/* html/
cp -r extjs-3.2.1 html/extensions/Testopia/extjs
chown -R www-data html
chgrp -R www-data html
```

Apache Setup
```bash
htpasswd -c /etc/apache2/.htpasswd admin@localhost.localdomain
chown www-data /etc/apache2/.htpasswd

cat <<EOF >/etc/apache2/sites-available/bugzilla.conf
ServerName localhost

<Directory /var/www/html>
  AddHandler cgi-script .cgi
  Options +ExecCGI
  DirectoryIndex index.cgi index.html
  AllowOverride All
  AuthType Basic
  AuthName "Basic Authentication"
  AuthUserFile /etc/apache2/.htpasswd
  require valid-user
</Directory>    
EOF

a2enmod cgi headers expires rewrite
a2ensite bugzilla
service apache2 restart
```

Perl Modules Installation
```bash
cd ~/Downloads
wget http://search.cpan.org/CPAN/authors/id/S/SA/SAMTREGAR/XML-Validator-Schema-1.10.tar.gz
cpanm XML-Validator-Schema-1.10.tar.gz 
cpanm Email::Send Text::CSV XML::Schema::Parser JSON Text::Diff PatchReader
```

Finish Bugzilla Installation
```bash
cd /var/www/html
./checksetup.pl

patch -p0 <<EOF
--- localconfig.orig	2017-05-06 21:57:56.809041390 +0200
+++ localconfig	2017-05-06 21:59:28.930825303 +0200
@@ -26,7 +26,7 @@
 # 
 # If you set this to anything other than "", you will need to run checksetup.pl
 # as root or as a user who is a member of the specified group.
-$webservergroup = 'apache';
+$webservergroup = 'www-data';
 
 # Set this to 1 if Bugzilla runs in an Apache SuexecUserGroup environment.
 # 
@@ -64,7 +64,7 @@
 # If you use apostrophe (') or a backslash (\) in your password, you'll
 # need to escape it by preceding it with a '\' character. (\') or (\)
 # (It is far simpler to just not use those characters.)
-$db_pass = '';
+$db_pass = 'your-MySQL-bugs-user-password';
 
 # Sometimes the database server is running on a non-standard port. If that's
 # the case for your database server, set this to the port number that your
EOF

./checksetup.pl
chown -R www-data /var/www/html
chgrp -R www-data /var/www/html
```

## Configure Bugzilla and Testopia using a Web Browser

```
http://<IP>/editparams.cgi?section=core
urlbase http://<IP>/

http://<IP>/editparams.cgi?section=testopia 
allow-test-deletion On
testopia-allow-group-member-deletes On
default-test-case-status CONFIRMED

http://<IP>/editparams.cgi?section=auth
auth_env_email=REMOTE_USER
user_info_class="Env,CGI" or "Env"

http://<IP>/editusers.cgi?action=edit&userid=1&grouprestrict=&enabled_only=1&matchtype=substr&matchvalue=login_name&groupid=1&matchstr=admin
Group access: Testers: Can read and write all test plans, runs, and cases.
```
