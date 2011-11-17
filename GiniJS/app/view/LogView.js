Ext.require('Ext.panel.Panel');

Ext.define('GiniJS.view.LogView', {
	id : "GiniJS.view.LogView",
	extend: 'Ext.panel.Panel',
	constructor: function(config){
		this.logContainerId = Ext.id();
		GiniJS.view.LogView.superclass.constructor.call(this, config);	
	},
	html: new Ext.XTemplate('<div id="{id}" style="font-family: Courier"></div>').apply({
		id: this.logContainerId
	}),
	log : function(args){
		console.log("logging a message:", args.message);
		Ext.get(this.logContainerId).insertHtml("afterEnd", args.message);
	}
});