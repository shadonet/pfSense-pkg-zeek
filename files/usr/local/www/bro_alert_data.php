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

				// Word wrap the URL
				// $logline[7] = htmlentities($logline[7]);
				// $logline[7] = html_autowrap($logline[7]);

				// Remove /(slash) in destination row
				// $logline_dest = preg_split("/\//", $logline[9]);

				// Apply filter and color
				// Need validate special chars
				/*if ($filter != "") {
					$logline = preg_replace("@($filter)@i","<span><font color='red'>$1</font></span>", $logline);
				}*/

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
//list_by_ext: returns an array containing an alphabetic list of files in the specified directory ($path) with a file extension that matches $extension

function list_by_ext($extension, $path){
    $list = array(); //initialise a variable
    $dir_handle = @opendir($path) or die("Unable to open $path"); //attempt to open path
    while($file = readdir($dir_handle)){ //loop through all the files in the path
        if($file == "." || $file == ".."){continue;} //ignore these
        $filename = explode(".",$file); //seperate filename from extenstion
        //var_dump($filename);
        $cnt = count($filename); $cnt--; $ext = $filename[$cnt]; //as above
        if(strtolower($ext) == strtolower($extension)){ //if the extension of the file matches the extension we are looking for...
            array_push($list, $file); //...then stick it onto the end of the list array
        }
    }
    if($list[0]){ //...if matches were found...
    return $list; //...return the array
    } else {//otherwise...
    return false;
    }
}

/*function html_autowrap($cont) {
	// split strings
	$p = 0;
	$pstep = 25;
	$str = $cont;
	$cont = '';
	for ($p = 0; $p < strlen($str); $p += $pstep) {
		$s = substr($str, $p, $pstep);
		if (!$s) {
			break;
		}
		$cont .= $s . "<wbr />";
	}
	return $cont;
}*/

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
	/*if (preg_match("/!/", htmlspecialchars($_POST['strfilter']))) {
		$grep_arg = "-iv";
	} else {
		$grep_arg = "-i";
	}*/

	// Check program to execute or no the parser
	/*if ($program == "bro") {
		$parser = "| /usr/local/bin/php-cgi -q bro_log_parser.php";
	} else {
		$parser = "";
	}*/

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
