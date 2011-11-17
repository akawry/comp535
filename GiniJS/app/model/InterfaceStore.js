Ext.define('GiniJS.model.Interface', {
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
	model: 'GiniJS.model.Interface',
	storeId: 'GiniJS.store.InterfaceStore'
});