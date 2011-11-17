Ext.require([
	'Ext.window.Window',
	'GiniJS.store.TaskStore'
]);

Ext.define('GiniJS.view.TaskView', {
	extend: 'Ext.window.Window',
	items: [
		Ext.create('Ext.grid.Panel', {
			store: 'GiniJS.store.TaskStore',
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