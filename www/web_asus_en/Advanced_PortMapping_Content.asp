<html>
<head>
<title>ZVMODELVZ Web Manager</title>
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script type="text/javascript" src="overlib.js"></script>
<script type="text/javascript" src="general.js"></script>
</head>  
<body onLoad="load_body()" onunLoad="return unload_body();">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="GET" name="form" action="apply.cgi">
<input type="hidden" name="current_page" value="Advanced_PortMapping_Content.asp">
<input type="hidden" name="next_page" value="Advanced_VirtualServer_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="IPConnection;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<!-- Table for the conntent page -->	    
<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">NAT Setting - Port Mapping</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">
Port Mapping function allows you to redirect certain incoming TCP and UDP ports to specific computer dynamically. These Port Mapping rules will be set or unset immediately each time you press Set(or Unset) button after description of the rule. But, if you like to save these settings permanently, please press Finish button to Save&amp;Restart ZVMODELVZ after you have set your rules. 
</td>
</tr>
</table>

<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="TriggerList">Mapped Port List<input type="hidden" name="IPConnection_TriggerCount_0" value="<% nvram_get("TriggerCount"); %>" readonly></td>
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
<td></td><td class="content_list_field_header_td" colspan="2">Well-Known Applications:</td><td class="content_list_field_header_td" colspan="2"><select name="IPConnection_MappedKnownApps" class="content_input_fd" onChange="change_widzard(this, 'MappedKnownApps');"><option value="User Defined">User Defined</option></select></td>
</tr>
<tr>
<td></td><td class="content_list_field_header_td" colspan="">Local IP</td><td class="content_list_field_header_td" colspan="">Mapped Port</td><td class="content_list_field_header_td" colspan="">Description</td><td class="content_list_field_header_td" colspan="">Action</td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" size="15" name="IPConnection_MappedIP_0" class="content_input_fd" value="<% nvram_get("MappedIP_0"); %>" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_fd"  size="11" name="IPConnection_MappedInPort_0" value="<% nvram_get("MappedInPort_0"); %>" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="24" class="content_input_fd" size="24" name="IPConnection_MappedDescript_0" value="<% nvram_get("MappedDescript_0"); %>"></td><td width="110"><font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return onSubmitApply('0')" type="submit" value="<% nvram_match_both("MappedAction_0", "Set", "Unset", "Set"); %>" name="IPConnection_MappedAction_0"></font></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" size="15" name="IPConnection_MappedIP_1" class="content_input_fd" value="<% nvram_get("MappedIP_1"); %>" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_fd"  size="11" name="IPConnection_MappedInPort_1" value="<% nvram_get("MappedInPort_1"); %>" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="24" class="content_input_fd" size="24" name="IPConnection_MappedDescript_1" value="<% nvram_get("MappedDescript_1"); %>"></td><td width="110"><font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return onSubmitApply('1')" type="submit" value="<% nvram_match_both("MappedAction_1", "Set", "Unset", "Set"); %>" name="IPConnection_MappedAction_1"></font></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" size="15" name="IPConnection_MappedIP_2" class="content_input_fd" value="<% nvram_get("MappedIP_2"); %>" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_fd"  size="11" name="IPConnection_MappedInPort_2" value="<% nvram_get("MappedInPort_2"); %>" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="24" class="content_input_fd" size="24" name="IPConnection_MappedDescript_2" value="<% nvram_get("MappedDescript_2"); %>"></td><td width="110"><font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return onSubmitApply('2')" type="submit" value="<% nvram_match_both("MappedAction_2", "Set", "Unset", "Set"); %>" name="IPConnection_MappedAction_2"></font></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" size="15" name="IPConnection_MappedIP_3" class="content_input_fd" value="<% nvram_get("MappedIP_3"); %>" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_fd"  size="11" name="IPConnection_MappedInPort_3" value="<% nvram_get("MappedInPort_3"); %>" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="24" class="content_input_fd" size="24" name="IPConnection_MappedDescript_3" value="<% nvram_get("MappedDescript_3"); %>"></td><td width="110"><font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return onSubmitApply('3')" type="submit" value="<% nvram_match_both("MappedAction_3", "Set", "Unset", "Set"); %>" name="IPConnection_MappedAction_3"></font></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" size="15" name="IPConnection_MappedIP_4" class="content_input_fd" value="<% nvram_get("MappedIP_4"); %>" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_fd"  size="11" name="IPConnection_MappedInPort_4" value="<% nvram_get("MappedInPort_4"); %>" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="24" class="content_input_fd" size="24" name="IPConnection_MappedDescript_4" value="<% nvram_get("MappedDescript_4"); %>"></td><td width="110"><font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return onSubmitApply('4')" type="submit" value="<% nvram_match_both("MappedAction_4", "Set", "Unset", "Set"); %>" name="IPConnection_MappedAction_4"></font></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" size="15" name="IPConnection_MappedIP_5" class="content_input_fd" value="<% nvram_get("MappedIP_5"); %>" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_fd"  size="11" name="IPConnection_MappedInPort_5" value="<% nvram_get("MappedInPort_5"); %>" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="24" class="content_input_fd" size="24" name="IPConnection_MappedDescript_5" value="<% nvram_get("MappedDescript_5"); %>"></td><td width="110"><font face="Arial"> <input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return onSubmitApply('5')" type="submit" value="<% nvram_match_both("MappedAction_5", "Set", "Unset", "Set"); %>" name="IPConnection_MappedAction_5"></font></td>
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
