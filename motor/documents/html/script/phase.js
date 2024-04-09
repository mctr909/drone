/// <reference path="math.js" />
/// <reference path="drawer.js" />
const SAMPLE_RATE = 6000;
const WIDTH = 768;
const HEIGHT = 256;

const COLOR_UP = new Color(0, 192, 0);
const COLOR_UN = new Color(192, 242, 192);
const COLOR_VP = new Color(0, 192, 192);
const COLOR_VN = new Color(192, 242, 242);
const COLOR_WP = new Color(255, 0, 0);
const COLOR_WN = new Color(255, 192, 192);
const COLOR_PHASE = new Color(128, 128, 255);
const COLOR_VELOCITY = new Color(0, 0, 0);
const COLOR_AXIZ = new Color(128, 128, 128);

let gDrawer = new Drawer("graph", WIDTH, HEIGHT);
let gChkUVW = document.getElementById("chkUVW");
let gChk48 = document.getElementById("chk48");
let gChkPhase = document.getElementById("chkPhase");
let gChkVelo = document.getElementById("chkVelo");
let gChkAcc = document.getElementById("chkAcc");
let gAmp = 127;
let gFreq = 1;
let gCalcVeloInterval = 100;
let gCalcVeloCount = 0;

function main() {
	gDrawer.clear();
	do_iteration();
	requestNextAnimationFrame(main);
}
function amp_onchange() {
	gAmp = document.getElementById("rangeAmp").value;
}
function freq_onchange() {
	gFreq = document.getElementById("rangeFreq").value * 0.25;
}
function interval_onchange() {
	gCalcVeloInterval = document.getElementById("rangeInterval").value;
}
{
	document.getElementById("rangeAmp").onchange = amp_onchange;
	document.getElementById("rangeAmp").onmousemove = amp_onchange;
	document.getElementById("rangeFreq").onchange = freq_onchange;
	document.getElementById("rangeFreq").onmousemove = freq_onchange;
	document.getElementById("rangeInterval").onchange = interval_onchange;
	document.getElementById("rangeInterval").onmousemove = interval_onchange;
	requestNextAnimationFrame(main);
}

let wave_u = 0, wave_v = 0, wave_w = 0;
let x0 = 0, x1 = 0;
let p0 = 0, p1 = 0;
let f0 = 0, f1 = 0;
let u0 = 0, u1 = 0;
let v0 = 0, v1 = 0;
let w0 = 0, w1 = 0;
let ua0 = 0, ua1 = 0;
let ub0 = 0, ub1 = 0;
let va0 = 0, va1 = 0;
let vb0 = 0, vb1 = 0;
let wa0 = 0, wa1 = 0;
let wb0 = 0, wb1 = 0;
function disp_clear() {
	x0 = 0, x1 = 0;
	p0 = 0, p1 = 0;
	f0 = 0, f1 = 0;
	u0 = 0, u1 = 0;
	v0 = 0, v1 = 0;
	w0 = 0, w1 = 0;
	ua0 = 0, ua1 = 0;
	ub0 = 0, ub1 = 0;
	va0 = 0, va1 = 0;
	vb0 = 0, vb1 = 0;
	wa0 = 0, wa1 = 0;
	wb0 = 0, wb1 = 0;
	gDrawer.drawLineXY(0, HEIGHT / 2, WIDTH - 1, HEIGHT / 2, COLOR_AXIZ);
	if (!gChkAcc.checked && gFreq <= 1.0) {
		for (let i = -48; i <= 48; i++) {
			let x = i * WIDTH / (96 * gFreq) + WIDTH / 2;
			let axizY;
			if (0 == i % 2) {
				gDrawer.drawStringC(new vec3(x, HEIGHT / 2 + 12), int((i + 48) / 2) % 24, 11);
				axizY = 8;
			} else {
				axizY = 4;
			}
			gDrawer.drawLineXY(x, HEIGHT / 2 - axizY, x, HEIGHT / 2 + axizY, COLOR_AXIZ);
		}
	}
}
function disp_wave() {
	if (gChk48.checked) {
		let colorUp, colorUn;
		let colorVp, colorVn;
		let colorWp, colorWn;
		if (gChkUVW.checked) {
			colorUp = COLOR_UN;
			colorUn = COLOR_UN;
			colorVp = COLOR_VN;
			colorVn = COLOR_VN;
			colorWp = COLOR_WN;
			colorWn = COLOR_WN;
		} else {
			colorUp = COLOR_UP;
			colorUn = COLOR_UN;
			colorVp = COLOR_VP;
			colorVn = COLOR_VN;
			colorWp = COLOR_WP;
			colorWn = COLOR_WN;
		}
		ua1 = (0.5 - (u_del - NEUTRAL) / gAmp * 0.8) * HEIGHT;
		ub1 = (0.5 - (u_adv - NEUTRAL) / gAmp * 0.8) * HEIGHT;
		va1 = (0.5 - (v_del - NEUTRAL) / gAmp * 0.8) * HEIGHT;
		vb1 = (0.5 - (v_adv - NEUTRAL) / gAmp * 0.8) * HEIGHT;
		wa1 = (0.5 - (w_del - NEUTRAL) / gAmp * 0.8) * HEIGHT;
		wb1 = (0.5 - (w_adv - NEUTRAL) / gAmp * 0.8) * HEIGHT;
		gDrawer.drawLineXY(x0, ua0, x1, ua1, colorUn);
		gDrawer.drawLineXY(x0, ub0, x1, ub1, colorUp);
		gDrawer.drawLineXY(x0, va0, x1, va1, colorVn);
		gDrawer.drawLineXY(x0, vb0, x1, vb1, colorVp);
		gDrawer.drawLineXY(x0, wa0, x1, wa1, colorWn);
		gDrawer.drawLineXY(x0, wb0, x1, wb1, colorWp);
		ua0 = ua1;
		ub0 = ub1;
		va0 = va1;
		vb0 = vb1;
		wa0 = wa1;
		wb0 = wb1;
	}
	if (gChkUVW.checked) {
		u1 = (0.5 - (wave_u - 128) / gAmp * 0.45) * HEIGHT;
		v1 = (0.5 - (wave_v - 128) / gAmp * 0.45) * HEIGHT;
		w1 = (0.5 - (wave_w - 128) / gAmp * 0.45) * HEIGHT;
		gDrawer.drawLineXY(x0, u0, x1, u1, COLOR_UP);
		gDrawer.drawLineXY(x0, v0, x1, v1, COLOR_VP);
		gDrawer.drawLineXY(x0, w0, x1, w1, COLOR_WP);
		u0 = u1;
		v0 = v1;
		w0 = w1;
	}
	if (gChkPhase.checked) {
		//p1 = (23 - g_phase + 0.5) / 24 * HEIGHT;
		//gDrawer.drawLineXY(x0, p0, x1, p1, COLOR_PHASE);
		//p0 = p1;
		p1 = (0.5 - 0.5 * Math.sin(g_phase * 2 * 3.141592 / 24)) * HEIGHT;
		gDrawer.drawLineXY(x0, p0, x1, p1, COLOR_PHASE);
		p0 = p1;
	}
	if (gChkVelo.checked) {
		f1 = (1 - g_velocity * 16 / 24 / 1024) * HEIGHT;
		gDrawer.drawLineXY(x0, f0, x1, f1, COLOR_VELOCITY);
		f0 = f1;
	}
	x0 = x1;
}
function do_iteration() {
	disp_clear();
	gCalcVeloCount = 0;
	g_phase = 0;
	g_phase_diff_sum = 0;
	g_velocity = 0;
	let nFreq = gFreq * 2;
	let delta = gChkAcc.checked ? 0 : nFreq;
	for (let i = 0; i < SAMPLE_RATE; i++) {
		x1 = parseInt(i * WIDTH / SAMPLE_RATE);
		let adc_u;
		let adc_v;
		{
			const PHI = 2 * Math.PI / 3;
			let theta = 2 * Math.PI * (i - (gChkAcc.checked ? 1 : 1) * SAMPLE_RATE / 2) / SAMPLE_RATE * delta;
			delta += gChkAcc.checked ? (nFreq / SAMPLE_RATE) : 0;
			let r = (Math.random() * 2 - 1) * gAmp * 0.05;
			wave_u = 128 + int(r + gAmp * Math.sin(theta));
			wave_v = 128 + int(r + gAmp * Math.sin(theta + PHI));
			wave_w = 128 + int(r + gAmp * Math.sin(theta - PHI));
			adc_u = wave_u;
			adc_v = wave_v;
			if (adc_u < 0) adc_u = 0;
			if (255 < adc_u) adc_u = 255;
			if (adc_v < 0) adc_v = 0;
			if (255 < adc_v) adc_v = 255;
		}
		set_phase(adc_u, adc_v);
		disp_wave();
	}
	document.getElementById("dispAmp").value = gAmp;
	document.getElementById("dispFreq").value = gFreq;
	document.getElementById("dispInterval").value = int(gCalcVeloInterval);
	document.getElementById("dispSFreq").value = "velocity: " + g_velocity;
}

const NEUTRAL = 103;

let g_phase = 0;
let g_phase_diff_sum = 0;
let g_velocity = 0;

let u_del, u_adv;
let v_del, v_adv;
let w_del, w_adv;

function set_phase(adc_u, adc_v) {
	/* u相,v相からw相を得る */
	/* オーバーフロー対策のため範囲(0-128-255)を(64-128-191)に変換 */
	let wave_u = adc_u >> 1;
	let wave_v = adc_v >> 1;
	let wave_w = 255 - wave_u;
	wave_w -= wave_v;
	wave_v += 64;
	wave_u += 64;

	/* u相,v相,w相の各相に対して
	 * adv: 1/48周期進んだ相と
	 * del: 1/48周期遅れた相を作る */
	//let u_del, u_adv, v_del, v_adv, w_del, w_adv;
	{
		let wave_z;
		wave_z = wave_v >> 2;
		wave_z -= wave_z >> 2;
		u_del = wave_u - wave_z;
		w_adv = wave_w - wave_z;
		wave_z = wave_w >> 2;
		wave_z -= wave_z >> 2;
		v_del = wave_v - wave_z;
		u_adv = wave_u - wave_z;
		wave_z = wave_u >> 2;
		wave_z -= wave_z >> 2;
		w_del = wave_w - wave_z;
		v_adv = wave_v - wave_z;
	}

	/* 1/24周期単位の位相を得る */
	let detected_phase;
	if (NEUTRAL < u_del) {
		if (u_adv <= NEUTRAL) {
			detected_phase = 12;
		}
		if (u_adv < v_adv) {
			detected_phase = 1;
		}
	} else {
		if (NEUTRAL < u_adv) {
			detected_phase = 0;
		}
		if (v_adv <= u_adv) {
			detected_phase = 13;
		}
	}
	if (NEUTRAL < v_del) {
		if (v_adv <= NEUTRAL) {
			detected_phase = 4;
		}
		if (v_adv < w_adv) {
			detected_phase = 17;
		}
	} else {
		if (NEUTRAL < v_adv) {
			detected_phase = 16;
		}
		if (w_adv <= v_adv) {
			detected_phase = 5;
		}
	}
	if (NEUTRAL < w_del) {
		if (w_adv <= NEUTRAL) {
			detected_phase = 20;
		}
		if (w_adv < u_adv) {
			detected_phase = 9;
		}
	} else {
		if (NEUTRAL < w_adv) {
			detected_phase = 8;
		}
		if (u_adv <= w_adv) {
			detected_phase = 21;
		}
	}
	if (u_del < w_del) {
		if (NEUTRAL < u_adv) {
			detected_phase = 11;
		}
		if (w_adv < u_adv) {
			detected_phase = 22;
		}
	} else {
		if (u_adv <= NEUTRAL) {
			detected_phase = 23;
		}
		if (u_adv <= w_adv) {
			detected_phase = 10;
		}
	}
	if (v_del < u_del) {
		if (NEUTRAL < v_adv) {
			detected_phase = 3;
		}
		if (u_adv < v_adv) {
			detected_phase = 14;
		}
	} else {
		if (v_adv <= NEUTRAL) {
			detected_phase = 15;
		}
		if (v_adv <= u_adv) {
			detected_phase = 2;
		}
	}
	if (w_del < v_del) {
		if (NEUTRAL < w_adv) {
			detected_phase = 19;
		}
		if (v_adv < w_adv) {
			detected_phase = 6;
		}
	} else {
		if (w_adv <= NEUTRAL) {
			detected_phase = 7;
		}
		if (w_adv <= v_adv) {
			detected_phase = 18;
		}
	}

	/* 位相変化を積算 */
	let phase_diff;
	if (detected_phase < g_phase) {
		phase_diff = 24;
	} else {
		phase_diff = 0;
	}
	phase_diff += detected_phase;
	phase_diff -= g_phase;
	if (phase_diff < 12) {
		// NOP
	} else {
		phase_diff = 0;
	}
	g_phase_diff_sum += phase_diff;
	g_phase = detected_phase;

	/* 一定間隔で位相変化の積算値を速度として設定 */
	if (gCalcVeloInterval <= gCalcVeloCount) {
		g_velocity = g_phase_diff_sum;
		g_phase_diff_sum = 0;
		gCalcVeloCount = 0;
	} else {
		gCalcVeloCount++;
	}
}
