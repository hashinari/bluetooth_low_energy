#ifndef FLUTTER_PLUGIN_CENTRAL_MANAGER_H_
#define FLUTTER_PLUGIN_CENTRAL_MANAGER_H_

#include <iomanip>
#include <sstream>

#include "winrt/Windows.Devices.Bluetooth.h"
#include "winrt/Windows.Devices.Bluetooth.Advertisement.h"
#include "winrt/Windows.Devices.Bluetooth.GenericAttributeProfile.h"
#include "winrt/Windows.Devices.Enumeration.h"
#include "winrt/Windows.Devices.Radios.h"
#include "winrt/Windows.Foundation.h"
#include "winrt/Windows.Foundation.Collections.h"
#include "winrt/Windows.Storage.Streams.h"

#include "bluetooth_low_energy_api.g.h"
#include "bluetooth_low_energy_exception.h"
#include "bluetooth_low_energy_format.h"

namespace bluetooth_low_energy_windows
{
	class CentralManagerImpl : public CentralManagerHostApi
	{
	public:
		CentralManagerImpl(flutter::BinaryMessenger *messenger);
		virtual ~CentralManagerImpl();

		// Disallow copy and assign.
		CentralManagerImpl(const CentralManagerImpl &) = delete;
		CentralManagerImpl &operator=(const CentralManagerImpl &) = delete;

		void Initialize(std::function<void(std::optional<FlutterError> reply)> result) override;
		ErrorOr<BluetoothLowEnergyStateArgs> GetState() override;
		std::optional<FlutterError> StartDiscovery(const flutter::EncodableList &service_uuids_args) override;
		std::optional<FlutterError> StopDiscovery() override;
		void Connect(int64_t address_args, bool maintain_args, std::function<void(std::optional<FlutterError> reply)> result) override;
		std::optional<FlutterError> Disconnect(int64_t address_args) override;
		ErrorOr<int64_t> GetMTU(int64_t address_args) override;
		void GetServices(int64_t address_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result) override;
		void GetIncludedServices(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result) override;
		void GetCharacteristics(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result) override;
		void GetDescriptors(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result) override;
		void ReadCharacteristic(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result) override;
		void WriteCharacteristic(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, const GATTCharacteristicWriteTypeArgs &type_args, std::function<void(std::optional<FlutterError> reply)> result) override;
		void SetCharacteristicNotifyState(int64_t address_args, int64_t handle_args, const GATTCharacteristicNotifyStateArgs &state_args, std::function<void(std::optional<FlutterError> reply)> result) override;
		void ReadDescriptor(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result) override;
		void WriteDescriptor(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, std::function<void(std::optional<FlutterError> reply)> result) override;
		void Pair(int64_t address_args, const DevicePairingProtectionLevelArgs &protection_level_args, const DevicePairingConsentArgs &consent_args, std::function<void(ErrorOr<DevicePairingResultStatusArgs> reply)> result) override;
		void Unpair(int64_t address_args, std::function<void(std::optional<FlutterError> reply)> result) override;
		void IsPaired(int64_t address_args, std::function<void(ErrorOr<bool> reply)> result) override;

	private:
		// プラットフォームスレッド(= このオブジェクトが生成される
		// プラグイン登録時のスレッド)の apartment。WinRT のイベントは
		// スレッドプール上で発火するが、Flutter のチャネル送信は
		// プラットフォームスレッドからしか行えないため、イベントを
		// Flutter へ流す前に co_await でここへ戻す。
		// (Flutter の Windows ランナーは main で CoInitializeEx(STA) を
		// 呼び、Win32 メッセージループを回している前提)
		winrt::apartment_context m_ui_thread;
		std::optional<CentralManagerFlutterApi> m_api;
		std::optional<winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher> m_watcher;
		std::optional<winrt::Windows::Devices::Bluetooth::BluetoothAdapter> m_adapter;
		std::optional<winrt::Windows::Devices::Radios::Radio> m_radio;
		std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::BluetoothLEDevice>> m_devices;
		std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSession>> m_sessions;
		std::map<int64_t, std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService>>> m_services;
		std::map<int64_t, std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic>>> m_characteristics;
		std::map<int64_t, std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor>>> m_descriptors;
		std::optional<winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher::Received_revoker> m_watcher_received_revoker;
		std::optional<winrt::Windows::Devices::Radios::Radio::StateChanged_revoker> m_radio_state_changed_revoker;
		std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::ConnectionStatusChanged_revoker>> m_device_connection_status_changed_revokers;
		std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSession::MaxPduSizeChanged_revoker>> m_session_max_pdu_size_changed_revokers;
		std::map<int64_t, std::map<int64_t, std::optional<winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic::ValueChanged_revoker>>> m_characteristic_value_changed_revokers;

		winrt::fire_and_forget InitializeAsync(std::function<void(std::optional<FlutterError> reply)> result);
		winrt::fire_and_forget ConnectAsync(int64_t address_args, bool maintain_args, std::function<void(std::optional<FlutterError> reply)> result);
		winrt::fire_and_forget GetServicesAsync(int64_t address_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result);
		winrt::fire_and_forget GetIncludedServicesAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result);
		winrt::fire_and_forget GetCharacteristicsAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result);
		winrt::fire_and_forget GetDescriptorsAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result);
		winrt::fire_and_forget ReadCharacteristicAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result);
		winrt::fire_and_forget WriteCharacteristicAsync(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, const GATTCharacteristicWriteTypeArgs &type_args, std::function<void(std::optional<FlutterError> reply)> result);
		winrt::fire_and_forget SetCharacteristicNotifyStateAsync(int64_t address_args, int64_t handle_args, const GATTCharacteristicNotifyStateArgs &state_args, std::function<void(std::optional<FlutterError> reply)> result);
		winrt::fire_and_forget ReadDescriptorAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result);
		winrt::fire_and_forget WriteDescriptorAsync(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, std::function<void(std::optional<FlutterError> reply)> result);
		winrt::fire_and_forget PairAsync(int64_t address_args, DevicePairingProtectionLevelArgs protection_level_args, DevicePairingConsentArgs consent_args, std::function<void(ErrorOr<DevicePairingResultStatusArgs> reply)> result);
		winrt::fire_and_forget UnpairAsync(int64_t address_args, std::function<void(std::optional<FlutterError> reply)> result);
		winrt::fire_and_forget IsPairedAsync(int64_t address_args, std::function<void(ErrorOr<bool> reply)> result);
		// ── WinRT イベント → Flutter の中継(プラットフォームスレッドへ
		//     戻してから送る)──
		// 引数は値渡し。イベントハンドラのスレッドで組み立てた値を
		// コルーチンの再開後も安全に使うため。
		// ネイティブ層のログを Dart の logger へ流す(レベル付き)。
		winrt::fire_and_forget Log(LogLevelArgs level_args, std::string message_args);
		winrt::fire_and_forget NotifyStateChanged(BluetoothLowEnergyStateArgs state_args);
		winrt::fire_and_forget NotifyDiscovered(PeripheralArgs peripheral_args, int64_t rssi_args, int64_t timestamp_args, AdvertisementTypeArgs type_args, AdvertisementArgs advertisement_args);
		winrt::fire_and_forget NotifyMTUChanged(PeripheralArgs peripheral_args, int64_t mtu_args);
		winrt::fire_and_forget NotifyCharacteristicNotified(PeripheralArgs peripheral_args, GATTCharacteristicArgs characteristic_args, std::vector<uint8_t> value_args);
		// 切断時の後片付け(OnDisconnected)も含めてプラットフォーム
		// スレッドで行う(内部マップの変更を 1 スレッドに寄せる)。
		winrt::fire_and_forget HandleConnectionStatusChanged(int64_t address_args, winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus status);

		// ConfirmOnly の同意を代行するハンドラ(同意の主体が app のときだけ
		// 登録する)。Custom ペアリングはこれが Accept() しないと
		// RequiredHandlerNotRegistered / RejectedByHandler で失敗する。
		// 登録して Accept() すると OS の同意 UI は出ない。
		void OnPairingRequested(const winrt::Windows::Devices::Enumeration::DeviceInformationCustomPairing &sender, const winrt::Windows::Devices::Enumeration::DevicePairingRequestedEventArgs &args);
		static DevicePairingResultStatusArgs PairingStatusToArgs(const winrt::Windows::Devices::Enumeration::DevicePairingResultStatus &status);
		// GATT の失敗を、ATT のエラーコードを保ったまま FlutterError にする。
		static FlutterError GattError(const std::string &operation, const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus &status, const winrt::Windows::Foundation::IReference<uint8_t> &protocol_error);

		void OnDisconnected(int64_t address_args);

		winrt::Windows::Devices::Bluetooth::BluetoothLEDevice &RetrieveDevice(int64_t address_args);
		winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService &RetrieveService(int64_t address_args, int64_t handle_args);
		winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic &RetrieveCharacteristic(int64_t address, int64_t handle_args);
		winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor &RetrieveDescriptor(int64_t address_args, int64_t handle_args);

		BluetoothLowEnergyStateArgs RadioStateToArgs(const winrt::Windows::Devices::Radios::RadioState &state);
		AdvertisementTypeArgs AdvertisementTypeToArgs(const winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType &type);
		ConnectionStateArgs ConnectionStatusToArgs(const winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus &status);
		ManufacturerSpecificDataArgs ManufacturerDataToArgs(const winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEManufacturerData &manufacturer_data);
		AdvertisementArgs AdvertisementToArgs(const winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisement &advertisement);
		GATTServiceArgs ServiceToArgs(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService &service);
		GATTCharacteristicArgs CharacteristicToArgs(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic &characteristic);
		flutter::EncodableList CharacteristicPropertiesToArgs(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties properties);
		GATTDescriptorArgs DescriptorToArgs(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor &descriptor);
		std::string GUIDToArgs(const winrt::guid &guid);

		winrt::Windows::Devices::Bluetooth::BluetoothCacheMode ArgsToCacheMode(const CacheModeArgs &mode_args);
		winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption ArgsToWriteOption(const GATTCharacteristicWriteTypeArgs &type_args);
		winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue ArgsToCCCDescriptorValue(const GATTCharacteristicNotifyStateArgs &state_args);
	};
}

#endif // !FLUTTER_PLUGIN_CENTRAL_MANAGER_H_
