Ext.require([
	'Ext.grid.Panel',
	'GiniJS.stores.InterfaceStore'
]);

Ext.define('GiniJS.views.InterfaceView', {
	extend: 'Ext.grid.Panel',
	store: 'GiniJS.stores.InterfaceStore',
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