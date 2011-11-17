Ext.require([
	'Ext.toolbar.Toolbar',
	'Ext.menu.Menu'
]);

Ext.define('GiniJS.view.Menu', {
	extend: 'Ext.toolbar.Toolbar',
	items: [{
		text: 'File',
		menu: Ext.create('Ext.menu.Menu', {
			type: 'file',
			items: [{
				text: 'New'
			}, {
				text: 'Open'
			}, {
				text: 'Save'
			}, {
				text: 'Save As'
			}, {
				text: 'Send File'
			}, {
				text: 'Export'
			}, {
				text: 'Close'
			}, {
				text: 'Quit'
			}]
		})
	}, {
		text: 'Project',
		menu: Ext.create('Ext.menu.Menu', {
			type: 'project',
			items: [{
				text: 'New'
			}, {
				text: 'Open'
			}, {
				text: 'Close'
			}]
		})
	}, {
		text: 'Edit',
		menu: Ext.create('Ext.menu.Menu', {
			type: 'edit',
			items: [{
				text: 'Copy'
			}, {
				text: 'Arrange'
			}, {
				text: 'Reset Layout'
			}, {
				text: 'Expand Scene'
			}]
		})
	}, {
		text: 'Run',
		menu: Ext.create('Ext.menu.Menu', {
			type: 'run',
			items: [{
				text: 'Compile'
			}, {
				text: 'Run'
			}, {
				text: 'Stop'
			}, {
				text: 'Start Server'
			}]
		})
	}, {
		text: 'Config',
		menu: Ext.create('Ext.menu.Menu', {
			type: 'config',
			items: [{	
				text: 'Options'
			}]
		})
	}, {
		text: 'Help',
		menu: Ext.create('Ext.menu.Menu', {
			type: 'help',
			items: [{
				text: 'Tutorial'
			}, {
				text: 'FAQ'
			}, {
				text: 'About'
			}]
		})
	}]
});