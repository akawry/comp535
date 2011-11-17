Ext.require([
	'Ext.app.Controller'
]);

Ext.define('GiniJS.controller.TopologyController', {
	extend: 'Ext.app.Controller',
	id: 'GiniJS.controller.TopologyController',
	init : function(){
		console.log("Initializing topology ... ");
		this.control ({
			'canvasview' : {
				'insertnode' : this.onInsertNode			
			}
		});
	},
	
	onInsertNode : function(ddSource, e, data, canvas){
		console.log("Inserting node ... ", ddSource, e, data, canvas);
		var x = e.getX() - canvas.getEl().getX(),
			 y = e.getY() - canvas.getEl().getY();

		canvas.surface.add({
			type: 'image',
			x: x,
			y: y,
			width: data.componentData.width,
			height: data.componentData.height,
			src: data.componentData.icon
		}).show(true);


		switch (	data.componentData.type ){
			case "Router":
				this.onInsertRouter(data);
				break;
			case "UML":
				this.onInsertUML(data);
				break;
			case "Switch":
				this.onInsertSwitch(data);
				break;
			case "Subnet":
				this.onInsertSubnet(data);
				break;
			case "UML_FreeDOS":
				this.onInsertFreeDOS(data);
				break;
			case "Mobile":
				this.onInsertMobile(data);
				break;
			case "UML_Android":
				this.onInsertUMLAndroid(data);
				break;
			case "Firewall":
				this.onInsertFirewall(data);
				break;
			case "Wireless_access_point":
				this.onInsertWirelessAccessPoint(data);
				break;
		}
					
	},
	
	onInsertRouter : function(data){
		console.log("Inserting router ... ", data);	
	},
	
	onInsertUML : function(data){
		console.log("Inserting UML ... ", data);	
	},
	
	onInsertSwitch : function(data){
		console.log("Inserting Switch ... ", data);	
	},
	
	onInsertSubnet : function(data){
		console.log("Inserting Subnet ... ", data);	
	},
	
	onInsertFreeDOS : function(data){
		console.log("Inserting Free DOS ... ", data);	
	},
	
	onInsertMobile : function(data){
		console.log("Inserting Mobile ... ", data);	
	},
	
	onInsertUMLAndroid : function(data){
		console.log("Inserting Android UML ... ", data);	
	},
	
	onInsertFirewall : function(data){
		console.log("Inserting Firewall ... ", data);	
	},
	
	onInsertWirelessAccessPoint : function(data){
		console.log("Inserting Wirless Access Point ... ", data);	
	},
});