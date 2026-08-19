import 'package:bluetooth_low_energy_platform_interface/bluetooth_low_energy_platform_interface.dart';

/// OS が関連付け(ペアリング)を保持している装置。
///
/// スキャン結果ではなく、OS の関連付けの一覧から得られる。装置が広告して
/// いなくても一覧には出る(接続できるかは別)。
class PairedPeripheral {
  /// 接続などの操作に使う装置。
  final Peripheral peripheral;

  /// OS が記録している表示名。
  final String? name;

  /// `DeviceInformation.Id`(関連付けの識別子)。アドレスと違い、OS が
  /// この装置を指すために使う正式な ID。
  final String id;

  PairedPeripheral({
    required this.peripheral,
    required this.name,
    required this.id,
  });
}
