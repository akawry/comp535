Ext.define('GiniJS.store.TopologyStore', {
	extend: 'Ext.data.Store',
	requires: 'GiniJS.model.TopologyNode',
	model: 'GiniJS.model.TopologyNode'
});

Ext.create('GiniJS.store.TopologyStore', {
	storeId: 'GiniJS.store.TopologyStore'
});