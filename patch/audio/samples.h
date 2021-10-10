#pragma once

using sample_id = unsigned int;

enum SampleId : sample_id
{
	smt_hf = 0x8afd3cb3,
	fsw1 = 0x6d6284,
	uzi_fr = 0xc80ed924,
	lwb_dp1 = 0xa4ff9652,
	spinhook = 0x94311707,
	foot01 = 0xdb8bc228,
	fsw2 = 0x3b11d6e0,
	lwb_dp2 = 0x487edc0a,
	bp_sws = 0x15d5709,
	foot02 = 0xbd94e833,
	fsw3 = 0xa5b5aafe,
	foot03 = 0x5e18aa54,
	clim_up3 = 0x66b9774b,
	hy_ft2 = 0xa7bf731e,
	fsw4 = 0x68bc2ff4,
	foot04 = 0x44f51845,
	clim_up2 = 0x2ec7864c,
	slipping = 0xe0dd6af2,
	harp_luw = 0xa6706602,
	lara_no = 0xdc0f20e,
	sqk2 = 0x460b5cf8,
	shiv_die = 0xfa65d2c5,
	lar_th1 = 0x1d906c44,
	lar_die1 = 0xf89f7e5c,
	l_criket = 0xa25ff154,
	landing = 0xb9f0097c,
	croc_die = 0xbc969e88,
	breath = 0x293caf42,
	clim_up1 = 0xf6ae9393,
	go_watr = 0x4761548f,
	bob_at2 = 0xf8631cb0,
	trx_foot = 0xc2587a98,
	hols_out = 0x6cc4468a,
	harp_fr = 0x59c809f5,
	meteor = 0xaa5db24,
	hols_in = 0xf391084a,
	cmpy_wt = 0x180cdf4c,
	magnum = 0xcd6ea87d,
	p_p04 = 0xe8a9786c,
	reload = 0x3ce16076,
	rico_01 = 0x5fce7657,
	sldr_cl1 = 0xb7b8f0d5,
	harp_fuw = 0x83503b8e,
	rico_02 = 0xa9b00af5,
	flre_ig = 0x822f220c,
	cm_clw = 0x30c7b6a9,
	flre_bu = 0xb83b6f9f,
	win_br2 = 0x4304672b,
	emp_gun = 0x198789ab,
	b1 = 0x94138efa,
	crow_caw = 0x6f29a121,
	flre_igw = 0xd1626448,
	shiv_bng = 0xfcd7beff,
	tb_laf = 0xbbc811ff,
	flre_buw = 0x4e3e41fb,
	wb_stb = 0xabd0fde8,
	win_gu2 = 0xb86120f1,
	smt_fr = 0x95cb899a,
	harp_lo = 0x99b6077c,
	wade = 0x79ba1187,
	l_rumb = 0x13319e59,
	sml_swt = 0xd8b27538,
	lwb_chn = 0xa60245db,
	lar_kn2 = 0x617510ff,
	boat_stp = 0x8dff15ce,
	barr3 = 0xa3935681,
	lar_kn1 = 0x76a03a75,
	fann = 0x9f07e1dd,
	bubbles = 0x4d076f30,
	push_swt = 0xccd2409,
	body_sl1 = 0x988c7f7d,
	body_sl2 = 0xaf4b2dde,
	hy_hf2 = 0xf7c3b034,
	clsl_01 = 0xe7e25269,
	hy_hf1 = 0x76bc2853,
	clsl_02 = 0xe73ed212,
	shot_gun = 0x24e35feb,
	uw_swt = 0x462a1126,
	lar_jmp = 0x9fbe4d,
	f2f_scrm = 0xc73a296e,
	takehit1 = 0x23fd1444,
	takehit2 = 0xe4fe95e0,
	skel = 0x1997b04e,
	dog_at1 = 0xf30621eb,
	rolling = 0xb60b724f,
	boat_mov = 0x1bc2f3f3,
	p_p02 = 0xcd7036ae,
	splash = 0x5f34493e,
	win_th1 = 0x5380156,
	swim = 0xc56810c2,
	shot_shl = 0xb4a15304,
	p_p01 = 0x2ce1b6a0,
	usekey = 0x1b8b73d4,
	p_p03 = 0x2fea85cd,
	obj_clk = 0xdc18ad3,
	sqk1 = 0x536dc74,
	lar_th2 = 0x54ff5785,
	lar_die2 = 0x6ac993a,
	alarm_fr = 0x3361a34a,
	uzi_stp = 0x4041d723,
	bul_flsh = 0x5b7bbd6f,
	tri_th2 = 0xc5ff9c27,
	p_p05 = 0xd89ed96f,
	tb_t2 = 0x847df66e,
	wb_r3 = 0xb9573c3,
	cm_f2 = 0x43a74e0,
	floatswm = 0x55a88a8f,
	trapd_op = 0xb4ee5483,
	f2f_hitg = 0x45ae31a8,
	scub_arm = 0x57d15266,
	back_jm1 = 0x83612d06,
	back_jm3 = 0x7d7c161c,
	back_jm2 = 0x7f1c259c,
	tick = 0x4b6f2a3c,
	cmpy_ft1 = 0xafe95045,
	swch_up = 0xf4a3a66e,
	l_wloop = 0x16c6336,
	l_fire = 0x3c1471ad,
	undwatr = 0x3c7f2fbe,
	pickup = 0x6c5da6df,
	l_gener = 0xbdd65cce,
	push_blk = 0x4ca334db,
	win_hu1 = 0x23eadfd0,
	sml_d = 0x2188eda8,
	l_heli = 0xd57004b2,
	l_roket = 0x8be53959,
	rokfall1 = 0xbfcca7b4,
	bp_die2 = 0xb39cf84f,
	rokfall2 = 0xe4ec7553,
	hy_ft1 = 0xf00984c5,
	jet = 0x11f4791b,
	staleg = 0xacf7dce3,
	sfan_on = 0xd56c72fc,
	swoosh1 = 0x3fc7256c,
	osmg_fr = 0xc35a6db9,
	l_vend = 0xa751504e,
	radar = 0x19a09396,
	upport = 0xd297506a,
	cmpy_ft2 = 0xfa75e5d9,
	swnflms = 0xb947ee31,
	warp = 0x402907dc,
	bazooka = 0x1c6af799,
	wb_th1 = 0x22f0d190,
	wtdrip1 = 0x1410336f,
	quad_stp = 0x9140509f,
	koch = 0xc5685ded,
	cmndr3 = 0x5677e449,
	l_wfall = 0x9e8d4ee8,
	croc_at = 0x25da27e3,
	lwb_th1 = 0x9516017c,
	port = 0xd7171365,
	win_gu3 = 0xc7c9c0b6,
	ti_roar = 0x930ca632,
	trx_at2 = 0x5f6a43a2,
	bod_slm1 = 0x8c2daaeb,
	mini_fr = 0x3658d494,
	bod_slm2 = 0xc2ed18fe,
	l_lazer = 0x26022d52,
	shiv_ft = 0xf49b26eb,
	bod_slm3 = 0xfc4a0c9b,
	bp_at2 = 0x7a20c4ad,
	l_power = 0x54c5f61b,
	trx_scrm = 0x766a1422,
	trx_snf = 0x57df1f60,
	fs_mud1 = 0x363adb26,
	vul_gld = 0x22cd0180,
	vul_at2 = 0x478613e4,
	asmg_fr = 0xd45a928,
	asmg_d1 = 0xf78249b5,
	wb_chg = 0x18a51be7,
	asmg_ft1 = 0x42d2757a,
	asmg_ft2 = 0x264a5b56,
	asmg_ft3 = 0x480f69c,
	sub_st = 0x1b4e2cdf,
	wb_r2 = 0x600e9ca8,
	cm_f3 = 0x7bb2e2c7,
	fmt_atk = 0x45690a6,
	ti_grwl = 0x4ea1865c,
	fmt_die = 0xc8fbab20,
	fmt_fly = 0xcbf9cfd4,
	rat_atk = 0x9464be06,
	rat_die = 0x164bd9c,
	ti_bite = 0x362bac32,
	lwb_fr = 0xd1059568,
	ti_strk = 0x4b9cdc2f,
	cln_fuse = 0x6eabd8a3,
	ti_deth = 0xc9f6015c,
	kochstop = 0x41d318f1,
	explos1 = 0xec7aded8,
	explos2 = 0xcc86f1d5,
	l_quake = 0x92bb4afa,
	fs_wd1 = 0x85005e2c,
	m_rotat = 0xa1397366,
	plgwnch = 0xe75d02ff,
	scub_div = 0x15172f7d,
	choose = 0xd06a4a19,
	lmn_fr = 0x49eaff0,
	menuin = 0xab83e285,
	dog_d2 = 0x35770441,
	m_gurel = 0x616f82a5,
	pass_prt = 0x347b6d73,
	medi_fix = 0xd69219f2,
	targ1 = 0x41b0bf7f,
	targ2 = 0xe7cf8ade,
	targ3 = 0x483bd33d,
	lsg_die = 0x8c3a595f,
	hiss = 0x8bd2737d,
	d_eagle = 0xbebdfbd0,
	wb_at2 = 0x7dbb0fdb,
	mini_lo = 0x1bd8915f,
	mini_lck = 0xb2b15717,
	fs_grv2 = 0x642059d5,
	gate = 0x304135b8,
	l_larel = 0xc94c0f8e,
	win_hu3 = 0xa10f57c7,
	lar_el2 = 0xf821a8b,
	win_hu2 = 0x6d1670de,
	lar_el3 = 0x27edd56e,
	lar_el4 = 0x34b16bad,
	rapt_ft1 = 0x92b09aea,
	cmndr1 = 0x8c27d1ec,
	rapt_ft2 = 0x1ad9333d,
	dog_f1 = 0x93fe6c5e,
	cmndr2 = 0x4fbb5828,
	swtdoor = 0x6173fc31,
	crow_flp = 0x1f29c535,
	gasmeter = 0xea959047,
	crow_die = 0x9f478ea0,
	crow_atk = 0x73457e4f,
	l_wind = 0xba11b4e5,
	alarm = 0x86ef0f95,
	silencer = 0xc91c78db,
	lmn_at1 = 0xb63f9611,
	lmn_at2 = 0x6bc07ff4,
	lmn_die = 0xb3f9c401,
	lmn_clm = 0x46a1e207,
	smt_ft = 0x7d255a8e,
	gbod_slm = 0xabd42dac,
	lar_spks = 0x597dce7b,
	tube = 0xf2501d0a,
	nextdoor = 0xbd7a2642,
	bp_blo = 0x6285812b,
	quad_stt = 0x8e4daa15,
	quad_idl = 0x7a645179,
	quad_rev = 0x38bd6ac1,
	rapt_at1 = 0x34186a11,
	quad_mve = 0x3c63587d,
	b2 = 0x8567fb4b,
	b3 = 0x9638a03c,
	sbld_bck = 0x49cab8c8,
	b4 = 0xe868c35b,
	b5 = 0x12aa1259,
	b6 = 0x7065c3cf,
	b7 = 0x85767158,
	win_th3 = 0x1f4db7bd,
	b8 = 0xd4870e18,
	lncher1 = 0xd8437c79,
	maq_die = 0x8f7c3dd8,
	lncher2 = 0x3964fb2,
	trapd_cl = 0x95f5ed21,
	flush = 0x6fe5dd6b,
	maq_stnd = 0xe90be4d1,
	maq_atl = 0x7c38de6,
	oil_rdie = 0x7ff16442,
	maq_atj = 0x785a7136,
	maq_jmp = 0x33d79860,
	tb_t1 = 0x8b0a777f,
	shit2 = 0xbca2141c,
	cm_f1 = 0xe2bf3332,
	smt_in = 0xc76d360e,
	smt_fr2 = 0x147b4ab8,
	wb_sht = 0xfa2986e9,
	smt_die = 0x371dc627,
	smt_brs = 0x7e7b75a,
	dog_at2 = 0x948ce3da,
	cmpy_at = 0x4ce7501e,
	dog_aw = 0x9ce673f2,
	vul_die = 0x8e46bd4a,
	dog_grl = 0x73b6f35e,
	barol = 0x2fb72818,
	dog_d1 = 0xc9815413,
	vul_flp = 0xff3bbb38,
	lwb_vlt = 0x9b9ecb5f,
	fs_mud2 = 0xcc538a89,
	vul_at1 = 0xdceb346c,
	scub_flp = 0x825650a5,
	scub_brw = 0x8a5b1bfc,
	scub_brs = 0x3f4381a2,
	tri_trn2 = 0x19e09f31,
	lmrc_d1 = 0x350b65c9,
	lmrc_d2 = 0xbe5ff29c,
	tonk = 0x1ca7ff74,
	l_clnr = 0xa98bcb17,
	scub_dth = 0xd250852b,
	boat_str = 0x28b35afc,
	hy_die = 0xd8613de7,
	boat_idl = 0x1646fef7,
	shut_sms = 0x40fb03e7,
	boat_acc = 0x17c9096f,
	boat_sld = 0x5496cf6d,
	jet2 = 0x4595847f,
	fs_sn3 = 0x7f9eca6e,
	quad_si = 0x173c0763,
	bp_ft = 0xd0481334,
	quad_fi = 0x160bad21,
	quad_lnd = 0x9436d96c,
	cm_sws = 0xd238c826,
	l_flm = 0x952c5da0,
	drillbit = 0xcfaf906,
	vs_wnch = 0x93f236ac,
	l_mncr = 0xda5df696,
	save = 0x85fe9cf3,
	l_mncr2 = 0xd2e009db,
	mncrt_k = 0xfa5b1c99,
	wdgt = 0x79f9b78b,
	uw_fan = 0x4ed8a4e1,
	uw_fanst = 0xe8bd60b0,
	win_gu1 = 0xd2526a47,
	rope = 0x810e3505,
	win_ft1 = 0xbe3809d0,
	mine_b = 0x6d831589,
	bmt_gas = 0x9fce4e4,
	spanner = 0x5d7bdec1,
	smsh_up = 0x4b831d26,
	sbld_swg = 0x4690e543,
	st2 = 0x2ddd744a,
	tb_ds = 0x323ac9f2,
	st1 = 0xe0a25d6d,
	l_st = 0xfa14348c,
	sldr_cl2 = 0x9732b5a1,
	tb_1 = 0x9fc7920d,
	win_sht = 0x2ca00253,
	oild_rol = 0x1d1ce810,
	bomgt = 0xb5e7918d,
	tb_die = 0x2d51ae07,
	lwb_fr2 = 0xc1fba7ed,
	cmpy_jmp = 0x1ee4cb85,
	cmpy_dth = 0x28745642,
	bp_at1 = 0xca1ae138,
	bp_die = 0x81dd2be,
	shiv_wlk = 0x4a31c537,
	whale = 0x6d39870c,
	rapt_at2 = 0xef9fe009,
	sml_dc = 0xa4a9ad31,
	rapt_at3 = 0x745a63bc,
	rapt_ror = 0x1bd48c45,
	rapt_d1 = 0x72acf6d7,
	rapt_d2 = 0x28db0838,
	shiv_s1 = 0x976146db,
	shiv_s2 = 0x9636d0ba,
	shiv_laf = 0x1e9207ad,
	hy_at1 = 0x8de4190a,
	hy_at2 = 0x3a68f0d1,
	win_ft2 = 0xffce1093,
	hy_sws = 0x1d423dce,
	cm_f4 = 0x9a2c8226,
	cm_at1 = 0x78cb3819,
	bob_ft = 0x5994cf10,
	cm_at2 = 0xfc993c5b,
	cm_die = 0xee84a85d,
	cm_bod = 0xa902100b,
	cm_laz = 0xf363522f,
	hy_bod = 0xfa5e97bc,
	cable_gt = 0xdfe735da,
	cable_go = 0x5dfe029e,
	cable_st = 0x984dfb10,
	bob_at1 = 0x479fb7cc,
	bob_die1 = 0xf5d97cc3,
	bob_die2 = 0x9ee55450,
	bob_clm = 0x67bd38ff,
	bob_gd = 0x340d93a8,
	fs_ice1 = 0xeadc3b51,
	fs_ice2 = 0x8890fef0,
	fs_grv1 = 0x90113092,
	fs_snd1 = 0x459d88da,
	fs_snd2 = 0x1bea2f98,
	fs_wd2 = 0x747128fa,
	fs_sn1 = 0xe52c17bb,
	fs_sn2 = 0xc49a5548,
	fs_met1 = 0x69fca6bb,
	fs_met2 = 0x91def7e7,
	l_london = 0xb8050ed7,
	l_drill = 0xf0b8bb9d,
	l_drill2 = 0x5346761f,
	eng_hoy1 = 0x82d43dc3,
	eng_hoy2 = 0x1271de14,
	eng_hoy3 = 0x54b95f3a,
	ynk_hoy1 = 0xf4e6c443,
	ynk_hoy2 = 0xae35b5c2,
	l_radio = 0x127f391c,
	punk_at1 = 0x5898d238,
	punk_at2 = 0xd20b4677,
	doorbell = 0xc4947cfa,
	punk_die = 0x950ff3d7,
	lsg_fr = 0xe65cac4a,
	win_br1 = 0xc73eec1a,
	win_cp1 = 0x2c028193,
	win_cp2 = 0x49185a0a,
	win_ht = 0xd65a2eb2,
	win_th2 = 0x4fcc9e1e,
	win_frt = 0x9c48dc2,
	wbld = 0xc2929c46,
	wb_r1 = 0xde88175d,
	shit3 = 0x811022a9,
	rattle = 0xafccb51b,
	wb_th2 = 0x29b96001,
	wtdrip2 = 0xc78116f2,
	wb_th3 = 0x21af4ccd,
	wtdrip3 = 0xc659111a,
	spit = 0xd9012b5,
	boat_scr = 0x3683e287,
	wb_ft = 0x766719d4,
	wb_at1 = 0x2be48923,
	wb_od1 = 0xd97012fb,
	wb_od2 = 0xd85018d3,
	sub_loop = 0xf3997516,
	sub_stp = 0xef00baaa,
	lwb_chg = 0x9a448774,
	tb_2 = 0x91ee028e,
	lwb_laf = 0xd9e8c8f2,
	watmill = 0x550f7070,
	metweel = 0x692347d1,
	tri_at1 = 0xee8e0228,
	tri_at2 = 0xa7ff7184,
	tri_th1 = 0x1a08c2d8,
	tri_trn = 0xf9eb2249,
	tri_sht = 0x7fa856d9,
	tri_die = 0x3ee6421b,
	tb_3 = 0xe9dd3157
};