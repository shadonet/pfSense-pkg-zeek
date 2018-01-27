<?php
/*
bro_alerts.php
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
require_once("/etc/inc/util.inc");
require_once("/etc/inc/functions.inc");
require_once("/etc/inc/pkg-utils.inc");
require_once("/etc/inc/globals.inc");
require_once("guiconfig.inc");

$pgtitle = array(gettext("Package"), gettext("Bro"), gettext("Alerts"));
$shortcut_section = "bro";
include("head.inc");

if ($savemsg) {
	print_info_box($savemsg);
}

$tab_array = array();
$tab_array[] = array(gettext("General"), false, "/pkg_edit.php?xml=bro.xml&amp;id=0");
$tab_array[] = array(gettext("BroControl Config"), false, "/pkg_edit.php?xml=bro_broctl.xml&amp;id=0");
$tab_array[] = array(gettext("Bro Cluster"), false, "/pkg_edit.php?xml=bro_cluster.xml&amp;id=0");
$tab_array[] = array(gettext("Bro Scripts"), false, "/pkg.php?xml=bro_script.xml");
$tab_array[] = array(gettext("Log Mgmt"), false, "/pkg_edit.php?xml=bro_log.xml&amp;id=0");
$tab_array[] = array(gettext("Real Time Inspection"), true, "/bro_alerts.php");
$tab_array[] = array(gettext("XMLRPC Sync"), false, "/pkg_edit.php?xml=bro_sync.xml");

display_top_tabs($tab_array);
?>

<div class="panel panel-default">
	<div class="panel-heading"><h2 class="panel-title"><?=gettext("Filtering"); ?></h2></div>
	<div class="panel-body">
		<div class="table-responsive">
			<form id="paramsForm" name="paramsForm" method="post" action="">
				<table class="table table-hover table-condensed">
					<tbody>
						<tr>
							<td width="22%" valign="top" class="vncellreq">Log file to view:</td>
							<td width="78%" class="vtable">
								<select id="logfile">
									<option value="0">- Select -</option>
								</select>
								<br/>
								<span class="vexpl">
									<?=gettext("Choose which log you want to view")?>
								</span>
							</td>
						</tr>
						<tr>
							<td width="22%" valign="top" class="vncellreq">Max lines:</td>
							<td width="78%" class="vtable">
								<select name="maxlines" id="maxlines">
									<option value="10" selected="selected">10 lines</option>
									<option value="15">15 lines</option>
									<option value="20">20 lines</option>
									<option value="25">25 lines</option>
									<option value="100">100 lines</option>
									<option value="200">200 lines</option>
								</select>
								<br/>
								<span class="vexpl">
									<?=gettext("Max. lines to be displayed.")?>
								</span>
							</td>
						</tr>
					</tbody>
				</table>
			</form>
		</div>
	</div>

	<div class="panel-heading"><h2 class="panel-title"><?=gettext("Log File Content"); ?></h2></div>
	<div class="panel-body">
		<div class="table-responsive">
			<table class="table table-hover table-condensed">
				<tbody>
					<tr><td>
						<table width="100%" border="0" cellspacing="20" cellpadding="10">
							<tbody id="broView">
								<tr><td></td></tr>
							</tbody>
						</table>
					</td></tr>
				</tbody>
			</table>
		</div>
	</div>
</div>
<?php include("foot.inc"); ?>
<!-- Function to call programs logs -->
<script type="text/javascript">
//<![CDATA[
$( "#logfile" ).change(function() {
	showLog('broView', 'bro_alert_data.php', 'bro');
});

function updateSelect() {
	var x = 'x='+document.getElementById("logfile").length;
	jQuery.ajax({
		type: "POST",
		url: "select_box_file.php",
		data: x,
		success: function(html)
		{
			if (html) {
				$("#logfile").html(html);
			}
		},
	});
}

function showLog(content, url, program) {
	jQuery.ajax(url,
		{
			type: 'post',
			data: {
				logfile: $('#logfile').val(),
				maxlines: $('#maxlines').val(),
				program: program,
				content: content
			},
			success: function(ret){
				$('#' + content).html(ret);
			}
		});
	}

	function updateAllLogs() {
		updateSelect();
		showLog('broView', 'bro_alert_data.php', 'bro');
		setTimeout(updateAllLogs, 10000);
	}

	events.push(function() {
		updateAllLogs();
	});
	//]]>
</script>
