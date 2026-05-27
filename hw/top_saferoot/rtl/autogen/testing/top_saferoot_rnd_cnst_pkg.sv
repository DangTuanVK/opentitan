// Copyright lowRISC contributors (OpenTitan project).
// Licensed under the Apache License, Version 2.0, see LICENSE for details.
// SPDX-License-Identifier: Apache-2.0
//
// ------------------- W A R N I N G: A U T O - G E N E R A T E D   C O D E !! -------------------//
// PLEASE DO NOT HAND-EDIT THIS FILE. IT HAS BEEN AUTO-GENERATED WITH THE FOLLOWING COMMAND:
//
// util/topgen.py -t hw/top_saferoot/data/top_saferoot.hjson
//                -o hw/top_saferoot/
//
// File is generated based on the following seed configuration:
//   hw/top_saferoot/data/top_saferoot_seed.testing.hjson


package top_saferoot_rnd_cnst_pkg;

  ////////////////////////////////////////////
  // aes
  ////////////////////////////////////////////
  // Default seed of the PRNG used for register clearing.
  parameter aes_pkg::clearing_lfsr_seed_t RndCnstAesClearingLfsrSeed = {
    64'h3F0332D5_302206D5
  };

  // Permutation applied to the LFSR of the PRNG used for clearing.
  parameter aes_pkg::clearing_lfsr_perm_t RndCnstAesClearingLfsrPerm = {
    128'h35F2A280_6166AF30_79BE3116_56D65EEA,
    256'h9B2140F7_54FD2468_BBD03497_C39AA82C_8C9FED7B_0F87312D_B2474F79_36230617
  };

  // Permutation applied to the clearing PRNG output for clearing the second share of registers.
  parameter aes_pkg::clearing_lfsr_perm_t RndCnstAesClearingSharePerm = {
    128'h22049C59_9B858A43_3A9A51EA_E4BF1D05,
    256'h88706AB4_3D3FFE33_C754F66C_D1BE0C49_CEBED0B2_5E902853_B45FD1E8_F754AB09
  };

  // Default seed of the PRNG used for masking.
  parameter aes_pkg::masking_lfsr_seed_t RndCnstAesMaskingLfsrSeed = {
    32'h390E8225,
    256'h92151F46_9B081F17_63A14688_D45789D8_4F8AE104_458E83E8_BA2972A8_5388BC03
  };

  // Permutation applied to the output of the PRNG used for masking.
  parameter aes_pkg::masking_lfsr_perm_t RndCnstAesMaskingLfsrPerm = {
    256'h3D820F89_51749093_9D25753B_49577A4E_42002702_7273238A_19874656_9115076B,
    256'h01202B60_034B7021_083E6276_6F1D8077_223A5F86_9C375496_99599230_4D7E0E10,
    256'h409F6463_2E84811E_530C7117_839A0636_7B7C948E_0B2C9732_5B9E7D5E_698C7904,
    256'h48357895_12056C8D_61283826_9B14662F_1355985A_0D398B85_6E4F116A_1A681B34,
    256'h447F2409_523F4A47_58883C8F_1C414550_672D2A1F_43654C31_16295C0A_5D336D18
  };

  ////////////////////////////////////////////
  // kmac
  ////////////////////////////////////////////
  // Compile-time random data for PRNG default seed
  parameter kmac_pkg::lfsr_seed_t RndCnstKmacLfsrSeed = {
    32'h8C6955EF,
    256'h9B74AE5A_ECDFD36D_D956B97D_74A05D6B_0697EDA6_9759E0F3_28375D78_0BCB0728
  };

  // Compile-time random permutation for PRNG output
  parameter kmac_pkg::lfsr_perm_t RndCnstKmacLfsrPerm = {
    64'hB043B525_4E87CBA3,
    256'h802B7395_FC7E5C8A_8E40A2EC_401C6B78_64989773_2997239E_F7EEE96B_8EF1CD68,
    256'h91253498_AA312D85_45BF44A1_D8C217A9_AAC4F470_64D71304_4D42CAD0_E8A6B45B,
    256'hC74D1F1A_081AF48F_1D16D2A2_E26121C2_0C799093_03243D16_CD7725DE_95C9DB50,
    256'h05C5A103_44653D4B_D8BF0638_9360819D_2D8097DC_3EAD0711_55ACC3C2_71E9CD9C,
    256'hD1518672_042E4BEA_E5B14AB5_D9C43DA1_3576B622_20885295_8E5E71ED_D7B3E56A,
    256'hB9EB2CE0_E01C576B_58AC0C52_BCA6B604_B17A1F05_FE0B5006_AB2C0807_D8FB0A23,
    256'h528584D0_7C386F10_CD852FA0_4C34FB72_AB746C93_902BE1B9_4DBE4875_69C07D41,
    256'h57887F1D_E5B89CC3_B205A8D1_FE73E244_74F08196_A13A9A6F_5C8638B2_231EA945,
    256'h8C9E1A21_185CB9C8_AC52B5C6_41BA4C21_BAD981DA_0A14C307_760251AA_780D53A6,
    256'hC78AC233_41C69394_C836A696_1A228829_19B84C3A_D428B54C_06894BBC_6FBAACD4,
    256'hE4F1571F_3C390948_D89815B9_BC2FB6C0_503DD384_948D3CE4_6AAD052D_C85B58E6,
    256'hBA1DF359_C7072F37_5309C6AA_F3E5AAA8_03466626_68E00225_B31AE8FB_6E36C426,
    256'h15651095_6D203814_08831A94_0A40C425_95177F4F_4C4CE75E_838AE6F6_C6ED325A,
    256'h4D9C47EA_30863A23_A2608066_856B1A31_C782F34C_9E444B94_69B18EF0_C6A24AA5,
    256'h9EE905F4_8F157471_722820F9_30EE5784_8AE8EA17_CCD366F4_6D188049_DBA0E844,
    256'h3D3E9B52_AACA7F75_5B7B5D7C_40A597B4_9C980AC0_B8BC28F1_D3F57D9D_3074BD20,
    256'h9B3120AA_35331477_2EAA3426_8058D849_E7E554B5_A6024769_951519F0_FDA7C154,
    256'hF19844A1_E7C2BE1E_37DF132E_FF374419_DECE18F1_131A098F_9605A462_66E75178,
    256'hA9941FCB_B8016A49_7A05788A_0DCA90C0_95135558_91915325_616B59EC_A061BE4F,
    256'hD744F25F_760FCAF0_59A41753_791E00CD_59F5C192_D6481EFE_A245113C_1E56D316,
    256'h0D2BB9A3_F679BA58_638A2C82_AA6CC2EC_137B06D4_28954DE9_E10DB829_25EA29C5,
    256'h7A56E9C0_49691B68_6802C62B_30011928_A797312C_0DE997E8_F878E82A_A0E34048,
    256'h4C5C9951_232C21C3_338D4452_9DAFC0BB_0D3B31EA_3A1AF70B_216589F9_19DA0A34,
    256'h72C2C9A9_390CA3B0_1549080C_9C259991_60AC01F5_481B1F25_2E3D9917_E45A0C43,
    256'h62546122_804069DB_15BF0DB9_1D803E8B_FC087503_E97BF6D0_9BDA5794_E92D2808,
    256'h48231298_BB6AE1B7_492B80B0_94839506_C7AE1D99_A8AE7481_99948BBE_14599E7A,
    256'h0D9A6653_0433DF66_3533999E_0A20358B_96CA8973_5529F096_943B67B7_92AD8849,
    256'hBA853A74_56F835B5_295245EC_F2485E19_269C5429_20F1706C_9046AE37_759E6BD6,
    256'hE38462B4_3B1C976E_D64ADF2E_19C6BEBB_2401729C_2D101065_0C0CA170_A6599086,
    256'h528B70C2_7E0EC12E_29862D59_68470352_D854C558_D29962C9_0461B684_1D348A16,
    256'h1C298B24_36EDDC7F_E6344F00_010605E4_FF93C9F4_B85D920F_EB104509_AB1B65D0
  };

  // Compile-time random data for PRNG buffer default seed
  parameter kmac_pkg::buffer_lfsr_seed_t RndCnstKmacBufferLfsrSeed = {
    32'hBBC62EB5,
    256'h93F43349_1D5A6B56_ADD49C4B_CE1410AE_81F226F8_633E00BC_E711F69D_E8B57F77,
    256'hD04A4C7F_0C472A06_01ACD6B1_5C90B1B2_DDA51373_A6FBED4B_64E9FBBF_8648E57E,
    256'h1E21D470_DEE98E4D_802BAEC1_67C73447_7D5A48BD_1B827B47_0D24F337_26B649C8
  };

  // Compile-time random permutation for LFSR Message output
  parameter kmac_pkg::msg_perm_t RndCnstKmacMsgPerm = {
    128'hE7F2078D_1C4ED6A0_C1C096AD_2E6D2B64,
    256'h530F5787_BD92C7EF_F96CD482_9B809FA9_53718136_6CDF0289_0C8A5FB4_A81A1DDD
  };

  ////////////////////////////////////////////
  // csrng
  ////////////////////////////////////////////
  // Compile-time random bits for csrng state group diversification value
  parameter csrng_pkg::cs_keymgr_div_t RndCnstCsrngCsKeymgrDivNonProduction = {
    128'hC9995B4C_75D4A87C_72C59AB2_7F0C0EDA,
    256'hCB4082B7_5463EB18_6FEEF330_6F2EFDBD_173CF5B6_E5C4A435_B33DA6E3_15DC4A70
  };

  // Compile-time random bits for csrng state group diversification value
  parameter csrng_pkg::cs_keymgr_div_t RndCnstCsrngCsKeymgrDivProduction = {
    128'h81EB1014_ED62AEAC_40D270A9_325A10C6,
    256'hC0CEB311_AADA0CDA_76EA4082_C2889252_4B54BDF9_A58708D7_AD318D66_5B4273A9
  };

  ////////////////////////////////////////////
  // otbn
  ////////////////////////////////////////////
  // Default seed of the PRNG used for URND.
  parameter otbn_pkg::urnd_prng_seed_t RndCnstOtbnUrndPrngSeed = {
    256'h243C8264_10CA4278_0CE8DAB9_AB70B1DF_1603C110_FF8B3484_070CCA41_A439EE44
  };

  // Compile-time random reset value for IMem/DMem scrambling key.
  parameter otp_ctrl_pkg::otbn_key_t RndCnstOtbnOtbnKey = {
    128'h2B177FFA_2A8F7269_EC915F6A_5805B99A
  };

  // Compile-time random reset value for IMem/DMem scrambling nonce.
  parameter otp_ctrl_pkg::otbn_nonce_t RndCnstOtbnOtbnNonce = {
    64'h8D666836_30B6FCF8
  };

  ////////////////////////////////////////////
  // keymgr
  ////////////////////////////////////////////
  // Compile-time random bits for initial LFSR seed
  parameter keymgr_pkg::lfsr_seed_t RndCnstKeymgrLfsrSeed = {
    64'h3E687338_CBADFE0A
  };

  // Compile-time random permutation for LFSR output
  parameter keymgr_pkg::lfsr_perm_t RndCnstKeymgrLfsrPerm = {
    128'h77A57ED9_28C17A26_04B055DC_193CE938,
    256'h0D49465C_8A7E4DF1_E4BC2D1E_7EC20D4B_AAD3D99B_35A30FBE_0648438A_3F5AB87C
  };

  // Compile-time random permutation for entropy used in share overriding
  parameter keymgr_pkg::rand_perm_t RndCnstKeymgrRandPerm = {
    160'h5881E48E_3CFB4980_D1D9619F_3DDD50A9_65DD58E8
  };

  // Compile-time random bits for revision seed
  parameter keymgr_pkg::seed_t RndCnstKeymgrRevisionSeed = {
    256'hD066A246_FC2A434A_06FE234F_F960E41B_65B7584F_7737D524_5DE3AA56_EC2ED286
  };

  // Compile-time random bits for creator identity seed
  parameter keymgr_pkg::seed_t RndCnstKeymgrCreatorIdentitySeed = {
    256'h44125C9C_FE08A41E_B8CCFDB5_84E284D5_CE558A31_E35CAC26_014D2A00_D95E1941
  };

  // Compile-time random bits for owner intermediate identity seed
  parameter keymgr_pkg::seed_t RndCnstKeymgrOwnerIntIdentitySeed = {
    256'hD810AD58_6BC388BF_C3857EE4_700AAC44_F564D2DE_1A39D353_5CFB6FDC_DC67C339
  };

  // Compile-time random bits for owner identity seed
  parameter keymgr_pkg::seed_t RndCnstKeymgrOwnerIdentitySeed = {
    256'hAF6B0C9B_1BE28138_23FED6CE_8D66E72F_5D928EC1_FF39D8AC_3D790314_656C9224
  };

  // Compile-time random bits for software generation seed
  parameter keymgr_pkg::seed_t RndCnstKeymgrSoftOutputSeed = {
    256'h08E3FAF2_D0FB1130_807540F9_78C3985A_A5DC56C9_7AD06EAA_1A353F5C_CD09D08D
  };

  // Compile-time random bits for hardware generation seed
  parameter keymgr_pkg::seed_t RndCnstKeymgrHardOutputSeed = {
    256'hACA4DA1C_E04622A5_16F03204_A6CF4F76_4BE25385_C2E4B65C_7145122D_1F178635
  };

  // Compile-time random bits for generation seed when aes destination selected
  parameter keymgr_pkg::seed_t RndCnstKeymgrAesSeed = {
    256'hBE773210_0161F683_6716DDA2_8ADE994B_A951ABAE_A73474C9_35F38B06_E26EE501
  };

  // Compile-time random bits for generation seed when kmac destination selected
  parameter keymgr_pkg::seed_t RndCnstKeymgrKmacSeed = {
    256'h8EB82572_F294146F_A1A9E186_A928511C_23912DC0_F873B588_8667CD3F_C5E46F47
  };

  // Compile-time random bits for generation seed when otbn destination selected
  parameter keymgr_pkg::seed_t RndCnstKeymgrOtbnSeed = {
    256'h6806081F_0F9C493E_7D3C4D9B_8F06A276_C465CD1D_E2293666_FF419CEB_5F2BE43E
  };

  // Compile-time random bits for generation seed when no CDI is selected
  parameter keymgr_pkg::seed_t RndCnstKeymgrCdi = {
    256'h81DB6846_B60852D6_FDE65315_D7269379_BB12A4FB_948D2DFA_38F0097B_B663417C
  };

  // Compile-time random bits for generation seed when no destination selected
  parameter keymgr_pkg::seed_t RndCnstKeymgrNoneSeed = {
    256'h38536BB6_27237F3E_663C21F0_A0731D33_80C3859B_F6DD7882_A7B894D1_FD9B752B
  };

  ////////////////////////////////////////////
  // lc_ctrl
  ////////////////////////////////////////////
  // Diversification value used for all invalid life cycle states.
  parameter lc_ctrl_pkg::lc_keymgr_div_t RndCnstLcCtrlLcKeymgrDivInvalid = {
    128'hD2E0CAC7_D5AB8C00_A0C32582_BBD38A46
  };

  // Diversification value used for the TEST_UNLOCKED* life cycle states.
  parameter lc_ctrl_pkg::lc_keymgr_div_t RndCnstLcCtrlLcKeymgrDivTestUnlocked = {
    128'hC5443FEC_58AB9CA4_7C8222FB_D165F019
  };

  // Diversification value used for the DEV life cycle state.
  parameter lc_ctrl_pkg::lc_keymgr_div_t RndCnstLcCtrlLcKeymgrDivDev = {
    128'h3A80420E_F55D8525_7E090418_06A51ACC
  };

  // Diversification value used for the PROD/PROD_END life cycle states.
  parameter lc_ctrl_pkg::lc_keymgr_div_t RndCnstLcCtrlLcKeymgrDivProduction = {
    128'hB201220B_A92B551A_3B9F9E9B_2A72F277
  };

  // Diversification value used for the RMA life cycle state.
  parameter lc_ctrl_pkg::lc_keymgr_div_t RndCnstLcCtrlLcKeymgrDivRma = {
    128'h575653D8_EA9CFE4D_F8D754D5_CF7AC059
  };

  // Compile-time random bits used for invalid tokens in the token mux
  parameter lc_ctrl_pkg::lc_token_mux_t RndCnstLcCtrlInvalidTokens = {
    256'hE13D1B10_89FC8573_678A5AB6_202910EC_D7F47D2E_F8719818_17820657_B57A05D9,
    256'hDE74B562_03A9475B_F14F98C5_D55A1813_CA077036_20424A2F_5A716049_6E7C0F8F,
    256'hBB9823A1_07A44D0C_72081D8C_7AF4E3C5_CF16CFBB_D92BFD52_C939D5A1_06614EC2,
    256'h459FC606_D2055CFA_ABAEA072_BD4005BE_DB692FDD_F1641DAC_6111DBA8_EACBA7DB
  };

  ////////////////////////////////////////////
  // alert_handler
  ////////////////////////////////////////////
  // Compile-time random bits for initial LFSR seed
  parameter alert_handler_pkg::lfsr_seed_t RndCnstAlertHandlerLfsrSeed = {
    32'hBC1E20C1
  };

  // Compile-time random permutation for LFSR output
  parameter alert_handler_pkg::lfsr_perm_t RndCnstAlertHandlerLfsrPerm = {
    160'h96C74D3D_2C713301_44B8FA3A_03DD76E0_67E519B5
  };

  ////////////////////////////////////////////
  // otp_ctrl
  ////////////////////////////////////////////
  // Compile-time random bits for initial LFSR seed
  parameter otp_ctrl_top_specific_pkg::lfsr_seed_t RndCnstOtpCtrlLfsrSeed = {
    40'h1A_DA0D09DD
  };

  // Compile-time random permutation for LFSR output
  parameter otp_ctrl_top_specific_pkg::lfsr_perm_t RndCnstOtpCtrlLfsrPerm = {
    240'h7D62_01614546_49D99A8D_B0A07A73_8490570D_0C74592C_F4250132_623215CA
  };

  // Compile-time random permutation for scrambling key/nonce register reset value
  parameter otp_ctrl_top_specific_pkg::scrmbl_key_init_t RndCnstOtpCtrlScrmblKeyInit = {
    256'h23C84B69_40DB5459_53533D2E_520E455D_BA2E799B_263E63FC_8FBAC9F4_AC1AF416
  };

  // Compile-time scrambling key
  parameter otp_ctrl_top_specific_pkg::key_t RndCnstOtpCtrlScrmblKey0 = {
    128'h74DF92D5_12A716C3_1DD89111_0FAC9CD9
  };

  // Compile-time scrambling key
  parameter otp_ctrl_top_specific_pkg::key_t RndCnstOtpCtrlScrmblKey1 = {
    128'hF4BF7374_70ACF864_3517D6C2_2764A4E9
  };

  // Compile-time scrambling key
  parameter otp_ctrl_top_specific_pkg::key_t RndCnstOtpCtrlScrmblKey2 = {
    128'h139CF71C_26DDC270_805FCD8B_1A9A5A59
  };

  // Compile-time digest const
  parameter otp_ctrl_top_specific_pkg::digest_const_t RndCnstOtpCtrlDigestConst0 = {
    128'h880645BF_EAD231E4_06B46D1A_EA1289E4
  };

  // Compile-time digest const
  parameter otp_ctrl_top_specific_pkg::digest_const_t RndCnstOtpCtrlDigestConst1 = {
    128'hD72A835C_11B55028_4A676D1C_301C1AA5
  };

  // Compile-time digest const
  parameter otp_ctrl_top_specific_pkg::digest_const_t RndCnstOtpCtrlDigestConst2 = {
    128'h96315937_B0AB7434_F088AD19_B129D24D
  };

  // Compile-time digest const
  parameter otp_ctrl_top_specific_pkg::digest_const_t RndCnstOtpCtrlDigestConst3 = {
    128'h8863D942_A3DCF09C_2D9F1253_80C59C33
  };

  // Compile-time digest initial vector
  parameter otp_ctrl_top_specific_pkg::digest_iv_t RndCnstOtpCtrlDigestIV0 = {
    64'hC3D4B116_BE4D7AE6
  };

  // Compile-time digest initial vector
  parameter otp_ctrl_top_specific_pkg::digest_iv_t RndCnstOtpCtrlDigestIV1 = {
    64'h34022B6C_0D789CB3
  };

  // Compile-time digest initial vector
  parameter otp_ctrl_top_specific_pkg::digest_iv_t RndCnstOtpCtrlDigestIV2 = {
    64'h46949A33_1561C9ED
  };

  // Compile-time digest initial vector
  parameter otp_ctrl_top_specific_pkg::digest_iv_t RndCnstOtpCtrlDigestIV3 = {
    64'h69052D88_45C62B58
  };

  // OTP invalid partition default for buffered partitions
  parameter logic [16383:0] RndCnstOtpCtrlPartInvDefault = {
    704'({
      320'h34746EBF11005FDBA4A7BCAD79317BA19D9E5E7E698B75E037D0B8BA82C0F67756F0A97070EF5619,
      384'hD30D9E8290188B1ECB42B4BDE08B4EFB18337BDD233F2FD03C5B51ECB7C82E36250994461FB4905F1C741242679383AC
    }),
    704'({
      64'h2CAFC1DDC02AF795,
      256'h1C7676B0792F8586778C9F867909F715589451438194E2CC8515C5CE008133D1,
      256'hC66F2225622803C74E135E673EC72E4EB2B429935276676C60DC29532231D449,
      128'h20CE7E288F11AAA16DB749B0FDD80272
    }),
    704'({
      64'h7A452A39F96655B7,
      128'hF06F0884166167EF3039BF4A17B29707,
      256'h5D01176B46E845363754B2F565ABC770A827CED886DB06FB2EC45B7AF0255A25,
      256'h64868057021975431A2951A88D52F652FF19CD22FDC920FD6FC8CB010CFD11F8
    }),
    320'({
      64'h4CDF3182B68DA444,
      128'hF8E684237C3ACCF0DA4C9B66620D996,
      128'h9C9F52A5E944C639163BE4ABEC887885
    }),
    128'({
      64'h9100227A454092DD,
      40'h0, // unallocated space
      8'h69,
      8'h69,
      8'h69
    }),
    576'({
      64'h398635E5831B5226,
      256'h614AB3C4140C1190706B044A784344FDB2942F60E16F6FF462601B5469146DC1,
      256'h74BCFF7802762A4323104A56EDFA3CC6F3BD1F6B0FC4EAB3B36012A1AC28842F
    }),
    320'({
      64'h72391CBBAAB0944E,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0
    }),
    3776'({
      64'h8AD874EDA2240BD6,
      256'h0,
      32'h0,
      256'h0,
      32'h0,
      32'h0,
      256'h0,
      32'h0,
      32'h0,
      256'h0,
      32'h0,
      32'h0,
      256'h0,
      32'h0,
      512'h0,
      32'h0,
      512'h0,
      32'h0,
      512'h0,
      32'h0,
      512'h0,
      32'h0
    }),
    5696'({
      64'hF3181B0967E68E84,
      96'h0, // unallocated space
      1024'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      96'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      512'h0,
      128'h0,
      128'h0,
      512'h0,
      2560'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0
    }),
    2944'({
      64'h5B102D8D8F2BAC87,
      96'h0, // unallocated space
      256'h0,
      256'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      32'h0,
      1248'h0
    }),
    512'({
      64'h718254911EF57E1A,
      448'h0
    })
  };

  ////////////////////////////////////////////
  // rom_ctrl
  ////////////////////////////////////////////
  // Fixed nonce used for address / data scrambling
  parameter bit [63:0] RndCnstRomCtrlScrNonce = {
    64'h6CADD32F_EE5E02C9
  };

  // Randomised constant used as a scrambling key for ROM data
  parameter bit [127:0] RndCnstRomCtrlScrKey = {
    128'h8CF9FAB9_E489A9DC_71997C95_49425FA4
  };

  ////////////////////////////////////////////
  // sram_ctrl
  ////////////////////////////////////////////
  // Compile-time random reset value for SRAM scrambling key.
  parameter otp_ctrl_pkg::sram_key_t RndCnstSramCtrlSramKey = {
    128'h51E01CEF_A2D7231C_FA71FB8F_E6ED90F0
  };

  // Compile-time random reset value for SRAM scrambling nonce.
  parameter otp_ctrl_pkg::sram_nonce_t RndCnstSramCtrlSramNonce = {
    128'hE96664F5_275C9431_2765A573_A32478E4
  };

  // Compile-time random bits for initial LFSR seed
  parameter sram_ctrl_pkg::lfsr_seed_t RndCnstSramCtrlLfsrSeed = {
    64'h4923DC5F_32223124
  };

  // Compile-time random permutation for LFSR output
  parameter sram_ctrl_pkg::lfsr_perm_t RndCnstSramCtrlLfsrPerm = {
    128'h4CBF98C5_BDAF5350_17EED0EC_1E5E550B,
    256'hA7088455_B7A44423_68A799A0_EF6B7FC4_8131183F_8A6CCF24_6C8D9F8A_B093476E
  };

  ////////////////////////////////////////////
  // flash_ctrl
  ////////////////////////////////////////////
  // Compile-time random bits for default address key
  parameter flash_ctrl_pkg::flash_key_t RndCnstFlashCtrlAddrKey = {
    128'h708E7F46_2EE7F458_C6632520_EDEC854A
  };

  // Compile-time random bits for default data key
  parameter flash_ctrl_pkg::flash_key_t RndCnstFlashCtrlDataKey = {
    128'h5B47F12A_006A3360_6C2B6386_752FFEF5
  };

  // Compile-time random bits for default seeds
  parameter flash_ctrl_top_specific_pkg::all_seeds_t RndCnstFlashCtrlAllSeeds = {
    256'h144CCAD6_8238FB81_6A7E7FFA_053C02C8_60D56F6F_C3670A89_F95FE805_9CAC442F,
    256'hA56CBC6C_D5C3D67D_2FBF291A_BD6B3DC8_5DF8BAA1_3FCE54F3_652C11EB_B9310847
  };

  // Compile-time random bits for initial LFSR seed
  parameter flash_ctrl_top_specific_pkg::lfsr_seed_t RndCnstFlashCtrlLfsrSeed = {
    32'h61323BC9
  };

  // Compile-time random permutation for LFSR output
  parameter flash_ctrl_top_specific_pkg::lfsr_perm_t RndCnstFlashCtrlLfsrPerm = {
    160'h057D3DF4_6C74AF9E_23EA5948_DA1CD880_5368E84F
  };

  ////////////////////////////////////////////
  // rv_core_ibex
  ////////////////////////////////////////////
  // Default seed of the PRNG used for random instructions.
  parameter ibex_pkg::lfsr_seed_t RndCnstRvCoreIbexLfsrSeed = {
    32'h91D177E9
  };

  // Permutation applied to the LFSR of the PRNG used for random instructions.
  parameter ibex_pkg::lfsr_perm_t RndCnstRvCoreIbexLfsrPerm = {
    160'h73C36271_7B823CAF_EAB361C0_628DA2BC_93DA4719
  };

  // Default icache scrambling key
  parameter logic [ibex_pkg::SCRAMBLE_KEY_W-1:0] RndCnstRvCoreIbexIbexKeyDefault = {
    128'hD1507232_827DD2EB_BDE1CD4A_2B643BCE
  };

  // Default icache scrambling nonce
  parameter logic [ibex_pkg::SCRAMBLE_NONCE_W-1:0] RndCnstRvCoreIbexIbexNonceDefault = {
    64'h944FA4DD_F315DDEC
  };

endpackage : top_saferoot_rnd_cnst_pkg
