Ext.define('GiniJS.view.InterfaceView', {
	constructor : function(config){
		this.emptyStore = Ext.create('Ext.data.Store', {
			requires: 'GiniJS.model.Interface',
			model: 'GiniJS.model.Interface'
		});
		GiniJS.view.InterfaceView.superclass.constructor.call(this, config);
	},
	extend: 'Ext.grid.Panel',
	store: this.emptyStore,
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