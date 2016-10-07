<?php

// Direct access not permitted
if (!function_exists('getDirBasename'))
	die('Direct access not permitted');

// Login?
$invalid_login = 0;
if (isset($_POST['password']))
{
	if ($_POST['password'] == "$password")
	{
		$_SESSION[getDirBasename()] = "$password";
		return;
	}
	else
	{
		unset($_SESSION[getDirBasename()]);
		$invalid_login = 1;
	}
}
// OK?
else if ($password == '' || (isset($_SESSION[getDirBasename()]) && $_SESSION[getDirBasename()] == "$password"))
	return;

// Show login form and exit
require_once( LANGUAGEFILEPATH ); // Must be here because it outputs the UTF8-BOM!
?>
<!DOCTYPE html>
<html>
    <head>
		<meta http-equiv="X-UA-Compatible" content="IE=edge" />
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<meta name="viewport" content="width=device-width, initial-scale=1" />
		<meta name="author" content="Oliver Pfister" />
        <title>Login</title>
		<?php echo "<link rel=\"stylesheet\" href=\"" . STYLEFILEPATH . "\" type=\"text/css\" />\n"; ?>
    </head>
    <body>
		<?php if ($invalid_login): ?>
			<p><? echo INVALID; ?></p>
		<?php endif; ?>
        <form name="login" action="" method="post">
            <? echo PASSWORD_LABEL; ?><br />
			<input type="password" name="password" value="" /><br />
            <input type="submit" name="submit" value="<? echo LOGIN; ?>" />
        </form>
    </body>
</html>
<?php
exit;
?>