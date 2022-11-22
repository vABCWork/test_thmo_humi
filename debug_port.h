
// Debug Port0用出力ポート(P15)の定義

#define DEBUG_0_PMR      (PORT1.PMR.BIT.B5)   //  汎用入出力ポート
#define DEBUG_0_PDR      (PORT1.PDR.BIT.B5)   //  出力ポートに指定
#define DEBUG_0_PODR     (PORT1.PODR.BIT.B5)  //  出力データ
#define DEBUG_0_PIDR     (PORT1.PIDR.BIT.B5)   //  ポート入力データ
