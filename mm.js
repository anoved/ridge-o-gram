
// allow images of arbitrary size; scale to fit
// or support output of variable size, as long as input images match

// allow drag and drop image setup - let user drag image files to canvas dropzones; don't need to upload, just display in browser.	

function LoadImage(id, path) {
	
	var canvas = document.getElementById(id);
	var context = canvas.getContext('2d');
	var img = new Image();
	img.onload = function() {
		context.drawImage(img, 0, 0);
	}
	img.src = path;
}

function LoadLeftImage() {
	LoadImage('leftimg', 'face1.jpg');
}

function LoadRightImage() {
	LoadImage('rightimg', 'face2.jpg');
}

function LoadImages() {
	LoadLeftImage();
	LoadRightImage();
}

function DoTheDeed() {
	
	var lum = document.getElementById('lum').value;
	
	var rcanvas = document.getElementById('rightimg');
	var lcanvas = document.getElementById('leftimg');
	
	var rcontext = rcanvas.getContext('2d');
	var lcontext = lcanvas.getContext('2d');
	
	// dimensions should be the same anyway
	var rdata = rcontext.getImageData(0, 0, rcanvas.width, rcanvas.height);
	var ldata = lcontext.getImageData(0, 0, lcanvas.width, lcanvas.height);
	
	var rpixels = rdata.data;
	var lpixels = ldata.data;
	
	var nw = rcanvas.width, nh = rcanvas.height;
	
	var rlum, llum, ron, lon;

	var o; // offset in pixel data array

	// arrays containing triangle vertices for the "on" and "off" pixel models
	var on = [];
	var off = [];

	for (var y = 0; y < nh; y++) {
		for (var x = 0; x < nw; x++) {
			
			o = ((y * nw) + x) * 4;


//	for (var o = 0; o < rpixels.length; o += 4) {
		
		
		
		rlum = (rpixels[o] * 0.3) + (rpixels[o+1] * 0.59) + (rpixels[o+2] * 0.11);
		llum = (lpixels[o] * 0.3) + (lpixels[o+1] * 0.59) + (lpixels[o+2] * 0.11);
		
		if (rlum >= lum) {
			ron = 1;
			rpixels[o] = 255;
			rpixels[o+1] = 255;
			rpixels[o+2] = 255;
		} else {
			ron = 0;
			rpixels[o] = 0;
			rpixels[o+1] = 0;
			rpixels[o+2] = 0;
		}
		
		if (llum >= lum) {
			lon = 1;
			lpixels[o] = 255;
			lpixels[o+1] = 255;
			lpixels[o+2] = 255;
		} else {
			lon = 0;
			lpixels[o] = 0;
			lpixels[o+1] = 0;
			lpixels[o+2] = 0;
		}
		
		if (ron && lon) {
			// both on
			
			on = on.concat(rquad(x, y));
			on = on.concat(lquad(x, y));
			
		}
		else if (!ron && !lon) {
			// both off
			
			off = off.concat(rquad(x, y));
			off = off.concat(lquad(x, y));
		}
		else if (ron && !lon) {
			// right on, left off
			
			on = on.concat(rquad(x, y));
			off = off.concat(lquad(x, y));
		}
		else if (!ron && lon) {
			// right off, left on
			off = off.concat(rquad(x, y));
			on = on.concat(lquad(x, y));
		}
	}
	}
	
	document.getElementById('rightbw').getContext('2d').putImageData(rdata, 0, 0);
	document.getElementById('leftbw').getContext('2d').putImageData(ldata, 0, 0);
	
	var on_stl = vlist2stl(on, "onpixels");
	var off_stl = vlist2stl(off, "offpixels");

	// even for small 100x100 pixel b&w images, the ascii stl can be huge - 
	// hundreds of thousands of lines - which makes it slow to produce.
	// (funny how much faster image processing can actually be!)
	// ideally we could pop these out as downloadable files.
	// - really makes more sense to write this as a command line tool, huh?
	// - but the prospect of easy web integration is tempting.
	
	// looks like a no-can-do there, at least without using flash or some server side support anyway.
	
	// what about throwing the stl at three.js to display? swanky, sure, but still probably slow.

	document.getElementById('onstl').value = on_stl;
	document.getElementById('offstl').value = off_stl;
}

function v2stl(v) {
	// v assumed to be array with 9 elements (xyz xyz xyz vertices)
	return "facet normal 0 0 0\n" +
                        "outer loop\n" +
                        "vertex " + v[0] + " " + v[1] + " " + v[2] + "\n" +
                        "vertex " + v[3] + " " + v[4] + " " + v[5] + "\n" +
                        "vertex " + v[6] + " " + v[7] + " " + v[8] + "\n" +
                        "endloop\n" +
                        "endfacet\n";
}

function vlist2stl(vlist, id) {
	var stl = "solid " + id + "\n";
	for (var i = 0; i < vlist.length; i++) {
		stl += v2stl(vlist[i]);
	}
	stl += "endsolid " + id + "\n";
	return stl;
}

function lquad(x, y) {
	var x0 = x, y0 = y, z0 = 0;
	var x1 = x + 0.5, y1 = y, z1 = 1;
	var x2 = x + 0.5, y2 = y + 1, z2 = 1;
	var x3 = x, y3 = y + 1, z3 = 0;
	var t1 = [x0, y0, z0, x1, y1, z1, x3, y3, z3];
	var t2 = [x1, y1, z1, x2, y2, z2, x3, y3, z3]	
	return [t1, t2];
}

function rquad(x, y) {
	var x0 = x + 0.5, y0 = y + 1, z0 = 1;
	var x1 = x + 0.5, y1 = y, z1 = 1;
	var x2 = x + 1, y2 = y, z2 = 0;
	var x3 = x + 1, y3 = y + 1, z3 = 0;
	var t1 = [x0, y0, z0, x1, y1, z1, x3, y3, z3];
	var t2 = [x1, y1, z1, x2, y2, z2, x3, y3, z3]	
	return [t1, t2];
}

