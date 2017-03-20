Setup Testopia Server

Docs
    http://bugzilla.readthedocs.org/en/latest/installing/quick-start.html

Check if really Ubuntu 14.04 LTS    
    user@business-server:~$ cat /etc/lsb-release
    DISTRIB_ID=Ubuntu
    DISTRIB_RELEASE=14.04
    DISTRIB_CODENAME=trusty
    DISTRIB_DESCRIPTION="Ubuntu 14.04.2 LTS"

    Also tested for:
    DISTRIB_ID=LinuxMint
    DISTRIB_RELEASE=17
    DISTRIB_CODENAME=qiana
    DISTRIB_DESCRIPTION="Linux Mint 17 Qiana"
    
Become root
    sudo bash

Install packages (ensure all APT repos enabled!)
    # keyserver.ubuntu.com not needed for Mint!
    apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 16126D3A3E5C1192
    apt-get update
    apt-get -y dist-upgrade
    apt-get -y install git nano apache2 mysql-server libappconfig-perl libdate-calc-perl libtemplate-perl libmime-perl build-essential libdatetime-timezone-perl libdatetime-perl libemail-sender-perl libemail-mime-perl libemail-mime-modifier-perl libdbi-perl libdbd-mysql-perl libcgi-pm-perl libmath-random-isaac-perl libmath-random-isaac-xs-perl apache2-mpm-prefork libapache2-mod-perl2 libapache2-mod-perl2-dev libchart-perl libxml-perl libxml-twig-perl perlmagick libgd-graph-perl libtemplate-plugin-gd-perl libsoap-lite-perl libhtml-scrubber-perl libjson-rpc-perl libdaemon-generic-perl libtheschwartz-perl libtest-taint-perl libauthen-radius-perl libfile-slurp-perl libencode-detect-perl libmodule-build-perl libnet-ldap-perl libauthen-sasl-perl libtemplate-perl-doc libfile-mimeinfo-perl libhtml-formattext-withlinks-perl libfile-which-perl libgd-dev libmysqlclient-dev lynx-cur graphviz python-sphinx rst2pdf unzip apache2-utils xsltproc
    Mint: apt-get -y install libtext-diff-perl libjson-xs-perl 
    -> Set and remember MySQL root password
    
Get Bugzilla
    cd /var/wwws
    rm -rf html
    git clone --branch release-4.4-stable https://git.mozilla.org/bugzilla/bugzilla html
    cd html

Patch MySQL Configuration
    cd /etc/mysql
    patch -p0 <<EOF 
--- my.cnf.orig 2014-02-19 22:18:21.000000000 +0100
+++ my.cnf      2016-02-16 07:48:57.442592065 +0100
@@ -31,2 +31,5 @@
 [mysqld]
+# Testopia settings
+ft_min_word_len=2
+
 #
@@ -51,3 +54,3 @@
 key_buffer             = 16M
-max_allowed_packet     = 16M
+max_allowed_packet     = 100M
 thread_stack           = 192K
EOF

Restart MySQL
    service mysql restart

Install DB
    export db_pass=<your-MySQL-bugs-user-password>
    mysql -u root -p -e "GRANT ALL PRIVILEGES ON bugs.* TO bugs@localhost IDENTIFIED BY '$db_pass'"

Apache Setup
    htpasswd -c /etc/apache2/.htpasswd admin@localhost.localdomain
    chown www-data /etc/apache2/.htpasswd

    cd /
    mv /var/www/html /var/www/html.old
    tar xf /home/rte/Testopia-Linux.tar.gz

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

Finish Bugzilla Installation
    cd /var/www/html
    ./checksetup.pl

Configure Bugzilla and Testopia using a Web Browser
    http://172.17.5.121/editparams.cgi?section=core
    urlbase http://172.17.5.121/

    http://192.168.130.128/editparams.cgi?section=testopia 
    allow-test-deletion On
    testopia-allow-group-member-deletes On
    default-test-case-status CONFIRMED

    http://192.168.130.128/editparams.cgi?section=auth
    auth_env_email=REMOTE_USER
    user_info_class="Env,CGI" oder "Env"

    http://192.168.130.128/editusers.cgi?action=edit&userid=1&grouprestrict=&enabled_only=1&matchtype=substr&matchvalue=login_name&groupid=1&matchstr=admin
    Group access: Testers: Can read and write all test plans, runs, and cases.



