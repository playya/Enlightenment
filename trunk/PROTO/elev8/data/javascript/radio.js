#!/usr/local/bin/elev8

var EXPAND_BOTH = { x : 1.0, y : 1.0 };
var FILL_BOTH = { x : -1.0, y : -1.0 };

var logo_icon = {
	type : "icon",
	image : elm.datadir + "data/images/logo_small.png",
};

var logo_icon_unscaled = {
	type : "icon",
	image : elm.datadir + "data/images/logo_small.png",
	scale_up : false,
	scale_down : false,
};

var my_window = new elm.main({
	type : "main",
	label : "Radios demo",
	elements : {
		the_background : {
			type : "background",
			weight : EXPAND_BOTH,
			resize : true,
		},
		radio_box : {
			type : "box",
			weight : EXPAND_BOTH,
			resize : true,
			elements : {
				rdg : {
					type : "radio",
					label : "Icon sized to radio",
					weight : EXPAND_BOTH,
					align : { x : 1.0, y : 0.5 },
					icon : logo_icon,
					value : 0,
				},
				unscaled_radio_icon : {
					type : "radio",
					label : "Icon no scale",
					weight : EXPAND_BOTH,
					align : { x : 1.0, y : 0.5 },
					icon : logo_icon_unscaled,
					value : 1,
					group : "rdg",
				},
				label_only_radio : {
					type : "radio",
					label : "Label Only",
					value : 2,
					group : "rdg",
				},
				disabled_radio : {
					type : "radio",
					label : "Disabled",
					disabled : true,
					value : 3,
					group : "rdg",
				},
				icon_radio : {
					type : "radio",
					icon : logo_icon_unscaled,
					value : 4,
					group : "rdg",
				},
				disabled_icon_radio : {
					type : "radio",
					disabled : true,
					icon : logo_icon_unscaled,
					value : 5,
					group : "rdg",
				},
			},
		},
	},
});

