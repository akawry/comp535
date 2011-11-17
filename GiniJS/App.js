Ext.Loader.setConfig({
	enabled: true,
	disableCaching: false,
	paths: {
		'GiniJS' : 'app',
		'GiniJS.store' : 'app/model'
	}
});

Ext.application({
	name: 'GiniJS',
	controllers: ['TopologyController', 'ActionController'],
	launch : function(){
		console.log("Launching GiniJS...");
		Ext.create('GiniJS.view.AppView');
	}
});