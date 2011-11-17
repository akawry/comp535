Ext.require('Ext.toolbar.Toolbar');

Ext.define('GiniJS.views.Menu', {
	extend: 'Ext.toolbar.Toolbar',
	items: [{
		text: 'File'
	}, {
		text: 'Project'
	}, {
		text: 'Edit'
	}, {
		text: 'Run'
	}, {
		text: 'Config'
	}, {
		text: 'Help'
	}]
});