Ext.require([
	'Ext.panel.Panel',
	'Ext.view.View',
	'GiniJS.store.ComponentStore'
]);

var componentTpl = new Ext.XTemplate(
	'<tpl for=".">',
		'<div class="thumb-wrap" style="padding: 1em;">',
      '<div class="thumb" class="componentIcon"><img src="{icon}"></div>',
      '<span>{type}</span></div>',
   '</tpl>'
);

Ext.define('GiniJS.view.ComponentView', {
	extend: 'Ext.panel.Panel',
	layout: {
		type: 'accordion'
	},
	listeners : {
		afterrender : function(){
			var store = Ext.data.StoreManager.lookup('GiniJS.store.ComponentStore');
			store.clearFilter();
			store.filter(function(rec){
				return rec.get('common') === true;
			});
			this.getComponent('commonView').fireEvent('expand');
		}
	},	
	items: [{
		xtype: 'panel',
		title: 'Common',
		itemId: 'commonView',
		listeners : {
			'beforeexpand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.store.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('common') === true;
				});
			}
		},
		items: [Ext.create('Ext.view.View', {
			store: 'GiniJS.store.ComponentStore',
			tpl: componentTpl,
			itemSelector: 'div.thumb-wrap',
			overItemCls: 'x-item-over',
			multiSelect: false,
			listeners : {
				'render' : initializeComponentDragZone
			}
		})]
	}, {
		xtype: 'panel',
		title: 'Host',
		itemId: 'hostView',
		listeners : {
			'beforeexpand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.store.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('category') === "host";
				});
			}
		},
		items: [Ext.create('Ext.view.View', {
			store: 'GiniJS.store.ComponentStore',
			tpl: componentTpl,
			itemSelector: 'div.thumb-wrap',
			overItemCls: 'x-item-over',
			multiSelect: false,
			listeners : {
				'render' : initializeComponentDragZone
			}
		})]
	}, {
		xtype: 'panel',
		title: 'Net',
		itemId: 'netView',
		listeners : {
			'beforeexpand' : function(){
				var store = Ext.data.StoreManager.lookup('GiniJS.store.ComponentStore');
				store.clearFilter();
				store.filter(function(rec){
					return rec.get('category') === "net";
				});
			}  
		},
		items: [Ext.create('Ext.view.View', {
			store: 'GiniJS.store.ComponentStore',
			tpl: componentTpl,
			itemSelector: 'div.thumb-wrap',
			overItemCls: 'x-item-over',
			multiSelect: false,
			listeners : {
				'render' : initializeComponentDragZone
			}
		})]
	}]
});

function initializeComponentDragZone(v) {
    v.dragZone = Ext.create('Ext.dd.DragZone', v.getEl(), {
        getDragData: function(e) {
            var sourceEl = e.getTarget(v.itemSelector), d;
            if (sourceEl) {
                d = sourceEl.cloneNode(true);
                d.id = Ext.id();
                return v.dragData = {
                    sourceEl: sourceEl,
                    repairXY: Ext.fly(sourceEl).getXY(),
                    ddel: d,
                    componentData: v.getRecord(sourceEl).data
                };
            }
        },
        
        getRepairXY: function() {
            return this.dragData.repairXY;
        }
    });
};