Ext.define('GiniJS.models.Interface', {
	extend: 'Ext.data.Model',
	fields: [{
		name: 'mac'
	}, {
		name: 'ipv4',
	}, {
		name: 'target'
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.models.Interface',
	storeId: 'GiniJS.stores.InterfaceStore'
});