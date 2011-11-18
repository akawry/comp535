Ext.define('GiniJS.model.TopologyNode', {
	requires: ['GiniJS.model.Interface', 'GiniJS.model.Property'],
	extend: 'Ext.data.Model',
	fields: [{
		name: 'id'
	}, {
		name: 'node'
	}, {
		name: 'iface'
	}],
	hasMany: [{
		model: 'GiniJS.model.TopologyNode', name: 'connections'
	}, {
		model: 'GiniJS.model.Interface', name: 'interfaces'
	}, {
		model: 'GiniJS.model.Property', name: 'properties'
	}],
	property : function(key){
		var prop = this.properties().findRecord('property', key);
		return Ext.isEmpty(prop) ? prop : prop.get('value');
	},
	setProperty : function(key, value){
		this.properties().loadRawData([{
			property: key,
			value: value
		}], true);
	},
	connectionsWith : function(type){
		var cons = [];
		this.connections().each(function(rec){
			if (rec.get('node').get('type') === type)
				cons.push(rec);
		});
		return cons;
	}
});