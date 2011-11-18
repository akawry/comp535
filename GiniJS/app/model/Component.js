Ext.require([
	'Ext.data.Model'
]);

Ext.define('GiniJS.model.Component', {
	extend: 'Ext.data.Model',
	idProperty: 'type',
	fields: [{
		name: 'id'
	}, {
		name: 'type'
	}, {
		name: 'icon'
	}, {
		name: 'category'
	}, {
		name: 'common', type: 'boolean'
	}, {
		name: 'width'
	}, {
		name: 'height'
	}]
});