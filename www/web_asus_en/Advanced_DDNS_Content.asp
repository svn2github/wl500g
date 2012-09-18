<html>
<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script type="text/javascript" src="overlib.js"></script>
<script type="text/javascript" src="general.js"></script>
</head>  
<body onLoad="load_body()" onunLoad="return unload_body();">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="GET" name="form" action="apply.cgi">
<input type="hidden" name="current_page" value="Advanced_DDNS_Content.asp">
<input type="hidden" name="next_page" value="SaveRestart.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="LANHostConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<!-- Table for the conntent page -->	    
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">IP Config - Miscellaneous</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('If you enable UPnP, your ZVMODELVZ will be found automatically by systems, such as Windows XP. And it allows these systems to automatically configure ZVMODELVZ for various Internet applications, such as gaming and videoconferencing.', LEFT);" onMouseOut="return nd();">Enable UPnP?</td>
<td class="content_input_td"><select name="upnp_enable" class="content_input_fd" onchange="return change_common(this, 'LANHostConfig', 'upnp_enable')">
<option class="content_input_fd" value="0" <% nvram_match("upnp_enable","0","selected"); %>>No</option>
<option class="content_input_fd" value="1" <% nvram_match("upnp_enable","1","selected"); %>>Yes, report WAN address</option>
<option class="content_input_fd" value="2" <% nvram_match("upnp_enable","2","selected"); %>>Yes, report MAN address</option>
</select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('Choose the port mapping protocol, if you are unsure, enable both.', LEFT);" onMouseOut="return nd();">UPNP Protocol:</td>
<td class="content_input_td"><select name="upnp_proto" class="content_input_fd">
<option class="content_input_fd" value="0" <% nvram_match("upnp_proto","0","selected"); %>>Both</option>
<option class="content_input_fd" value="1" <% nvram_match("upnp_proto","1","selected"); %>>UPNP</option>
<option class="content_input_fd" value="2" <% nvram_match("upnp_proto","2","selected"); %>>NAT-PMP</option>
</select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field indicates the port number used by IPTV proxy. Set to 0 to disable', LEFT);" onMouseOut="return nd();">Multicast to HTTP Proxy Port:</td>
<td class="content_input_td">
	<input type="text" maxlength="5" size="5" name="udpxy_enable_x" class="content_input_fd" value="<% nvram_get("udpxy_enable_x"); %>" onBlur="validate_range(this, 0, 65535)" onKeyPress="return is_number(event, this)"> (0 - disabled)</input>
	<input type="checkbox" style="margin-left:30" name="udpxy_wan_check" value="" onClick="return change_common_radio(this, 'LANHostConfig', 'udpxy_wan', '1')">Enable access from WAN</input>
	<input type="hidden" name="udpxy_wan_x" value="<% nvram_get("udpxy_wan_x"); %>">
</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field enables efficient multicast streams forwarding to reduce the bandwidth utilization and latency', LEFT);" onMouseOut="return nd();">Efficient Multicast Forwarding:</td>
<td class="content_input_td">
	<input type="radio" value="1" name="emf_enable" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'emf_enable', '1')" <% nvram_match("emf_enable", "1", "checked"); %>>Enabled</input>
	<input type="radio" value="0" name="emf_enable" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'emf_enable', '0')" <% nvram_match("emf_enable", "0", "checked"); %>>Disabled</input>
</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This feature allows you to assign a remote server to record log messages of ZVMODELVZ. If you leave it blank, system will record up to 1024 mesages on ZVMODELVZ only.', LEFT);" onMouseOut="return nd();">Remote Log Server:
           </td><td class="content_input_td"><input type="text" maxlength="15" class="content_input_fd" size="15" name="log_ipaddr" value="<% nvram_get("log_ipaddr"); %>" onBlur="return validate_ipaddr(this, 'log_ipaddr')" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field indicates time zone where you are locating in.', LEFT);" onMouseOut="return nd();">Time Zone:
           </td><td class="content_input_td"><select name="TimeZoneList" class="content_input_fd" onChange="return change_common(this, 'LANHostConfig', 'time_zone')"><option class="content_input_fd" value="manual">Manual</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field indicates time zone where you are locating in.', LEFT);" onMouseOut="return nd();">Time Zone Abbreviation:
	   </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="time_zone" value="<% nvram_get("time_zone"); %>" onKeyPress="return is_string(event, this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('The NTP Server used to synchronize your system time.', LEFT);" onMouseOut="return nd();">NTP Server
           </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="ntp_server0" value="<% nvram_get("ntp_server0"); %>" onKeyPress="return is_string(event, this)" onBlur="validate_string(this)"><a href="javascript:openLink('x_NTPServer1')" class="content_input_link" name="x_NTPServer1_link">NTP Link
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field specifies interval between time synchronization requests.', LEFT);" onMouseOut="return nd();">NTP synchronization interval (hours):
           </td><td class="content_input_td"><input type="text" maxlength="3" size="3" name="ntp_interval_x" class="content_input_fd" value="<% nvram_get("ntp_interval_x"); %>" onBlur="validate_range(this, 1, 144)" onKeyPress="return is_number(event, this)"></td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">DDNS Setting
            </td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">Dynamic-DNS (DDNS) allows you to export your server to Internet with an unique name, even though you have no static IP address. Currently, several DDNS clients are embedded in ZVMODELVZ. You can click Free Trial below to start with a free trial account.
         </td>
</tr>
<tr>
<td class="content_header_td">Enable the DDNS Client?
           </td><td class="content_input_td"><input type="radio" value="1" name="ddns_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '1')" <% nvram_match("ddns_enable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="ddns_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_enable_x', '0')" <% nvram_match("ddns_enable_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field determines if dynamic dns service has to autodetect public IP address.', LEFT);" onMouseOut="return nd();">Autodetect public IP address?
           </td><td class="content_input_td">
<select name="ddns_realip_x" class="content_input_fd" onchange="return change_common(this, 'LANHostConfig', 'ddns_realip_x')">
<option class="content_input_fd" value="0" <% nvram_match("ddns_realip_x","0","selected"); %>>Yes</option>
<option class="content_input_fd" value="1" <% nvram_match("ddns_realip_x","1","selected"); %>>No, report WAN address</option>
<option class="content_input_fd" value="2" <% nvram_match("ddns_realip_x","2","selected"); %>>No, report MAN address</option>
</select>
</td>
</tr>
<tr>
<td class="content_header_td">Server:
           </td><td class="content_input_td"><select name="ddns_server_x" class="content_input_fd" onChange="return change_common(this, 'LANHostConfig', 'ddns_server_x')">
		<option class="content_input_fd" value="update@asus.com" <% nvram_match("ddns_server_x", "update@asus.com","selected"); %>>asus.com</option>
		<option class="content_input_fd" value="default@dnsexit.com" <% nvram_match("ddns_server_x", "default@dnsexit.com","selected"); %>>dnsexit.com</option>
		<option class="content_input_fd" value="default@dnsomatic.com" <% nvram_match("ddns_server_x", "default@dnsomatic.com","selected"); %>>dnsomatic.com</option>
		<option class="content_input_fd" value="default@dyndns.org" <% nvram_match("ddns_server_x", "default@dyndns.org","selected"); %>>dyndns.org</option>
		<option class="content_input_fd" value="default@dynsip.org" <% nvram_match("ddns_server_x", "default@dynsip.org","selected"); %>>dynsip.org</option>
		<option class="content_input_fd" value="default@easydns.com" <% nvram_match("ddns_server_x", "default@easydns.com","selected"); %>>easydns.com</option>
		<option class="content_input_fd" value="default@freedns.afraid.org" <% nvram_match("ddns_server_x", "default@freedns.afraid.org","selected"); %>>freedns.afraid.org</option>
		<option class="content_input_fd" value="dyndns@he.net" <% nvram_match("ddns_server_x", "dyndns@he.net","selected"); %>>dns.he.net</option>
		<option class="content_input_fd" value="ipv6tb@he.net" <% nvram_match("ddns_server_x", "ipv6tb@he.net","selected"); %>>tunnelbroker.net</option>
		<option class="content_input_fd" value="default@no-ip.com" <% nvram_match("ddns_server_x", "default@no-ip.com","selected"); %>>no-ip.com</option>
		<option class="content_input_fd" value="default@sitelutions.com" <% nvram_match("ddns_server_x", "default@sitelutions.com","selected"); %>>sitelutions.com</option>
		<option class="content_input_fd" value="default@tzo.com" <% nvram_match("ddns_server_x", "default@tzo.com","selected"); %>>tzo.com</option>
		<option class="content_input_fd" value="default@zoneedit.com" <% nvram_match("ddns_server_x", "default@zoneedit.com","selected"); %>>zoneedit.com</option>
		</select>
		<input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('ddnsregister')" size="12" name="LANHostConfig_x_DDNSSRegister_button" value="Register"></td>
             </a></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field is used as an identity to log in Dynamic-DNS service.', LEFT);" onMouseOut="return nd();">User Name or E-mail Address:
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="ddns_username_x" value="<% nvram_get("ddns_username_x"); %>" onKeyPress="return is_string(event, this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field is used as a password to log in Dynamic-DNS service.', LEFT);" onMouseOut="return nd();">Password or DDNS Key:
           </td><td class="content_input_td"><input type="password" maxlength="64" class="content_input_fd" size="32" name="ddns_passwd_x" value="<% nvram_get("ddns_passwd_x"); %>" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field represents the Host Name you register to Dynamic-DNS service and expect to export to the world.', LEFT);" onMouseOut="return nd();">Host Name:
           </td><td class="content_input_td"><input type="text" maxlength="128" class="content_input_fd" size="32" name="ddns_hostname_x" value="<% nvram_get("ddns_hostname_x"); %>" onKeyPress="return is_string(event, this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field determines if domain name with wildcard is also redirected to your ip address.', LEFT);" onMouseOut="return nd();">Enable wildcard?
           </td><td class="content_input_td"><input type="radio" value="1" name="ddns_wildcard_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '1')" <% nvram_match("ddns_wildcard_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="ddns_wildcard_x" class="content_input_fd" onClick="return change_common_radio(this, 'LANHostConfig', 'ddns_wildcard_x', '0')" <% nvram_match("ddns_wildcard_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This button allows you to update DDNS database manually. It is available only when automatic DDNS update failed. You can get current status of DDNS update from System Log.', LEFT);" onMouseOut="return nd();">Update Manually:
           </td><td class="content_input_td"><input type="hidden" maxlength="15" class="content_input_fd_ro" size="12" name="" value="<% nvram_get("DDNSStatus"); %>"><input type="submit" maxlength="15" class="content_input_fd_ro" onClick="return onSubmitApply('ddnsclient')" size="12" name="LANHostConfig_x_DDNSStatus_button" value="Update"></td>
</tr>
</table>

<% include("footer_buttons.inc"); %>

</form>
</body>
</html>
