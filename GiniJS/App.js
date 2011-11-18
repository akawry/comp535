Ext.Loader.setConfig({
	enabled: true,
	disableCaching: false,
	paths: {
		'GiniJS' : 'app'
	}
});

// Init the singleton.  Any tag-based quick tips will start working.
Ext.tip.QuickTipManager.init();

Ext.application({
	name: 'GiniJS',
	models: ['TopologyNode', 'Component', 'Property', 'Interface', 'Task'],
	views: ['AppView'],
	controllers: ['ViewController', 'TopologyController', 'ActionController'],
	stores: ['TopologyStore', 'TaskStore', 'ComponentStore'],
	launch : function(){
		console.log("Launching GiniJS...");
		Ext.create('GiniJS.view.AppView');
	}
});