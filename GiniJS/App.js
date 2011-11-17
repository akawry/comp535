Ext.Loader.setConfig({
	enabled: true,
	disableCaching: false,
	paths: {
		'GiniJS' : 'app',
		'GiniJS.stores' : 'app/model'
	}
});
Ext.require(['*']);

Ext.ns('GiniJS');
GiniJS.log = function(msg, cmp){
	Ext.getCmp("GiniJS.views.LogView").log({
    	message: msg,
    	origin: cmp
   });
};

Ext.onReady(function() {
	Ext.create('GiniJS.views.AppView');
});