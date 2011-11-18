Ext.define('GiniJS.view.InterfaceView', {
	constructor : function(config){
		this.emptyStore = Ext.create('Ext.data.Store', {
			requires: 'GiniJS.model.Property',
			model: 'GiniJS.model.Property',
			storeId: 'GiniJS.store.EmptyInterfaces'
		});
		GiniJS.view.InterfaceView.superclass.constructor.call(this, config);
	},
	alias: 'widget.interfaceview',
	extend: 'Ext.grid.Panel',
	store: this.emptyStore,
	columns: [{
		id: 'iface_property',
		header: 'Property',
		dataIndex: 'property',
		flex: 1
	}, {
		id: 'iface_value',
		header: 'Value',
		dataIndex: 'value',
		flex: 1
	}],
	dockedItems: [{
		xtype: 'toolbar',
		dock: 'bottom',
		items: [{
			xtype: 'button',
			text: '<'
		}, {
			xtype: 'tbspacer',
			flex: 1
		}, {
			xtype: 'button',
			text: '>'
		}]
	}]
});