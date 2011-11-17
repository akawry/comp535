Ext.require([
	'Ext.window.Window',
	'GiniJS.stores.TaskStore'
]);

Ext.define('GiniJS.views.TaskView', {
	extend: 'Ext.window.Window',
	items: [
		Ext.create('Ext.grid.Panel', {
			store: 'GiniJS.stores.TaskStore',
			columns: [{
				id: 'name',
				header: 'Name',
				dataIndex: 'name',
				flex: 1,
				field: {
					allowBlank: false
				}
			}, {
				id: 'process',
				header: 'PID',
				dataIndex: 'process',
				flex: 1,
				field: {
					allowBlank: false
				}
			}, {
				id: 'status',
				header: 'Status',
				dataIndex: 'status',
				flex: 1,
				field: {
					allowBlank: false
				}
			}]
		})
	],
	height: 300
});	 