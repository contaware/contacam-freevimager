<?php session_start(); ?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
    "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=edge" />
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta name="author" content="Oliver Pfister" />
<title>Update Timezone Name</title>
</head>
<body>
<?php
	function tz_offset_to_name($offset_sec)
	{
		$time_zones = timezone_identifiers_list();
		foreach ($time_zones as $time_zone_id)
		{	
			$dateTime = new DateTime(null, new DateTimeZone($time_zone_id));
			if ($dateTime->getOffset() == $offset_sec) // getOffset() includes eventual daylight saving offset
				return $time_zone_id;
		}
		return false;
	}

	if (isset($_GET['timezoneoffset']))
	{
		// Store timezonename in a session variable
		$timezonename = tz_offset_to_name($_GET['timezoneoffset']);
		if ($timezonename !== false)
			$_SESSION['timezonename'] = $timezonename;
		else
			$_SESSION['timezonename'] = 'UTC';
		
		// Force getting timezoneoffset from browser with javascript every 10 minutes
		$_SESSION['timezonename_discard_after'] = time() + 600;
	}
?>
</body>
</html>