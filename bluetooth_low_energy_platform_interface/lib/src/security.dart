/// リンクの保護（ペアリング）に関する中立な型。
///
/// 各プラットフォームの型・列挙・エラーはここへ写す。名前の付け替えではなく、
/// プラットフォーム差を型で吸収するための層である。OS 固有の語（WinRT の
/// DevicePairingProtectionLevel 等）は API に出さない。
library;

/// ペアリングで要求する保護の水準。
///
/// 指定できないプラットフォームでは無視される（OS が自分で決める）。
enum PairingProtection {
  /// OS に選ばせる。
  osDefault,

  /// 保護なし。
  none,

  /// 暗号化。
  encryption,

  /// 暗号化と認証（MITM 保護）。
  encryptionAndAuthentication,
}

/// ペアリングの同意を誰が与えるか。
///
/// 既定は [system]。`pair` は同意・PIN の代理入力を持たないのが原則で、
/// 利用者の承認は OS の同意 UI で行う。[app] は Just Works(ConfirmOnly)
/// の同意を呼び出し側が代行する選択で、OS の UI は出ない。PIN の入力・
/// 表示が要るセレモニーはどちらでも代行しない(OS に委ねる)。
enum PairingConsent {
  /// OS が同意 UI を出し、利用者が承認する。
  system,

  /// 呼び出し側が ConfirmOnly を承認する(代行)。OS の UI は出ない。
  app,
}

/// ペアリングの結果。
///
/// **失敗も例外ではなくこの値で返る。**キャンセル・拒否・既に記録ありは
/// 正常系の分岐であって、呼び出し側が次の手を選ぶ材料である。
///
/// **[paired] はリンクが保護されたことを保証しない。**OS のペアリング結果で
/// あって、リンクの状態ではない。保護されたかは属性への実アクセスで確かめる。
enum PairingResult {
  /// 成立した。
  paired,

  /// 既に OS に記録がある。**何も行われていない。**記録が相手の鍵と食い違って
  /// いる可能性は残る（食い違いは接続時に [BluetoothLowEnergyErrorReason.pairingMismatch]
  /// として現れる）。
  alreadyPaired,

  /// 利用者または OS がキャンセルした。
  canceled,

  /// 相手または OS が拒否した。
  rejected,

  /// 時間切れ。
  timeout,

  /// 上記以外の失敗。
  failed,
}

/// 失敗の理由。
///
/// 分岐に使う粗い分類で、細部は [BluetoothLowEnergyException.attCode] が持つ。
enum BluetoothLowEnergyErrorReason {
  /// 相手が居ない。OS が装置を見つけられない。
  deviceNotFound,

  /// リンクが立たない。
  linkFailed,

  /// リンクは立ったが GATT に届かない。
  gattUnreachable,

  /// GATT に届かず、OS に記録が残っている。**記録と相手の鍵が食い違っている。**
  /// 復旧は `unpair`。
  pairingMismatch,

  /// 保護が足りない（ATT 0x05 Insufficient Authentication /
  /// 0x0F Insufficient Encryption）。`pair` するか、装置の昇格を待つ。
  protectionRequired,

  /// 相手が拒否した（保護不足以外の ATT エラー）。[attCode] を見る。
  rejected,

  /// 分類できない失敗。
  unknown,
}

/// 失敗。理由の分類と、あれば生の ATT エラーコードを持つ。
///
/// `pair` 以外の操作はこれを投げる（`pair` は [PairingResult] を返す）。
final class BluetoothLowEnergyException implements Exception {
  /// 理由の分類。
  final BluetoothLowEnergyErrorReason reason;

  /// 標準の ATT エラーコード。ATT の応答に由来する失敗のときだけ入る。
  /// 分類に収まらない装置固有のコードもそのまま保持する。
  final int? attCode;

  /// 人が読むための説明。分岐には [reason] と [attCode] を使うこと。
  final String message;

  /// Constructs a [BluetoothLowEnergyException].
  const BluetoothLowEnergyException(
    this.reason,
    this.message, {
    this.attCode,
  });

  @override
  String toString() {
    final att = attCode;
    final suffix = att == null
        ? ''
        : ' (ATT 0x${att.toRadixString(16).padLeft(2, '0')})';
    return 'BluetoothLowEnergyException(${reason.name}): $message$suffix';
  }
}
