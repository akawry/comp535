Ext.require([
	'Ext.data.Model',
	'Ext.data.Store'
]);

Ext.define('GiniJS.model.Property', {
	extend: 'Ext.data.Model',
	fields: [{
		name: 'property'
	}, {
		name: 'value'
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.model.Property',
	storeId: 'GiniJS.store.PropertyStore'
});