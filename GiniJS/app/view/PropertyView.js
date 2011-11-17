Ext.require([
	'Ext.grid.Panel',
	'GiniJS.store.PropertyStore'
]);

Ext.define('GiniJS.view.PropertyView', {
	extend: 'Ext.grid.Panel',
	store: 'GiniJS.store.PropertyStore',
	columns: [{
		id: 'property',
		header: 'Property',
		dataIndex: 'property',
		flex: 1
	}, {
		id: 'value',
		header: 'Value',
		dataIndex: 'value',
		flex: 1
	}]
});