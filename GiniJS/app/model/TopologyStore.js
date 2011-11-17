Ext.require([
	'Ext.data.Store',
	'Ext.data.Model'
]);

Ext.define('GiniJS.model.TopologyNode', {
	extend: 'Ext.data.Model',
	fields: [{
		name: 'kind'
	}, {
		name: 'x'
	}, {
		name: 'y'
	}, {
		name: 'connections'
	}, {
		name : 'info' // JSON object storing additional info such as ipv4, mac, etc
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.model.TopologyNode',
	storeId: 'GiniJS.model.TopologyNode'
});