import 'dart:async';
import 'dart:typed_data';

import 'package:bluetooth_low_energy_platform_interface/bluetooth_low_energy_platform_interface.dart';
import 'package:logging/logging.dart';

import 'api.dart';
import 'pairing.dart';
import 'api.g.dart';
import 'gatt_impl.dart';
import 'peripheral_impl.dart';

/// ログの段付け(呼び出し側のログレベル設定が意味を持つように):
///   severe … 失敗のみ
///   info   … ライフサイクル(スキャン開始/停止・接続/切断・ペアリング・
///            MTU)。頻度は操作回数に比例し、洪水しない
///   fine   … GATT 操作単位(read/write/subscribe。値のダンプは載せない)
///   finer  … イベント毎すべて(広告 1 件・通知 1 件・値ダンプ・診断)
Logger get _logger => Logger('CentralManager');

final class DiscoveryArgs {
  final PeripheralArgs peripheralArgs;
  final int rssiArgs;
  final int timestampArgs;
  final AdvertisementTypeArgs typeArgs;
  final AdvertisementArgs advertisementArgs;

  DiscoveryArgs(
    this.peripheralArgs,
    this.rssiArgs,
    this.timestampArgs,
    this.typeArgs,
    this.advertisementArgs,
  );
}

final class CentralManagerImpl
    implements CentralManager, CentralManagerFlutterApi {
  final CentralManagerHostApi _api;
  final StreamController<BluetoothLowEnergyStateChangedEventArgs>
  _stateChangedController;
  final StreamController<DiscoveredEventArgs> _discoveredController;
  final StreamController<PeripheralConnectionStateChangedEventArgs>
  _connectionStateChangedController;
  final StreamController<PeripheralMTUChangedEventArgs> _mtuChangedController;
  final StreamController<GATTCharacteristicNotifiedEventArgs>
  _characteristicNotifiedController;
  final Map<int, DiscoveryArgs> _discoveriesArgs;

  BluetoothLowEnergyState _state;

  CentralManagerImpl()
    : _api = CentralManagerHostApi(),
      _stateChangedController = StreamController.broadcast(),
      _discoveredController = StreamController.broadcast(),
      _connectionStateChangedController = StreamController.broadcast(),
      _mtuChangedController = StreamController.broadcast(),
      _characteristicNotifiedController = StreamController.broadcast(),
      _discoveriesArgs = {},
      _state = BluetoothLowEnergyState.unknown {
    CentralManagerFlutterApi.setUp(this);
    _initialize();
  }

  @override
  BluetoothLowEnergyState get state => _state;
  @override
  Stream<BluetoothLowEnergyStateChangedEventArgs> get stateChanged =>
      _stateChangedController.stream;
  @override
  Stream<DiscoveredEventArgs> get discovered => _discoveredController.stream;
  @override
  Stream<PeripheralConnectionStateChangedEventArgs>
  get connectionStateChanged => _connectionStateChangedController.stream;
  @override
  Stream<PeripheralMTUChangedEventArgs> get mtuChanged =>
      _mtuChangedController.stream;
  @override
  Stream<GATTCharacteristicNotifiedEventArgs> get characteristicNotified =>
      _characteristicNotifiedController.stream;

  @override
  Future<bool> authorize() {
    throw UnsupportedError('authorize is not supported on Windows.');
  }

  @override
  Future<void> showAppSettings() {
    throw UnsupportedError('showAppSettings is not supported on Windows.');
  }

  @override
  Future<void> startDiscovery({List<UUID>? serviceUUIDs}) async {
    _discoveriesArgs.clear();
    final serviceUUIDsArgs =
        serviceUUIDs?.map((uuid) => uuid.toArgs()).toList() ?? [];
    _logger.info('startDiscovery: $serviceUUIDsArgs');
    await _api.startDiscovery(serviceUUIDsArgs);
  }

  @override
  Future<void> stopDiscovery() async {
    _logger.info('stopDiscovery');
    await _api.stopDiscovery();
  }

  @override
  Future<Peripheral> getPeripheral(String address) {
    // TODO: Check is this supported on Windows.
    throw UnsupportedError('getPeripheral is not supported on Windows.');
  }

  @override
  Future<List<Peripheral>> retrieveConnectedPeripherals() {
    throw UnsupportedError(
      'retrieveConnectedPeripherals is not supported on Windows.',
    );
  }

  @override
  Future<void> connect(Peripheral peripheral) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    _logger.info('connect: $addressArgs');
    await _api.connect(addressArgs);
  }

  @override
  Future<void> disconnect(Peripheral peripheral) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    _logger.info('disconnect: $addressArgs');
    await _api.disconnect(addressArgs);
  }

  @override
  Future<int> requestMTU(Peripheral peripheral, {required int mtu}) {
    throw UnsupportedError('requestMTU is not supported on Windows.');
  }

  @override
  Future<int> getMaximumWriteLength(
    Peripheral peripheral, {
    required GATTCharacteristicWriteType type,
  }) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    _logger.info('getMTU: $addressArgs');
    final mtuArgs = await _api.getMTU(addressArgs);
    final maximumWriteLength = (mtuArgs - 3).clamp(20, 512);
    return maximumWriteLength;
  }

  @override
  Future<int> readRSSI(Peripheral peripheral) async {
    throw UnsupportedError('readRSSI is not supported on Windows.');
  }

  @override
  Future<List<GATTService>> discoverGATT(Peripheral peripheral) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final servicesArgs = await _getServices(
      addressArgs,
      CacheModeArgs.uncached,
    );
    final services = servicesArgs.map((args) => args.toService()).toList();
    return services;
  }

  @override
  Future<Uint8List> readCharacteristic(
    Peripheral peripheral,
    GATTCharacteristic characteristic,
  ) async {
    if (peripheral is! PeripheralImpl ||
        characteristic is! GATTCharacteristicImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final handleArgs = characteristic.handle;
    const modeArgs = CacheModeArgs.uncached;
    _logger.fine('readCharacteristic: $addressArgs.$handleArgs - $modeArgs');
    final value = await _api.readCharacteristic(
      addressArgs,
      handleArgs,
      modeArgs,
    );
    return value;
  }

  @override
  Future<void> writeCharacteristic(
    Peripheral peripheral,
    GATTCharacteristic characteristic, {
    required Uint8List value,
    required GATTCharacteristicWriteType type,
  }) async {
    if (peripheral is! PeripheralImpl ||
        characteristic is! GATTCharacteristicImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final handleArgs = characteristic.handle;
    final valueArgs = value;
    final typeArgs = type.toArgs();
    // 値のダンプは載せない(fine は GATT 操作単位の記録。生値が要る調査は
    // finer の通知ログ側で行う)。
    _logger.fine(
      'writeCharacteristic: $addressArgs.$handleArgs - '
      '${valueArgs.length} bytes, $typeArgs',
    );
    await _api.writeCharacteristic(
      addressArgs,
      handleArgs,
      valueArgs,
      typeArgs,
    );
  }

  @override
  Future<void> setCharacteristicNotifyState(
    Peripheral peripheral,
    GATTCharacteristic characteristic, {
    required bool state,
  }) async {
    if (peripheral is! PeripheralImpl ||
        characteristic is! GATTCharacteristicImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final handleArgs = characteristic.handle;
    final stateArgs = state
        ? characteristic.properties.contains(GATTCharacteristicProperty.notify)
              ? GATTCharacteristicNotifyStateArgs.notify
              : GATTCharacteristicNotifyStateArgs.indicate
        : GATTCharacteristicNotifyStateArgs.none;
    _logger.fine(
      'setCharacteristicNotifyState: $addressArgs.$handleArgs - $stateArgs',
    );
    await _api.setCharacteristicNotifyState(addressArgs, handleArgs, stateArgs);
  }

  /// 特性への無線通信に要求する GATT セキュリティ
  /// (`GattCharacteristic.ProtectionLevel`)を設定する。
  ///
  /// 設定後にその特性へ read/write すると、スタックは要求水準を満たす
  /// リンク(暗号化等)を操作の前提として確立しようとする。装置側の
  /// セキュリティ要求と一致させることで、ATT エラー(0x0F 等)を起点と
  /// した事後の再試行ではなく、OS 主導の事前確立に切り替えられる。
  Future<void> setCharacteristicProtectionLevel(
    Peripheral peripheral,
    GATTCharacteristic characteristic, {
    required GATTProtectionLevel level,
  }) async {
    if (peripheral is! PeripheralImpl ||
        characteristic is! GATTCharacteristicImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final handleArgs = characteristic.handle;
    _logger.info(
      'setCharacteristicProtectionLevel: $addressArgs.$handleArgs - $level',
    );
    await _api.setCharacteristicProtectionLevel(
      addressArgs,
      handleArgs,
      level.toArgs(),
    );
  }

  @override
  Future<Uint8List> readDescriptor(
    Peripheral peripheral,
    GATTDescriptor descriptor,
  ) async {
    if (peripheral is! PeripheralImpl || descriptor is! GATTDescriptorImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final handleArgs = descriptor.handle;
    const modeArgs = CacheModeArgs.uncached;
    _logger.fine('readDescriptor: $addressArgs.$handleArgs - $modeArgs');
    final value = await _api.readDescriptor(addressArgs, handleArgs, modeArgs);
    return value;
  }

  @override
  Future<void> writeDescriptor(
    Peripheral peripheral,
    GATTDescriptor descriptor, {
    required Uint8List value,
  }) async {
    if (peripheral is! PeripheralImpl || descriptor is! GATTDescriptorImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final handleArgs = descriptor.handle;
    final valueArgs = value;
    _logger.fine(
      'writeDescriptor: $addressArgs.$handleArgs - ${valueArgs.length} bytes',
    );
    await _api.writeDescriptor(addressArgs, handleArgs, valueArgs);
  }

  /// ペアリング(暗号化リンクの確立)を開始し、結果が出るまで待つ。
  ///
  /// 保護された属性は、リンクが暗号化されていないと読み書きできない。
  /// ボンディングしない装置では鍵が保存されないため、**接続ごとに**
  /// これを済ませてから初期読み出しへ進む必要がある。
  ///
  /// デスクトップでは同意は必ずシステムダイアログで行われ、アプリからは
  /// 抑止できない(Microsoft の文書と実測の両方で確認)。承認完了まで
  /// この Future は解決しない(初回は利用者の操作時間がかかる)。
  ///
  /// 失敗は例外にせず [DevicePairingResultStatus] として返す。呼び出し側は
  /// キャンセル・タイムアウト・拒否を区別できる。
  ///
  /// [protectionLevel] は要求する保護レベル。装置側のセキュリティ要求
  /// (暗号化必須の GATT 等)に合わせて指定する。既定はライブラリとして
  /// 中立な [DevicePairingProtectionLevel.defaultLevel](OS に選ばせる)。
  Future<DevicePairingResultStatus> pair(
    Peripheral peripheral, {
    DevicePairingProtectionLevel protectionLevel =
        DevicePairingProtectionLevel.defaultLevel,
  }) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    _logger.info('pair: $addressArgs (protectionLevel: $protectionLevel)');
    final statusArgs = await _api.pair(addressArgs, protectionLevel.toArgs());
    _logger.info('pair: $addressArgs -> $statusArgs');
    return statusArgs.toStatus();
  }

  /// OS が保持しているペアリング(関連付け)を解除する。接続は不要。
  Future<void> unpair(Peripheral peripheral) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    _logger.info('unpair: $addressArgs');
    await _api.unpair(addressArgs);
  }

  /// OS がこの装置をペアリング済みとみなしているか。接続は不要。
  Future<bool> isPaired(Peripheral peripheral) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    final paired = await _api.isPaired(addressArgs);
    _logger.info('isPaired: $addressArgs -> $paired');
    return paired;
  }

  /// `GattSession.MaintainConnection` 方式で接続を開始する。
  ///
  /// [connect](GATT 操作起点)の接続待ちは OS 内部で 7 秒固定・キャンセル
  /// 不可(公式文書)。こちらは MaintainConnection を true にして
  /// 「デバイスが現れ次第 OS が接続する」を無期限で依頼し、**リンク確立を
  /// 待たずに返る**。確立は connectionStateChanged(connected)で通知される
  /// ため、待ち時間の上限と中断([disconnect] = 参照解放で依頼ごと消える)は
  /// 呼び出し側が管理する。
  ///
  /// true のままだとリンク断のたびに OS が自動で張り直すため、確立後は
  /// [setMaintainConnection] で false へ戻すこと(再接続の主導権をアプリに
  /// 残す)。
  Future<void> connectMaintained(Peripheral peripheral) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    _logger.info('connectMaintained: $addressArgs');
    await _api.connectMaintained(addressArgs);
  }

  /// `GattSession.MaintainConnection` を設定する。
  /// [connectMaintained] で確立した後に false へ戻す用途。
  /// セッション未保持(未接続)の装置に対してはエラー。
  Future<void> setMaintainConnection(
    Peripheral peripheral, {
    required bool enable,
  }) async {
    if (peripheral is! PeripheralImpl) {
      throw TypeError();
    }
    final addressArgs = peripheral.address;
    _logger.info('setMaintainConnection: $addressArgs - $enable');
    await _api.setMaintainConnection(addressArgs, enable);
  }

  @override
  void onStateChanged(BluetoothLowEnergyStateArgs stateArgs) {
    _logger.info('onStateChanged: $stateArgs');
    final state = stateArgs.toState();
    if (_state == state) {
      return;
    }
    _state = state;
    final eventArgs = BluetoothLowEnergyStateChangedEventArgs(state);
    _stateChangedController.add(eventArgs);
  }

  @override
  void onDiscovered(
    PeripheralArgs peripheralArgs,
    int rssiArgs,
    int timestampArgs,
    AdvertisementTypeArgs typeArgs,
    AdvertisementArgs advertisementArgs,
  ) {
    final addressArgs = peripheralArgs.addressArgs;
    // 広告 1 件ごとに発火する(数件/秒)ため finer。
    _logger.finer(
      'onDiscovered: $addressArgs - $rssiArgs, $timestampArgs, $typeArgs, $advertisementArgs',
    );
    if (typeArgs == AdvertisementTypeArgs.connectableDirected ||
        typeArgs == AdvertisementTypeArgs.nonConnectableUndirected ||
        typeArgs == AdvertisementTypeArgs.extended) {
      // No need to wait SCAN_REQ.
      final peripheral = peripheralArgs.toPeripheral();
      final rssi = rssiArgs;
      final advertisement = advertisementArgs.toAdvertisement();
      final eventArgs = DiscoveredEventArgs(peripheral, rssi, advertisement);
      _discoveredController.add(eventArgs);
    } else {
      final oldDiscoveryArgs = _discoveriesArgs.remove(addressArgs);
      final newDiscoveryArgs = DiscoveryArgs(
        peripheralArgs,
        rssiArgs,
        timestampArgs,
        typeArgs,
        advertisementArgs,
      );
      // TODO: Should we ignore this?
      final ignored =
          oldDiscoveryArgs == null ||
          _checkDiscoveryArgs(oldDiscoveryArgs, newDiscoveryArgs);
      if (ignored) {
        // Note that ADV_IND will be ignored if the advertiser never reply the
        // SCAN_REQ.
        _discoveriesArgs[addressArgs] = newDiscoveryArgs;
      } else {
        final peripheral = oldDiscoveryArgs.peripheralArgs.toPeripheral();
        final rssi = oldDiscoveryArgs.rssiArgs;
        final oldAdvertisement = typeArgs == AdvertisementTypeArgs.scanResponse
            ? oldDiscoveryArgs.advertisementArgs.toAdvertisement()
            : advertisementArgs.toAdvertisement();
        final newAdvertisement = typeArgs == AdvertisementTypeArgs.scanResponse
            ? advertisementArgs.toAdvertisement()
            : oldDiscoveryArgs.advertisementArgs.toAdvertisement();
        final name = newAdvertisement.name?.isNotEmpty == true
            ? newAdvertisement.name
            : oldAdvertisement.name;
        final serviceUUIDs = {
          ...oldAdvertisement.serviceUUIDs,
          ...newAdvertisement.serviceUUIDs,
        }.toList();
        final serviceData = {
          ...oldAdvertisement.serviceData,
          ...newAdvertisement.serviceData,
        };
        final manufacturerSpecificData = [
          ...oldAdvertisement.manufacturerSpecificData,
          ...newAdvertisement.manufacturerSpecificData,
        ];
        final advertisement = Advertisement(
          name: name,
          serviceUUIDs: serviceUUIDs,
          serviceData: serviceData,
          manufacturerSpecificData: manufacturerSpecificData,
        );
        final eventArgs = DiscoveredEventArgs(peripheral, rssi, advertisement);
        _discoveredController.add(eventArgs);
      }
    }
  }

  @override
  void onConnectionStateChanged(
    PeripheralArgs peripheralArgs,
    ConnectionStateArgs stateArgs,
  ) {
    final addressArgs = peripheralArgs.addressArgs;
    _logger.info('onConnectionStateChanged: $addressArgs - $stateArgs');
    final peripheral = peripheralArgs.toPeripheral();
    final state = stateArgs.toState();
    final eventArgs = PeripheralConnectionStateChangedEventArgs(
      peripheral,
      state,
    );
    _connectionStateChangedController.add(eventArgs);
  }

  @override
  void onMTUChanged(PeripheralArgs peripheralArgs, int mtuArgs) {
    final addressArgs = peripheralArgs.addressArgs;
    _logger.info('onMTUChanged: $addressArgs - $mtuArgs');
    final peripheral = peripheralArgs.toPeripheral();
    final mtu = mtuArgs;
    final eventArgs = PeripheralMTUChangedEventArgs(peripheral, mtu);
    _mtuChangedController.add(eventArgs);
  }

  @override
  void onCharacteristicNotified(
    PeripheralArgs peripheralArgs,
    GATTCharacteristicArgs characteristicArgs,
    Uint8List valueArgs,
  ) {
    final addressArgs = peripheralArgs.addressArgs;
    final handleArgs = characteristicArgs.handleArgs;
    // 通知 1 件ごとに発火する(ストリーミング中は数十件/秒)ため finer。
    _logger.finer(
      'onCharacteristicNotified: $addressArgs.$handleArgs - $valueArgs',
    );
    final peripheral = peripheralArgs.toPeripheral();
    final characteristic = characteristicArgs.toCharacteristic();
    final value = valueArgs;
    final eventArgs = GATTCharacteristicNotifiedEventArgs(
      peripheral,
      characteristic,
      value,
    );
    _characteristicNotifiedController.add(eventArgs);
  }

  @override
  void onLogged(String messageArgs) {
    // ネイティブ層の診断メッセージの中継(ネイティブに独自のログ機構が
    // 無いため)。調査用の観測値なので info。
    _logger.info(messageArgs);
  }

  Future<void> _initialize() async {
    // Here we use `Future()` to make it possible to change the `logLevel` before `initialize()`.
    await Future(() async {
      try {
        _logger.info('initialize');
        await _api.initialize();
        _getState();
      } catch (e) {
        _logger.severe('initialize failed.', e);
      }
    });
  }

  Future<void> _getState() async {
    try {
      _logger.info('getState');
      final stateArgs = await _api.getState();
      onStateChanged(stateArgs);
    } catch (e) {
      _logger.severe('getState failed.', e);
    }
  }

  Future<List<GATTServiceArgs>> _getServices(
    int addressArgs,
    CacheModeArgs modeArgs,
  ) async {
    _logger.info('getServices: $addressArgs - $modeArgs');
    final servicesArgs = await _api.getServices(addressArgs, modeArgs);
    for (var serviceArgs in servicesArgs) {
      final handleArgs = serviceArgs.handleArgs;
      final includedServicesArgs = await _getIncludedServices(
        addressArgs,
        handleArgs,
        modeArgs,
      );
      serviceArgs.includedServicesArgs = includedServicesArgs;
      final characteristicsArgs = await _getCharacteristics(
        addressArgs,
        handleArgs,
        modeArgs,
      );
      serviceArgs.characteristicsArgs = characteristicsArgs;
    }
    return servicesArgs;
  }

  Future<List<GATTServiceArgs>> _getIncludedServices(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  ) async {
    _logger.info('getIncludedServices: $addressArgs.$handleArgs - $modeArgs');
    final servicesArgs = await _api.getIncludedServices(
      addressArgs,
      handleArgs,
      modeArgs,
    );
    for (var serviceArgs in servicesArgs) {
      final handleArgs = serviceArgs.handleArgs;
      final includedServicesArgs = await _getIncludedServices(
        addressArgs,
        handleArgs,
        modeArgs,
      );
      serviceArgs.includedServicesArgs = includedServicesArgs;
      final characteristicsArgs = await _getCharacteristics(
        addressArgs,
        handleArgs,
        modeArgs,
      );
      serviceArgs.characteristicsArgs = characteristicsArgs;
    }
    return servicesArgs;
  }

  Future<List<GATTCharacteristicArgs>> _getCharacteristics(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  ) async {
    _logger.info('getCharacteristics: $addressArgs.$handleArgs - $modeArgs');
    final characteristicsArgs = await _api.getCharacteristics(
      addressArgs,
      handleArgs,
      modeArgs,
    );
    for (var characteristicArgs in characteristicsArgs) {
      final handleArgs = characteristicArgs.handleArgs;
      final descriptorsArgs = await _getDescriptors(
        addressArgs,
        handleArgs,
        modeArgs,
      );
      characteristicArgs.descriptorsArgs = descriptorsArgs;
    }
    return characteristicsArgs;
  }

  Future<List<GATTDescriptorArgs>> _getDescriptors(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  ) async {
    _logger.info('getDescriptors: $addressArgs,$handleArgs - $modeArgs');
    final descriptorsArgs = await _api.getDescriptors(
      addressArgs,
      handleArgs,
      modeArgs,
    );
    return descriptorsArgs;
  }

  bool _checkDiscoveryArgs(
    DiscoveryArgs oldDiscoveryArgs,
    DiscoveryArgs newDiscoveryArgs,
  ) {
    final oldAddressArgs = oldDiscoveryArgs.peripheralArgs.addressArgs;
    final newAddressArgs = newDiscoveryArgs.peripheralArgs.addressArgs;
    if (oldAddressArgs != newAddressArgs) {
      _logger.finer(
        'ignored by different addressArgs $oldAddressArgs, $newAddressArgs',
      );
      return true;
    }
    final address = (newAddressArgs & 0xFFFFFFFFFFFF)
        .toRadixString(16)
        .padLeft(12, '0');
    if (oldDiscoveryArgs.typeArgs == newDiscoveryArgs.typeArgs) {
      _logger.finer(
        'ignored by same typeArgs $address: ${oldDiscoveryArgs.typeArgs}:${oldDiscoveryArgs.timestampArgs}, ${newDiscoveryArgs.typeArgs}:${newDiscoveryArgs.timestampArgs}',
      );
      return true;
    }
    if (oldDiscoveryArgs.typeArgs != AdvertisementTypeArgs.scanResponse &&
        newDiscoveryArgs.typeArgs != AdvertisementTypeArgs.scanResponse) {
      _logger.finer(
        'ignored by wrong typeArgs $address:  ${oldDiscoveryArgs.typeArgs}:${oldDiscoveryArgs.timestampArgs}, ${newDiscoveryArgs.typeArgs}:${newDiscoveryArgs.timestampArgs}',
      );
      return true;
    }
    final interval =
        newDiscoveryArgs.typeArgs == AdvertisementTypeArgs.scanResponse
        ? newDiscoveryArgs.timestampArgs - oldDiscoveryArgs.timestampArgs
        : oldDiscoveryArgs.timestampArgs - newDiscoveryArgs.timestampArgs;
    final ignored = interval < 0 || interval > 1000;
    if (ignored) {
      _logger.finer(
        'ignored by wrong timestampArgs $address: $interval, ${oldDiscoveryArgs.typeArgs}:${oldDiscoveryArgs.timestampArgs}, ${newDiscoveryArgs.typeArgs}:${newDiscoveryArgs.timestampArgs}',
      );
    }
    return ignored;
  }
}

final class CentralManagerChannelImpl extends CentralManagerChannel {
  @override
  CentralManager create() => CentralManagerImpl();
}
