Ext.require(['Ext.app.Controller']);

Ext.define('GiniJS.controller.ActionController', {
	extend: 'Ext.app.Controller',
	init : function(){
		console.log("Initializing action handlers ... ");
		this.control({
			'menu' : {
				'click' : this.onMenuSelect
			}
		});
	},

	onMenuSelect : function(menu, item, e, eOpts){
		switch(menu.type){
			case "file":
				this.onFileSelect(item);
				break;
			case "project":
				this.onProjectSelect(item);
				break;
			case "edit":
				this.onEditSelect(item);
				break;
			case "run":
				this.onRunSelect(item);
				break;
			case "config":
				this.onConfigSelect(item);
				break;
			case "help":
				this.onHelpSelect(item);
				break;
		}
	},	
	
	onFileSelect : function(data){
		console.log("Item selected from the 'File' menu...", data);
	},
	
	onProjectSelect : function(data){
		console.log("Item selected from the 'Project' menu...", data);
	},
	
	onEditSelect : function(data){
		console.log("Item selected from the 'Edit' menu...", data);
	},
	
	onRunSelect : function(data){
		console.log("Item selected from the 'Run' menu...", data);
	},
	
	onConfigSelect : function(data){
		console.log("Item selected from the 'Config' menu...", data);
	},
	
	onHelpSelect : function(data){
		console.log("Item selected from the 'Help' menu...", data);
	}
});