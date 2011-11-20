Ext.define('GiniJS.model.Interface', {
	extend: 'Ext.data.Model',
	field: ['id', 'tid'],
	belongsTo: [{
		model: 'GiniJS.model.TopologyNode', foreignKey: 'tid'
	}],
	hasMany: [{
		model: 'GiniJS.model.Property', name: 'properties'
	}],
	property : function(key){
		var prop = this.properties().findRecord('property', key);
		return Ext.isEmpty(prop) ? prop : prop.get('value');
	},
	setProperty : function(key, value){
		var prop = this.properties().findRecord('property', key);
		if (prop){
			prop.set('value', value);
		} else {
			this.properties().loadData([{
				property: key,
				value: value
			}], true);
		}
	}
});