Ext.require([
	'Ext.data.Model',
	'Ext.data.Store'
]);

Ext.define('GiniJS.models.Property', {
	extend: 'Ext.data.Model',
	fields: [{
		name: 'property'
	}, {
		name: 'value'
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.models.Property',
	storeId: 'GiniJS.stores.PropertyStore'
});