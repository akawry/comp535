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
						if (data.componentData){
							me.fireEvent('insertnode', ddSource, e, data, me);
						}
						return true;
					}
				});
				
				this.getEl().on({
					'contextmenu' : function(e){
						e.preventDefault();
						me.fireEvent('rightclick', e);	
					}
				});
			}
		}
	}
});
	
