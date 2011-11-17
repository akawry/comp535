Ext.require([
	'Ext.draw.Component'
]);

Ext.define('GiniJS.view.CanvasView', {
	extend: 'Ext.draw.Component',
	alias: 'widget.canvasview',
	viewBox: false,
	listeners : {
		'afterrender' : function(){
			if (!this.dropZone){
				var me = this;
				this.dropZone = new Ext.dd.DropTarget(this.getEl().dom, {
					notifyDrop  : function(ddSource, e, data){
						console.log("Received drop: ", ddSource, e, data);
						me.fireEvent('insertnode', ddSource, e, data, me);
						return true;
					}
				});
			}
		}
	}
});
	
