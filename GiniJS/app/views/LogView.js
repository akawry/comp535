Ext.require('Ext.panel.Panel');

Ext.define('GiniJS.views.LogView', {
	id : "GiniJS.views.LogView",
	constructor: function(config){
		this.logContainerId = Ext.id();
		GiniJS.views.LogView.superclass.constructor.call(this, config);	
	},
	extend: 'Ext.panel.Panel',
	html: new Ext.XTemplate('<div id="{id}" style="font-family: Courier"></div>').apply({
		id: this.logContainerId
	}),
	log : function(args){
		console.log("logging a message:", args.message);
		Ext.get(this.logContainerId).insertHtml("afterEnd", args.message);
	}
});