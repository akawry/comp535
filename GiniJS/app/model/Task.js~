Ext.require([
	'Ext.data.Model',
	'Ext.data.Store'
]);

Ext.define('GiniJS.model.Task', {
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
	model: 'GiniJS.model.Task',
	storeId: 'GiniJS.store.TaskStore'
});