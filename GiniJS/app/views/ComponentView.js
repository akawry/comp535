Ext.require([
	'Ext.panel.Panel',
	'Ext.view.View',
	'GiniJS.stores.ComponentStore'
]);

var componentTpl = new Ext.XTemplate(
	'<tpl for=".">',
		'<div class="thumb-wrap">',
      '<div class="thumb"><img src="{icon}"></div>',
      '<span>{type}</span></div>',
   '</tpl>'
);

Ext.define('GiniJS.views.ComponentView', {
	extend: 'Ext.panel.Panel',
	layout: {
		type: 'accordion'
	},
	items: [Ext.create('Ext.panel.Panel', {
		title: 'Common',
		store: 'GiniJS.stores.ComponentStore',
		tpl: componentTpl,
		itemSelector: 'div.thumb-wrap',
		overItemCls: 'x-item-over',
		multiSelect: false,
		listeners : {
			'expand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('common') === true;
				});
			}
		}
	}), Ext.create('Ext.panel.Panel', {
		title: 'Host',
		store: 'GiniJS.stores.ComponentStore',
		tpl: componentTpl,
		itemSelector: 'div.thumb-wrap',
		overItemCls: 'x-item-over',
		multiSelect: false,
		listeners : {
			'expand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('category') === "host";
				});
			}
		}
	}), Ext.create('Ext.panel.Panel', {
		title: 'Net',
		store: 'GiniJS.stores.ComponentStore',
		tpl: componentTpl,
		itemSelector: 'div.thumb-wrap',
		overItemCls: 'x-item-over',
		multiSelect: false,
		listeners : {
			'expand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('category') === "net";
				});
			}
		}
	})]
});