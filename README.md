**controll your openVPN server through ubus!**

all information about openVPN management-interface commands: https://openvpn.net/community-resources/management-interface/

**openVPN management-interface wont work on openWRT if openVPN source is build with disabled OPENVPN_openssl_ENABLE_MANAGEMENT** <br>


show existing ubus methods: ubus -v list openvpn.(server_name)
      
**method:** ubus call openvpn.(server_name) dis_client '{"kill":"string"}' <br>
      kill common-name/source addres:port - disconnect a particular client instance. <br> 
      
**method:** ubus call openvpn.(server_name) clients <br>
      get information about connected clients <br>
