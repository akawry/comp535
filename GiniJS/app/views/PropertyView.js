Ext.require([
	'Ext.grid.Panel',
	'GiniJS.stores.PropertyStore'
]);

Ext.define('GiniJS.views.PropertyView', {
	extend: 'Ext.grid.Panel',
	store: 'GiniJS.stores.PropertyStore',
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