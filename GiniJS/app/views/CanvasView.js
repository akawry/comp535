Ext.require([
	'Ext.draw.Component'
]);

Ext.define('GiniJS.views.CanvasView', {
	extend: 'Ext.draw.Component',
	listeners : {
		'afterrender' : function(){
			if (!this.dropZone){
				this.dropZone = new Ext.dd.DropTarget(this.getEl().dom, {
					notifyDrop  : function(ddSource, e, data){
						console.log(ddSource, e, data);
						return true;
					}
				});
			}
		}
	}
});
	
