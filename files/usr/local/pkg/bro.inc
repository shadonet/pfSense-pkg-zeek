<?php
/*
bro.inc
part of pfSense (https://www.pfSense.org/)
Copyright (C) 2009 Prosper Doko

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
require_once('config.inc');
require_once('globals.inc');
require_once('notices.inc');
require_once('pfsense-utils.inc');
require_once('pkg-utils.inc');
require_once('services.inc');
require_once('service-utils.inc');
require_once("functions.inc");
require_once('util.inc');
require_once("interfaces.inc");
require_once('xmlrpc.inc');
require_once('xmlrpc_client.inc');

if (!function_exists("filter_configure")) {
	require_once("filter.inc");
}

$shortcut_section = "bro";

define('BRO_LOCALBASE', '/usr/local');
define('BRO_ETC', BRO_LOCALBASE .'/etc');
define('BRO_LOGDIR', BRO_LOCALBASE .'/logs/current');
define('BRO_SITEDIR', BRO_LOCALBASE .'/share/bro/site');

/*
* Utility functions
*/

/* Handle base64 encoding and linebreaks in textarea configuration fields*/
function bro_text_area_decode($text) {
	return preg_replace('/\r\n/', "\n", base64_decode($text));
}

/* Get interface IP and netmask for Bro interfaces*/
function bro_get_real_interface_address($iface) {
	if (!function_exists("get_interface_ip")) {
		require_once("interfaces.inc");
	}
	return array(get_interface_ip($iface), gen_subnet_mask(get_interface_subnet($iface)));
}

/* Check whether Bro is enabled*/
function bro_enabled() {
	global $config, $bro_enabled;
	$bro_enabled = false;

	if (is_array($config['installedpackages']['bro']['config'])) {
		// check whether Bro is enabled ...
		if ($config['installedpackages']['bro']['config'][0]['enable_bro'] == "on") {
			// ... and has at least one interface configured ...
			if (!empty($config['installedpackages']['bro']['config'][0]['active_interface'])) {
				$bro_enabled = true;
			}
		}
	}
	return $bro_enabled;
}
// Check whether Bro Cluster is enabled
function bro_cluster_enabled() {
	global $config, $bro_cluster_enabled;
	$bro_cluster_enabled = false;

	if (is_array($config['installedpackages']['brocluster'])) {
		// check whether bro cluster is enabled ...
		if ($config['installedpackages']['brocluster']['config'][0]['enable_cluster'] == "on"){
			$bro_cluster_enabled = true;
		}
	}
	return $bro_cluster_enabled;
}

// Start and/or stop services according to Bro configuration
function bro_restart_services() {
	global $config;
	// do not (re)start bro services on boot
	if (platform_booting()) {
		return;
	}

	if (bro_enabled()) {
		if (!is_service_running('bro')) {
			log_error("[bro] Starting service...");
			mwexec(BRO_ETC . "/rc.d/bro start");
		} else {
			log_error("[bro] Reloading for configuration sync...");
			mwexec(BRO_ETC . "/rc.d/bro restart");
		}
		// sleep for a couple seconds to give bro a chance to fire up fully.
		for ($i = 0; $i < 10; $i++) {
			if (!is_service_running('bro')) {
				sleep(1);
			}
		}
	} else {
		//Bro is disabled
		if (is_service_running('bro')) {
			log_error("[bro] Stopping service...");
			//stop_service("bro");
			mwexec(BRO_ETC . "/rc.d/bro stop");
		}
	}}

/*
 * rc scripts, services and cronjobs
 */

 // Handle cronjob install/uninstall
 function bro_install_cron($should_install) {
	 if (platform_booting()) {
		 return;
	 }
	 $cron_cmd = BRO_BASE . "/bin/broctl cron ";

	 if (($should_install) && (bro_enabled())) {
		 log_error("[bro] Adding cronjobs ...");
		 install_cron_job("{$cron_cmd}", $should_install, "*/5", "*", "*", "*", "*", "root");
	 } else {
		 log_error("[bro] Removing cronjobs ...");
		 install_cron_job("{$cron_cmd}", false);
	 }
 }

 function bro_write_rcfile(){
	 $bro_etc_dir = BRO_ETC;
	 $rcfile = array();
	 $rcfile['file'] = 'bro.sh';
	 $rcfile['start'] = <<< EOD
	 {$bro_etc_dir}/rc.d/bro start

EOD;
	 $rcfile['stop'] = <<< EOD
	 {$bro_etc_dir}/rc.d/bro stop
	 # Just to be sure...
	 sleep 5
	 if [ -n "`/bin/ps auxw | /usr/bin/grep "bro" | /usr/bin/awk '{print $2}'`" ]; then
	 /usr/bin/killall -9 bro 2>/dev/null
	 fi
EOD;

	 conf_mount_rw();
	 write_rcfile($rcfile);
	 conf_mount_ro();
 }

 /*
 * Bro package install/uninstall
 */

 function bro_install_command() {
	 // Put your own code to execute on package reconfiguration here
	 global $g, $config;

	 /* Hard kill any running bro processes that may have been started by any   */
	 //of the pfSense scripts such as check_reload_status() or rc.start_packages
	 if(is_process_running("bro")) {
		 exec("/usr/bin/killall -z bro");
		 sleep(2);
	 }
	 // Turn off tcp segmentation offload.
	 exec("sysctl net.inet.tcp.tso=0");

	 // creating a backup file of the original broctl.cfg no matter if user checked this or not
	 if (!file_exists(BRO_ETC . "/broctl.cfg.backup")) {
		 log_error("BRO: Creating backup of the original file to " . BRO_ETC . "/broctl.cfg.backup");
		 copy(BRO_ETC . "/broctl.cfg", BRO_ETC . "/broctl.cfg.backup");
	 }

	 // creating a backup file of the original /etc/node no matter if user checked this or not
	 if (!file_exists(BRO_ETC . "/node.cfg.backup")) {
		 log_error("BRO: Creating backup of the original file to " . BRO_ETC . "/node.cfg.backup");
		 copy(BRO_ETC . "/node.cfg", BRO_ETC . "/node.cfg.backup");
	 }

	 // creating a backup file of the original /etc/networks no matter if user checked this or not
	 if (!file_exists(BRO_ETC . "/networks.cfg.backup")) {
		 log_error("BRO: Creating backup of the original file to " . BRO_ETC . "/networks.cfg.backup");
		 copy(BRO_ETC . "/networks.cfg", BRO_ETC . "/networks.cfg.backup");
	 }
	 // Change owner of bro created files
	 exec("chown -R root:wheel /usr/local/logs");

	 exec("chown -R root:wheel /usr/local/spool");

	 unlink_if_exists("/usr/local/etc/rc.d/bro.sh");

	 // create bro rcfile
	 bro_write_rcfile();

	 write_config("[bro] Package installed.");
 }


 function bro_deinstall_command() {
	 // Put your own code to execute on package reconfiguration here
	 bro_install_cron(false);
	 mwexec("/bin/ps awux | /usr/bin/egrep -i 'bro -f|\(bro\)' | /usr/bin/awk '{ print $2 }' | /usr/bin/xargs kill");
	 unlink_if_exists("/usr/local/etc/rc.d/bro.sh");

	 if (bro_log_resync()) {
		 if (is_dir("/usr/local/logs")) {
			 exec("/bin/rm -rf /usr/local/logs/*");
		 }
	 }

	 if (is_array($config['installedpackages']['bro'])) {
		 unset($config['installedpackages']['bro']);
	 }
	 if (is_array($config['installedpackages']['brocontrol'])) {
		 unset($config['installedpackages']['brocontrol']);
	 }
	 if (is_array($config['installedpackages']['brocluster'])) {
		 unset($config['installedpackages']['brocluster']);
	 }
	 if (is_array($config['installedpackages']['broscript'])) {
		 unset($config['installedpackages']['broscript']);
	 }
	 if (is_array($config['installedpackages']['brolog'])) {
		 unset($config['installedpackages']['brolog']);
	 }

	 if (is_array($config['installedpackages']['brosync'])) {
		 unset($config['installedpackages']['brosync']);
	 }
 }

 /*
 * Bro input validation
 */

 // Bro NSM: General Settings input validation
 function bro_validate_general($post, &$input_errors) {
	 global $config;

	 if (is_array($config['installedpackages']['bro'])) {
		 $settings = $config['installedpackages']['bro']['config'][0];
	 } else {
		 $settings = array();
	 }

	 if ($post['enable_bro'] == "on") {
		 if (!bro_cluster_enabled()) {
			 if (empty($post['active_interface'])) {
				 $input_errors[] = "You must select at least one interface under 'Bro Interface(s)' to enable Bro instance.";
				 $input_errors[] = "If you intend to use Bro as cluster, then visit Bro NSM: Bro cluster, configure and save the cluster settings first.";
			 }
		 } else {
			 log_error("[bro] Enabled as cluster. If this is not what you intended, visit Bro NSM: General and configure bro interfaces.");
		 }
	 }

	 if(!empty($post['hostname']) && !is_ipaddr($post['hostname']) && !is_domain($post['hostname'])) {
		 $input_errors[] = "'Hostname' must contain a valid IP address or domain name.";
	 }

	 $local_networks = explode("\n", $post['local_networks']);
	 foreach ($local_networks as $subnet) {
		 $subnet = trim($subnet);
		 if (!empty($subnet) && !is_subnet($subnet)) {
			 $input_errors[] = "'Local Network Address' must be a valid CIDR range. The subnet '$subnet' is not valid.";
		 }
	 }
	 unset($local_networks);

 }

 function bro_validate_broctl($post, &$input_errors) {
	 $mail_connection_summary = trim($post['mail_connection_summary']);
	 if (!empty($mail_connection_summary) && (!is_numericint($mail_connection_summary))) {
		 $input_errors[] = "You must enter a valid number in the 'mail connection summary' field.";
	 }
	 unset($mail_connection_summary);

	 $min_disk_space = trim($post['min_disk_space']);
	 if (!empty($min_disk_space) && (!is_numericint($min_disk_space))) {
		 $input_errors[] = "You must enter a valid number in the 'min disk space' field.";
	 }
	 unset($min_disk_space);

	 $mail_host_updown = trim($post['mail_host_updown']);
	 if (!empty($mail_host_updown) && (!is_numericint($mail_host_updown))) {
		 $input_errors[] = "You must enter a valid number in the 'mail host updown' field.";
	 }
	 unset($mail_host_updown);

	 $log_expiration = trim($post['log_expiration']);
	 if (!empty($log_expiration) && (!is_numericint($log_expiration))) {
		 $input_errors[] = "You must enter a valid number in the 'mail host updown' field.";
	 }
	 unset($log_expiration);

	 if (substr($post['log_dir'], -1, 1) == '/') {
		 $input_errors[] = 'Log location must not end with a / character.';
	 }

	 if ($post['log_dir']{0} != '/') {
		 $input_errors[] = 'Log location must start with a / character.';
	 }

	 if (strlen($post['log_dir']) <= 3) {
		 $input_errors[] = "Configured log location directory is not valid.";
	 }

	 $log_rotate = trim($post['log_rotate']);
	 if (!empty($log_rotate) && (!is_numericint($log_rotate))) {
		 $input_errors[] = "You must enter a valid number of days in the 'Log rotate' field.";
	 }
	 unset($log_rotate);

 }

 function bro_validate_cluster($post, &$input_errors) {
	 if ($post['enable_cluster'] == 'on') {
		 $managerhost = trim($post['manager_host']);

		 if (!is_ipaddr($managerhost) && !is_domain($managerhost)) {
			 $input_errors[] = "'Manager host' must contain a valid IP address or domain name.";
		 }

		 $proxyhost = trim($post['proxy_host']);
		 if (!is_ipaddr($proxyhost) && !is_domain($proxyhost)) {
			 $input_errors[] = "'Proxy host' must contain a valid IP address or domain name.";
		 }

		 $proxyhost = trim($post['proxy_host']);
		 if (!is_ipaddr($proxyhost) && !is_domain($proxyhost)) {
			 $input_errors[] = "'Proxy host' must contain a valid IP address or domain name.";
		 }

		 $worker1host = trim($post['worker1_host']);
		 if (!is_ipaddr($worker1host) && !is_domain($worker1host)) {
			 $input_errors[] = "'Worker 1 host' must contain a valid IP address or domain name.";
		 }
		 if (empty($post['worker1_interface'])) {
			 $input_errors[] = "You must select one interface under 'Worker 1 Interface' to enable Worker 1.";
		 }

		 $worker2host = trim($post['worker2_host']);
		 if (!is_ipaddr($worker2host) && !is_domain($worker2host)) {
			 $input_errors[] = "'Worker 2' must contain a valid IP address or domain name.";
		 }
		 if (empty($post['worker2_interface'])) {
			 $input_errors[] = "You must select one interface under 'Worker 2 Interface' to enable Worker 2.";
		 }
	 }
 }

 function bro_validate_script($post, &$input_errors) {
	 global $config;

	 //$settings = $config['installedpackages']['broscript']['config'];
	 if (!bro_enabled()) {
		 $input_errors[] = 'You must enabled Bro instance first';
	 }else {

		 if (substr($post['broscriptpath'], -1, 1) == '/') {
			 $input_errors[] = 'Script location must not end with a / character.';
		 }

		 if ($post['broscriptpath']{0} != '/') {
			 $input_errors[] = 'Script location must start with a / character.';
		 }

		 if (!file_exists($post['broscriptpath'])) {
			 $input_errors[] = "Configured script file does not exist.";
		 }

		 $ext = pathinfo($post['broscriptpath'], PATHINFO_EXTENSION);
		 if ($ext!="bro") {
			 $input_errors[] = "Configured script file has no valid extension. The extension must be 'bro'";
		 }
	 }
 }

 function bro_settings_resync() {
	 global $config;

	 if (is_array($config['installedpackages']['bro'])) {
		 $settings = $config['installedpackages']['bro']['config'][0];
	 } else {
		 $settings = array();
	 }
	 $hostname = ($settings['hostname'] ? $settings['hostname'] : 'localhost');
	 // Read assigned interface
	 if ($settings['active_interface']) {
		 $ifaces =  $settings['active_interface'];
	 } else {
		 $ifaces = array();
	 }

	 $iface = get_real_interface($ifaces);
	 if ($iface) {

		 $conf = '';
		 $conf .= <<< EOD
		 # Example BroControl node configuration.
		 #
		 # This example has a standalone node ready to go except for possibly changing
		 # the sniffing interface.

		 # This is a complete standalone configuration.  Most likely you will
		 # only need to change the interface.
		 [bro]
		 type=standalone
		 host=$hostname
		 interface=$iface

		 ## Below is an example clustered configuration. If you use this,
		 ## remove the [bro] node above.

		 #[logger]
		 #type=logger
		 #host=localhost
		 #
		 #[manager]
		 #type=manager
		 #host=localhost
		 #
		 #[proxy-1]
		 #type=proxy
		 #host=localhost
		 #
		 #[worker-1]
		 #type=worker
		 #host=localhost
		 #interface=eth0
		 #
		 #[worker-2]
		 #type=worker
		 #host=localhost
		 #interface=eth0

EOD;

$nodefile =  BRO_ETC . '/node.cfg';
conf_mount_rw();
file_put_contents($nodefile, $conf);
chmod($nodefile, 0640);
conf_mount_ro();
}

$conf = '';
$conf .= <<< EOD
# List of local networks in CIDR notation, optionally followed by a
# descriptive tag.
# For example, "10.0.0.0/8" or "fe80::/64" are valid prefixes.

EOD;

$local_networks = preg_replace("/\s+/","\n", bro_text_area_decode($settings['local_networks']));
if (!empty($local_networks)) {
	$conf .= "\n$local_networks";
}
$networksfile =  BRO_ETC . '/networks.cfg';
conf_mount_rw();
file_put_contents($networksfile, $conf);
chmod($networksfile, 0640);
conf_mount_ro();
bro_sync_on_changes();
bro_restart_services();
bro_install_cron(true);
}

function 	bro_broctl_resync() {
	// Put your own code to execute on package reconfiguration here
	global $config;

	if (is_array($config['installedpackages']['brocontrol'])) {
		$settings = $config['installedpackages']['brocontrol']['config'][0];
	} else {
		$settings = array();
	}

	$email = ($settings['admin_email'] ? $settings['admin_email'] : 'admin@localhost');
	$mailconnection = ($settings['mail_connection_summary'] ? $settings['mail_connection_summary'] : 0);
	$diskspace = ($settings['min_disk_space'] ? $settings['min_disk_space'] : 5);
	$mailhostupdown = ($settings['mail_host_updown'] ? $settings['mail_host_updown'] : 1);
	$rotate = ($settings['log_rotate'] ?  $settings['log_rotate'] : 3000);
	$logexpiration = ($settings['log_expiration'] ? $settings['log_expiration'] : 0);
	$statslog = ($settings['stats_log'] ? $settings['stats_log'] : 1);
	$statsduration = ($settings['stats_duration'] ? $settings['stats_duration'] : 0);
	$logdir = ($settings['log_dir'] ? $settings['log_dir'] : '/usr/local/logs');
	if (!is_dir($logdir)) {
		log_error("[bro] Creating Bro log dir '{$logdir}' ...");
		safe_mkdir($logdir, 0755);
	}

	$conf = '';
	$conf .= <<< EOD
	## Global BroControl configuration file.

	###############################################
	# Mail Options

	# Recipient address for all emails sent out by Bro and BroControl.
	MailTo = $email

	# Mail connection summary reports each log rotation interval.  A value of 1
	# means mail connection summaries, and a value of 0 means do not mail
	# connection summaries.  This option has no effect if the trace-summary
	# script is not available.
	MailConnectionSummary = $mailconnection

	# Lower threshold (in percentage of disk space) for space available on the
	# disk that holds SpoolDir. If less space is available, "broctl cron" starts
	# sending out warning emails.  A value of 0 disables this feature.
	MinDiskSpace = $diskspace

	# Send mail when "broctl cron" notices the availability of a host in the
	# cluster to have changed.  A value of 1 means send mail when a host status
	# changes, and a value of 0 means do not send mail.
	MailHostUpDown = $mailhostupdown

	###############################################
	# Logging Options

	# Rotation interval in seconds for log files on manager (or standalone) node.
	# A value of 0 disables log rotation.
	LogRotationInterval = $rotate

	# Expiration interval for archived log files in LogDir.  Files older than this
	# will be deleted by "broctl cron".  The interval is an integer followed by
	# one of these time units:  day, hr, min.  A value of 0 means that logs
	# never expire.
	LogExpireInterval = $logexpiration

	# Enable BroControl to write statistics to the stats.log file.  A value of 1
	# means write to stats.log, and a value of 0 means do not write to stats.log.
	StatsLogEnable = $statslog

	# Number of days that entries in the stats.log file are kept.  Entries older
	# than this many days will be removed upon running "broctl cron".  A value of 0
	# means that entries never expire.
	StatsLogExpireInterval = $statsduration

	###############################################
	# Other Options

	# Show all output of the broctl status command.  If set to 1, then all output
	# is shown.  If set to 0, then broctl status will not collect or show the peer
	# information (and the command will run faster).
	StatusCmdShowAll = 0

	# Site-specific policy script to load. Bro will look for this in
	# $PREFIX/share/bro/site. A default local.bro comes preinstalled
	# and can be customized as desired.
	SitePolicyStandalone = local.bro

	# Location of the log directory where log files will be archived each rotation
	# interval.
	LogDir = $logdir

	# Location of the spool directory where files and data that are currently being
	# written are stored.
	SpoolDir = /usr/local/spool

	# Location of other configuration files that can be used to customize
	# BroControl operation (e.g. local networks, nodes).
	CfgDir = /usr/local/etc

EOD;

$brocontrolfile =  BRO_ETC . '/broctl.cfg';
conf_mount_rw();
file_put_contents($brocontrolfile, $conf);
chmod($brocontrolfile, 0640);
conf_mount_ro();
bro_sync_on_changes();
bro_restart_services();
}

function 	bro_cluster_resync() {
	// Put your own code to execute on package reconfiguration here
	global $config;

	if (is_array($config['installedpackages']['brocluster'])) {
		$settings = $config['installedpackages']['brocluster']['config'][0];
	} else {
		$settings = array();
	}
	$managerhost = ($settings['manager_host'] ? $settings['manager_host'] : 'localhost');
	$proxyhost = ($settings['proxy_host'] ? $settings['proxy_host'] : 'localhost');
	$worker1host = ($settings['worker1_host'] ? $settings['worker1_host'] : 'localhost');
	$worker2host = ($settings['worker2_host'] ? $settings['worker2_host'] : 'localhost');

	if ($settings['worker1_interface']) {
		$worker1iface =  $settings['worker1_interface'];
	} else {
		$worker1iface = array();
	}
	$iface1 = get_real_interface($worker1iface);

	if ($settings['worker2_interface']) {
		$worker2iface =  $settings['worker2_interface'];
	} else {
		$worker2iface = array();
	}
	$iface2 = get_real_interface($worker2iface);

	if ($iface1 && $iface2) {

		$conf = '';
		$conf .= <<< EOD
		# Example BroControl node configuration.
		#
		# This example has a standalone node ready to go except for possibly changing
		# the sniffing interface.

		# This is a complete standalone configuration.  Most likely you will
		# only need to change the interface.
		#[bro]
		#type=standalone
		#host=localhost
		#interface=eth0

		## Below is an example clustered configuration. If you use this,
		## remove the [bro] node above.

		#[logger]
		#type=logger
		#host=localhost
		#
		[manager]
		type=manager
		host=$managerhost
		#
		[proxy-1]
		type=proxy
		host=$proxyhost
		#
		[worker-1]
		type=worker
		host=$worker1host
		interface=$iface1
		#
		[worker-2]
		type=worker
		host=$worker2host
		interface=$iface2

EOD;

$nodefile =  BRO_ETC . '/node.cfg';
conf_mount_rw();
file_put_contents($nodefile, $conf);
chmod($nodefile, 0640);
conf_mount_ro();
}
bro_sync_on_changes();
bro_restart_services();
}

function 	bro_log_resync() {
	global $config, $brologstate;
	$brologstate = false;

	if (is_array($config['installedpackages']['brolog'])) {
		// check whether bro cluster is enabled ...
		if ($config['installedpackages']['brolog']['config'][0]['remove_bro_log'] == "on"){
			$brologstate = true;
		}
	}
	return $brologstate;
}

function 	bro_script_resync() {
	global $config;

	$conf = '';
	$conf .= <<<EOD
	##! Local site policy. Customize as appropriate.
	##!
	##! This file will not be overwritten when upgrading or reinstalling!

	# This script logs which scripts were loaded during each run.
	@load misc/loaded-scripts

	# Apply the default tuning scripts for common tuning settings.
	@load tuning/defaults

	# Load the scan detection script.
	@load misc/scan

	# Log some information about web applications being used by users
	# on your network.
	@load misc/app-stats

	# Detect traceroute being run on the network.
	@load misc/detect-traceroute

	# Generate notices when vulnerable versions of software are discovered.
	# The default is to only monitor software found in the address space defined
	# as "local".  Refer to the software framework's documentation for more
	# information.
	@load frameworks/software/vulnerable

	# Detect software changing (e.g. attacker installing hacked SSHD).
	@load frameworks/software/version-changes

	# This adds signatures to detect cleartext forward and reverse windows shells.
	@load-sigs frameworks/signatures/detect-windows-shells

	# Load all of the scripts that detect software in various protocols.
	@load protocols/ftp/software
	@load protocols/smtp/software
	@load protocols/ssh/software
	@load protocols/http/software
	# The detect-webapps script could possibly cause performance trouble when
	# running on live traffic.  Enable it cautiously.
	#@load protocols/http/detect-webapps

	# This script detects DNS results pointing toward your Site::local_nets
	# where the name is not part of your local DNS zone and is being hosted
	# externally.  Requires that the Site::local_zones variable is defined.
	@load protocols/dns/detect-external-names

	# Script to detect various activity in FTP sessions.
	@load protocols/ftp/detect

	# Scripts that do asset tracking.
	@load protocols/conn/known-hosts
	@load protocols/conn/known-services
	@load protocols/ssl/known-certs

	# This script enables SSL/TLS certificate validation.
	@load protocols/ssl/validate-certs

	# This script prevents the logging of SSL CA certificates in x509.log
	@load protocols/ssl/log-hostcerts-only

	# Uncomment the following line to check each SSL certificate hash against the ICSI
	# certificate notary service; see http://notary.icsi.berkeley.edu .
		# @load protocols/ssl/notary

		# If you have libGeoIP support built in, do some geographic detections and
		# logging for SSH traffic.
		@load protocols/ssh/geo-data
		# Detect hosts doing SSH bruteforce attacks.
		@load protocols/ssh/detect-bruteforcing
		# Detect logins using "interesting" hostnames.
		@load protocols/ssh/interesting-hostnames

		# Detect SQL injection attacks.
		@load protocols/http/detect-sqli

		#### Network File Handling ####

		# Enable MD5 and SHA1 hashing for all files.
		@load frameworks/files/hash-all-files

		# Detect SHA1 sums in Team Cymru's Malware Hash Registry.
		@load frameworks/files/detect-MHR

		# Uncomment the following line to enable detection of the heartbleed attack. Enabling
		# this might impact performance a bit.
		# @load policy/protocols/ssl/heartbleed

EOD;

$scripts = $config['installedpackages']['broscript']['config'];

if (bro_enabled()){
	if (is_array($scripts) && !empty($scripts)) {
		foreach ($scripts as $script) {

			$path = $script['broscriptpath'];

			$filename = '';
			if ($script['broscriptpath'] == '') {
				$filename = '';
			}else {
				$filename = "@load ".basename($path,".bro")."";
			}

			$test =  escapeshellarg($path);
			$dest = escapeshellarg('/usr/local/share/bro/site');
			exec("/bin/cp $test $dest");
			$newline = "\n";

			$conf .= <<<EOD
			$filename
			$newline
EOD;
}
}

$destpath = '/usr/local/share/bro/site/local.bro';
conf_mount_rw();
file_put_contents($destpath, $conf);
chmod($destpath, 0640);
conf_mount_ro();

}else {
	log_error("You must enabled Bro instance first.");
}
bro_sync_on_changes();
bro_restart_services();
}

/*
* Bro XMLRPC sync
*/

// Uses XMLRPC to synchronize the changes to a remote node
function bro_sync_on_changes() {
	global $config;

	if (is_array($config['installedpackages']['brosync']['config'])) {
		$bro_sync = $config['installedpackages']['brosync']['config'][0];
		$synconchanges = $bro_sync['synconchanges'];
		$synctimeout = $bro_sync['synctimeout'] ?: '250';
		switch ($synconchanges) {
			case "manual":
			if (is_array($bro_sync['row'])) {
				$rs = $bro_sync['row'];
			} else {
				log_error("[bro] XMLRPC sync is enabled but there are no hosts configured as replication targets.");
				return;
			}
			break;
			case "auto":
			if (is_array($config['hasync'])) {
				$system_carp = $config['hasync'];
				$rs[0]['ipaddress'] = $system_carp['synchronizetoip'];
				$rs[0]['username'] = $system_carp['username'];
				$rs[0]['password'] = $system_carp['password'];
				$rs[0]['syncdestinenable'] = FALSE;
				// XMLRPC sync is currently only supported over connections using the same protocol and port as this system
				if ($config['system']['webgui']['protocol'] == "http") {
					$rs[0]['syncprotocol'] = "http";
					$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '80';
				} else {
					$rs[0]['syncprotocol'] = "https";
					$rs[0]['syncport'] = $config['system']['webgui']['port'] ?: '443';
				}
				if ($system_carp['synchronizetoip'] == "") {
					log_error("[bro] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
					return;
				} else {
					$rs[0]['syncdestinenable'] = TRUE;
				}
			} else {
				log_error("[bro] XMLRPC CARP/HA sync is enabled but there are no system backup hosts configured as replication targets.");
				return;
			}
			break;
			default:
			return;
			break;
		}
		if (is_array($rs)) {
			log_error("[bro] XMLRPC sync is starting.");
			foreach ($rs as $sh) {
				// Only sync enabled replication targets
				if ($sh['syncdestinenable']) {
					$sync_to_ip = $sh['ipaddress'];
					$port = $sh['syncport'];
					$username = $sh['username'] ?: 'admin';
					$password = $sh['password'];
					$protocol = $sh['syncprotocol'];

					$error = '';
					$valid = TRUE;

					if ($password == "") {
						$error = "Password parameter is empty. ";
						$valid = FALSE;
					}
					if (!is_ipaddr($sync_to_ip) && !is_hostname($sync_to_ip) && !is_domain($sync_to_ip)) {
						$error .= "Misconfigured Replication Target IP Address or Hostname. ";
						$valid = FALSE;
					}
					if (!is_port($port)) {
						$error .= "Misconfigured Replication Target Port. ";
						$valid = FALSE;
					}
					if ($valid) {
						bro_do_xmlrpc_sync($sync_to_ip, $port, $protocol, $username, $password, $synctimeout);
					} else {
						log_error("[bro] XMLRPC sync with '{$sync_to_ip}' aborted due to the following error(s): {$error}");
					}
				}
			}
			log_error("[bro] XMLRPC sync completed.");
		}
	}
}

// Do the actual XMLRPC sync
function bro_do_xmlrpc_sync($sync_to_ip, $username, $password, $varsyncport, $varsyncprotocol, $varsynctimeout) {
	global $config, $g;

	if ($username == "" || $password == "" || $sync_to_ip == "" || $port == "" || $protocol == "") {
		log_error("[bro] A required XMLRPC sync parameter (username, password, replication target, port or protocol) is empty ... aborting pkg sync");
		return;
	}
	// Take care of IPv6 literal address
	if (is_ipaddrv6($sync_to_ip)) {
		$sync_to_ip = "[{$sync_to_ip}]";
	}
	$url = "{$protocol}://{$sync_to_ip}";
	/* XML will hold the sections to sync */
	$xml = array();
	$xml['bro'] = $config['installedpackages']['bro'];
	$xml['brocontrol'] = $config['installedpackages']['brocontrol'];
	$xml['brocluster'] = $config['installedpackages']['brocluster'];
	$xml['broscript'] = $config['installedpackages']['broscript'];

	/* Assemble XMLRPC payload */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($xml));

	/* Set a few variables needed for sync */
	$method = 'pfsense.merge_installedpackages_section_xmlrpc';
	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	if ($g['debug']) {
		$cli->setDebug(1);
	}
	/* Send our XMLRPC message and timeout after defined sync timeout value*/
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communication error occurred while attempting XMLRPC sync with {$url}:{$port}.";
		log_error("[bro] {$error}");
		file_notice("sync_settings", $error, "bro Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[bro] {$error}");
		file_notice("sync_settings", $error, "bro Settings Sync", "");
	} else {
		log_error("[bro] XMLRPC sync successfully completed with {$url}:{$port}.");
	}

	/* Tell bro to reload our settings on the destination sync host. */
	$method = 'pfsense.exec_php';
	$execcmd = "require_once('/usr/local/pkg/bro.inc');\n";
	$execcmd .= "bro_all_after_XMLRPC_resync();";
	/* Assemble XMLRPC payload */
	$params = array(XML_RPC_encode($password), XML_RPC_encode($execcmd));

	$msg = new XML_RPC_Message($method, $params);
	$cli = new XML_RPC_Client('/xmlrpc.php', $url, $port);
	$cli->setCredentials($username, $password);
	$resp = $cli->send($msg, $synctimeout);
	if (!$resp) {
		$error = "A communication error occurred while attempting XMLRPC sync with {$url}:{$port} (pfsense.exec_php).";
		log_error("[bro] {$error}");
		file_notice("sync_settings", $error, "bro Settings Sync", "");
	} elseif ($resp->faultCode()) {
		$cli->setDebug(1);
		$resp = $cli->send($msg, $synctimeout);
		$error = "An error code was received while attempting XMLRPC sync with {$url}:{$port} - Code " . $resp->faultCode() . ": " . $resp->faultString();
		log_error("[bro] {$error}");
		file_notice("sync_settings", $error, "bro Settings Sync", "");
	} else {
		log_error("[bro] XMLRPC reload data success with {$url}:{$port} (pfsense.exec_php).");
	}
}

function bro_all_after_XMLRPC_resync() {
	bro_broctl_resync();
	bro_cluster_resync();
	bro_script_resync();

	log_error("[bro]: Finished XMLRPC process. It should be OK. For more information look at the host which started sync.");

	exec(BRO_ETC . "/rc.d/bro restart");
}

?>
