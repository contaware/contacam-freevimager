<?php
require_once( 'configuration.php' );
$host  = (!empty($_SERVER['HTTP_X_FORWARDED_HOST'])) ? $_SERVER['HTTP_X_FORWARDED_HOST'] : $_SERVER['HTTP_HOST'];
$uri   = rtrim(dirname($_SERVER['PHP_SELF']), '/\\');
$file  = DEFAULTPAGE;
header("Location: http://$host$uri/$file");
?>
