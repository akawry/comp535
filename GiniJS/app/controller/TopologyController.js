Ext.require([
	'Ext.app.Controller'
]);

Ext.define('GiniJS.controller.TopologyController', {
	extend: 'Ext.app.Controller',
	id: 'GiniJS.controller.TopologyController',
	init : function(){
		console.log("Initializing topology ... ");
		this.control ({
			'canvasview' : {
				'insertnode' : this.onInsertNode			
			}
		});
	},
	
	onInsertNode : function(data){
		console.log("Inserting node ... ", data);
	}
});