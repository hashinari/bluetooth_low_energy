import 'api.g.dart';

/// ペアリングの結果。Windows の `DevicePairingResultStatus` に対応する。
///
/// 文字列へ潰さず列挙で返すことで、呼び出し側が「利用者がキャンセルした」
/// 「認証がタイムアウトした」「拒否された」を区別できる。
enum DevicePairingResultStatus {
  /// 成立した。
  paired,

  /// ペアリングできる状態にない。
  notReadyToPair,

  /// ペアリングされなかった。
  notPaired,

  /// 既にペアリング済み。
  alreadyPaired,

  /// 接続を拒否された。
  connectionRejected,

  /// 接続数が上限。
  tooManyConnections,

  /// ハードウェア障害。
  hardwareFailure,

  /// 認証がタイムアウトした。
  authenticationTimeout,

  /// 認証が許可されていない。
  authenticationNotAllowed,

  /// 認証に失敗した。
  authenticationFailure,

  /// 対応プロファイルが無い。
  noSupportedProfiles,

  /// 要求した保護レベルを満たせなかった。
  protectionLevelCouldNotBeMet,

  /// アクセスが拒否された。
  accessDenied,

  /// セレモニーのデータが不正。
  invalidCeremonyData,

  /// 利用者がキャンセルした。
  pairingCanceled,

  /// 別のペアリングが進行中。
  operationAlreadyInProgress,

  /// 必要なハンドラが登録されていない。
  requiredHandlerNotRegistered,

  /// ハンドラが拒否した。
  rejectedByHandler,

  /// 相手が既に別の関連付けを持っている。
  remoteDeviceHasAssociation,

  /// 上記以外の失敗。理由は不明。
  failed,
}

/// 生成された引数型 → 公開する列挙。
extension DevicePairingResultStatusArgsX on DevicePairingResultStatusArgs {
  /// [DevicePairingResultStatus] へ写す。
  DevicePairingResultStatus toStatus() {
    switch (this) {
      case DevicePairingResultStatusArgs.paired:
        return DevicePairingResultStatus.paired;
      case DevicePairingResultStatusArgs.notReadyToPair:
        return DevicePairingResultStatus.notReadyToPair;
      case DevicePairingResultStatusArgs.notPaired:
        return DevicePairingResultStatus.notPaired;
      case DevicePairingResultStatusArgs.alreadyPaired:
        return DevicePairingResultStatus.alreadyPaired;
      case DevicePairingResultStatusArgs.connectionRejected:
        return DevicePairingResultStatus.connectionRejected;
      case DevicePairingResultStatusArgs.tooManyConnections:
        return DevicePairingResultStatus.tooManyConnections;
      case DevicePairingResultStatusArgs.hardwareFailure:
        return DevicePairingResultStatus.hardwareFailure;
      case DevicePairingResultStatusArgs.authenticationTimeout:
        return DevicePairingResultStatus.authenticationTimeout;
      case DevicePairingResultStatusArgs.authenticationNotAllowed:
        return DevicePairingResultStatus.authenticationNotAllowed;
      case DevicePairingResultStatusArgs.authenticationFailure:
        return DevicePairingResultStatus.authenticationFailure;
      case DevicePairingResultStatusArgs.noSupportedProfiles:
        return DevicePairingResultStatus.noSupportedProfiles;
      case DevicePairingResultStatusArgs.protectionLevelCouldNotBeMet:
        return DevicePairingResultStatus.protectionLevelCouldNotBeMet;
      case DevicePairingResultStatusArgs.accessDenied:
        return DevicePairingResultStatus.accessDenied;
      case DevicePairingResultStatusArgs.invalidCeremonyData:
        return DevicePairingResultStatus.invalidCeremonyData;
      case DevicePairingResultStatusArgs.pairingCanceled:
        return DevicePairingResultStatus.pairingCanceled;
      case DevicePairingResultStatusArgs.operationAlreadyInProgress:
        return DevicePairingResultStatus.operationAlreadyInProgress;
      case DevicePairingResultStatusArgs.requiredHandlerNotRegistered:
        return DevicePairingResultStatus.requiredHandlerNotRegistered;
      case DevicePairingResultStatusArgs.rejectedByHandler:
        return DevicePairingResultStatus.rejectedByHandler;
      case DevicePairingResultStatusArgs.remoteDeviceHasAssociation:
        return DevicePairingResultStatus.remoteDeviceHasAssociation;
      case DevicePairingResultStatusArgs.failed:
        return DevicePairingResultStatus.failed;
    }
  }
}
