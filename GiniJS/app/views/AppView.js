Ext.require([
	'Ext.container.Viewport',
	'GiniJS.views.Menu',
	'GiniJS.views.TaskView',
	'GiniJS.views.PropertyView',
	'GiniJS.views.LogView',
	'GiniJS.views.ComponentView',
	'GiniJS.views.Console'
]);

Ext.define('GiniJS.views.AppView', {
	constructor : function(config){
		this.taskManager = Ext.create('GiniJS.views.TaskView', {
			title: 'Task Manager',
			minWidth: 100,
			width: 300,
		});			
		this.taskManager.show();
		
		this.sampleConsole = Ext.create('GiniJS.views.Console', {
			title: 'Sample Console',
			minWidth: 300,
			minHeight: 300,
			width: 300,
			height: 300
		});
		this.sampleConsole.show();				
		
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
		layout: {
			type: 'fit'
		},
		minWidth: 200,
		width: 200,
		items: [Ext.create('GiniJS.views.ComponentView')]
	}, {
		region: 'south',
		minHeight: 200,
		height: 200,		
		layout: {
			type: 'fit'
		},
		items: [Ext.create('GiniJS.views.LogView', {
			title: 'Log',
			minHeight: 200
		})]
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