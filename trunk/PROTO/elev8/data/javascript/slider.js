#!/usr/local/bin/elev8

var EXPAND_BOTH = { x : 1.0, y : 1.0 };

var logo_icon = {
	type : "icon",
	image : "data/images/logo_small.png",
};

var win = new elm.main({
	label : "Slider",
	elements : {
		the_background : {
			type : "background",
			resize : true,
			weight : EXPAND_BOTH,
		},
		the_box : {
			type : "pack",
			weight : EXPAND_BOTH,
			resize : true,
			elements : {
				horz_slider : {
					type : "slider",
					label : "Horizontal",
					units : "%1.1f units",
					span : 120,
					align : { x : -1.0, y : 0.5 },
					weight : EXPAND_BOTH,
					icon : logo_icon,
				},
				disabled_slider : {
					type : "slider",
					label : "Disabled",
					units : "%1.1f units",
					span : 120,
					min : 50,
					max : 150,
					value : 80,
					disabled : true,
					align : { x : -1.0, y : 0.5 },
					weight : EXPAND_BOTH,
					icon : logo_icon,
				},
			},
		},
	},
});

print("min   = " + win.elements.the_box.elements.disabled_slider.min);
print("max   = " + win.elements.the_box.elements.disabled_slider.max);
print("value = " + win.elements.the_box.elements.disabled_slider.value);
