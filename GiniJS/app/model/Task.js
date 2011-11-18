Ext.require([
	'Ext.data.Model'
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