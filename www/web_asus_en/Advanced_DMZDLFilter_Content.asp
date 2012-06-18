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
<input type="hidden" name="current_page" value="Advanced_DMZDLFilter_Content.asp">
<input type="hidden" name="next_page" value="SaveRestart.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="FirewallConfig;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<!-- Table for the conntent page -->	    
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">Wireless Firewall - WLAN &amp; LAN Filter</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">WLAN vs. LAN filter allows you to block specified packets between WLAN and LAN, if Wireless Firewall is enabled. At first, you can choose the default action for filter in both directions. Then, insert the rules for any exceptions.
         </td>
</tr>
<tr>
<td class="content_header_td">Enable WLAN vs. LAN filter?
           </td><td class="content_input_td"><input type="radio" value="1" name="" class="content_input_fd" onClick="return change_common_radio(this, 'FirewallConfig', '', '1')" <% nvram_match("", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="" class="content_input_fd" onClick="return change_common_radio(this, 'FirewallConfig', '', '0')" <% nvram_match("", "0", "checked"); %>>No</input></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field defines those WLAN to LAN packets which are not specified in WLAN to LAN Filter Table will be accepted or dropped.', LEFT);" onMouseOut="return nd();">Packets(WLAN to LAN) not specified will be:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'FirewallConfig', '')"><option class="content_input_fd" value="DROP" <% nvram_match("", "DROP","selected"); %>>DROP</option><option class="content_input_fd" value="ACCEPT" <% nvram_match("", "ACCEPT","selected"); %>>ACCEPT</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field defines a list of WLAN to LAN ICMP packets type that will be filtered. For example, if you would like to filter Echo(type 8) and Echo Reply(type 0) ICMP packets, you need to enter a string of numbers separated by blank, such as, 0 5.', LEFT);" onMouseOut="return nd();">Filtered ICMP(WLAN to LAN) packet types:
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get(""); %>" onBlur="return validate_portlist(this, '')" onKeyPress="return is_portlist(event, this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field defines those LAN to WLAN packets which are not specified in LAN to WLAN Filter Table will be accepted or dropped.', LEFT);" onMouseOut="return nd();">Packets(LAN to WLAN) not specified will be:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'FirewallConfig', '')"><option class="content_input_fd" value="DROP" <% nvram_match("", "DROP","selected"); %>>DROP</option><option class="content_input_fd" value="ACCEPT" <% nvram_match("", "ACCEPT","selected"); %>>ACCEPT</option></select></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field defines a list of LAN to WLAN ICMP packets type that will be filtered. For example, if you would like to filter Echo(type 8) and Echo Reply(type 0) ICMP packets, you need to enter a string of numbers separated by blank, such as, 0 5.', LEFT);" onMouseOut="return nd();">Filtered ICMP(LAN to WLAN) packet types:
           </td><td class="content_input_td"><input type="text" maxlength="32" class="content_input_fd" size="32" name="" value="<% nvram_get(""); %>" onBlur="return validate_portlist(this, '')" onKeyPress="return is_portlist(event, this)"></td>
</tr>
<tr>
<td class="content_header_td" onMouseOver="return overlib('This field indicates what kind of packets between WLAN and LAN will be logged.', LEFT);" onMouseOut="return nd();">Log type between WLAN and LAN:
           </td><td class="content_input_td"><select name="" class="content_input_fd" onChange="return change_common(this, 'FirewallConfig', '')"><option class="content_input_fd" value="None" <% nvram_match("", "None","selected"); %>>None</option><option class="content_input_fd" value="Dropped" <% nvram_match("", "Dropped","selected"); %>>Dropped</option><option class="content_input_fd" value="Accepted" <% nvram_match("", "Accepted","selected"); %>>Accepted</option><option class="content_input_fd" value="Both" <% nvram_match("", "Both","selected"); %>>Both</option></select></td>
</tr>
</table>

<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="DLFilterList">WLAN to LAN Filter Table
         <input type="hidden" name="DmzLanRuleCount_0" value="<% nvram_get("DmzLanRuleCount"); %>" readonly></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'DLFilterList', 32, ' Add ');" name="DLFilterList" value="Add" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'DLFilterList', 32, ' Del ');" name="DLFilterList" value="Del" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="button" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return openHelp(this, 'FilterHelp');" name="DLFilterList" value="Help" size="12">
</div>
</td><td width="5%"></td>
</tr>
<table class="content_list_table" width="640" border="0" cellspacing="0" cellpadding="0">
<tr>
<td colspan="3" height="10"></td>
</tr>
<tr>
<td colspan="3">
<div align="center">
<table class="content_list_value_table" border="1" cellspacing="0" cellpadding="0">
<tr>
<td></td><td class="content_list_field_header_td" colspan="">Source IP	                
           	        </td><td class="content_list_field_header_td" colspan="">Port Range	                
           	        </td><td class="content_list_field_header_td" colspan="">Destination IP	                
           	        </td><td class="content_list_field_header_td" colspan="">Port Range	                
           	        </td><td class="content_list_field_header_td" colspan="">Protocol	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_iprange(event, this)" onKeyUp="change_iprange(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_list_fd" size="10" name="_0" value="" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_iprange(event, this)" onKeyUp="change_iprange(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_list_fd" size="10" name="_0" value="" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="TCP" <% nvram_match_list("", "TCP","selected", 0); %>>TCP</option><option value="TCP ALL" <% nvram_match_list("", "TCP ALL","selected", 0); %>>TCP ALL</option><option value="TCP SYN" <% nvram_match_list("", "TCP SYN","selected", 0); %>>TCP SYN</option><option value="TCP ACK" <% nvram_match_list("", "TCP ACK","selected", 0); %>>TCP ACK</option><option value="TCP FTN" <% nvram_match_list("", "TCP FTN","selected", 0); %>>TCP FTN</option><option value="TCP RST" <% nvram_match_list("", "TCP RST","selected", 0); %>>TCP RST</option><option value="TCP URG" <% nvram_match_list("", "TCP URG","selected", 0); %>>TCP URG</option><option value="TCP PSH" <% nvram_match_list("", "TCP PSH","selected", 0); %>>TCP PSH</option><option value="UDP" <% nvram_match_list("", "UDP","selected", 0); %>>UDP</option></select></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="DLFilterList_s" multiple="true" style="font-family: 'monospace'; font-size: '8pt'; width: 100%">
<% nvram_get_table("DLFilterList"); %>
</select></td>
</tr>
</table>
</div>
</td>
</tr>
<tr>
<td colspan="3" height="10"></td>
</tr>
</table>
</table>

<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="LDFilterList">LAN to WLAN Filter Table
         <input type="hidden" name="LanDmzRuleCount_0" value="<% nvram_get("LanDmzRuleCount"); %>" readonly></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'LDFilterList', 32, ' Add ');" name="LDFilterList" value="Add" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'LDFilterList', 32, ' Del ');" name="LDFilterList" value="Del" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="button" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return openHelp(this, 'FilterHelp');" name="LDFilterList" value="Help" size="12">
</div>
</td><td width="5%"></td>
</tr>
<table class="content_list_table" width="640" border="0" cellspacing="0" cellpadding="0">
<tr>
<td colspan="3" height="10"></td>
</tr>
<tr>
<td colspan="3">
<div align="center">
<table class="content_list_value_table" border="1" cellspacing="0" cellpadding="0">
<tr>
<td></td><td class="content_list_field_header_td" colspan="">Source IP	                
           	        </td><td class="content_list_field_header_td" colspan="">Port Range	                
           	        </td><td class="content_list_field_header_td" colspan="">Destination IP	                
           	        </td><td class="content_list_field_header_td" colspan="">Port Range	                
           	        </td><td class="content_list_field_header_td" colspan="">Protocol	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_iprange(event, this)" onKeyUp="change_iprange(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_list_fd" size="10" name="_0" value="" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="_0" onKeyPress="return is_iprange(event, this)" onKeyUp="change_iprange(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_list_fd" size="10" name="_0" value="" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><select name="_0" class="content_input_list_fd"><option value="TCP" <% nvram_match_list("", "TCP","selected", 0); %>>TCP</option><option value="TCP ALL" <% nvram_match_list("", "TCP ALL","selected", 0); %>>TCP ALL</option><option value="TCP SYN" <% nvram_match_list("", "TCP SYN","selected", 0); %>>TCP SYN</option><option value="TCP ACK" <% nvram_match_list("", "TCP ACK","selected", 0); %>>TCP ACK</option><option value="TCP FTN" <% nvram_match_list("", "TCP FTN","selected", 0); %>>TCP FTN</option><option value="TCP RST" <% nvram_match_list("", "TCP RST","selected", 0); %>>TCP RST</option><option value="TCP URG" <% nvram_match_list("", "TCP URG","selected", 0); %>>TCP URG</option><option value="TCP PSH" <% nvram_match_list("", "TCP PSH","selected", 0); %>>TCP PSH</option><option value="UDP" <% nvram_match_list("", "UDP","selected", 0); %>>UDP</option></select></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="LDFilterList_s" multiple="true" class="content_list_body">
<% nvram_get_table("LDFilterList"); %>
</select></td>
</tr>
</table>
</div>
</td>
</tr>
<tr>
<td colspan="3" height="10"></td>
</tr>
</table>
</table>

<% include("footer_buttons.inc"); %>

</form>
</body>
</html>
