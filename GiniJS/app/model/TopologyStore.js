Ext.require([
	'Ext.data.Store',
	'Ext.data.Model'
]);

Ext.define('GiniJS.models.TopologyNode', {
	extend: 'Ext.data.Model',
	fields: [{
		name: 'kind'
	}, {
		name: 'x'
	}, {
		name: 'y'
	}, {
		name: 'connections'
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.models.TopologyNode',
	storeId: 'GiniJS.models.TopologyNode'
});