<?php
if (!isset($_SESSION['timezonename'])) {
	// Set a default value to avoid warnings in PHP >= 5.3 if date.timezone is not set in php.ini
	if (function_exists('date_default_timezone_set'))
		date_default_timezone_set('UTC');
    ?>
	<script src="js/detect_timezone.js" type="text/javascript"></script>
	<script type="text/javascript">
	//<![CDATA[
	var timezone = jstz.determine();
	var ajax = new XMLHttpRequest();
	ajax.open('GET', 'updatetimezonename.php?timezonename=' + encodeURIComponent(timezone.name()), false); // sync. call
	ajax.send(null);
	if (ajax.status == 200)
		location.href = "<?php echo $_SERVER['REQUEST_URI']; ?>"; // reload page, timezonename session var is set
	//]]>
	</script>
	<?php
}
else if (function_exists('date_default_timezone_set'))
	date_default_timezone_set($_SESSION['timezonename']);