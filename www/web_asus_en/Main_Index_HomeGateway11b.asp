<html>
<head>
<title>ASUS SpaceLink Web Manager</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<meta http-equiv="Content-Type" content="text/html">
<meta http-equiv="Expires" content="Fri, Jun 12 1981 00:00:00 GMT">
<meta http-equiv="Cache-control" content="no-cache,must-revalidate">
<meta http-equiv="Pragma" content="no-cache">
<link rel="stylesheet" type="text/css" href="style.css" media="screen">
<style type="text/css">
<!--
.wounderline {  font-family: "Arial", "Helvetica", "sans-serif"; text-decoration: none; color: #FFFFFF; font-size: 9pt}
-->
</style>
<script src="general.js"></script>
</head>

<body bgcolor="#FFFFFF" onLoad="load_body(); load_wizard();">
<form method="GET" name="form" action="apply.cgi">
<input type="hidden" name="current_page" value="Main_Index_Content.asp">
<input type="hidden" name="next_page" value="Basic_GOperation_Content.asp">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="first_time" value="<% nvram_get_x("General","x_Setting"); %>">
<table border="0" width="68%" cellpadding="0" cellspacing="0">
  <tr height="36">
    <td width="100%"></td>
  </tr>
</table>
<table height="22" width="666">
  <tr>
  <td width="666" align="left" bordercolor="#FFFFFF" bordercolorlight="#FFFFFF" bordercolordark="#FFFFFF" bgcolor="#FFFFFF">  
  <div align="center">
  <center>
  <table width="568" cellspacing="0" cellpadding="0" bordercolor="#C0C0C0" border="0">
  <tr bgcolor="#FFFF00"> 
    <td class="content_desc_td_home"> 
      <p align="center">
      <p align="left" style="color: #660066; font-size: 14pt; margin-top: 0; margin-bottom: 0; margin-left: 25"><b>Wireless Home Gateway</b>    
      <ul>
        <li> 
          <p style="word-spacing: 0; margin-top: 0; margin-bottom: 0">
            <b><a href="javascript:top.openBranch('Quick Setup')">Quick Setup</a></b> allows users to complete basic setting by just answering several questions.</li>     
        <li><b><a href="javascript:top.openBranch('Wireless')">802.11b and WPA</a></b> supports up to 11Mbps transmission rate and interoperable security enhancement.</li>        
        <li><b><a href="javascript:top.openBranch('USB Application')">USB Application</a></b> Plug a USB storage to become a FTP server or plug a USB web camera to monitor your home environment.</li>        
        <li><b><a href="javascript:top.openBranch('NAT Setting')">IP Sharing</a></b> all computers share the same IP to Internet.</li>    
        <li><b><a href="javascript:top.openBranch('Internet Firewall')">Internet Firewall</a></b> protect LAN or Wireless environment through flexible filter rule setting.</li>    
        <li><b><a href="javascript:top.openBranch('Status &amp; Log')">Status &amp; Log</a></b> log status of system in details.</li>
        <p><font face="Arial">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 
        &nbsp;&nbsp;      
        <font style="color: #000000; font-size: 10pt;"><b>Click NEXT to start Quick Setup</b></font>&nbsp; </font><input class=inputSubmit onMouseOut="buttonOut(this)" onMouseOver=buttonOver(this) type="submit" onClick="onSubmitCtrl(this, 'NEXT')" value="Next" name="action">       
        </p>
        <p></p>
      </ul>
    </td>
  </tr>
  <tr bgcolor="#FFFFFF">
    <td width="644" bordercolor="#FFFFFF" bordercolorlight="#FFFFFF" bordercolordark="#FFFFFF" bgcolor="#FFFFFF">
    </td>
  </tr>
  </table>
  </center>
  </div>
  </td>
  </tr>
  <tr><td height="18" align="left" width="630"></td></tr></table>
</form>
</body>
</html>
