Ext.require([
	'Ext.app.Controller'
]);

Ext.define('GiniJS.controller.TopologyController', {
	extend: 'Ext.app.Controller',
	id: 'GiniJS.controller.TopologyController',
	
	init : function(app, config){
		console.log("Initializing topology ... ", app, config);
		this.control ({
			'canvasview' : {
				'insertnode' : this.onInsertNode			
			}
		});
		
		this.routers = 0;
		this.umls = 0;
		this.switches = 0;
		this.subnets = 0;
		this.freedos = 0;
		this.androids = 0;
		this.firewalls = 0;
		this.wireless = 0;
		this.mobiles = 0;
	},
	
	onInsertNode : function(ddSource, e, data, canvas){
		console.log("Inserting node ... ", ddSource, e, data, canvas);

		var store = Ext.data.StoreManager.lookup('GiniJS.store.TopologyStore'),
			 comps = Ext.data.StoreManager.lookup('GiniJS.store.ComponentStore'),
			 mdl = comps.findRecord('type', data.componentData.type);
		
		var node = Ext.create('GiniJS.model.TopologyNode', {
			node: mdl,
			properties: {},
			interfaces: [],
			connections: []
		});			
		
		var x = e.getX() - canvas.getEl().getX(),
			 y = e.getY() - canvas.getEl().getY(),
			 id = Ext.id();
		
		canvas.surface.add({
			type: 'image',
			x: x,
			y: y,
			id: id,
			width: data.componentData.width,
			height: data.componentData.height,
			src: data.componentData.icon,
			draggable: {
				constrain: true,
				constrainTo: canvas.getEl()
        },
        listeners : {
	        'click' : this.handleNodeClick
	     },
	     model : node
		}).show(true);	 		 
			 
		switch (	data.componentData.type ){
			case "Router":
				this.onInsertRouter(node);
				break;
			case "UML":
				this.onInsertUML(node);
				break;
			case "Switch":
				this.onInsertSwitch(node);
				break;
			case "Subnet":
				this.onInsertSubnet(node);
				break;
			case "UML_FreeDOS":
				this.onInsertFreeDOS(node);
				break;
			case "Mobile":
				this.onInsertMobile(node);
				break;
			case "UML_Android":
				this.onInsertUMLAndroid(node);
				break;
			case "Firewall":
				this.onInsertFirewall(node);
				break;
			case "Wireless_access_point":
				this.onInsertWirelessAccessPoint(node);
				break;
		}
		
		Ext.tip.QuickTipManager.register({
		    target: id,
		    text: node.get('properties')['name'],
		    dismissDelay: 4000
		});		
				
		store.add(node);			
					
	},	
	
	onInsertRouter : function(data){
		console.log("Inserting router ... ", data);
		this.routers += 1;
		var properties = data.get('properties');
		properties['name'] = "Router_"+this.routers; 	
	},
	
	onInsertUML : function(data){
		console.log("Inserting UML ... ", data);
		this.umls += 1;
		var properties = data.get('properties');
		properties['name'] = "UML_"+this.umls;	
	},
	
	onInsertSwitch : function(data){
		console.log("Inserting Switch ... ", data);
		this.switches += 1;
		var properties = data.get('properties');
		properties['name'] = "Switch_"+this.switches;	
	},
	
	onInsertSubnet : function(data){
		console.log("Inserting Subnet ... ", data);
		this.subnets += 1;
		var properties = data.get('properties');
		properties['name'] = "Subnet_"+this.umls;	
	},
	
	onInsertFreeDOS : function(data){
		console.log("Inserting Free DOS ... ", data);
		this.freedos += 1;
		var properties = data.get('properties');
		properties['name'] = "UML_Free_DOS_"+this.freedos;	
	},
	
	onInsertMobile : function(data){
		console.log("Inserting Mobile ... ", data);
		this.mobiles += 1;
		var properties = data.get('properties');
		properties['name'] = "Mobile_"+this.mobiles;	
	},
	
	onInsertUMLAndroid : function(data){
		console.log("Inserting Android UML ... ", data);
		this.androids += 1;
		var properties = data.get('properties');
		properties['name'] = "UML_Android_"+this.androids;	
	},
	
	onInsertFirewall : function(data){
		console.log("Inserting Firewall ... ", data);
		this.firewalls += 1;
		var properties = data.get('properties');
		properties['name'] = "Firewall_"+this.firewalls;	
	},
	
	onInsertWirelessAccessPoint : function(data){
		console.log("Inserting Wirless Access Point ... ", data);
		this.wireless += 1;
		var properties = data.get('properties');
		properties['name'] = "Wireless_access_point_"+this.wireless;	
	},
	
	handleNodeClick : function(node, e, eOpts){
		var mdl = node.model
		var store = Ext.data.StoreManager.lookup('GiniJS.store.PropertyStore');
		store.remove(store.getRange());
		var props = [];
		for (var prop in mdl.get('properties')){
			props.push({
				property: prop,
				value: mdl.get('properties')[prop]
			}); 
		}
		store.loadData(props);
	}
});