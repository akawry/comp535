Ext.require([
	'Ext.grid.Panel',
	'GiniJS.store.InterfaceStore'
]);

Ext.define('GiniJS.view.InterfaceView', {
	extend: 'Ext.grid.Panel',
	store: 'GiniJS.store.InterfaceStore',
	columns: [{
		id: 'mac',
		header: 'Mac',
		dataIndex: 'mac',
		flex: 1
	}, {
		id: 'ipv4',
		header: 'IPv4',
		dataIndex: 'ipv4',
		flex: 1
	}, {
		id: 'target',
		header: 'Target',
		dataIndex: 'target',
		flex: 1
	}]
});