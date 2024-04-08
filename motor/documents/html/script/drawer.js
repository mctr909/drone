/// <reference path="math.js" />
class Color {
	static BLACK = new Color(0, 0, 0);
	static GRAY33 = new Color(85, 85, 85);
	static GRAY50 = new Color(127, 127, 127);
	static GRAY66 = new Color(170, 170, 170);
	static GRAY75 = new Color(191, 191, 191);
	static WHITE = new Color(255, 255, 255);
	static RED = new Color(211, 0, 0);
	static GREEN = new Color(0, 191, 0);
	static BLUE = new Color(0, 0, 255);
	static CYAN = new Color(0, 191, 191);
	static YELLOW = new Color(211, 211, 0);
	static MAGENTA = new Color(191, 0, 167);
	R = 0;
	G = 0;
	B = 0;
	A = 1;
	constructor(r=0, g=0, b=0, a=1) {
		this.R = r;
		this.G = g;
		this.B = b;
		this.A = a;
	}
}

class Drawer {
	static #FONT_NAME = "Segoe UI";
	static FRAME_RATE = 60;

	/** @type {CanvasRenderingContext2D} */
	#ctx;
	/** @type {HTMLCanvasElement} */
	#element;
	/** @type {vec3} */
	#offset = new vec3();

	/** @param {vec3} offset */
	set Offset(offset) { this.#offset = offset; }

	get Width() { return this.#element.width; }
	get Height() { return this.#element.height; }

	/**
	 * @param {string} canvasId
	 * @param {number} width
	 * @param {number} height
	 */
	constructor(canvasId, width, height) {
		this.#element = document.getElementById(canvasId);
		this.#element.width = width;
		this.#element.height = height;
		this.#ctx = this.#element.getContext("2d");
		this.#ctx.scale(1, 1);

		window.requestNextAnimationFrame = (function () {
			var originalWebkitRequestAnimationFrame = undefined;
			var wrapper = undefined;
			var callback = undefined;
			var self = this;

			// Workaround for Chrome 10 bug where Chrome
			// does not pass the time to the animation function
			if (window.webkitRequestAnimationFrame) {
				// Define the wrapper
				wrapper = function (time) {
					if (time === undefined) {
						time = +new Date();
					}
					self.callback(time);
				};

				// Make the switch
				originalWebkitRequestAnimationFrame = window.webkitRequestAnimationFrame;
				window.webkitRequestAnimationFrame = function (callback, element) {
					self.callback = callback;
					// Browser calls the wrapper and wrapper calls the callback
					originalWebkitRequestAnimationFrame(wrapper, element);
				}
			}

			return window.requestAnimationFrame    ||
				window.webkitRequestAnimationFrame ||
				window.oRequestAnimationFrame      ||
				window.msRequestAnimationFrame     ||
				function (callback, element) {
					var start, finish;
					window.setTimeout( function () {
						start = +new Date();
						callback(start);
						finish = +new Date();
						self.timeout = 1000 / Drawer.FRAME_RATE - (finish - start);
					}, self.timeout);
				}
			;
		})();
	}

	/**
	 * @param {vec3} a
	 * @param {vec3} b
	 * @param {Color} color
	 * @param {number} width
	 */
	drawLine(a, b, color = Color.BLACK, width = 1) {
		this.drawLineXY(a.X, a.Y, b.X, b.Y, color, width);
	}

	/**
	 * @param {number} ax
	 * @param {number} ay
	 * @param {number} bx
	 * @param {number} by
	 * @param {Color} color
	 * @param {number} width
	 * @param {Array<number>} dashPattern
	 */
	drawLineXY(ax, ay, bx, by, color = Color.BLACK, width = 1, dashPattern = []) {
		let x1 = ax + this.#offset.X;
		let y1 = ay + this.#offset.Y;
		let x2 = bx + this.#offset.X;
		let y2 = by + this.#offset.Y;
		this.#ctx.beginPath();
		this.#ctx.strokeStyle = "rgba(" + color.R + "," + color.G + "," + color.B + ",1)";
		this.#ctx.lineWidth = width;
		this.#ctx.moveTo(x1, y1);
		this.#ctx.lineTo(x2, y2);
		this.#ctx.setLineDash(dashPattern);
		this.#ctx.stroke();
	}

	/**
	 * @param {vec3} center
	 * @param {number} radius
	 * @param {Color} color
	 * @param {vec3} ofs
	 */
	fillCircle(center, radius, color = Color.BLACK, ofs=new vec3()) {
		this.fillCircleXY(center.X, center.Y, radius, color, ofs);
	}

	/**
	 * @param {number} x
	 * @param {number} y
	 * @param {number} radius
	 * @param {Color} color
	 * @param {vec3} ofs
	*/
	fillCircleXY(x, y, radius, color = Color.BLACK, ofs=new vec3()) {
		this.#ctx.beginPath();
		this.#ctx.arc(
			x + ofs.X,
			y + ofs.Y,
			radius,
			0 * Math.PI / 180,
			360 * Math.PI / 180,
			false
		);
		this.#ctx.fillStyle = "rgba(" + color.R + "," + color.G + "," + color.B + "," + color.A + ")";
		this.#ctx.fill();
	}

	/**
	 * @param {Array<vec3>} points
	 * @param {vec3} ofs
	 * @param {Color} color
	 */
	fillPolygon(points, ofs = new vec3(), color = Color.BLACK) {
		this.#ctx.beginPath();
		this.#ctx.moveTo(points[0].X + ofs.X, points[0].Y + ofs.Y);
		for (let i=1; i<points.length; i++) {
			this.#ctx.lineTo(points[i].X + ofs.X, points[i].Y + ofs.Y);
		}
		this.#ctx.fillStyle = "rgba(" + color.R + "," + color.G + "," + color.B + "," + color.A + ")";
		this.#ctx.fill();
	}

	/**
	 * @param {vec3} p
	 * @param {string} value
	 * @param {number} size
	 * @param {Color} color
	 */
	drawString(p, value, size = 11, color = Color.BLACK) {
		this.#ctx.font = size + "px " + Drawer.#FONT_NAME;
		this.#ctx.fillStyle = "rgba(" + color.R + "," + color.G + "," + color.B + ",1)" ;
		let px = p.X + this.#offset.X;
		let py = p.Y + this.#offset.Y;
		let lines = value.split("\n");
		for(let i=0; i<lines.length; i++) {
			this.#ctx.fillText(lines[i], px, py);
			py += size;
		}
	}

	/**
	 * @param {vec3} p
	 * @param {string} value
	 * @param {number} size
	 * @param {Color} color
	 */
	drawStringC(p, value, size = 11, color = Color.BLACK) {
		this.#ctx.font = size + "px " + Drawer.#FONT_NAME;
		this.#ctx.fillStyle = "rgba(" + color.R + "," + color.G + "," + color.B + ",1)" ;
		var met = this.#ctx.measureText(value);
		this.#ctx.fillText(value,
			p.X + this.#offset.X - met.width / 2,
			p.Y + this.#offset.Y + (size - 4) / 2);
	}

	clear() {
		var w = this.#element.width;
		var h = this.#element.height;
		this.#ctx.clearRect(0, 0, w, h);
		this.drawLine(new vec3(0, 0), new vec3(w, 0)); // Top
		this.drawLine(new vec3(w, 0), new vec3(w, h)); // Right
		this.drawLine(new vec3(0, h), new vec3(w, h)); // Bottom
		this.drawLine(new vec3(0, 0), new vec3(0, h)); // Left
	}

	/**
	 * @param {number} value (-1～1)
	 * @returns {Color}
	 */
	static ToHue(value) {
		if (1 < value) value = 1;
		if (value < -1) value = -1;
		value = 2 * value / (value*value+1);
		value = parseInt(1023*(value/2+0.5));

		let r, g, b;
		if(value < 256) {
			r = 0;
			g = value;
			b = 255;
		} else if(value < 512) {
			value -= 256;
			r = 0;
			g = 255;
			b = 255-value;
		} else if(value < 768) {
			value -= 512;
			r = value;
			g = 255;
			b = 0;
		} else {
			value -= 768;
			r = 255;
			g = 255 - value;
			b = 0;
		}
		return new Color(r, g, b);
	}
}

class SelectElement {
	name = "";
	value = null;
	selected = false;
	constructor(name, value, selected=false) {
		this.name = name;
		this.value = value;
		this.selected = selected;
	}
}

/**
 * @param {HTMLSelectElement} elm 
 * @param {Array<SelectElement>} list 
 */
function initSelectList(elm, list) {
	elm.innerHTML = "";
	for(let i=0; i<list.length; i++) {
		elm.innerHTML += "<option>" + list[i].name + "</option>";
	}
	for(let i=0; i<list.length; i++) {
		if (list[i].selected) {
			elm.selectedIndex = i;
		}
	}
}

/**
 * @param {HTMLButtonElement} elm 
 * @param {*} func 
 */
function addClickEvent(elm, func) {
	elm.addEventListener("click", (e) => { func(elm) });
	func(elm);
}

/**
 * @param {HTMLSelectElement} elm 
 * @param {*} func 
 */
function addSelectEvent(elm, func) {
	elm.addEventListener("change", (e) => { func(elm) });
	func(elm);
}

/**
 * @param {HTMLTrackElement} elm 
 * @param {*} func 
 */
function addScrollEvent(elm, func) {
	elm.addEventListener("mousemove", (e) => { func(elm) });
	elm.addEventListener("change", (e) => { func(elm) });
	func(elm);
}
