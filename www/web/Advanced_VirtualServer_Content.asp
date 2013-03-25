<html>
<head>
<title><% model(2); %></title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<script type="text/javascript" src="overlib.js"></script>
<script type="text/javascript" src="general.js"></script>
</head>  
<body onLoad="load_body()" onunLoad="return unload_body();">
<div id="overDiv" style="position:absolute; visibility:hidden; z-index:1000;"></div>    
<form method="GET" name="form" action="apply.cgi">
<input type="hidden" name="current_page" value="Advanced_VirtualServer_Content.asp">
<input type="hidden" name="next_page" value="Advanced_Exposed_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="sid_list" value="IPConnection;PPPConnection;">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="">
<!-- Table for the conntent page -->	    
<table width="666" border="1" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_header_tr">
<td class="content_header_td_title" colspan="2">NAT Setting - Virtual Server</td>
</tr>
<tr>
<td class="content_desc_td" colspan="2">To make services, like WWW, FTP, provided  by a server in your local network accessible for outside users, you should specify a local IP address to the server. Then, add the IP address and network protocol type, port number, and name of the service in the  following list. Based on the list, the gateway will forward service request from outside users to the corresponding local server.
         </td>
</tr>
<tr>
<td class="content_header_td">Enable Virtual Server?
           </td><td class="content_input_td"><input type="radio" value="1" name="vts_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'vts_enable_x', '1')" <% nvram_match("vts_enable_x", "1", "checked"); %>>Yes</input><input type="radio" value="0" name="vts_enable_x" class="content_input_fd" onClick="return change_common_radio(this, 'IPConnection', 'vts_enable_x', '0')" <% nvram_match("vts_enable_x", "0", "checked"); %>>No</input></td>
</tr>
</table>

<table width="666" border="2" cellpadding="0" cellspacing="0" bordercolor="#E0E0E0">
<tr class="content_list_header_tr">
<td class="content_list_header_td" width="60%" id="VSList">Virtual Server List
         <input type="hidden" name="vts_num_x_0" value="<% nvram_get("vts_num_x"); %>" readonly></td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'VSList', 24, ' Add ');" name="VSList" value="Add" size="12">
</div>
</td><td width="10%">
<div align="center">
<input class="inputSubmit" type="submit" onMouseOut="buttonOut(this)" onMouseOver="buttonOver(this)" onClick="return markGroup(this, 'VSList', 24, ' Del ');" name="VSList" value="Del" size="12">
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
<td></td><td class="content_list_field_header_td" colspan="4">Well-Known Applications:         
         </td><td class="content_list_field_header_td" colspan="3"><select name="KnownApps" class="content_input_fd" onChange="change_widzard(this, 'KnownApps');"><option value="User Defined">User Defined</option></select></td>
</tr>
<tr>
<td></td><td class="content_list_field_header_td" colspan="">Source IP	                
           	        </td><td class="content_list_field_header_td" colspan="">Port Range	                
           	        </td><td class="content_list_field_header_td" colspan="">Local IP	                
           	        </td><td class="content_list_field_header_td" colspan="">Local Port	                
           	        </td><td class="content_list_field_header_td" colspan="">Protocol	                
           	        </td><td class="content_list_field_header_td" colspan="">Protocol No.	                
           	        </td><td class="content_list_field_header_td" colspan="">Description	                
           	        </td><td></td>
</tr>
<tr>
<td></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="vts_sipaddr_x_0" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="11" class="content_input_list_fd" size="10" name="vts_port_x_0" value="" onKeyPress="return is_portrange(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="15" class="content_input_list_fd" size="14" name="vts_ipaddr_x_0" onKeyPress="return is_ipaddr(event, this)" onKeyUp="change_ipaddr(this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="10" size="5" class="content_input_list_fd" name="vts_lport_x_0" onKeyPress="return is_number(event, this)"></td><td class="content_list_input_td" colspan=""><select name="vts_proto_x_0" class="content_input_list_fd"><option value="TCP" <% nvram_match_list("vts_proto_x", "TCP","selected", 0); %>>TCP</option><option value="UDP" <% nvram_match_list("vts_proto_x", "UDP","selected", 0); %>>UDP</option><option value="BOTH" <% nvram_match_list("vts_proto_x", "BOTH","selected", 0); %>>BOTH</option><option value="OTHER" <% nvram_match_list("vts_proto_x", "OTHER","selected", 0); %>>OTHER</option></select></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="5" class="content_list_body" size="10" name="vts_protono_x_0" onKeyPress="return is_number(event, this)"></td><td class="content_list_input_td" colspan=""><input type="text" maxlength="20" size="20" name="vts_desc_x_0" class="content_input_list_fd" onKeyPress="return is_string(event, this)"></td>
</tr>
<tr>
<td></td><td colspan="10"><select size="8" name="VSList_s" multiple="true" class="content_list_body">
<% nvram_get_table("VSList"); %>
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
