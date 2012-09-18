<?php session_start(); ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<title>Update Timezone Name</title>
</head>
<body>
<?php
	if (isset($_GET['timezonename']))
		$_SESSION['timezonename'] = $_GET['timezonename'];
	else
		$_SESSION['timezonename'] = 'UTC';
?>
</body>
</html>