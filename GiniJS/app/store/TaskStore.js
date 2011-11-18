Ext.define('GiniJS.store.TaskStore', {
	extend: 'Ext.data.Store',
	requires: 'GiniJS.model.Task',
	model: 'GiniJS.model.Task',
});

Ext.create('GiniJS.store.TaskStore', {
	storeId: 'GiniJS.store.TaskStore'
});