Ext.define('GiniJS.models.Component', {
	extend: 'Ext.data.Model',
	idProperty: 'type',
	fields: [{
		name: 'type'
	}, {
		name: 'icon'
	}, {
		name: 'category'
	}, {
		name: 'common', type: 'boolean'
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.models.Component',
	storeId: 'GiniJS.stores.ComponentStore',
	
	data: [{
		type: 'Router', icon: 'app/resources/images/Router.gif', category: 'net', common: true
	}, {
		type: 'UML', icon: 'app/resources/images/UML.gif', category: 'host', common: true
	}, {
		type: 'Switch', icon: 'app/resources/images/Switch.gif', category: 'net', common: true
	},{
		type: 'Subnet', icon: 'app/resources/images/Subnet.gif', category: 'net', common: true
	}, {
		type: 'UML_FreeDOS', icon: 'app/resources/images/UML_FreeDOS.gif', category: 'host', common: false
	}, {
		type: 'Mobile', icon: 'app/resources/images/Mobile.gif', category: 'host', common: false
	}, {
		type: 'UML_Android', icon: 'app/resources/images/UML_Android.gif', category: 'host', common: false
	}, {
		type: 'Firewall', icon: 'app/resources/images/Firewall.gif', category: 'net', common: false
	}, {
		type: 'Wireless_access_point', icon: 'app/resources/images/Wireless_access_point.gif', category: 'net', common: false
	}]	
	
});