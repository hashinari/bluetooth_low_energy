import 'dart:typed_data';

import 'package:plugin_platform_interface/plugin_platform_interface.dart';

import 'bluetooth_low_energy_manager.dart';
import 'event_args.dart';
import 'gatt.dart';
import 'peripheral.dart';
import 'security.dart';
import 'uuid.dart';

/// An object that scans for, discovers, connects to, and manages peripherals.
abstract interface class CentralManager implements BluetoothLowEnergyManager {
  static CentralManager? _instance;

  /// Gets the instance of [CentralManager] to use.
  factory CentralManager() {
    var instance = _instance;
    if (instance == null) {
      _instance = instance = CentralManagerChannel.instance.create();
    }
    return instance;
  }

  /// Tells the central manager discovered a peripheral while scanning for devices.
  Stream<DiscoveredEventArgs> get discovered;

  /// Tells that retrieving the specified peripheral's connection state changed.
  Stream<PeripheralConnectionStateChangedEventArgs> get connectionStateChanged;

  /// Callback indicating the MTU for a given device connection has changed.
  ///
  /// This callback is triggered in response to the BluetoothGatt#requestMtu
  /// function, or in response to a connection event.
  ///
  /// This event is available on Android and Windows, throws [UnsupportedError]
  /// on other platforms.
  Stream<PeripheralMTUChangedEventArgs> get mtuChanged;

  /// Tells that retrieving the specified characteristic’s value changed.
  Stream<GATTCharacteristicNotifiedEventArgs> get characteristicNotified;

  /// Scans for peripherals that are advertising services.
  ///
  /// The [serviceUUIDs] argument is an array of [UUID] objects that the app is
  /// interested in. Each [UUID] object represents the [UUID] of a service that
  /// a peripheral advertises.
  Future<void> startDiscovery({List<UUID>? serviceUUIDs});

  /// Asks the central manager to stop scanning for peripherals.
  Future<void> stopDiscovery();

  /// Get a peripheral object for the given bluetooth hardware address.
  ///
  /// This method is available on Android, throws [UnsupportedError] on other platforms.
  Future<Peripheral> getPeripheral(String address);

  /// Returns a list of the peripherals connected to the system.
  ///
  /// This method is available on Android, iOS, macOS and Linux, throws
  /// [UnsupportedError] on other platforms.
  Future<List<Peripheral>> retrieveConnectedPeripherals();

  /// Establishes a local connection to a peripheral.
  ///
  /// 保護されていない GATT DB に接続できる状態にして返る。サービスの列挙結果は
  /// 返さない（取るのは [discoverGATT]）。失敗は [BluetoothLowEnergyException]
  /// で理由が分かれる — リンクが立たないのか、リンクは立ったが GATT に
  /// 届かないのか。後者で OS に記録が残っていれば
  /// [BluetoothLowEnergyErrorReason.pairingMismatch]（復旧は [unpair]）。
  ///
  /// [maintain] は、リンクが落ちたときに OS へ張り直させるかの指定。既定は
  /// 維持あり。維持中の再確立は切断・接続として [connectionStateChanged] に
  /// 届き、ハンドルはそのまま使える。通知の購読（CCCD）は相手側が切断で
  /// 落とすため、張り直しは呼び出し側が行う。**維持しない接続は、Windows では
  /// アイドルになった時点で落ちうる**（リンクを保持するのは維持依頼か実行中の
  /// GATT 操作だけであるため）。
  ///
  /// 待ちの上限は持たない。切り上げるときは呼び出し側が `.timeout()` を掛け、
  /// [disconnect] で中断する。
  Future<void> connect(Peripheral peripheral, {bool maintain = true});

  /// Cancels an active or pending local connection to a peripheral.
  Future<void> disconnect(Peripheral peripheral);

  /// ペアリングを開始し、結果が出るまで待つ。**保護を得るための操作**である。
  ///
  /// 失敗は例外にせず [PairingResult] で返す。キャンセル・拒否・既に記録ありは
  /// 正常系の分岐であって、呼び出し側が次の手を選ぶ材料である。
  ///
  /// **戻り値は OS のペアリング結果であり、リンクが保護されたことは保証
  /// しない。**保護されたかは属性への実アクセスで確かめる。OS に記録が残って
  /// いると何もせず [PairingResult.alreadyPaired] で返るため、鍵を保存しない
  /// 相手では切断ごとの [unpair] と対にする。
  ///
  /// [protection] は要求する保護の水準。指定できないプラットフォームでは
  /// 無視される。
  ///
  /// This method is available on Windows, throws [UnsupportedError] on other
  /// platforms.
  Future<PairingResult> pair(
    Peripheral peripheral, {
    PairingProtection protection = PairingProtection.osDefault,
  });

  /// OS が保持しているペアリングの記録を消す。接続は不要。
  ///
  /// This method is available on Windows, throws [UnsupportedError] on other
  /// platforms.
  Future<void> unpair(Peripheral peripheral);

  /// OS がこの相手の記録を持っているかを照会する。接続は不要。
  ///
  /// **リンクが保護されているかは、これでは分からない。**記録があっても保護
  /// されているとは限らない。保護は属性への実アクセスで確かめる。
  ///
  /// This method is available on Windows, throws [UnsupportedError] on other
  /// platforms.
  Future<bool> isPaired(Peripheral peripheral);

  /// Request an MTU size used for a given connection. Please note that starting
  /// from Android 14, the Android Bluetooth stack requests the BLE ATT MTU to
  /// 517 bytes when the first GATT client requests an MTU, and disregards all
  /// subsequent MTU requests. Check out [MTU is set to 517 for the first GATT
  /// client requesting an MTU](https://developer.android.com/about/versions/14/behavior-changes-all#mtu-set-to-517)
  /// for more information.
  ///
  /// This method is available on Android, throws [UnsupportedError] on other
  /// platforms.
  Future<int> requestMTU(Peripheral peripheral, {required int mtu});

  /// The maximum amount of data, in bytes, you can send to a characteristic in
  /// a single write type.
  Future<int> getMaximumWriteLength(
    Peripheral peripheral, {
    required GATTCharacteristicWriteType type,
  });

  /// Retrieves the current RSSI value for the peripheral while connected to the
  /// central manager.
  ///
  /// This method is available on Android, iOS, macOS and Linux, throws
  /// [UnsupportedError] on other platforms.
  Future<int> readRSSI(Peripheral peripheral);

  /// Discovers the GATT services, characteristics and descriptors of the peripheral.
  ///
  /// [cached] にキャッシュを使うかを指定する（既定は使わない＝毎回実際に探索する）。
  /// Windows は指定どおりに動く。キャッシュを OS が管理するプラットフォームは、
  /// 指定に関わらず OS の規定で動く。
  Future<List<GATTService>> discoverGATT(
    Peripheral peripheral, {
    bool cached = false,
  });

  /// Retrieves the value of a specified characteristic.
  Future<Uint8List> readCharacteristic(
    Peripheral peripheral,
    GATTCharacteristic characteristic,
  );

  /// Writes the value of a characteristic.
  Future<void> writeCharacteristic(
    Peripheral peripheral,
    GATTCharacteristic characteristic, {
    required Uint8List value,
    required GATTCharacteristicWriteType type,
  });

  /// Sets notifications or indications for the value of a specified characteristic.
  Future<void> setCharacteristicNotifyState(
    Peripheral peripheral,
    GATTCharacteristic characteristic, {
    required bool state,
  });

  /// Retrieves the value of a specified characteristic descriptor.
  Future<Uint8List> readDescriptor(
    Peripheral peripheral,
    GATTDescriptor descriptor,
  );

  /// Writes the value of a characteristic descriptor.
  Future<void> writeDescriptor(
    Peripheral peripheral,
    GATTDescriptor descriptor, {
    required Uint8List value,
  });
}

/// Platform-specific implementations should implement this class to support
/// [CentralManagerChannel].
abstract base class CentralManagerChannel extends PlatformInterface {
  static final Object _token = Object();

  static CentralManagerChannel? _instance;

  /// The default instance of [CentralManagerChannel] to use.
  static CentralManagerChannel get instance {
    final instance = _instance;
    if (instance == null) {
      throw UnimplementedError(
        'CentralManager is not implemented on this platform.',
      );
    }
    return instance;
  }

  /// Platform-specific implementations should set this with their own
  /// platform-specific class that extends [CentralManagerChannel] when
  /// they register themselves.
  static set instance(CentralManagerChannel instance) {
    PlatformInterface.verifyToken(instance, _token);
    _instance = instance;
  }

  /// Constructs a [CentralManagerChannel].
  CentralManagerChannel() : super(token: _token);

  CentralManager create();
}
