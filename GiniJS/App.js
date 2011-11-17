Ext.Loader.setConfig({
	enabled: true,
	disableCaching: false,
	paths: {
		'GiniJS' : 'app',
		'GiniJS.store' : 'app/model'
	}
});

// Init the singleton.  Any tag-based quick tips will start working.
Ext.tip.QuickTipManager.init();

Ext.application({
	name: 'GiniJS',
	views: ['AppView'],
	controllers: ['TopologyController', 'ActionController'],
	launch : function(){
		console.log("Launching GiniJS...");
		Ext.create('GiniJS.view.AppView');
	}
});