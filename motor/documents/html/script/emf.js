/// <reference path="math.js" />
/// <reference path="drawer.js" />
const ROTOR_DIAMETER = 150;
const STATOR_DIAMETER = 280;
const WAVE_SCOPE_WIDTH = 320;
const WAVE_SCOPE_HEIGHT = 220;
const DISPLAY_SIZE = STATOR_DIAMETER + 20;
const STATOR_POLES = [
	new SelectElement("1", 1),
	new SelectElement("2", 2, true),
	new SelectElement("3", 3),
	new SelectElement("4", 4)
];
const ROTOR_POLES = [
	new SelectElement("2", 2),
	new SelectElement("4", 4, true),
	new SelectElement("6", 6),
	new SelectElement("8", 8),
	new SelectElement("12", 12),
	new SelectElement("16", 16)
];

class MagneticDipole {
	/**
	 * @param {vec3} pos 位置
	 * @param {vec3} m 磁気モーメント
	 */
	constructor(pos, m) {
		/** 固定位置
	 	 * @type {vec3} */
		this.pos_const = new vec3(pos.X, pos.Y, pos.Z);
		/** 移動位置
	 	 * @type {vec3} */
		this.pos_move = new vec3(pos.X, pos.Y, pos.Z);
		/** 磁気モーメント(固定)
	 	 * @type {vec3} */
		this.m_const = new vec3(m.X, m.Y, m.Z);
		/** 磁気モーメント(回転)
	 	 * @type {vec3} */
		this.m_rot = new vec3(m.X, m.Y, m.Z);
	}
}

class Rotor {
	static #CALC_DIV = 50;

	/** 半径
	 * @type {number} */
	radius = 0;
	/** 極数
	 * @type {number} */
	poles = 0;
	/** 磁気双極子
	 * @type {Array<MagneticDipole>} */
	dipole = [];

	/**
	 * @param {number} radius 半径
	 * @param {number} poles 極数
	 * @param {number} gap 磁極間ギャップ
	 */
	constructor(radius, poles, gap) {
		const PI2 = 8*Math.atan(1);
		const DIV = 3*Rotor.#CALC_DIV;
		this.radius = radius;
		this.poles = poles;
		this.dipole = [];
		for (let idx_d = 0; idx_d < DIV; idx_d++) {
			let p = parseInt(poles * idx_d / DIV);
			let th = PI2 * idx_d / DIV;
			let thA = PI2 * (p + gap * 0.5) / poles;
			let thB = PI2 * (p + (1 - gap) + gap * 0.5) / poles;
			if (th < thA || thB < th) {
			} else {
				let ns = (0 == p % 2) ? 1 : -1;
				let pos = new vec3(
					this.radius * Math.cos(th),
					this.radius * Math.sin(th)
				);
				let m = new vec3(
					ns * Math.cos(th),
					ns * Math.sin(th)
				);
				this.dipole.push(new MagneticDipole(pos, m));
			}
		}
	}
}

class Slot {
	static #CALC_DIV = 100;

	/** 位置
	 * @type {Array<vec3>} */
	pos = [];
	/** 磁場(表示位置)
	 * @type {Array<vec3>} */
	magnetic_pos = [];
	/** 磁場(磁力)
	 * @type {Array<number>} */
	magnetic_force = [];
	/** 電場(表示位置)
	 * @type {Array<vec3>} */
	bemf_pos = [];
	/** 電場(電位)
	 * @type {Array<number>} */
	bemf = [];

	/**
	 * @param {number} radius 半径
	 * @param {number} gap スロット間ギャップ
	 * @param {number} slots スロット数
	 * @param {number} slot_index スロット番号
	 * @param {boolean} clear 磁力・電位のクリアを行うか
	 */
	create(radius, gap, slots, slot_index, clear) {
		const PI2 = 8*Math.atan(1);
		const DIV = parseInt(Slot.#CALC_DIV * 3 / slots);
		const OFS_ANGLE = -Math.PI / slots;

		// 位置
		this.pos = [];
		for (let d=DIV; 0<=d; d--) {
			let th = OFS_ANGLE + PI2 * (slot_index + d*(1-gap) / DIV + gap*0.5) / slots;
			this.pos.push(new vec3(radius*Math.cos(th), radius*Math.sin(th)));
		}

		// 磁場・電場
		const MAGNETIC_R = radius + 7;
		const BEMF_R = radius + 13;
		let magnetic_force = [];
		let bemf = [];
		this.magnetic_pos = [];
		this.bemf_pos = [];
		for (let d=0; d<=DIV; d++) {
			magnetic_force.push(0.0);
			bemf.push(0.0);
			let th = OFS_ANGLE + PI2 * (slot_index + d*(1-gap) / DIV + gap*0.5) / slots;
			this.magnetic_pos.push(new vec3(MAGNETIC_R*Math.cos(th), MAGNETIC_R*Math.sin(th)));
			this.bemf_pos.push(new vec3(BEMF_R*Math.cos(th), BEMF_R*Math.sin(th)));
		}
		if (clear) {
			this.magnetic_force = magnetic_force;
			this.bemf = bemf;
		}
	}
}

class Motor {
	static #COLOR_U = Color.GREEN;
	static #COLOR_V = Color.BLUE;
	static #COLOR_W = Color.RED;
	static #COLOR_AXIZ = Color.BLACK;

	Pos = new vec3();
	TargetFreq = 0.0;
	AccTime = 1.0;
	OnlyU = false;
	WaveScale = 0.2;

	/** @type {Array<Slot>} */
	#stator = [];
	/** @type {Rotor} */
	#rotor = null;
	#freq = 0.0;
	#theta = 0.0;
	#scopeX = 0;
	#scopeA = new vec3();
	#scopeU = new vec3();
	#scopeV = new vec3();
	#scopeW = new vec3();

	/**
	 * @param {number} diameter 直径
	 * @param {number} poles 極数
	 * @param {number} gap スロット間ギャップ
	 * @param {boolean} clear 磁力・電位のクリアを行うか
	 */
	createStator(diameter, poles, gap, clear=true) {
		const SLOTS = 3 * poles;
		if (clear) {
			this.#stator = [];
		}
		for (let s=0; s<SLOTS; s++) {
			if (clear) {
				this.#stator.push(new Slot());
			}
			this.#stator[s].create(diameter/2, gap, SLOTS, s, clear);
		}
	}

	/**
	 * @param {number} diameter 直径
	 * @param {number} poles 極数
	 * @param {number} gap 磁極間ギャップ
	 */
	createRotor(diameter, poles, gap) {
		this.#rotor = new Rotor(diameter/2, poles, gap);
	}

	/**
	 * @param {Drawer} drawer
	 */
	draw(drawer) {
		// 回転子を描画
		let rotor_dipole = this.#rotor.dipole;
		for (let idx_p=0; idx_p<rotor_dipole.length; idx_p++) {
			let dipole = rotor_dipole[idx_p];
			let pos = dipole.pos_move;
			let dir = dipole.m_rot;
			let ax = pos.X + dir.X * 2;
			let ay = pos.Y + dir.Y * 2;
			let bx = pos.X - dir.X * 2;
			let by = pos.Y - dir.Y * 2;
			drawer.fillCircleXY(ax, ay, 2.5, Color.RED, this.Pos);
			drawer.fillCircleXY(bx, by, 2.5, Color.BLUE, this.Pos);
		}
		// 固定子を描画
		const scale_v = 1.0 / 2.0;
		const scale_m = 0.25 / 2.0;
		let posA = new vec3();
		let posB = new vec3();
		for(let idx_s=0; idx_s<this.#stator.length; idx_s++) {
			let slot = this.#stator[idx_s];
			let value_v = slot.bemf;
			let value_m = slot.magnetic_force;
			let pos_v = slot.bemf_pos;
			let pos_m = slot.magnetic_pos;
			for(let idx_v=0, idx_p=pos_v.length-2; idx_v<value_v.length-1; idx_v++, idx_p--) {
				let v = (value_v[idx_v] + value_v[idx_v+1]) * scale_v;
				pos_v[idx_p].add(this.Pos, posA);
				pos_v[idx_p+1].add(this.Pos, posB);
				drawer.drawLine(posA, posB, Drawer.ToHue(v), 20);
				let m = (value_m[idx_v] + value_m[idx_v+1]) * scale_m;
				pos_m[idx_p].add(this.Pos, posA);
				pos_m[idx_p+1].add(this.Pos, posB);
				drawer.drawLine(posA, posB, Drawer.ToHue(m), 10);
			}

			let name;
			let background;
			switch(idx_s%3) {
			case 0:
				name = "U", background = Color.GREEN; break;
			case 1:
				name = "V", background = Color.BLUE; break;
			case 2:
				name = "W", background = Color.RED; break;
			}
			let middle = slot.pos[slot.pos.length >> 1];
			middle.normalizeScale(middle.abs + 36, posA);
			posA.add(this.Pos, posA);
			drawer.fillCircle(posA, 12, background);
			drawer.drawStringC(posA, name, 16, Color.WHITE);
		}
	}

	/**
	 * @param {Drawer} drawer
	 */
	drawWave(drawer) {
		let sum_du = 0.0;
		let sum_dv = 0.0;
		let sum_dw = 0.0;
		for(let s=0; s<this.#stator.length; s++) {
			let slot = this.#stator[s];
			let sum = 0.0;
			for(let i=0; i<slot.bemf.length; i++) {
				sum += slot.bemf[i];
			}
			sum /= slot.bemf.length;
			sum *= this.WaveScale;
			switch(s%3) {
			case 0:
				sum_du += sum; break;
			case 1:
				sum_dv += sum; break;
			case 2:
				sum_dw += sum; break;
			}
		}
		if (1 < sum_du) sum_du = 1;
		if (sum_du < -1) sum_du = -1;
		if (1 < sum_dv) sum_dv = 1;
		if (sum_dv < -1) sum_dv = -1;
		if (1 < sum_dw) sum_dw = 1;
		if (sum_dw < -1) sum_dw = -1;

		let disp_a = drawer.Height * (0.5-0.49*Math.cos(this.#theta*this.#rotor.poles/2));
		let disp_u = drawer.Height * (0.5-0.5*sum_du);
		let disp_v = drawer.Height * (0.5-0.5*sum_dv);
		let disp_w = drawer.Height * (0.5-0.5*sum_dw);
		let posA = new vec3(this.#scopeX, disp_a);
		let posU = new vec3(this.#scopeX, disp_u);
		let posV = new vec3(this.#scopeX, disp_v);
		let posW = new vec3(this.#scopeX, disp_w);
		drawer.drawLine(this.#scopeA, posA, Color.BLACK, 1);
		drawer.drawLine(this.#scopeU, posU, Motor.#COLOR_U, 1);
		if (!this.OnlyU) {
			drawer.drawLine(this.#scopeV, posV, Motor.#COLOR_V, 1);
			drawer.drawLine(this.#scopeW, posW, Motor.#COLOR_W, 1);
		}
		this.#scopeA = posA;
		this.#scopeU = posU;
		this.#scopeV = posV;
		this.#scopeW = posW;
		this.#scopeX += 2;

		if (drawer.Width <= this.#scopeX) {
			const NEUTRAL = drawer.Height/2;
			drawer.clear();
			drawer.drawLine(new vec3(0, NEUTRAL), new vec3(drawer.Width, NEUTRAL), Motor.#COLOR_AXIZ, 1);
			this.#scopeA = new vec3(0, NEUTRAL);
			this.#scopeU = new vec3(0, NEUTRAL);
			this.#scopeV = new vec3(0, NEUTRAL);
			this.#scopeW = new vec3(0, NEUTRAL);
			this.#scopeX = 0;
		}
	}

	step() {	
		// 回転子を動かす
		for (let idx_p=0; idx_p<this.#rotor.dipole.length; idx_p++) {
			let dipole = this.#rotor.dipole[idx_p];
			let px = dipole.pos_const.X;
			let py = dipole.pos_const.Y;
			let mx = dipole.m_const.X;
			let my = dipole.m_const.Y;
			let rx = Math.cos(this.#theta);
			let ry = Math.sin(this.#theta);
			dipole.pos_move.X = px*rx - py*ry;
			dipole.pos_move.Y = px*ry + py*rx;
			dipole.m_rot.X = mx*rx - my*ry;
			dipole.m_rot.Y = mx*ry + my*rx;
		}
		// 起電力の計算
		this.calcEMF();
		// 時間を進める
		this.#freq += (this.TargetFreq - this.#freq) / this.AccTime;
		this.#theta += 8*Math.atan(1)*this.#freq / Drawer.FRAME_RATE;
		if (16*Math.atan(1) <= this.#theta) {
			this.#theta -= 16*Math.atan(1);
		}
	}

	calcEMF() {
		const R_MIN = 5.0 / this.#rotor.radius;
		for (let idx_s=0; idx_s<this.#stator.length; idx_s++) {
			let slot = this.#stator[idx_s];
			for (let idx_d=0; idx_d<slot.bemf.length; idx_d++) {
				let slot_pos = slot.pos[idx_d];
				let magnetic_force = 0.0;
				for (let idx_r=0; idx_r<this.#rotor.dipole.length; idx_r++) {
					let dipole = this.#rotor.dipole[idx_r];
					let rx = (slot_pos.X - dipole.pos_move.X) / this.#rotor.radius;
					let ry = (slot_pos.Y - dipole.pos_move.Y) / this.#rotor.radius;
					let r = Math.sqrt(rx*rx + ry*ry);
					if (r < R_MIN) {
						r = R_MIN;
					}
					let dot = dipole.m_rot.X * rx + dipole.m_rot.Y * ry;
					magnetic_force += dot / r / r / r;
				}
				magnetic_force /= this.#rotor.dipole.length;
				slot.bemf[idx_d] = -(magnetic_force - slot.magnetic_force[idx_d]);
				slot.magnetic_force[idx_d] = magnetic_force;
			}
		}
	}
}

class Form {
	constructor() {
		addSelectEvent(document.getElementById("chkOnlyU"), (sender) => gMotor.OnlyU = sender.checked);
		addScrollEvent(document.getElementById("trbWaveScale"), (sender) => gMotor.WaveScale = sender.value * 0.01);
		initSelectList(document.getElementById("cmbStatorPole"), STATOR_POLES);
		addSelectEvent(document.getElementById("cmbStatorPole"), this.#cmbStatorPole_onSelect);
		initSelectList(document.getElementById("cmbRotorPole"), ROTOR_POLES);
		addSelectEvent(document.getElementById("cmbRotorPole"), this.#cmbRotorPole_onSelect);
		addClickEvent(document.getElementById("btnPlayStop"), this.#btnPlayStop_onClick);
		addClickEvent(document.getElementById("btnStep"), this.#btnStep_onClick);
		addScrollEvent(document.getElementById("trbStatorGap"), this.#trbStatorGap_onScroll);
		addScrollEvent(document.getElementById("trbMagnetGap"), this.#trbMagnetGap_onScroll);
		addScrollEvent(document.getElementById("trbGap"), this.#trbGap_onScroll);
		addScrollEvent(document.getElementById("trbFreqMax"), this.#trbFreqMax_onScroll);
		addScrollEvent(document.getElementById("trbAcc"), this.#trbAcc_onScroll);
	}
	#btnPlayStop_onClick(sender) {
		if (gIsPlay) {
			sender.value = "　再生　";
		} else {
			sender.value = "　停止　";
		}
		gIsPlay = !gIsPlay;
	}
	#btnStep_onClick(sender) {
		gIsPlay = false;
		gIsStep = true;
		document.getElementById("btnPlayStop").value = "　再生　";
	}
	#cmbStatorPole_onSelect(sender) {
		let idx = sender.selectedIndex;
		gStatorPole = STATOR_POLES[idx].value;
		gMotor.createStator(ROTOR_DIAMETER+gGap, gStatorPole, gStatorGap);
	}
	#cmbRotorPole_onSelect(sender) {
		let idx = sender.selectedIndex;
		gRotorPole = ROTOR_POLES[idx].value;
		gMotor.createRotor(ROTOR_DIAMETER, gRotorPole, gMagnetGap);
	}
	#trbStatorGap_onScroll(sender) {
		let tmp = sender.value / 24;
		if (tmp == gStatorGap) return;
		gStatorGap = tmp;
		document.getElementById("lblStatorGap").innerHTML = gStatorGap*24 + "/24";
		gMotor.createStator(ROTOR_DIAMETER+gGap, gStatorPole, gStatorGap, false);
	}
	#trbMagnetGap_onScroll(sender) {
		let tmp = sender.value / 16;
		if (tmp == gMagnetGap) return;
		gMagnetGap = tmp;
		document.getElementById("lblMagnetGap").innerHTML = gMagnetGap * 16 + "/16";
		gMotor.createRotor(ROTOR_DIAMETER, gRotorPole, gMagnetGap);
	}
	#trbGap_onScroll(sender) {
		let tmp = 1*sender.value;
		gGap = tmp;
		document.getElementById("lblGap").innerHTML = gGap;
		gMotor.createStator(ROTOR_DIAMETER+gGap, gStatorPole, gStatorGap, false);
	}
	#trbFreqMax_onScroll(sender) {
		let rpm = 1*sender.value / 10;
		document.getElementById("lblFreqMax").innerHTML = rpm + "rpm";
		gMotor.TargetFreq = rpm/60;
	}
	#trbAcc_onScroll(sender) {
		let acc = sender.value;
		document.getElementById("lblAcc").innerHTML = acc;
		gMotor.AccTime = acc;
	}
}

let gMotor = new Motor();
let gDrawerM = new Drawer("motor", DISPLAY_SIZE, DISPLAY_SIZE);
let gDrawerW = new Drawer("scope", WAVE_SCOPE_WIDTH, WAVE_SCOPE_HEIGHT);
let gIsPlay = false;
let gIsStep = false;
let gGap = 5;
let gStatorPole = 4;
let gStatorGap = 0;
let gRotorPole = 16;
let gMagnetGap = 0;
function loop() {
	if (gIsPlay || gIsStep) {
		gDrawerM.clear();
		gMotor.step();
		gMotor.draw(gDrawerM);
		gMotor.drawWave(gDrawerW);
		gIsStep = false;
	}
	requestNextAnimationFrame(loop);
}
{
	new Form();
	gMotor.Pos = new vec3(DISPLAY_SIZE/2, DISPLAY_SIZE/2, 0);
	gMotor.createStator(ROTOR_DIAMETER+gGap, gStatorPole, gStatorGap);
	gMotor.createRotor(ROTOR_DIAMETER, gRotorPole, gMagnetGap);
	gMotor.calcEMF();
	gDrawerW.clear();
	gDrawerW.drawLine(
		new vec3(0, gDrawerW.Height/2),
		new vec3(gDrawerW.Width, gDrawerW.Height/2),
		Color.BLACK, 1
	);
	requestNextAnimationFrame(loop);
}
