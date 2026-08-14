// Run with `dart run pigeon --input api.dart`.
import 'package:pigeon/pigeon.dart';

// TODO: Use `@ProxyApi` to manage instancs when this feature released:
// https://github.com/flutter/flutter/issues/147486
@ConfigurePigeon(
  PigeonOptions(
    dartOut: 'lib/src/api.g.dart',
    dartOptions: DartOptions(),
    cppHeaderOut: 'windows/bluetooth_low_energy_api.g.h',
    cppSourceOut: 'windows/bluetooth_low_energy_api.g.cpp',
    cppOptions: CppOptions(namespace: 'bluetooth_low_energy_windows'),
  ),
)
enum BluetoothLowEnergyStateArgs { unknown, unsupported, disabled, off, on }

enum AdvertisementTypeArgs {
  connectableUndirected,
  connectableDirected,
  scannableUndirected,
  nonConnectableUndirected,
  scanResponse,
  extended,
}

enum ConnectionStateArgs { disconnected, connected }

/// Windows の `DevicePairingResultStatus` をそのまま写したもの。
///
/// 文字列へ潰さず列挙で返すことで、呼び出し側が
/// 「利用者がキャンセルした」「認証がタイムアウトした」「拒否された」を
/// 区別できる。
enum DevicePairingResultStatusArgs {
  paired,
  notReadyToPair,
  notPaired,
  alreadyPaired,
  connectionRejected,
  tooManyConnections,
  hardwareFailure,
  authenticationTimeout,
  authenticationNotAllowed,
  authenticationFailure,
  noSupportedProfiles,
  protectionLevelCouldNotBeMet,
  accessDenied,
  invalidCeremonyData,
  pairingCanceled,
  operationAlreadyInProgress,
  requiredHandlerNotRegistered,
  rejectedByHandler,
  remoteDeviceHasAssociation,
  failed,
}

enum GATTCharacteristicPropertyArgs {
  read,
  write,
  writeWithoutResponse,
  notify,
  indicate,
}

enum GATTCharacteristicWriteTypeArgs { withResponse, withoutResponse }

enum GATTCharacteristicNotifyStateArgs { none, notify, indicate }

enum GATTProtectionLevelArgs {
  plain,
  authenticationRequired,
  entryptionRequired,
  encryptionAndAuthenticationRequired,
}

enum GATTProtocolErrorArgs {
  invalidHandle,
  readNotPermitted,
  writeNotPermitted,
  invalidPDU,
  insufficientAuthentication,
  requestNotSupported,
  invalidOffset,
  insufficientAuthorization,
  prepareQueueFull,
  attributeNotFound,
  attributeNotLong,
  insufficientEncryptionKeySize,
  invalidAttributeValueLength,
  unlikelyError,
  insufficientEncryption,
  unsupportedGroupType,
  insufficientResources,
}

enum CacheModeArgs { cached, uncached }

class ManufacturerSpecificDataArgs {
  final int idArgs;
  final Uint8List dataArgs;

  ManufacturerSpecificDataArgs(this.idArgs, this.dataArgs);
}

class AdvertisementArgs {
  final String? nameArgs;
  final List<String> serviceUUIDsArgs;
  final Map<String, Uint8List> serviceDataArgs;
  final List<ManufacturerSpecificDataArgs> manufacturerSpecificDataArgs;

  AdvertisementArgs(
    this.nameArgs,
    this.serviceUUIDsArgs,
    this.serviceDataArgs,
    this.manufacturerSpecificDataArgs,
  );
}

class CentralArgs {
  final int addressArgs;

  CentralArgs(this.addressArgs);
}

class PeripheralArgs {
  final int addressArgs;

  PeripheralArgs(this.addressArgs);
}

class GATTDescriptorArgs {
  final int handleArgs;
  final String uuidArgs;

  GATTDescriptorArgs(this.handleArgs, this.uuidArgs);
}

class GATTCharacteristicArgs {
  final int handleArgs;
  final String uuidArgs;
  final List<int> propertyNumbersArgs;
  final List<GATTDescriptorArgs> descriptorsArgs;

  GATTCharacteristicArgs(
    this.handleArgs,
    this.uuidArgs,
    this.propertyNumbersArgs,
    this.descriptorsArgs,
  );
}

class GATTServiceArgs {
  final int handleArgs;
  final String uuidArgs;
  final bool isPrimaryArgs;
  final List<GATTServiceArgs> includedServicesArgs;
  final List<GATTCharacteristicArgs> characteristicsArgs;

  GATTServiceArgs(
    this.handleArgs,
    this.uuidArgs,
    this.isPrimaryArgs,
    this.includedServicesArgs,
    this.characteristicsArgs,
  );
}

class MutableGATTDescriptorArgs {
  final int hashCodeArgs;
  final String uuidArgs;
  final Uint8List? valueArgs;
  final GATTProtectionLevelArgs? readProtectionLevelArgs;
  final GATTProtectionLevelArgs? writeProtectionLevelArgs;

  MutableGATTDescriptorArgs(
    this.hashCodeArgs,
    this.uuidArgs,
    this.valueArgs,
    this.readProtectionLevelArgs,
    this.writeProtectionLevelArgs,
  );
}

class MutableGATTCharacteristicArgs {
  final int hashCodeArgs;
  final String uuidArgs;
  final Uint8List? valueArgs;
  final List<int> propertyNumbersArgs;
  final GATTProtectionLevelArgs? readProtectionLevelArgs;
  final GATTProtectionLevelArgs? writeProtectionLevelArgs;
  final List<MutableGATTDescriptorArgs> descriptorsArgs;

  MutableGATTCharacteristicArgs(
    this.hashCodeArgs,
    this.uuidArgs,
    this.valueArgs,
    this.propertyNumbersArgs,
    this.readProtectionLevelArgs,
    this.writeProtectionLevelArgs,
    this.descriptorsArgs,
  );
}

class MutableGATTServiceArgs {
  final int hashCodeArgs;
  final String uuidArgs;
  final bool isPrimaryArgs;
  final List<MutableGATTServiceArgs> includedServicesArgs;
  final List<MutableGATTCharacteristicArgs> characteristicsArgs;

  MutableGATTServiceArgs(
    this.hashCodeArgs,
    this.uuidArgs,
    this.isPrimaryArgs,
    this.includedServicesArgs,
    this.characteristicsArgs,
  );
}

class GATTReadRequestArgs {
  final int idArgs;
  final int offsetArgs;
  final int lengthArgs;

  GATTReadRequestArgs(this.idArgs, this.offsetArgs, this.lengthArgs);
}

class GATTWriteRequestArgs {
  final int idArgs;
  final int offsetArgs;
  final Uint8List valueArgs;
  final GATTCharacteristicWriteTypeArgs typeArgs;

  GATTWriteRequestArgs(
    this.idArgs,
    this.offsetArgs,
    this.valueArgs,
    this.typeArgs,
  );
}

@HostApi()
abstract class CentralManagerHostApi {
  @async
  void initialize();
  BluetoothLowEnergyStateArgs getState();
  void startDiscovery(List<String> serviceUUIDsArgs);
  void stopDiscovery();
  @async
  void connect(int addressArgs);
  void disconnect(int addressArgs);
  int getMTU(int addressArgs);
  @async
  List<GATTServiceArgs> getServices(int addressArgs, CacheModeArgs modeArgs);
  @async
  List<GATTServiceArgs> getIncludedServices(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  );
  @async
  List<GATTCharacteristicArgs> getCharacteristics(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  );
  @async
  List<GATTDescriptorArgs> getDescriptors(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  );
  @async
  Uint8List readCharacteristic(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  );
  @async
  void writeCharacteristic(
    int addressArgs,
    int handleArgs,
    Uint8List valueArgs,
    GATTCharacteristicWriteTypeArgs typeArgs,
  );
  @async
  void setCharacteristicNotifyState(
    int addressArgs,
    int handleArgs,
    GATTCharacteristicNotifyStateArgs stateArgs,
  );
  @async
  Uint8List readDescriptor(
    int addressArgs,
    int handleArgs,
    CacheModeArgs modeArgs,
  );
  @async
  void writeDescriptor(int addressArgs, int handleArgs, Uint8List valueArgs);

  /// ペアリング(暗号化リンクの確立)を開始し、結果が出るまで待つ。
  ///
  /// 保護された属性は、リンクが暗号化されていないと読み書きできない。
  /// ボンディングしない装置では鍵が保存されないため、**接続ごとに**
  /// これを済ませてから初期読み出しへ進む必要がある。
  ///
  /// [autoAcceptArgs] が true のとき、`ConfirmOnly` の同意要求を
  /// アプリ側で自動承認する(Windows の同意ダイアログは出ない)。
  /// false のときは承認しないため、OS の判断に委ねられる。
  ///
  /// 失敗を例外にせず結果として返すので、キャンセル・タイムアウト・拒否を
  /// 呼び出し側で区別できる。
  @async
  DevicePairingResultStatusArgs pair(int addressArgs, bool autoAcceptArgs);

  /// OS が保持しているペアリング(関連付け)を解除する。接続は不要。
  @async
  void unpair(int addressArgs);

  /// OS がこの装置をペアリング済みとみなしているか。接続は不要。
  ///
  /// WinRT はプラットフォームスレッド(STA)でのブロック待ちを禁じているため、
  /// 同期メソッドにはできない(`.get()` が `!is_sta_thread()` で落ちる)。
  @async
  bool isPaired(int addressArgs);
}

@FlutterApi()
abstract class CentralManagerFlutterApi {
  void onStateChanged(BluetoothLowEnergyStateArgs stateArgs);
  void onDiscovered(
    PeripheralArgs peripheralArgs,
    int rssiArgs,
    int timestampArgs,
    AdvertisementTypeArgs typeArgs,
    AdvertisementArgs advertisementArgs,
  );
  void onConnectionStateChanged(
    PeripheralArgs peripheralArgs,
    ConnectionStateArgs stateArgs,
  );
  void onMTUChanged(PeripheralArgs peripheralArgs, int mtuArgs);
  void onCharacteristicNotified(
    PeripheralArgs peripheralArgs,
    GATTCharacteristicArgs characteristicArgs,
    Uint8List valueArgs,
  );
}

@HostApi()
abstract class PeripheralManagerHostApi {
  @async
  void initialize();
  BluetoothLowEnergyStateArgs getState();
  @async
  void addService(MutableGATTServiceArgs serviceArgs);
  void removeService(int hashCodeArgs);
  void startAdvertising(AdvertisementArgs advertisementArgs);
  void stopAdvertising();
  int getMaxNotificationSize(int addressArgs);
  void respondReadRequestWithValue(int idArgs, Uint8List valueArgs);
  void respondReadRequestWithProtocolError(
    int idArgs,
    GATTProtocolErrorArgs errorArgs,
  );
  void respondWriteRequest(int idArgs);
  void respondWriteRequestWithProtocolError(
    int idArgs,
    GATTProtocolErrorArgs errorArgs,
  );
  @async
  void notifyValue(int addressArgs, int hashCodeArgs, Uint8List valueArgs);
}

@FlutterApi()
abstract class PeripheralManagerFlutterApi {
  void onStateChanged(BluetoothLowEnergyStateArgs stateArgs);
  void onMTUChanged(CentralArgs centralArgs, int mtuArgs);
  void onCharacteristicReadRequest(
    CentralArgs centralArgs,
    int hashCodeArgs,
    GATTReadRequestArgs requestArgs,
  );
  void onCharacteristicWriteRequest(
    CentralArgs centralArgs,
    int hashCodeArgs,
    GATTWriteRequestArgs requestArgs,
  );
  void onCharacteristicSubscribedClientsChanged(
    int hashCodeArgs,
    List<CentralArgs> centralsArgs,
  );
  void onDescriptorReadRequest(
    CentralArgs centralArgs,
    int hashCodeArgs,
    GATTReadRequestArgs requestArgs,
  );
  void onDescriptorWriteRequest(
    CentralArgs centralArgs,
    int hashCodeArgs,
    GATTWriteRequestArgs requestArgs,
  );
}
