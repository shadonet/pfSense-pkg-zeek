<?php
/*
bro_monitor_data.php
part of pfSense (https://www.pfSense.org/)
Copyright (C) 2015-2016 Prosper Doko
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/
require_once("guiconfig.inc");

/* Requests */
if ($_POST) {
	global $program, $logfile;
	// Actions
	//$filter = preg_replace('/(@|!|>|<)/', "", htmlspecialchars($_POST['strfilter']));
	$program = strtolower($_POST['program']);
	$logfile = $_POST['logfile'];

	switch ($program) {
		case 'bro':
		// Define log file
		$log = '/usr/local/spool/bro/'.$logfile;
		$loghead = fetch_head($log);
		echo "<thead>";

		foreach ($loghead as $value) {
			if (preg_match("/\bfields\b/", $value)) {
				//	$headfield = preg_split("/[\s,]+/", $value);
				$headfield = preg_split("/[\t,]/", $value);
				show_tds($headfield);
				break;
			}
		}
		echo "</thead>";
		// Fetch lines
		$logarr = fetch_log($log);
		// Print lines
		foreach ($logarr as $logent) {
			if(!is_numeric($logent[0]))
			continue;
			// Split line by space delimiter
			$logline = preg_split("/[\t,]/", $logent);
			$logline[0] = date("d.m.Y H:i:s", $logline[0]);

			echo "<tr>";
			foreach ($logline as $value) {
				echo "<td class=\"col-md-4\">".$value."</td>\t";
			}
			echo "</tr>\n";
			echo "<tr><td></td></tr>";
		}
		break;
	}
}

/* Functions */
function fetch_head($log) {
	$log = escapeshellarg($log);
	// Get logs based in filter expression
	exec("/usr/bin/head -n 10 {$log}", $loghead);
	// Return logs head
	return $loghead;
}

// Show bro Logs
function fetch_log($log) {
	$log = escapeshellarg($log);
	// Get data from form post
	$lines = escapeshellarg(is_numeric($_POST['maxlines']) ? $_POST['maxlines'] : 50);
	// Get logs based in filter expression
	exec("/usr/bin/tail -r -n {$lines} {$log}", $logarr);
	// Return logs
	return $logarr;
}

function show_tds($tds) {
	echo "<tr>";
	$index = 0;
	foreach ($tds as $td){
		if ($index != 0){
			echo "<th class=\"col-md-4\">" .$td. "</th>";
		}
		$index = $index + 1 ;
	}
	echo "</tr>";
}

?>
