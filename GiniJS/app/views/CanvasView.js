Ext.require([
	'Ext.draw.Component'
]);

Ext.define('GiniJS.views.CanvasView', {
	extend: 'Ext.draw.Component',
	listeners : {
		'afterrender' : function(){
			if (!this.dropZone){
				this.dropZone = new Ext.dd.DropTarget(this.up('panel').getEl().dom, {
					onNodeDrop : function(target, dd, e, data){
						console.log(target, dd, e, data);
						return true;
					}
				});
			}
		}
	}
});
	
