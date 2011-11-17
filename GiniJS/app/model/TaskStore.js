Ext.define('GiniJS.models.Task', {
	extend: 'Ext.data.Model',
	fields: [{
		name: 'name',
		type: 'string'
	}, {
		name: 'process',
		type: 'int'
	}, {
		name: 'status',
		type: 'string'
	}]
});

Ext.create('Ext.data.Store', {
	model: 'GiniJS.models.Task',
	storeId: 'GiniJS.stores.TaskStore'
});