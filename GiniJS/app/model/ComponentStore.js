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
		type: 'router', icon: 'app/resources/images/Router.gif', category: 'net', common: true
	}, {
		type: 'uml', icon: 'app/resources/images/UML.gif', category: 'host', common: true
	}]	
	
});