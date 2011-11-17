Ext.Loader.setConfig({
	enabled: true,
	disableCaching: false,
	paths: {
		'GiniJS' : 'app',
		'GiniJS.stores' : 'app/model'
	}
});


Ext.require(['*']);
Ext.onReady(function() {
    Ext.create('GiniJS.views.AppView'); 
});