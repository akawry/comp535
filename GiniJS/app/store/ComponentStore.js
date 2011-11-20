Ext.define('GiniJS.store.ComponentStore', {
	extend: 'Ext.data.Store',
	requires: 'GiniJS.model.Component',
	model: 'GiniJS.model.Component'
});

Ext.create('GiniJS.store.ComponentStore', {
	storeId: 'GiniJS.store.ComponentStore',
	
	data: [{
		type: 'Router', icon: 'app/resources/images/Router.gif', category: 'net', common: true, id: Ext.id(), width: 51, height: 43
	}, {
		type: 'UML', icon: 'app/resources/images/UML.gif', category: 'host', common: true, id: Ext.id(), width: 61, height: 61
	}, {
		type: 'Switch', icon: 'app/resources/images/Switch.gif', category: 'net', common: true, id: Ext.id(), width: 51, height: 39
	},{
		type: 'Subnet', icon: 'app/resources/images/Subnet.gif', category: 'net', common: true, id: Ext.id(), width: 57, height: 29
	}, {
		type: 'UML_FreeDOS', icon: 'app/resources/images/UML_FreeDOS.gif', category: 'host', common: false, id: Ext.id(), width: 61, height: 61
	}, {
		type: 'Mobile', icon: 'app/resources/images/Mobile.gif', category: 'host', common: false, id: Ext.id(), width: 36, height: 55
	}, {
		type: 'UML_Android', icon: 'app/resources/images/UML_Android.gif', category: 'host', common: false, id: Ext.id(), width: 61, height: 61
	}, {
		type: 'Firewall', icon: 'app/resources/images/Firewall.gif', category: 'net', common: false, id: Ext.id(), width: 38, height: 59
	}, {
		type: 'Wireless_access_point', icon: 'app/resources/images/Wireless_access_point.gif', category: 'net', common: false, id: Ext.id(), width: 42, height: 64
	}]	
});