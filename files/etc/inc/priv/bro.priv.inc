<?php
/*
	* bro.priv.inc
  * part of pfSense (https://www.pfSense.org/)
  * Copyright (c) 2018 Prosper Doko
  * All rights reserved.
  *
  * Licensed under the Apache License, Version 2.0 (the "License");
  * you may not use this file except in compliance with the License.
  * You may obtain a copy of the License at
  *
  * http://www.apache.org/licenses/LICENSE-2.0
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
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
