import 'package:bluetooth_low_energy_platform_interface/bluetooth_low_energy_platform_interface.dart';

import 'api.g.dart';

/// 中立な型(platform_interface の security.dart)と、pigeon 生成型の写像。
///
/// WinRT の語(DevicePairingProtectionLevel 等)はここで吸収し、公開 API には
/// 出さない。

/// 中立な保護水準 → 生成された引数型。
extension PairingProtectionX on PairingProtection {
  /// [DevicePairingProtectionLevelArgs] へ写す。
  DevicePairingProtectionLevelArgs toArgs() {
    switch (this) {
      case PairingProtection.osDefault:
        return DevicePairingProtectionLevelArgs.defaultLevel;
      case PairingProtection.none:
        return DevicePairingProtectionLevelArgs.none;
      case PairingProtection.encryption:
        return DevicePairingProtectionLevelArgs.encryption;
      case PairingProtection.encryptionAndAuthentication:
        return DevicePairingProtectionLevelArgs.encryptionAndAuthentication;
    }
  }
}

/// 中立な同意の主体 → 生成された引数型。
extension PairingConsentX on PairingConsent {
  /// [DevicePairingConsentArgs] へ写す。
  DevicePairingConsentArgs toArgs() {
    switch (this) {
      case PairingConsent.system:
        return DevicePairingConsentArgs.system;
      case PairingConsent.app:
        return DevicePairingConsentArgs.app;
    }
  }
}

/// 生成された結果 → 中立なペアリング結果。
///
/// WinRT の 20 値を分岐に使う粒度へ畳む。細部はネイティブのログが持つ。
extension DevicePairingResultStatusArgsToResultX on DevicePairingResultStatusArgs {
  /// [PairingResult] へ写す。
  PairingResult toResult() {
    switch (this) {
      case DevicePairingResultStatusArgs.paired:
        return PairingResult.paired;
      case DevicePairingResultStatusArgs.alreadyPaired:
        return PairingResult.alreadyPaired;
      case DevicePairingResultStatusArgs.pairingCanceled:
        return PairingResult.canceled;
      case DevicePairingResultStatusArgs.connectionRejected:
      case DevicePairingResultStatusArgs.rejectedByHandler:
      case DevicePairingResultStatusArgs.authenticationNotAllowed:
      case DevicePairingResultStatusArgs.accessDenied:
        return PairingResult.rejected;
      case DevicePairingResultStatusArgs.authenticationTimeout:
        return PairingResult.timeout;
      default:
        return PairingResult.failed;
    }
  }
}
