Ext.require([
	'Ext.panel.Panel',
	'Ext.view.View',
	'GiniJS.stores.ComponentStore'
]);

var componentTpl = new Ext.XTemplate(
	'<tpl for=".">',
		'<div class="thumb-wrap" style="padding: 1em;">',
      '<div class="thumb" id="drag-{type}"><img src="{icon}"></div>',
      '<span>{type}</span></div>',
   '</tpl>'
);

Ext.define('GiniJS.views.ComponentView', {
	constructor: function(config){
		this.initDragItems = function(){
			var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
			store.each(function(rec){
				new Ext.dd.DragSource(Ext.get("drag-"+rec.get('type')));
			});
		};
		this.callParent(config);
	},
	extend: 'Ext.panel.Panel',
	layout: {
		type: 'accordion'
	},
	listeners : {
		afterrender : function(){
			var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
			store.clearFilter();
			store.filter(function(rec){
				return rec.get('common') === true;
			});
		}
	},	
	items: [{
		xtype: 'panel',
		title: 'Common',
		listeners : {
			'beforeexpand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('common') === true;
				});
			},
			'expand': function(){
				this.up('panel').initDragItems();
			} 
		},
		items: [Ext.create('Ext.view.View', {
			store: 'GiniJS.stores.ComponentStore',
			tpl: componentTpl,
			itemSelector: 'div.thumb-wrap',
			overItemCls: 'x-item-over',
			multiSelect: false
		})]
	}, {
		xtype: 'panel',
		title: 'Host',
		listeners : {
			'beforeexpand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('category') === "host";
				});
			},
			'expand': function(){
				this.up('panel').initDragItems();
			} 
		},
		items: [Ext.create('Ext.view.View', {
			store: 'GiniJS.stores.ComponentStore',
			tpl: componentTpl,
			itemSelector: 'div.thumb-wrap',
			overItemCls: 'x-item-over',
			multiSelect: false
		})]
	}, {
		xtype: 'panel',
		title: 'Net',
		listeners : {
			'beforeexpand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.stores.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('category') === "net";
				});
			},
			'expand': function(){
				this.up('panel').initDragItems();
			} 
		},
		items: [Ext.create('Ext.view.View', {
			store: 'GiniJS.stores.ComponentStore',
			tpl: componentTpl,
			itemSelector: 'div.thumb-wrap',
			overItemCls: 'x-item-over',
			multiSelect: false
		})]
	}]
});