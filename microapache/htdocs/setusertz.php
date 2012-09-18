<?php
if (!isset($_SESSION['timezonename'])) {
	// Set a default value to avoid warnings in PHP >= 5.3 if date.timezone is not set in php.ini
	if (function_exists('date_default_timezone_set'))
		date_default_timezone_set('UTC');
    ?>
	<script language="JavaScript" src="js/detect_timezone.js" type="text/javascript"></script>
	<script language="JavaScript" type="text/javascript">
	//<![CDATA[
	function CreateXMLHttpRequest() {
		var httpReq;
		// Branch for native XMLHttpRequest object
		if (typeof window.XMLHttpRequest != 'undefined') {
			try {
				httpReq = new XMLHttpRequest();
			}
			catch(e) {
				httpReq = null;
			}
		}
		// Branch for IE/Windows ActiveX version
		else if (typeof window.ActiveXObject != 'undefined') {
			try {
				httpReq = new ActiveXObject('Msxml2.XMLHTTP');
			}
			catch(e) {
				try{
					httpReq = new ActiveXObject('Microsoft.XMLHTTP');
				}
				catch(e) {
					httpReq = null;
				}
			}
		}
		/* For ICEbrowser
		 * per their site http://support.icesoft.com/jive/entry.jspa?entryID=471&categoryID=21
		 * There are a number of restrictions on the implementation and it, only does get
		 */
		else if (typeof window.createRequest != 'undefined') {
			try {
				httpReq = window.createRequest();
			}
			catch(e) {
				httpReq = null;
			}
		}
		else
			httpReq = null;
		return httpReq;
	}
	var timezone = jstz.determine();
	var ajax = CreateXMLHttpRequest();
	if (ajax != null) {
		ajax.open('GET', 'updatetimezonename.php?timezonename=' + encodeURIComponent(timezone.name()), false); // sync. call
		ajax.send(null);
		if (ajax.status == 200)
			location.href = "<?php echo $_SERVER['REQUEST_URI']; ?>"; // reload page, timezonename session var is set
	}
	//]]>
	</script>
	<?php
}
else if (function_exists('date_default_timezone_set'))
	date_default_timezone_set($_SESSION['timezonename']);
?>