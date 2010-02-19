MicroApache 2.0.xx README.TXT
====================================

Website: http://microapache.amadis.sytes.net

License: see the enclosed apache-license.txt and php-license.txt files

Source code is unchanged and available from: http://www.apache.org
                                             and
                                             http://www.php.net

CGI Demo Programs
=================
You may delete any example app(s) in .\cgi-bin if you wish

asmdemo.exe	Demo assembler CGI output
cgitest.exe     A brief CGI test diagnostic (YOU MUST DELETE THIS)
tailcgi.exe     A useful real-time CGI-based log viewer
/css/tail.css   (If included) This is a css file for tailcgi.exe - you may 
                either delete it or move the folder to your document root

Misc Files
==========
The following files *may* be included with your distro...
GO.BAT          Batch file to start the server
STOP.BAT        Batch file to halt the server (and all other copies of it)
KILLPROC.EXE    Program to kill Win32 processes used by STOP.BAT

Configuration Etc.
==================
This distro was designed to run a complete website from floppy disk

The default webroot is \wwwroot\htdocs (/wwwroot/htdocs)
Apache uses Unix slash characters for the config (/ rather than \)
Move phpinfo.php to your webroot for testing and/or delete it

Valid document roots include 
"." (current dir)
"/" (root dir)
x:/path (rooted path)

Due to a bug you cannot configure a rooted path with no drive letter
"/rootdir" is NOT valid in a DocumentRoot or ServerRoot statement

You may delete any modules to reduce the size of the distro but
remember to uncomment any relevant config in httpd.conf

The .\conf\httpd.conf is for EXAMPLE ONLY - tweak to suit your setup

The binaries are unchanged standard Apache distro so please read the
relevant Apache documentation before asking questions!. This distro
is able to provide the complete Apache feature set (with tweaking!)

NOTE: Please pay attention to proper Apache security config before 
making any public server live!

Stopping and Starting the Server
================================
Two batch files may be included with your distro, GO.BAT and STOP.BAT respectively

The meaning of these should be obvious. GO.BAT will launch mapache.exe minimised.
STOP.BAT will use "KILLPROC.EXE" to abruptly terminate the mapache.exe server process
Please note that if you have multiple copies of mapache.running that this batch 
file will kill ALL instances of the program.
Alternatively you may maximise the Apache console and press CTRL+C to terminate
all mapache processes.

PHP - IMPORTANT
===============

This section applies if you have downloaded a copy of MicroApache with PHP bundled
or you intend to install MicroPHP.

*** YOU *MUST* CHECK AND CONFIGURE PHP SECURITY SETTiNGS BEFORE GOING LIVE! ***
*** YOU *SHOULD* REMOVE ALL PHPINFO.PHP FILE(S) BEFORE GOING LIVE! ***

Please note support PHP developers will cease support for PHP 4.x.x 31/12/2007

For mor information go to http://www.php.net

Please note that you need to correctly configure PHP security in PHP.INI!!!

PHP 4.x files...

php4apache2.dll    PHP4 main DLL
php4ts.dll         PHP4 transaction DLL (usually in \WINNT or \WINDOWS)
php.ini            Example config file - please check security settings!

PHP 5.x files...

php5apache2.dll    PHP5 main DLL
php5ts.dll         PHP5 transaction DLL (usually in \WINNT or \WINDOWS)
php.ini            Example config file - please check security settings!

No support provided whatsoever but CONSTRUCTIVE suggestions are welcomed.
If you find MicroApache useful then a simple thank-you email would be
appreciated! :)

--

M Shaw - December 18th 2007