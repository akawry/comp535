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
			}, 
			'interfaceview > toolbar > button' : {
				'click' : this.onInterfaceChange
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
			src: data.componentData.icon
		
			//TODO : Handle making components draggable ... 
			
			/*draggable: {
				constrain: true,
				constrainTo: canvas.getEl()
        }*/,
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
		data.setProperty('name', 'Router_' + (getNodeByType("Router").length + 1));
	},
	
	onInsertUML : function(data){
		console.log("Inserting UML ... ", data);
		data.setProperty('name', 'UML_' + (getNodeByType("UML").length + 1));
		data.setProperty('filesystem', 'root_fs_beta2');
		data.setProperty('filetype', 'cow');
	},
	
	onInsertSwitch : function(data){
		console.log("Inserting Switch ... ", data);
		data.setProperty('name', 'Switch_' + (getNodeByType("Switch").length + 1));
	},
	
	onInsertSubnet : function(data){
		console.log("Inserting Subnet ... ", data);
		data.setProperty('name', 'Subnet_' + (getNodeByType("Subnet").length + 1));
	},
	
	onInsertFreeDOS : function(data){
		console.log("Inserting Free DOS ... ", data);
		data.setProperty('name', 'UML_FreeDOS_' + (getNodeByType("UML_FreeDOS").length + 1));
	},
	
	onInsertMobile : function(data){
		console.log("Inserting Mobile ... ", data);
		data.setProperty('name', 'Mobile_' + (getNodeByType("Mobile").length + 1));
	},
	
	onInsertUMLAndroid : function(data){
		console.log("Inserting Android UML ... ", data);
		data.setProperty('name', 'UML_Android_' + (getNodeByType("UML_Android").length + 1));
	},
	
	onInsertFirewall : function(data){
		console.log("Inserting Firewall ... ", data);
		data.setProperty('name', 'Firewall_' + (getNodeByType("Firewall").length + 1));
	},
	
	onInsertWirelessAccessPoint : function(data){
		console.log("Inserting Wirless Access Point ... ", data);
		data.setProperty('name', 'Wireless_access_point_' + (getNodeByType("Wireless_access_point").length + 1));
	},
	
	onNodeClick : function(node, e, eOpts){
		// handle connections 
		if (e.ctrlKey === true){
			console.log("control was pressed...");
			if (!this.dragStart){
				this.dragStart = node;
				
				// draw selection box 
				node.selectionBox = Ext.create('Ext.draw.Sprite', {
					type: 'path',
					path: new Ext.XTemplate('M {lx} {ty} L {rx} {ty} M {rx} {ty} L {rx} {by} M {rx} {by} L {lx} {by} M {lx} {by} L {lx} {ty}').apply({
						lx: node.x,
						rx: node.x + node.width,
						ty: node.y,
						by: node.y + node.height
					}),
					'stroke-width' : 1,
					'stroke': "#CDCDCD"
				});				
				
				this.canvas.surface.add(node.selectionBox).show(true);				
				
			} else if (node != this.dragStart){
			
				// remove other selection box
				this.dragStart.selectionBox.destroy();
				this.onInsertConnection(this.dragStart, node);
				this.dragStart = undefined;
			} else {
				this.dragStart.selectionBox.destroy();
				// deselect connection 
				this.dragStart = undefined;
			}
			return false;
		}
		
		// fill the properties view
		Ext.ComponentQuery.query('propertyview')[0].reconfigure(node.model.properties());
		var store = Ext.isEmpty(node.model.get('iface')) ? Ext.data.StoreManager.lookup('GiniJS.store.EmptyInterfaces') : node.model.get('iface').properties();
		Ext.ComponentQuery.query('interfaceview')[0].reconfigure(store);
		
		this.selected = node.model;
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
					
					if (sm.connectionsWith("Subnet").length > 0){
						errorMsg = "Switch cannot have more than one Subnet!";
					} else {
						success = true;
					}
					
				} 
				
				break;
			
			case "Router":
			
				if (endType === "Subnet"){
					if (em.connections().getCount() < 2){
						success = true;
					} else {
						errorMsg = "Subnet cannot have more than two connections!";
					}
				}
				
				break;
			
			case "Subnet":
				
				if (sm.connections().getCount() >= 2){
					errorMsg = "Subnet cannot have more than two connections!";
				} else {
					if (endType === "Router"){
						success = true;
					} else if (endType === "Switch"){
						
						if (sm.connectionsWith("Switch").length > 0){
							errorMsg = "Subnet cannot have more than one Switch!";
						} else {
							success = true;
						}
					} else if (endType === "UML"){
						success = true;
					}
				}
				
				break;	
		}
		
		if (success === true){

			if (sm.connections().indexOf(em) === -1 && em.connections().indexOf(sm) === -1){
				sm.connections().loadRecords([em], {
					addRecords: true
				});
				em.connections().loadRecords([sm], {
					addRecords: true
				});
				
				if (startType === "UML" || startType === "Router"){
					console.log("Adding interface to " + sm.property("name"), sm.interfaces().getCount());
					var iface = Ext.create('GiniJS.model.Interface', {
						id: Ext.id(),
						tid: sm.get('id')
					});
					iface.properties().filterOnLoad = false;
					
					if (startType === "UML"){
						if (endType === "Switch"){
							iface.setProperty('target', em.property('name'));
						} else if (endType === "Subnet"){
							// find other side of connection
							if (em.connections().getCount() > 1){
								iface.setProperty('target', em.otherConnection(sm).property('name'));
							}
						}
					} else if (startType === "Router"){
						if (em.connections().getCount() > 1){
							var other = em.otherConnection(sm);
							iface.setProperty('target', other.property('name'));
							if (other.get('node').get('type') === "UML" || other.get('node').get('type') === "Router"){
								var other_iface = other.emptyInterface();
								other_iface.setProperty('target', sm.property('name'));
								other.set('iface', other_iface);
							}
						}
					}
					
					// TODO: These are allocated dynamically
					iface.setProperty('mac', '');
					iface.setProperty('ipv4', '');
					
					sm.interfaces().loadRecords([iface], {
						addRecords : true
					});
					if (!Ext.isEmpty(iface.property('target'))){
						sm.set('iface', iface);
						console.log("Setting " + sm.property('name')+"'s interface target as "+iface.property('target'));
					}					
				}
				
				if (endType === "Router" || endType === "UML"){
					console.log("Adding interface to "+em.property("name"), em.interfaces().getCount());
					var iface = Ext.create('GiniJS.model.Interface', {
						id: Ext.id(),
						tid: sm.get('id')
					});
					iface.properties().filterOnLoad = false;
					if (startType === "Subnet"){
						if (sm.connections().getCount() > 1){
							var other = sm.otherConnection(em);
							iface.setProperty('target', other.property('name'));
							if (other.get('node').get('type') === "UML" || other.get('node').get('type') === "Router"){
								var other_iface = other.emptyInterface();
								other_iface.setProperty('target', em.property('name'));
								other.set('iface', other_iface);
							}
						}
					} else if (endType === "UML"){
						iface.setProperty('target', sm.property('name'));
					}
					
					// TODO: These are allocated dynamically
					iface.setProperty('mac', '');
					iface.setProperty('ipv4', '');
					
					em.interfaces().loadRecords([iface], {
						addRecords: true
					});
					
					if (!Ext.isEmpty(iface.property('target'))){
						em.set('iface', iface);
						console.log("Setting " + em.property('name')+"'s interface target as "+iface.property('target'));
					}				
				} 	
				
				this.onDrawConnection(start, end);
			}
			
		} else {
			Ext.Msg.alert("Error", errorMsg || ("Cannot connect " + startType + " and " + endType));
		}
			 
	},
	
	onInterfaceChange : function(btn){
		var idx = this.selected.interfaces().indexOf(this.selected.get('iface'));
		if (btn.text === "<"){
			if (idx > 0){
				this.selected.set('iface', this.selected.interfaces().getAt(idx - 1));
				Ext.ComponentQuery.query('interfaceview')[0].reconfigure(this.selected.get('iface').properties());
			}			
		} else if (btn.text === ">"){
			if (idx < this.selected.interfaces().getCount() - 1){
				this.selected.set('iface', this.selected.interfaces().getAt(idx + 1));
				Ext.ComponentQuery.query('interfaceview')[0].reconfigure(this.selected.get('iface').properties());
			}
		}
		 
	}
	
});