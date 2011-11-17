Ext.require([
	'Ext.data.Store',
	'Ext.data.Model'
]);

Ext.define('GiniJS.model.TopologyNode', {
	extend: 'Ext.data.Model',
	fields: [{
		name: 'node'
	}, {
		name: 'properties'
	}, {
		name: 'interfaces'
	}, {
		name: 'connections'
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.model.TopologyNode',
	storeId: 'GiniJS.store.TopologyStore'
});