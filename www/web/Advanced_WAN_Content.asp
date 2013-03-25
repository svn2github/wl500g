<!--<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">-->
<html>
<head>
	<title><% model(2); %></title>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<link rel="stylesheet" type="text/css" href="style.css" media="screen">

	<script type="text/javascript" src="overlib.js"></script>

	<script type="text/javascript" src="general.js"></script>

</head>
<body onload="load_body()" onunload="return unload_body();">
	<div id="overDiv" style="position: absolute; visibility: hidden; z-index: 1000;">
	</div>
	<form method="GET" name="form" action="apply.cgi">
	<input type="hidden" name="current_page" value="Advanced_WAN_Content.asp">
	<input type="hidden" name="next_page" value="Advanced_DHCP_Content.asp">
	<input type="hidden" name="next_host" value="">
	<input type="hidden" name="sid_list" value="Layer3Forwarding;LANHostConfig;IPConnection;PPPConnection;">
	<input type="hidden" name="group_id" value=""><input type="hidden" name="modified"
		value="0">
	<input type="hidden" name="action_mode" value=""><input type="hidden" name="first_time"
		value="">
	<input type="hidden" name="action_script" value="">
	<input type="hidden" name="wan_pppoe_txonly_x" value="<% nvram_get("wan_pppoe_txonly_x"); %>">

	<input type="hidden" name="dhcp_start" value="<% nvram_get("dhcp_start"); %>">
	<input type="hidden" name="dhcp_end" value="<% nvram_get("dhcp_end"); %>">
	<input type="hidden" name="lan_ipaddr" value="<% nvram_get("lan_ipaddr"); %>">

	<!-- Table for the conntent page -->
				<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
					<tr class="content_header_tr">
						<td class="content_header_td_title" colspan="2">
							IP Config - WAN
						</td>
					</tr>
					<tr>
						<td class="content_desc_td" colspan="2">
							The router supports several connection types to WAN. These types are selected from
							the drop-down menu beside WAN Connection Type. The setting fields will differ depending
							on what kind of connection type you select.
						</td>
					</tr>
					<tr>
						<td class="content_header_td">
							WAN Connection Type:
						</td>
						<td class="content_input_td">
							<select name="wan_proto" class="content_input_fd" onchange="return change_common(this, 'Layer3Forwarding', 'wan_proto')">
								<option class="content_input_fd" value="dhcp" <% nvram_match("wan_proto", "dhcp","selected"); %>>
									Automatic IP</option>
								<option class="content_input_fd" value="static" <% nvram_match("wan_proto", "static","selected"); %>>
									Static IP</option>
								<option class="content_input_fd" value="pppoe" <% nvram_match("wan_proto", "pppoe","selected"); %>>
									PPPoE</option>
								<option class="content_input_fd" value="pptp" <% nvram_match("wan_proto", "pptp","selected"); %>>
									PPTP</option>
								<option class="content_input_fd" value="l2tp" <% nvram_match("wan_proto", "l2tp","selected"); %>>
									L2TP</option>
								<option class="content_input_fd" value="bigpond" <% nvram_match("wan_proto", "bigpond","selected"); %>>
									BigPond</option>
								<option class="content_input_fd" value="wimax" <% nvram_match("wan_proto", "wimax","selected"); %>>
									WiMAX</option>
								<option class="content_input_fd" value="usbmodem" <% nvram_match("wan_proto", "usbmodem","selected"); %>>
									USB Modem</option>
								<option class="content_input_fd" value="usbnet" <% nvram_match("wan_proto", "usbnet","selected"); %>>
									Ethernet over USB</option>
							</select>
						</td>
					</tr>
					<tr>
						<td class="content_header_td">
							WAN Connection Speed:
						</td>
						<td class="content_input_td">
							<select name="wan_etherspeed_x" class="content_input_fd" onchange="return change_common(this, 'Layer3Forwarding', 'wan_etherspeed_x')">
								<option class="content_input_fd" value="auto" <% nvram_match("wan_etherspeed_x", "auto","selected"); %>>
									Auto negotiation</option>
								<option class="content_input_fd" value="10half" <% nvram_match("wan_etherspeed_x", "10half","selected"); %>>
									10Mbps half-duplex</option>
								<option class="content_input_fd" value="10full" <% nvram_match("wan_etherspeed_x", "10full","selected"); %>>
									10Mbps full-duplex</option>
								<option class="content_input_fd" value="100half" <% nvram_match("wan_etherspeed_x", "100half","selected"); %>>
									100Mpbs half-duplex</option>
								<option class="content_input_fd" value="100full" <% nvram_match("wan_etherspeed_x", "100full","selected"); %>>
									100Mpbs full-duplex</option>
							</select>
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('Choose the LAN port to bridge to WAN port. If you have another device to connect to WAN but your ISP only provide one WAN link, you can specify some LAN port to receive packets from WAN port. For example, you can connect your IPTV Set-top box to the specified port and get the signal and IP address from your ISP directly. Do not enable this function if you use a manual configuration of vlan ports', LEFT);"
							onmouseout="return nd();">
							IPTV STB Port:
						</td>
						<td class="content_input_td">
							<select name="wan_stb_x" class="content_input_fd">
								<option class="content_input_fd" value="0" <% nvram_match("wan_stb_x", "0","selected"); %>>None</option>
								<option class="content_input_fd" value="1" <% nvram_match("wan_stb_x", "1","selected"); %>>LAN1</option>
								<option class="content_input_fd" value="2" <% nvram_match("wan_stb_x", "2","selected"); %>>LAN2</option>
								<option class="content_input_fd" value="3" <% nvram_match("wan_stb_x", "3","selected"); %>>LAN3</option>
								<option class="content_input_fd" value="4" <% nvram_match("wan_stb_x", "4","selected"); %>>LAN4</option>
								<option class="content_input_fd" value="5" <% nvram_match("wan_stb_x", "5","selected"); %>>LAN3 & LAN4</option>
							</select>
						</td>
					</tr>

					<tr class="content_section_header_tr">
						<td class="content_section_header_td" colspan="2">
							WAN IP Setting
						</td>
					</tr>
					<tr>
						<td class="content_header_td_less">
							Get IP automatically?
						</td>
						<td class="content_input_td">
							<input type="radio" value="1" name="x_DHCPClient" class="content_input_fd" onclick="changeDHCPClient()">Yes
							<input type="radio" value="0" name="x_DHCPClient" class="content_input_fd" onclick="changeDHCPClient()">No
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This is IP address of the router as seen on the remote network. If you set it to 0.0.0.0, the router will get IP address from DHCP Server automatically.', LEFT);"
							onmouseout="return nd();">
							IP Address:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_ipaddr"
								value="<% nvram_get("wan_ipaddr"); %>" onblur="return validate_ipaddr(this, 'wan_ipaddr')"
								onkeypress="return is_ipaddr(event, this)" onkeyup="change_ipaddr(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This is Subnet Mask of the router as seen on the remote network.', LEFT);"
							onmouseout="return nd();">
							Subnet Mask:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_netmask"
								value="<% nvram_get("wan_netmask"); %>" onblur="return validate_ipaddr(this, 'wan_netmask')"
								onkeypress="return is_ipaddr(event, this)" onkeyup="change_ipaddr(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This is the IP address of default gateway that allows for contact between the router and the remote network or host.', LEFT);"
							onmouseout="return nd();">
							Default Gateway:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_gateway"
								value="<% nvram_get("wan_gateway"); %>" onblur="return validate_ipaddr(this, 'wan_gateway')"
								onkeypress="return is_ipaddr(event, this)" onkeyup="change_ipaddr(this)">
						</td>
					</tr>
					<!--<tr>
<td class="content_header_td" onMouseOver="return overlib('This is the priority of default gateway (1-10).', LEFT);" onMouseOut="return nd();">Priority:</td>
<td class="content_input_td">
<input type="text" maxlength="3" class="content_input_fd" size="3" name="wan_priority" value="<% nvram_get("wan_priority"); %>" onBlur="return validate_range(this, 1, 10)" onKeyPress="return is_number(event, this)"</td>
</tr>-->
					<tr class="content_section_header_tr">
						<td class="content_section_header_td" colspan="2">
							WAN DNS Setting
						</td>
					</tr>
					<tr>
						<td class="content_header_td">
							Get DNS Server automatically?
						</td>
						<td class="content_input_td">
							<input type="radio" value="1" name="wan_dnsenable_x" class="content_input_fd" onclick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', '1')"
								<% nvram_match("wan_dnsenable_x", "1", "checked"); %>>Yes
							<input type="radio" value="0" name="wan_dnsenable_x" class="content_input_fd" onclick="return change_common_radio(this, 'IPConnection', 'wan_dnsenable_x', '0')"
								<% nvram_match("wan_dnsenable_x", "0", "checked"); %>>No
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This field indicates the IP address of DNS that the router contact to.', LEFT);"
							onmouseout="return nd();">
							DNS Server1:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_dns1_x"
								value="<% nvram_get("wan_dns1_x"); %>" onblur="return validate_ipaddr(this, 'wan_dns1_x')"
								onkeypress="return is_ipaddr(event, this)" onkeyup="change_ipaddr(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This field indicates the IP address of DNS that the router contact to.', LEFT);"
							onmouseout="return nd();">
							DNS Server2:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="15" class="content_input_fd" size="15" name="wan_dns2_x"
								value="<% nvram_get("wan_dns2_x"); %>" onblur="return validate_ipaddr(this, 'wan_dns2_x')"
								onkeypress="return is_ipaddr(event, this)" onkeyup="change_ipaddr(this)">
						</td>
					</tr>

					<tr class="content_section_header_tr">
						<td class="content_section_header_td" colspan="2">
							PPPoE, PPTP or L2TP Account
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('Choose the additional authentication, if it is required', LEFT);"
							onmouseout="return nd();">
							Authentication:
						</td>
						<td class="content_input_td">
							<select name="wan_auth_x" class="content_input_fd" onchange="return change_common(document.form.wan_proto, 'Layer3Forwarding', 'wan_proto')">
								<option class="content_input_fd" value="" <% nvram_match("wan_auth_x", "","selected"); %>>None</option>
								<option class="content_input_fd" value="eap-md5" <% nvram_match("wan_auth_x", "eap-md5","selected"); %>>802.1x MD5</option>
								<option class="content_input_fd" value="telenet" <% nvram_match("wan_auth_x", "telenet","selected"); %>>ISP KabiNET</option>
								<option class="content_input_fd" value="convex" <% nvram_match("wan_auth_x", "convex","selected"); %>>ISP Convex</option>
							</select>
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This field is only available, when you set WAN Connection Type as PPPoE.', LEFT);"
							onmouseout="return nd();">
							User Name:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="64" class="content_input_fd" size="32" name="wan_pppoe_username"
								value="<% nvram_get("wan_pppoe_username"); %>" onkeypress="return is_string(event, this)"
								onblur="validate_string(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This field is only available, when you set WAN Connection Type as PPPoE', LEFT);"
							onmouseout="return nd();">
							Password:
						</td>
						<td class="content_input_td">
							<input type="password" maxlength="64" class="content_input_fd" size="32" name="wan_pppoe_passwd"
								value="<% nvram_get("wan_pppoe_passwd"); %>" onblur="validate_string(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This field allows you to configure to terminate your ISP connection after a specified period of time. A value of zero allows infinite idle time. If Tx Only is checked, the data from Internet will be skipped for counting idle time.', LEFT);"
							onmouseout="return nd();">
							Idle Disconnect Time in seconds(option):
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="10" class="content_input_fd" size="10" name="wan_pppoe_idletime"
								value="<% nvram_get("wan_pppoe_idletime"); %>" onblur="validate_range(this, 0, 4294967295)"
								onkeypress="return is_number(event, this)"><input type="checkbox" style="margin-left: 30"
									name="wan_pppoe_idletime_check" value="" onclick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_idletime', '1')">Tx
							Only
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('That is Maximum Transmission Unit(MTU) of PPPoE packet.', LEFT);"
							onmouseout="return nd();">
							MTU:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="5" size="5" name="wan_pppoe_mtu" class="content_input_fd"
								value="<% nvram_get("wan_pppoe_mtu"); %>" onblur="validate_range(this, 576, 1492)"
								onkeypress="return is_number(event, this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('That is Maximum Receive Unit(MRU) of PPPoE packet.', LEFT);"
							onmouseout="return nd();">
							MRU:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="5" size="5" name="wan_pppoe_mru" class="content_input_fd"
								value="<% nvram_get("wan_pppoe_mru"); %>" onblur="validate_range(this, 576, 1492)"
								onkeypress="return is_number(event, this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This item may be specified by some ISPs. Check with your ISP and fill them in if required.', LEFT);"
							onmouseout="return nd();">
							Service Name(option):
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_pppoe_service"
								value="<% nvram_get("wan_pppoe_service"); %>" onkeypress="return is_string(event, this)"
								onblur="validate_string(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This item may be specified by some ISPs. Check with your ISP and fill them in if required.', LEFT);"
							onmouseout="return nd();">
							Access Concentrator Name(option):
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_pppoe_ac"
								value="<% nvram_get("wan_pppoe_ac"); %>" onkeypress="return is_string(event, this)"
								onblur="validate_string(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This item may be specified by some ISPs. Check with your ISP and fill them in if required', LEFT);"
							onmouseout="return nd();">
							PPTP Options:
						</td>
						<td class="content_input_td">
							<select name="wan_pptp_options_x" class="content_input_fd">
								<option class="content_input_fd" value="" <% nvram_match("wan_pptp_options_x", "","selected"); %>>
									None</option>
								<option class="content_input_fd" value="-mppc" <% nvram_match("wan_pptp_options_x", "-mppc","selected"); %>>
									No Encryption</option>
								<option class="content_input_fd" value="+mppe-40" <% nvram_match("wan_pptp_options_x", "+mppe-40","selected"); %>>
									MPPE 40</option>
								<option class="content_input_fd" value="+mppe-56" <% nvram_match("wan_pptp_options_x", "+mppe-56","selected"); %>>
									MPPE 56</option>
								<option class="content_input_fd" value="+mppe-128" <% nvram_match("wan_pptp_options_x", "+mppe-128","selected"); %>>
									MPPE 128</option>
							</select>
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This item may be specified by some ISPs. Check with your ISP and fill them in if required.', LEFT);"
							onmouseout="return nd();">
							Additional pppd options:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="255" class="content_input_fd" size="32" name="wan_pppoe_options_x"
								value="<% nvram_get("wan_pppoe_options_x"); %>" onkeypress="return is_string(event, this)"
								onblur="validate_string(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('Enable PPPoE relay allows stations in LAN to setup individual PPPoE connections that are passthrough NAT.', LEFT);"
							onmouseout="return nd();">
							Enable PPPoE Relay?
						</td>
						<td class="content_input_td">
							<input type="radio" value="1" name="wan_pppoe_relay_x" class="content_input_fd" onclick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_relay_x', '1')"
								<% nvram_match("wan_pppoe_relay_x", "1", "checked"); %>>Yes
							<input type="radio" value="0" name="wan_pppoe_relay_x" class="content_input_fd" onclick="return change_common_radio(this, 'PPPConnection', 'wan_pppoe_relay_x', '0')"
								<% nvram_match("wan_pppoe_relay_x", "0", "checked"); %>>No
						</td>
					</tr>
					<tr class="content_section_header_tr">
						<td class="content_section_header_td" colspan="2">
							Special Requirement from ISP
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This field allows you to provide a host name for the router. It is usually requested by your ISP.', LEFT);"
							onmouseout="return nd();">
							Host Name:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="32" class="content_input_fd" size="32" name="wan_hostname"
								value="<% nvram_get("wan_hostname"); %>" onkeypress="return is_string(event, this)"
								onblur="validate_string(this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td" onmouseover="return overlib('This field allows you to provide a unique MAC address for the router to connect Internet. It is usually requested by your ISP.', LEFT);"
							onmouseout="return nd();">
							MAC Address:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="12" class="content_input_fd" size="12" name="wan_hwaddr_x"
								value="<% nvram_get("wan_hwaddr_x"); %>" onblur="return validate_hwaddr(this)"
								onkeypress="return is_hwaddr(event, this)">
						</td>
					</tr>
					<tr>
						<td class="content_header_td">
							Heart-Beat or PPTP/L2TP (VPN) Server:
						</td>
						<td class="content_input_td">
							<input type="text" maxlength="256" class="content_input_fd" size="32" name="wan_heartbeat_x"
								value="<% nvram_get("wan_heartbeat_x"); %>" onkeypress="return is_string(event, this)"
								onblur="validate_string(this)">
						</td>
					</tr>
				</table>

<% include("footer_buttons.inc"); %>

	</form>
</body>
</html>
