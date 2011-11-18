function getNodeByType(type){
	var store = Ext.data.StoreManager.lookup('GiniJS.store.TopologyStore');
	var r = [];
	store.each(function(rec){
		if (rec.get('node').get('type') === type)
			r.push(rec);
	});
	return r;
}

Ext.define('GiniJS.controller.TopologyController', {
	extend: 'Ext.app.Controller',
	id: 'GiniJS.controller.TopologyController',
	init : function(app){
		console.log("Initializing topology ... ");
		this.control ({
			'canvasview' : {
				'insertnode' : this.onInsertNode			
			}
		});
		
	},
	
	onInsertNode : function(ddSource, e, data, canvas){
		console.log("Inserting node ... ", ddSource, e, data, canvas);

		if (!this.canvas){
			this.canvas = canvas;
		}
		
		var store = Ext.data.StoreManager.lookup('GiniJS.store.TopologyStore'),
			 comps = Ext.data.StoreManager.lookup('GiniJS.store.ComponentStore'),
			 mdl = comps.findRecord('type', data.componentData.type);			
		
		var x = e.getX() - canvas.getEl().getX(),
			 y = e.getY() - canvas.getEl().getY(),
			 id = Ext.id();
				
		var node = Ext.create('GiniJS.model.TopologyNode', {
			node: mdl
		});
		
		node.properties().filterOnLoad = false;			
		node.connections().filterOnLoad = false;
		node.interfaces().filterOnLoad = false;	
	
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
	        'click' : this.onNodeClick,
	        scope : this
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
		    text: node.properties().findRecord('property', 'name').get('value'),
		    dismissDelay: 4000
		});		
				
		store.add(node);			
					
	},	
	
	onInsertRouter : function(data){
		console.log("Inserting router ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'Router_' + (getNodeByType("Router").length + 1)
		}], true);
	},
	
	onInsertUML : function(data){
		console.log("Inserting UML ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'UML_' + (getNodeByType("UML").length + 1)
		}], true);
	},
	
	onInsertSwitch : function(data){
		console.log("Inserting Switch ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'Switch_' + (getNodeByType("Switch").length + 1)
		}], true);
	},
	
	onInsertSubnet : function(data){
		console.log("Inserting Subnet ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'Subnet_' + (getNodeByType("Subnet").length + 1)
		}], true);
	},
	
	onInsertFreeDOS : function(data){
		console.log("Inserting Free DOS ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'UML_FreeDOS_' + (getNodeByType("UML_FreeDOS").length + 1)
		}], true);

	},
	
	onInsertMobile : function(data){
		console.log("Inserting Mobile ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'Mobile_' + (getNodeByType("Mobile").length + 1)
		}], true);
	},
	
	onInsertUMLAndroid : function(data){
		console.log("Inserting Android UML ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'UML_Android_' + (getNodeByType("UML_Android").length + 1)
		}], true);
	},
	
	onInsertFirewall : function(data){
		console.log("Inserting Firewall ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'Firewall_' + (getNodeByType("Firewall").length + 1)
		}], true);
	},
	
	onInsertWirelessAccessPoint : function(data){
		console.log("Inserting Wirless Access Point ... ", data);
		data.properties().loadRawData([{
			property: 'name',
			value: 'Wireless_access_point_' + (getNodeByType("Wireless_access_point").length + 1)
		}], true);
	},
	
	onNodeClick : function(node, e, eOpts){
		// handle connections 
		if (e.ctrlKey === true){
			console.log("control was pressed...");
			if (!this.dragStart){
				this.dragStart = node;
				console.log("setting start to", this.dragStart);
			} else if (node != this.dragStart){
				this.onInsertConnection(this.dragStart, node);
				this.dragStart = undefined;
			} else {
				// deselect connection 
				this.dragStart = undefined;
			}
			return false;
		}
		
		// fill the properties view
		Ext.ComponentQuery.query('propertyview')[0].reconfigure(node.model.properties());
	},
	
	onDrawConnection : function(start, end){
		var p = new Ext.XTemplate('M {startx},{starty} L {endx},{endy}').apply({
			startx: start.x + start.width/2,
			starty: start.y + start.height/2,
			endx: end.x + end.width/2,
			endy: end.y + end.height/2
		});
		this.canvas.surface.add({
			type: 'path',
			path: p,
			'stroke-width' : 2,
			'stroke' : '#000000'
		}).show(true);
	},

	
	onInsertConnection : function(start, end){
		console.log("got a connection!!!");
		console.log(start, end);
		var sm = start.model,
			 em = end.model,
			 startType = sm.get('node').get('type'),
			 endType = em.get('node').get('type'),
			 success = false,
			 errorMsg = undefined;

		/**
		 * TODO: Consider the other component types ... 
		 */

		switch (startType){
			case "UML":

				if (endType === "Switch"){
					success = true;			
			
				} else if (endType === "Subnet") {
	
					if (sm.connections().getCount() > 1){
						errorMsg = "UML cannot have more than one connection!";
					} else {
						success = true;
					}
				}
	
				break;
				
			case "Switch":
			
				if (endType === "UML"){
					success = true;
					
				} else if (endType === "Subnet"){
					
					var subnets = false;
					sm.connections().each(function(rec){
						if (rec.get('type') === "Subnet"){
							subnets = true;
						}
					});
					if (subnets === true){
						errorMsg = "Switch cannot have more than one Subnet!";
					} else {
						success = true;
					}
					
				} 
				
				break;
			
			case "Router":
			
				if (endType === "Subnet")
					success = true;
					
				break;
			
			case "Subnet":
				
				if (endType === "Router"){
					success = true;
				} else if (endType === "Switch"){
					var switches = false;
					sm.connections().each(function(rec){
						if (rec.get('type') === "Switch"){
							switches = true;
						}
					});
					if (switches === true){
						errorMsg = "Subnet cannot have more than one Switch!";
					} else {
						success = true;
					}
				} else if (endType === "UML"){
					if (sm.connections().getCount() > 2){
						errorMsg = "Subnet cannot have more than two connections!";
					} else {
						success = true;
					}
				}
				
				break;	
		}
		
		if (success === true){
			sm.properties().loadRawData([{
				property: 'target',
				value: em.properties().findRecord('property', 'name').get('value')
			}], true);
			this.onDrawConnection(start, end);
		} else {
			Ext.Msg.alert("Error", errorMsg || ("Cannot connect " + startType + " and " + endType));
		}
			 
	}
	
});