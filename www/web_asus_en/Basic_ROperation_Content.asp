<html>
<head>
<title>ZVMODELVZ Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script type="text/javascript" src="overlib.js"></script>
<script type="text/javascript" src="general.js"></script>
<script type="text/javascript" src="quick.js"></script>
</head>
<body bgcolor="#FFFFFF" onLoad="loadQuick()">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>
<form method="GET" name="form" action="apply.cgi">
<input type="hidden" name="x_Mode" value="0">
<input type="hidden" name="current_page" value="Basic_ROperation_Content.asp">
<input type="hidden" name="next_page" value="Basic_HomeGateway_SaveRestart.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="Layer3Forwarding;IPConnection;PPPConnection;WLANConfig11a;WLANConfig11b;LANHostConfig;FirewallConfig;">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="wan_proto" value="<% nvram_get_x("Layer3Forwarding","wan_proto"); %>">
<input type="hidden" name="wan_route_x" value="<% nvram_get_x("IPConnection","wan_route_x"); %>">
<input type="hidden" name="wan_nat_x" value="<% nvram_get_x("IPConnection","wan_nat_x"); %>">
<input type="hidden" name="wl_auth_mode" value="<% nvram_get_x("WLANConfig11b","wl_auth_mode"); %>">
<input type="hidden" name="wl_crypto" value="<% nvram_get_x("WLANConfig11b","wl_crypto"); %>">
<input type="hidden" name="wl_wep_x" value="<% nvram_get_x("WLANConfig11b","wl_wep_x"); %>">
<!-- Table for the conntent page -->
<table width="666" border="0" cellpadding="0" cellspacing="0">
<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr id="Country" class="content_header_tr">
<td class="content_header_td_title" colspan="2">Quick Setup</td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">Select Time Zone</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" height="50">
Please choose the time zone where you are locating in.</td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field indicates time zone where you are locating in.', LEFT);" onMouseOut="return nd();">Time Zone:
           </td><td class="content_input_td"><select name="TimeZoneList" class="content_input_fd" onChange="return change_common(this, 'LANHostConfig', 'time_zone')"><option class="content_input_fd" value="manual">Manual</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field indicates time zone where you are locating in.', LEFT);" onMouseOut="return nd();">Time Zone Abbreviation:
	   </td><td class="content_input_td"><input type="text" maxlength="256" class="content_input_fd" size="32" name="time_zone" value="<% nvram_get_x("LANHostConfig","time_zone"); %>" onKeyPress="return is_string(this)" onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_input_td" colspan="2">
<table>
<tr>
<td width="500" height="100"></td><td>
<div align="center">
<!--<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toPrevTag('Country')" type="hidden" value="Prev" name="action1"></font>&nbsp;&nbsp;-->
<font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toTag('Country')" type="button" value="Next" name="action"></font>
</div>
</td>
</tr>
<tr><td colspan="2" height="360"></td></tr>
</table>
</td>
</tr>
<tr id="Broadband" class="content_header_tr">
<td class="content_header_td_title" colspan="2">Quick Setup</td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">Select Internet Connection Type</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" height="50">
ZVMODELVZ supports several kinds of connection to Internet through its WAN port. Please select connection type you need. In addition, before getting on Internet, please make sure you have connected ZVMODELVZ's WAN port to your DSL or Cable Modem.</td>
</tr>
<tr> 
          <td class="content_header_td_less" onMouseOut="return nd();" colspan="2">          
          <p></p>
          <p><input type="radio" checked name="x_WANType" value="0" class="content_input_fd" onClick="changeWANType()">Cable Modem or other connection type that gets IP automatically.</p>          
          <p><input type="radio" checked name="x_WANType" value="1" class="content_input_fd" onClick="changeWANType()">ADSL or other connection that requires username and password. It is known as PPPoE.</p>          
          <p><input type="radio" checked name="x_WANType" value="2" class="content_input_fd" onClick="changeWANType()">ADSL or other connection that requires username, password and IP address. It is known as PPTP.</p>          
          <p><input type="radio" checked name="x_WANType" value="3" class="content_input_fd" onClick="changeWANType()">ADSL or other connection type that uses static IP address.</p>
          <p><input type="radio" checked name="x_WANType" value="4" class="content_input_fd" onClick="changeWANType()">Telstra BigPond Cable Modem Service.</p>
          <p><input type="radio" checked name="x_WANType" value="5" class="content_input_fd" onClick="changeWANType()">ADSL or other connection that requires username, password and IP address. It is known as L2TP.</p>          
          <p></p>
          <p></p>
          </td>
</tr>
<tr>
<td class="content_input_td" colspan="2">
<table>
<tr>
<td width="444" height="100"></td><td>
<div align="center">
<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toPrevTag('Broadband')" type="button" value="Prev" name="action"></font>&nbsp;&nbsp;
<font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toTag('Broadband')" type="button" value="Next" name="action"></font>
</div>
</td>
</tr>
<tr><td colspan="2" height="360"></td></tr>
</table>
</td>
</tr>
<tr id="PPPoE" class="content_header_tr">
<td class="content_header_td_title" colspan="2">Quick Setup</td>
</tr>
<tr class="content_section_header_tr">
<td  class="content_section_header_td" colspan="2">Set Your Account to ISP</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" height="50">If you apply an account with dynamic IP. You must get user account and password from your ISP. Please fill this data into the following fields carefully. Or, if you apply an ADSL account with static IP, just ignore user name and pasword information.</td>
</tr>
<tr>
<td class="content_header_td_less">User Name:</td><td class="content_input_td"><input type="text" maxlength="64" size="32" name="wan_pppoe_username" class="content_input_fd" value="<% nvram_get_x("PPPConnection","wan_pppoe_username"); %>"></td>
</tr>
<tr>
<td class="content_header_td_less">Password:</td><td class="content_input_td"><input type="password" maxlength="64" size="32" name="wan_pppoe_passwd" class="content_input_fd" value="<% nvram_get_x("PPPConnection","wan_pppoe_passwd"); %>"></td>
</tr>
<tr>
<td class="content_input_td" colspan="2">
<table>
<tr>
<td width="444" height="100"></td>
<td>
<div align="center">
<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toPrevTag('PPPoE')" type="button" value="Prev" name="action"></font>&nbsp;&nbsp;
<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toTag('PPPoE')" type="button" value="Next" name="action"></font>
</div>
</td>
</tr>
<tr><td colspan="2" height="360"></td></tr>
</table>
</td>
</tr>
<tr id="MacHost" class="content_header_tr">
<td class="content_header_td_title" colspan="2">Quick Setup</td>
</tr>
<tr class="content_section_header_tr">
<td  class="content_section_header_td" colspan="2">Fill Information Required by ISP</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" height="50">Your ISP may require the following information to identify your account. If not, just press Next to ignore it.</td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('Please enter the server name or server ip of the authentication server of BigPond service.', LEFT);" onMouseOut="return nd();">Heart-Beat or PPTP/L2TP (VPN) Server:</td><td class="content_input_td"><input type="text" maxlength="12" size="12" name="wan_heartbeat_x" class="content_input_fd" value="<% nvram_get_x("PPPConnection","wan_heartbeat_x"); %>"></td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('This field allows you to provide a host name for ZVMODELVZ. It is usually requested by your ISP.', LEFT);" onMouseOut="return nd();">Host Name:</td><td class="content_input_td"><input type="text" maxlength="32" size="32" name="wan_hostname" class="content_input_fd" value="<% nvram_get_x("PPPConnection","wan_hostname"); %>"></td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('This field allows you to provide a unique MAC address for ZVMODELVZ to connect to Internet. It is usually requested by your ISP.', LEFT);" onMouseOut="return nd();">MAC Address:</td><td class="content_input_td"><input type="text" maxlength="12" size="12" name="wan_hwaddr_x" class="content_input_fd" value="<% nvram_get_x("PPPConnection","wan_hwaddr_x"); %>" onBlur="return validate_hwaddr(this)" onKeyPress="return is_hwaddr()"></td>
</tr>
<tr>
<td class="content_input_td_less" colspan="2">
<table>
<tr>
<td width="444" height="100"></td>
<td>
<div align="center">
<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toPrevTag('MacHost')" type="button" value="Prev" name="action"></font>&nbsp;&nbsp;
<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toTag('MacHost')" type="button" value="Next" name="action"></font>
</div>
</td>
</tr>
<tr><td colspan="2" height="360"></td></tr>
</table>
</td>
</tr>
<tr id="WANSetting" class="content_header_tr">
<td class="content_header_td_title" colspan="2">Quick Setup</td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">WAN IP Setting</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" height="50">Fill TCP/IP setting for ZVMODELVZ to connect to Internet through WAN port.</td>
</tr>
<tr>
<td class="content_header_td_less">Get IP automatically?</td><td class="content_input_td"><input type="radio" value="1" name="x_DHCPClient" class="content_input_fd" onClick="changeDHCPClient()">Yes</input><input type="radio" value="0" name="x_DHCPClient" class="content_input_fd" onClick="changeDHCPClient()">No</input></td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('IP address of WAN Interface. If you leave it blank, ZVMODELVZ will get IP address from DHCP Server automatically.', LEFT);" onMouseOut="return nd();">IP Address:</td><td class="content_input_td"><input type="text" maxlength="15" size="15" name="wan_ipaddr" class="content_input_fd" value="<% nvram_get_x("IPConnection","wan_ipaddr"); %>" onBlur="return validate_ipaddr(this, 'wan_ipaddr')" onKeyPress="return is_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td_less">Subnet Mask:</td><td class="content_input_td"><input type="text" maxlength="15" size="15" name="wan_netmask" class="content_input_fd" value="<% nvram_get_x("IPConnection","wan_netmask"); %>" onBlur="return validate_ipaddr(this)" onKeyPress="return is_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td_less">Default Gateway:</td><td class="content_input_td"><input type="text" maxlength="15" size="15" name="wan_gateway" class="content_input_fd" value="<% nvram_get_x("IPConnection","wan_gateway"); %>" onBlur="return validate_ipaddr(this)" onKeyPress="return is_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td_less">Get DNS Server automatically?</td><td class="content_input_td"><input type="radio" value="1" name="wan_dnsenable_x" class="content_input_fd" onClick="changeDNSServer()" <% nvram_match_x("IPConnection","wan_dnsenable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="wan_dnsenable_x" class="content_input_fd" onClick="changeDNSServer()" <% nvram_match_x("IPConnection","wan_dnsenable_x", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td_less">DNS Server 1:</td><td class="content_input_td"><input type="text" maxlength="15" size="15" name="wan_dns1_x" class="content_input_fd" value="<% nvram_get_x("IPConnection","wan_dns1_x"); %>" onBlur="return validate_ipaddr(this)" onKeyPress="return is_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_header_td_less">DNS Server 2:</td><td class="content_input_td"><input type="text" maxlength="15" size="15" name="wan_dns2_x" class="content_input_fd" value="<% nvram_get_x("IPConnection","wan_dns2_x"); %>" onBlur="return validate_ipaddr(this)" onKeyPress="return is_ipaddr(this)"></td>
</tr>
<tr>
<td class="content_input_td" colspan="2">
<table>
<tr>
<td width="444" height="100"></td><td>
<div align="center">
<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toPrevTag('WANSetting')" type="button" value="Prev" name="action"></font>&nbsp;&nbsp;
<font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toTag('WANSetting')" type="button" value="Next" name="action"></font>
</div>
</td>
</tr>
<tr><td colspan="2" height="360"></td></tr>
</table>
</td>
</tr>

<tr  id="Wireless"  class="content_header_tr">
<td class="content_header_td_title" colspan="2">Quick Setup</td>
</tr>
<tr class="content_section_header_tr">
<td class="content_section_header_td" colspan="2">Configure Wireless Interface</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2" height="50">First step to set your wireless interface is to give it a name, called SSID. In addition, if you would like to protect transmitted data, please select the Security Level and assign a password for authentication and data transmission if it is required.</td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('Assign an identification string, consisting of up to 32 characters, for your WLAN.', LEFT);" onMouseOut="return nd();">SSID:</td><td class="content_input_td"><input type="text" maxlength="32" size="32" name="wl_ssid" class="content_input_fd" value="<% nvram_get_x("WLANConfig11b","wl_ssid"); %>"  onBlur="validate_string(this)"></td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('Selecting Low allows any users to connect to this access point and to transmit data without encryption. Selecting Middle allows only those users use the same WEP key to connect to this access point and to transmit data with WEP encryption. Selecting High allows only those users use the same WPA pre-shared key to connect to this access point and to transmit data with TKIP encryption.', LEFT);" onMouseOut="return nd();">Security Level:</td>
<td class="content_input_td">
 <select name="SecurityLevel" class="content_input_fd" onChange="return change_security(this, 'WLANConfig11b', 0)">
     <option value="0">Low(None)</option>
     <option value="1">Medium(WEP-64bits)</option>
     <option value="2">Medium(WEP-128bits)</option>
     <option value="3">High(WPA-PSK)</option>
 </select>    
</td>
</tr>
<tr>
<td class="content_header_td_less" onMouseOver="return overlib('Selecting High Security Level, this filed will be used as a password to kicks off the TKIP encryption process. A 8~63 characters password is required. Selecting Middle Security Level, this field will be used to generate four WEP keys automatically.', LEFT);" onMouseOut="return nd();">Passphrase:</td><td class="content_input_td"><script type="text/javascript" src="md5.js"></script><input type="password" maxlength="64" size="32" name="wl_wpa_psk" class="content_input_fd"  value="<% nvram_get_x("WLANConfig11b","wl_wpa_psk"); %>" onKeyUp="return is_wlphrase_q('WLANConfig11b', this)" onBlur="return validate_wlphrase_q('WLANConfig11b', this)"></td>
</tr>
<tr>
<td class="content_header_td_less">WEP Key 1 (10 or 26 hex digits):</td><td class="content_input_td"><input type="password" maxlength="32" size="32" name="wl_key1" class="content_input_fd" value="<% nvram_get_x("WLANConfig11b","wl_key1"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>				   
<tr>
<td class="content_header_td_less">WEP Key 2 (10 or 26 hex digits):</td><td class="content_input_td"><input type="password" maxlength="32" size="32" name="wl_key2" class="content_input_fd" value="<% nvram_get_x("WLANConfig11b","wl_key2"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td_less">WEP Key 3 (10 or 26 hex digits):</td><td class="content_input_td"><input type="password" maxlength="32" size="32" name="wl_key3" class="content_input_fd" value="<% nvram_get_x("WLANConfig11b","wl_key3"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td_less">WEP Key 4 (10 or 26 hex digits):</td><td class="content_input_td"><input type="password" maxlength="32" size="32" name="wl_key4" class="content_input_fd" value="<% nvram_get_x("WLANConfig11b","wl_key4"); %>" onBlur="return validate_wlkey(this, 'WLANConfig11b')" onKeyPress="return is_wlkey(this, 'WLANConfig11b')" onKeyUp="return change_wlkey(this, 'WLANConfig11b')"></td>
</tr>
<tr>
<td class="content_header_td_less">Key Index:</td><td class="content_input_td"><select name="wl_key" class="content_input_fd" onChange="return change_common(this, 'WLANConfig11b', 'wl_key')"><option value="1" <% nvram_match_x("WLANConfig11b","wl_key", "1","selected"); %>>1</option><option value="2" <% nvram_match_x("WLANConfig11b","wl_key", "2","selected"); %>>2</option><option value="3" <% nvram_match_x("WLANConfig11b","wl_key", "3","selected"); %>>3</option><option value="4" <% nvram_match_x("WLANConfig11b","wl_key", "4","selected"); %>>4</option></select></td>
</tr>
</table>
</td>
</tr>

<tr>
<td>		
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="#B0B0B0">
<tr bgcolor="#CCCCCC"><td colspan="3"><font face="arial" size="2">&nbsp;</font></td></tr>
<tr bgcolor="#FFFFFF">  
   <td height="25" width="75%">  
   </td>
   <td height="25">  
   <div align="center">
   <font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="toPrevTag('Wireless')" type="button" value="Prev" name="action"></font>&nbsp;&nbsp;&nbsp;&nbsp;
   <font face="Arial"><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" type="submit" value="Finish" name="action"  onClick="saveQuick(this)"></font></div>
   </td>
</tr>
</table>
</td>
</tr>

<tr>
<td>
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="#B0B0B0">
<tr>
    <td colspan="2" width="666" height="25" bgcolor="#FFBB00"></td> 
</tr>                   
</table>
</td>
</tr>
<tr><td colspan="2" height="240"></td></tr>
</table>
</form>
</body>
</html>
