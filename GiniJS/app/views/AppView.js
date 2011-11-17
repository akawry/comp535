//Ext.require('GiniJS.views.CanvasView');

Ext.require([
	'Ext.container.Viewport',
	'GiniJS.views.Menu',
	'GiniJS.views.TaskView',
	'GiniJS.views.PropertyView'
]);

Ext.define('GiniJS.views.AppView', {
	constructor : function(config){
		console.log("Making an app view...");

		this.taskManager = Ext.create('GiniJS.views.TaskView', {
			title: 'Task Manager',
			minWidth: 100,
			width: 300,
		});			
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
		minWidth: 200,
		layout: {
			type: 'vbox',
			align: 'stretch'
		},
		items: [Ext.create('GiniJS.views.PropertyView', {
			flex: 1,
			title: 'Properties',
			minWidth: 200,
			width: 200
		}),
		Ext.create('GiniJS.views.InterfaceView', {
			flex: 1,
			title: 'Interfaces',
			minWidth: 200,
			width: 200
		})]
	}, {
		region: 'center',
	}],
	
		
});