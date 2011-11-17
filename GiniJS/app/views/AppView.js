//Ext.require('GiniJS.views.CanvasView');

Ext.require([
	'Ext.container.Viewport',
	'GiniJS.views.Menu',
	'GiniJS.views.TaskView'
]);

Ext.define('GiniJS.views.AppView', {
	constructor : function(config){
		console.log("Making an app view...");

		this.taskManager = Ext.create('GiniJS.views.TaskView');			
		this.taskManager.show();		
		
		GiniJS.views.AppView.superclass.constructor.call(this, config);
	},
	extend: 'Ext.container.Viewport',	
	plain: true, 
	layout: {
		type: 'border'
	},
	defaults: {
		split: true
	},
	
	items: [{
		region: 'north',
		items: [Ext.create('GiniJS.views.Menu')]
	}, {
		region: 'west',
	}, {
		region: 'south',
	}, {
		region: 'east',
	}, {
		region: 'center',
	}],
	
		
});