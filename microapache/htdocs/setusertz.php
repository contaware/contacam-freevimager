<?php
if (!isset($_SESSION['timezonename']))
{
	// Set a default value to avoid warnings in PHP >= 5.3 if date.timezone is not set in php.ini
	date_default_timezone_set('UTC');
    ?>
	<script type="text/javascript">
	//<![CDATA[
	var timezoneoffset = -60 * ((new Date()).getTimezoneOffset()); // includes eventual daylight saving offset
	var ajax = new XMLHttpRequest();
	ajax.open('GET', 'updatetimezonename.php?timezoneoffset=' + encodeURIComponent(timezoneoffset), false); // sync. call
	ajax.send(null);
	if (ajax.status == 200)
		location.href = "<?php echo $_SERVER['REQUEST_URI']; ?>"; // reload page, timezonename session var is set
	//]]>
	</script>
	<?php
}
else
{
	// Timezone must be set with each script load
	date_default_timezone_set($_SESSION['timezonename']);
	
	// Force getting timezoneoffset from browser after given time
	if (isset($_SESSION['timezonename_discard_after']) && time() > $_SESSION['timezonename_discard_after'])
	{
		unset($_SESSION['timezonename']);
		unset($_SESSION['timezonename_discard_after']);
    }	
}