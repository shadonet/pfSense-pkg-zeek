<?php
/*
	bro.priv.inc
	part of pfSense (http://www.pfSense.org/)
	Copyright (C) 2015 ESF, LLC
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
global $priv_list;

$priv_list['page-services-bro'] = array();
$priv_list['page-services-bro']['name'] = "WebCfg - Services: bro package";
$priv_list['page-services-bro']['descr'] = "Allow access to bro package GUI";

$priv_list['page-services-bro']['match'] = array();
$priv_list['page-services-bro']['match'][] = "pkg_edit.php?xml=bro_script.xml*";

$priv_list['page-services-bro']['match'][] = "pkg_edit.php?xml=bro.xml*";
$priv_list['page-services-bro']['match'][] = "pkg_edit.php?xml=bro_broctl.xml*";
$priv_list['page-services-bro']['match'][] = "pkg_edit.php?xml=bro_cluster.xml*";
$priv_list['page-services-bro']['match'][] = "pkg_edit.php?xml=bro_script.xml*";
$priv_list['page-services-bro']['match'][] = "pkg_edit.php?xml=bro_log.xml*";
$priv_list['page-services-bro']['match'][] = "pkg_edit.php?xml=bro_sync.xml*";

$priv_list['page-services-bro']['match'][] = "shortcuts/pkg_bro.inc*";
$priv_list['page-services-bro']['match'][] = "bro_alerts.php*";
$priv_list['page-services-bro']['match'][] = "bro_alert_data.php*";
$priv_list['page-services-bro']['match'][] = "select_box_file.php*";

?>
