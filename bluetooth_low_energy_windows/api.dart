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

/// Windows の `DevicePairingProtectionLevel` をそのまま写したもの。
///
/// pair で要求する保護レベル。装置側のセキュリティ要求(暗号化必須の
/// GATT 等)に合わせてセントラル側からも同じ水準を要求するための口。
/// `defaultLevel` は OS に適切なレベルを選ばせる。
enum DevicePairingProtectionLevelArgs {
  defaultLevel,
  none,
  encryption,
  encryptionAndAuthentication,
}

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

  /// 特性への無線通信に要求する GATT セキュリティ
  /// (`GattCharacteristic.ProtectionLevel`)を設定する。
  ///
  /// 設定後にその特性へ read/write すると、スタックは要求水準を満たす
  /// リンク(暗号化等)を操作の前提として確立しようとする。装置側の
  /// セキュリティ要求と一致させることで、ATT エラー(0x0F 等)を起点と
  /// した事後の再試行ではなく、OS 主導の事前確立に切り替えられる。
  void setCharacteristicProtectionLevel(
    int addressArgs,
    int handleArgs,
    GATTProtectionLevelArgs levelArgs,
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
  /// デスクトップでは同意は必ずシステムダイアログで行われ、アプリからは
  /// 抑止できない(Microsoft の文書と実測の両方で確認)。アプリ側の
  /// PairingRequested ハンドラは、ConfirmOnly を受理するために常に登録する
  /// (登録しないと RequiredHandlerNotRegistered / RejectedByHandler で
  /// 失敗する)。つまり「自動承認するか」という選択肢は存在しない。
  ///
  /// 失敗を例外にせず結果として返すので、キャンセル・タイムアウト・拒否を
  /// 呼び出し側で区別できる。
  ///
  /// [protectionLevelArgs] は要求する保護レベル。装置側のセキュリティ要求に
  /// 合わせて指定する(未ペアリング機器の `Pairing().ProtectionLevel()` は
  /// 「現在の水準 = None」を返すだけで要求水準ではないため、ここで明示する)。
  @async
  DevicePairingResultStatusArgs pair(
    int addressArgs,
    DevicePairingProtectionLevelArgs protectionLevelArgs,
  );

  /// OS が保持しているペアリング(関連付け)を解除する。接続は不要。
  @async
  void unpair(int addressArgs);

  /// OS がこの装置をペアリング済みとみなしているか。接続は不要。
  ///
  /// WinRT はプラットフォームスレッド(STA)でのブロック待ちを禁じているため、
  /// 同期メソッドにはできない(`.get()` が `!is_sta_thread()` で落ちる)。
  @async
  bool isPaired(int addressArgs);

  /// `GattSession.MaintainConnection` を立てて接続を開始する。
  ///
  /// connect の接続待ちは OS 内部で 7 秒固定・キャンセル不可である。
  /// こちらは「デバイスが現れ次第つなぐ」を OS へ依頼し、リンク確立を
  /// 待たずに返る。確立は onConnectionStateChanged(connected)で通知する
  /// ので、待ち時間の上限と中断は呼び出し側が決める。中断は disconnect。
  ///
  /// 依頼はセッションが生きている限り有効であり、接続中に維持を解除しては
  /// ならない。Windows でリンクを保持するのは維持依頼か実行中の GATT 操作の
  /// どちらかで、確立直後はまだ後者を持たないため、そこで解除すると OS が
  /// リンクを解体する。解除は disconnect が行う。
  ///
  /// 維持が有効な間、リンクが失われても OS が張り直す。呼び出し側には
  /// 切断と再接続として通知され、装置・セッション・GATT オブジェクトは
  /// 保持されるのでハンドルはそのまま使える。ただし通知の購読(CCCD)は
  /// 装置側が切断で落とすため、張り直しは呼び出し側の責務である。
  @async
  void connectMaintained(int addressArgs);

  /// `GattSession.MaintainConnection` を明示的に切り替える。
  ///
  /// 通常は connectMaintained と disconnect で足りる。接続中の解除は
  /// リンクの解体を招くため、その用途では使わないこと。
  /// セッションを保持していない(未接続の)装置に対してはエラーになる。
  void setMaintainConnection(int addressArgs, bool enableArgs);
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

  /// ネイティブ層の診断メッセージ。Dart 側の logger(info)へ中継する。
  ///
  /// ネイティブには独自のログ機構が無いため、調査時の観測値
  /// (例: ペアリングの保護レベル・儀式の発火有無)はこの口で表出する。
  void onLogged(String messageArgs);
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
