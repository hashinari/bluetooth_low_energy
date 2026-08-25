#undef _HAS_EXCEPTIONS

#include "central_manager_impl.h"

namespace bluetooth_low_energy_windows
{
	CentralManagerImpl::CentralManagerImpl(flutter::BinaryMessenger *messenger)
	{
		const auto api = CentralManagerFlutterApi(messenger);
		const auto watcher = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher();
		watcher.ScanningMode(winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEScanningMode::Active);
		m_api = api;
		m_watcher = watcher;
	}

	CentralManagerImpl::~CentralManagerImpl()
	{
	}

	void CentralManagerImpl::Initialize(std::function<void(std::optional<FlutterError> reply)> result)
	{
		InitializeAsync(std::move(result));
	}

	ErrorOr<BluetoothLowEnergyStateArgs> CentralManagerImpl::GetState()
	{
		try
		{
			const auto has_adapter = m_adapter.has_value();
			if (has_adapter)
			{
				const auto &adapter = m_adapter.value();
				const auto supported = adapter.IsCentralRoleSupported();
				if (supported)
				{
					const auto &radio = m_radio.value();
					const auto state = radio.State();
					const auto state_args = RadioStateToArgs(state);
					return state_args;
				}
			}
			return BluetoothLowEnergyStateArgs::kUnsupported;
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			return FlutterError(code, message);
		}
	}

	std::optional<FlutterError> CentralManagerImpl::StartDiscovery(const flutter::EncodableList &service_uuids_args)
	{
		try
		{
			const auto &watcher = m_watcher.value();
			const auto filter = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementFilter::BluetoothLEAdvertisementFilter();
			const auto advertisement = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisement();
			const auto service_uuids = advertisement.ServiceUuids();
			for (const auto &service_uuid_args_value : service_uuids_args)
			{
				const auto &service_uuid_args = std::get<std::string>(service_uuid_args_value);
				const auto service_uuid = winrt::guid(service_uuid_args);
				service_uuids.Append(service_uuid);
			}
			filter.Advertisement(advertisement);
			watcher.AdvertisementFilter(filter);
			watcher.Start();
			return std::nullopt;
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			return FlutterError(code, message);
		}
	}

	std::optional<FlutterError> CentralManagerImpl::StopDiscovery()
	{
		try
		{
			const auto &watcher = m_watcher.value();
			watcher.Stop();
			return std::nullopt;
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			return FlutterError(code, message);
		}
	}

	void CentralManagerImpl::Connect(int64_t address_args, bool maintain_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		ConnectAsync(address_args, maintain_args, std::move(result));
	}

	std::optional<FlutterError> CentralManagerImpl::Disconnect(int64_t address_args)
	{
		try
		{
			// 維持依頼(MaintainConnection)を明示的に取り下げてから参照を
			// 手放す。参照解放だけでも依頼は消えるが、OS 側の解体には
			// 「小さなタイムアウト」がある(公式文書)ため、切断要求の意図が
			// 即座に伝わるよう先に false を書く。
			const auto it = m_sessions.find(address_args);
			if (it != m_sessions.end() && it->second.has_value())
			{
				it->second.value().MaintainConnection(false);
			}
			OnDisconnected(address_args);
			auto &api = m_api.value();
			const auto peripheral_args = PeripheralArgs(address_args);
			const auto state_args = ConnectionStateArgs::kDisconnected;
			// ホスト呼び出し(同期)はプラットフォームスレッドで届くため、
			// ここからの送信はそのままで良い。
			api.OnConnectionStateChanged(peripheral_args, state_args, [] {}, [](auto error) {});
			return std::nullopt;
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			return FlutterError(code, message);
		}
	}

	ErrorOr<int64_t> CentralManagerImpl::GetMTU(int64_t address_args)
	{
		try
		{
			const auto &session = m_sessions[address_args].value();
			const auto mtu = session.MaxPduSize();
			const auto mtu_args = static_cast<int64_t>(mtu);
			return mtu_args;
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			return FlutterError(code, message);
		}
	}

	void CentralManagerImpl::GetServices(int64_t address_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		GetServicesAsync(address_args, mode_args, std::move(result));
	}

	void CentralManagerImpl::GetIncludedServices(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		GetIncludedServicesAsync(address_args, handle_args, mode_args, std::move(result));
	}

	void CentralManagerImpl::GetCharacteristics(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		GetCharacteristicsAsync(address_args, handle_args, mode_args, std::move(result));
	}

	void CentralManagerImpl::GetDescriptors(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		GetDescriptorsAsync(address_args, handle_args, mode_args, std::move(result));
	}

	void CentralManagerImpl::ReadCharacteristic(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result)
	{
		ReadCharacteristicAsync(address_args, handle_args, mode_args, std::move(result));
	}

	void CentralManagerImpl::WriteCharacteristic(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, const GATTCharacteristicWriteTypeArgs &type_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		WriteCharacteristicAsync(address_args, handle_args, value_args, type_args, std::move(result));
	}

	void CentralManagerImpl::SetCharacteristicNotifyState(int64_t address_args, int64_t handle_args, const GATTCharacteristicNotifyStateArgs &state_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		SetCharacteristicNotifyStateAsync(address_args, handle_args, state_args, std::move(result));
	}

	void CentralManagerImpl::ReadDescriptor(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result)
	{
		ReadDescriptorAsync(address_args, handle_args, mode_args, std::move(result));
	}

	void CentralManagerImpl::WriteDescriptor(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		WriteDescriptorAsync(address_args, handle_args, value_args, std::move(result));
	}

	winrt::fire_and_forget CentralManagerImpl::InitializeAsync(std::function<void(std::optional<FlutterError> reply)> result)
	{
		try
		{
			const auto &watcher = m_watcher.value();
			const auto status = watcher.Status();
			if (status == winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcherStatus::Started)
			{
				watcher.Stop();
			}

			for (const auto &item : m_devices)
			{
				const auto address_args = item.first;
				OnDisconnected(address_args);
			}

			const auto &adapter = co_await winrt::Windows::Devices::Bluetooth::BluetoothAdapter::GetDefaultAsync();
			if (adapter != NULL)
			{
				const auto supported = adapter.IsCentralRoleSupported();
				if (supported)
				{
					const auto &radio = co_await adapter.GetRadioAsync();
					m_radio_state_changed_revoker = radio.StateChanged(
						winrt::auto_revoke,
						[this](winrt::Windows::Devices::Radios::Radio radio, auto obj)
						{
							const auto state = radio.State();
							const auto state_args = RadioStateToArgs(state);
							NotifyStateChanged(state_args);
						});
					m_radio = radio;
				}
				else
				{
					m_radio.reset();
				}
				m_adapter = adapter;
			}
			else
			{
				m_adapter.reset();
			}

			m_watcher_received_revoker = watcher.Received(
				winrt::auto_revoke,
				[this](winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementWatcher watcher, winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementReceivedEventArgs event_args)
				{
					const auto address = event_args.BluetoothAddress();
					const auto address_args = static_cast<int64_t>(address);
					const auto peripheral_args = PeripheralArgs(address_args);
					const auto rssi = event_args.RawSignalStrengthInDBm();
					const auto rssi_args = static_cast<int64_t>(rssi);
					const auto &file_timestamp = event_args.Timestamp();
					const auto timestamp = std::chrono::clock_cast<std::chrono::system_clock>(file_timestamp).time_since_epoch();
					const auto timestamp_args = std::chrono::duration_cast<std::chrono::milliseconds>(timestamp).count();
					const auto type = event_args.AdvertisementType();
					const auto type_args = AdvertisementTypeToArgs(type);
					const auto advertisement = event_args.Advertisement();
					const auto advertisement_args = AdvertisementToArgs(advertisement);
					NotifyDiscovered(peripheral_args, rssi_args, timestamp_args, type_args, advertisement_args);
				});
			result(std::nullopt);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::ConnectAsync(int64_t address_args, bool maintain_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		try
		{
			const auto address = static_cast<uint64_t>(address_args);
			const auto &device = co_await winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(address);
			if (device == nullptr)
			{
				// 未ペアリングかつシステムキャッシュに無い装置では null が
				// 返る(公式文書)。他の失敗と区別できるよう明示エラーにする。
				const auto error = FlutterError("DeviceNotFound", "FromBluetoothAddressAsync returned null (device not in system cache)");
				result(error);
				co_return;
			}
			const auto id = device.BluetoothDeviceId();
			const auto &session = co_await winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSession::FromDeviceIdAsync(id);
			if (maintain_args)
			{
				if (!session.CanMaintainConnection())
				{
					const auto error = FlutterError("LinkFailed", "GattSession.CanMaintainConnection is false.");
					result(error);
					co_return;
				}
				// 維持は引き金より先に立てる。逆にすると、引き金で張った
				// リンクが依頼の前に落ちうる。
				session.MaintainConnection(true);
			}
			// 観測のみ(挙動は変えない)。散発の接続失敗と突き合わせるため、
			// 引き金を引く直前の装置オブジェクトの見え方を残す。
			// - addressType: OS が装置をどの種別で見ているか。Unspecified の
			//   ままなら種別を確定できずに接続しにいっている
			// - connectedBefore: 引き金の前からリンクがあると OS が思っているか
			const auto address_type = device.BluetoothAddressType();
			const auto address_type_name =
				address_type == winrt::Windows::Devices::Bluetooth::BluetoothAddressType::Public	  ? std::string("Public")
				: address_type == winrt::Windows::Devices::Bluetooth::BluetoothAddressType::Random ? std::string("Random")
																								   : std::string("Unspecified");
			const auto connected_before = device.ConnectionStatus() == winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Connected;
			Log(LogLevelArgs::kInfo,
				"connect: addressType=" + address_type_name +
					" connectedBefore=" + (connected_before ? std::string("true") : std::string("false")));
			// 装置オブジェクトを作っただけでは接続しない。uncached のサービス
			// 探索を引き金にリンクを確立する。**結果は使わない** ── サービスは
			// discoverGATT が取る。
			// See: https://learn.microsoft.com/en-us/windows/uwp/devices-sensors/gatt-client#connecting-to-the-device
			const auto &r = co_await device.GetGattServicesAsync(winrt::Windows::Devices::Bluetooth::BluetoothCacheMode::Uncached);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				// 失敗を分類する。リンクが立ったか・OS に記録があるかで、
				// 呼び出し側の次の手が変わる。
				const auto link_up = device.ConnectionStatus() == winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Connected;
				auto paired = false;
				try
				{
					paired = device.DeviceInformation().Pairing().IsPaired();
				}
				catch (...)
				{
					// 記録の照会に失敗しても分類を落とすだけで、失敗自体は返す。
				}
				if (maintain_args)
				{
					try
					{
						session.MaintainConnection(false);
					}
					catch (...)
					{
					}
				}
				std::string code;
				std::string message;
				if (!link_up)
				{
					code = "LinkFailed";
					message = "Connect failed: link was not established";
				}
				else if (paired)
				{
					// リンクは立つのに GATT に届かず、OS に記録がある。記録と
					// 相手の鍵が食い違っている状態(実測で 8/8 再現)。復旧は unpair。
					code = "PairingMismatch";
					message = "Connect failed: link established but GATT is unreachable, and an OS pairing record exists (stale record). Recover with unpair";
				}
				else
				{
					code = "GattUnreachable";
					message = "Connect failed: link established but GATT is unreachable";
				}
				auto details = flutter::EncodableMap{
					{flutter::EncodableValue("status"), flutter::EncodableValue(static_cast<int32_t>(status))},
					{flutter::EncodableValue("linkUp"), flutter::EncodableValue(link_up)},
					{flutter::EncodableValue("paired"), flutter::EncodableValue(paired)},
					{flutter::EncodableValue("addressType"), flutter::EncodableValue(address_type_name)},
					{flutter::EncodableValue("connectedBefore"), flutter::EncodableValue(connected_before)},
				};
				const auto protocol_error = r.ProtocolError();
				if (protocol_error != nullptr)
				{
					const auto att = protocol_error.Value();
					details.insert({flutter::EncodableValue("protocolError"), flutter::EncodableValue(static_cast<int32_t>(att))});
				}
				result(FlutterError(code, message, flutter::EncodableValue(details)));
				co_return;
			}
			const auto peripheral_args = PeripheralArgs(address_args);
			const auto state_args = ConnectionStateArgs::kConnected;
			const auto mtu = session.MaxPduSize();
			const auto mtu_args = static_cast<int64_t>(mtu);
			// co_await の再開後はスレッドプール上に居る。イベント送出・
			// ハンドラ登録・内部マップの更新・result 応答をプラットフォーム
			// スレッドで行うため、ここで一度戻す。
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			auto &api = m_api.value();
			api.OnConnectionStateChanged(peripheral_args, state_args, [] {}, [](auto error) {});
			api.OnMTUChanged(peripheral_args, mtu_args, [] {}, [](auto error) {});
			m_device_connection_status_changed_revokers[address_args] = device.ConnectionStatusChanged(
				winrt::auto_revoke,
				[this, address_args](winrt::Windows::Devices::Bluetooth::BluetoothLEDevice device, auto obj)
				{
					const auto status = device.ConnectionStatus();
					HandleConnectionStatusChanged(address_args, status);
				});
			m_session_max_pdu_size_changed_revokers[address_args] = session.MaxPduSizeChanged(
				winrt::auto_revoke,
				[this, peripheral_args](winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattSession session, auto obj)
				{
					const auto mtu = session.MaxPduSize();
					const auto mtu_args = static_cast<int64_t>(mtu);
					NotifyMTUChanged(peripheral_args, mtu_args);
				});
			m_devices[address_args] = device;
			m_sessions[address_args] = session;
			result(std::nullopt);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::GetServicesAsync(int64_t address_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		try
		{
			const auto &device = RetrieveDevice(address_args);
			const auto mode = ArgsToCacheMode(mode_args);
			const auto &r = co_await device.GetGattServicesAsync(mode);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Get services", status, r.ProtocolError()));
				co_return;
			}
			const auto services = r.Services();
			auto services_args = flutter::EncodableList();
			for (const auto service : services)
			{
				const auto service_args = ServiceToArgs(service);
				const auto service_args_value = flutter::CustomEncodableValue(service_args);
				const auto service_handle_args = service_args.handle_args();
				m_services[address_args][service_handle_args] = service;
				services_args.emplace_back(service_args_value);
			}
			result(services_args);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::GetIncludedServicesAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		try
		{
			const auto &service = RetrieveService(address_args, handle_args);
			const auto mode = ArgsToCacheMode(mode_args);
			const auto &r = co_await service.GetIncludedServicesAsync(mode);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Get included services", status, r.ProtocolError()));
				co_return;
			}
			const auto included_services = r.Services();
			auto included_services_args = flutter::EncodableList();
			for (const auto included_service : included_services)
			{
				const auto service_args = ServiceToArgs(included_service);
				const auto service_args_value = flutter::CustomEncodableValue(service_args);
				const auto service_handle_args = service_args.handle_args();
				m_services[address_args][service_handle_args] = included_service;
				included_services_args.emplace_back(service_args_value);
			}
			result(included_services_args);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::GetCharacteristicsAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		try
		{
			const auto &service = RetrieveService(address_args, handle_args);
			const auto mode = ArgsToCacheMode(mode_args);
			const auto &r = co_await service.GetCharacteristicsAsync(mode);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Get characteristics", status, r.ProtocolError()));
				co_return;
			}
			const auto characteristics = r.Characteristics();
			auto characteristics_args = flutter::EncodableList();
			for (const auto characteristic : characteristics)
			{
				const auto characteristic_args = CharacteristicToArgs(characteristic);
				const auto characteristic_args_value = flutter::CustomEncodableValue(characteristic_args);
				const auto characteristic_handle_args = characteristic_args.handle_args();
				m_characteristic_value_changed_revokers[address_args][characteristic_handle_args] = characteristic.ValueChanged(
					winrt::auto_revoke,
					[this, address_args](const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic &characteristic, const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattValueChangedEventArgs &event_args)
					{
						const auto peripheral_args = PeripheralArgs(address_args);
						const auto characteristic_args = CharacteristicToArgs(characteristic);
						const auto value = event_args.CharacteristicValue();
						const auto value_length = value.Length();
						auto value_args = std::vector<uint8_t>(value_length);
						const auto value_reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(value);
						value_reader.ReadBytes(value_args);
						NotifyCharacteristicNotified(peripheral_args, characteristic_args, std::move(value_args));
					});
				m_characteristics[address_args][characteristic_handle_args] = characteristic;
				characteristics_args.emplace_back(characteristic_args_value);
			}
			result(characteristics_args);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::GetDescriptorsAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<flutter::EncodableList> reply)> result)
	{
		try
		{
			const auto &characteristic = RetrieveCharacteristic(address_args, handle_args);
			const auto mode = ArgsToCacheMode(mode_args);
			const auto &r = co_await characteristic.GetDescriptorsAsync(mode);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Get descriptors", status, r.ProtocolError()));
				co_return;
			}
			const auto descriptors = r.Descriptors();
			auto descriptors_args = flutter::EncodableList();
			for (const auto descriptor : descriptors)
			{
				const auto descriptor_args = DescriptorToArgs(descriptor);
				const auto descriptor_args_value = flutter::CustomEncodableValue(descriptor_args);
				const auto descriptor_handle_args = descriptor_args.handle_args();
				m_descriptors[address_args][descriptor_handle_args] = descriptor;
				descriptors_args.emplace_back(descriptor_args_value);
			}
			result(descriptors_args);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::ReadCharacteristicAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result)
	{
		try
		{
			const auto &characteristic = RetrieveCharacteristic(address_args, handle_args);
			const auto mode = ArgsToCacheMode(mode_args);
			const auto &r = co_await characteristic.ReadValueAsync(mode);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Read characteristic", status, r.ProtocolError()));
				co_return;
			}
			const auto value = r.Value();
			const auto value_length = value.Length();
			auto value_args = std::vector<uint8_t>(value_length);
			const auto value_reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(value);
			value_reader.ReadBytes(value_args);
			result(value_args);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::WriteCharacteristicAsync(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, const GATTCharacteristicWriteTypeArgs &type_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		try
		{
			const auto &characteristic = RetrieveCharacteristic(address_args, handle_args);
			const auto value_writer = winrt::Windows::Storage::Streams::DataWriter();
			value_writer.WriteBytes(value_args);
			const auto value = value_writer.DetachBuffer();
			const auto option = ArgsToWriteOption(type_args);
			const auto &r = co_await characteristic.WriteValueWithResultAsync(value, option);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Write characteristic", status, r.ProtocolError()));
				co_return;
			}
			result(std::nullopt);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::SetCharacteristicNotifyStateAsync(int64_t address_args, int64_t handle_args, const GATTCharacteristicNotifyStateArgs &state_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		try
		{
			const auto &characteristic = RetrieveCharacteristic(address_args, handle_args);
			const auto value = ArgsToCCCDescriptorValue(state_args);
			const auto &r = co_await characteristic.WriteClientCharacteristicConfigurationDescriptorWithResultAsync(value);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Notify characteristic", status, r.ProtocolError()));
				co_return;
			}
			result(std::nullopt);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::ReadDescriptorAsync(int64_t address_args, int64_t handle_args, const CacheModeArgs &mode_args, std::function<void(ErrorOr<std::vector<uint8_t>> reply)> result)
	{
		try
		{
			const auto &descriptor = RetrieveDescriptor(address_args, handle_args);
			const auto mode = ArgsToCacheMode(mode_args);
			const auto &r = co_await descriptor.ReadValueAsync(mode);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Read descriptor", status, r.ProtocolError()));
				co_return;
			}
			const auto value = r.Value();
			const auto value_length = value.Length();
			auto value_args = std::vector<uint8_t>(value_length);
			const auto value_reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(value);
			value_reader.ReadBytes(value_args);
			result(value_args);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	winrt::fire_and_forget CentralManagerImpl::WriteDescriptorAsync(int64_t address_args, int64_t handle_args, const std::vector<uint8_t> &value_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		try
		{
			const auto &descriptor = RetrieveDescriptor(address_args, handle_args);
			const auto value_writer = winrt::Windows::Storage::Streams::DataWriter();
			value_writer.WriteBytes(value_args);
			const auto value = value_writer.DetachBuffer();
			const auto &r = co_await descriptor.WriteValueWithResultAsync(value);
			const auto status = r.Status();
			if (status != winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus::Success)
			{
				result(GattError("Write descriptor", status, r.ProtocolError()));
				co_return;
			}
			result(std::nullopt);
		}
		catch (const winrt::hresult_error &ex)
		{
			const auto code = "winrt::hresult_error";
			const auto winrt_message = ex.message();
			const auto message = winrt::to_string(winrt_message);
			const auto error = FlutterError(code, message);
			result(error);
		}
		catch (const std::exception &ex)
		{
			const auto code = "std::exception";
			const auto message = ex.what();
			const auto error = FlutterError(code, message);
			result(error);
		}
	}

	void CentralManagerImpl::OnDisconnected(int64_t address_args)
	{
		m_device_connection_status_changed_revokers.erase(address_args);
		m_session_max_pdu_size_changed_revokers.erase(address_args);
		m_characteristic_value_changed_revokers.erase(address_args);
		m_devices.erase(address_args);
		m_sessions.erase(address_args);
		m_services.erase(address_args);
		m_characteristics.erase(address_args);
		m_descriptors.erase(address_args);
	}

	winrt::Windows::Devices::Bluetooth::BluetoothLEDevice &CentralManagerImpl::RetrieveDevice(int64_t address_args)
	{
		return m_devices[address_args].value();
	}

	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService &CentralManagerImpl::RetrieveService(int64_t address_args, int64_t handle_args)
	{
		return m_services[address_args][handle_args].value();
	}

	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic &CentralManagerImpl::RetrieveCharacteristic(int64_t address_args, int64_t handle_args)
	{
		return m_characteristics[address_args][handle_args].value();
	}

	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor &CentralManagerImpl::RetrieveDescriptor(int64_t address_args, int64_t handle_args)
	{
		return m_descriptors[address_args][handle_args].value();
	}

	BluetoothLowEnergyStateArgs CentralManagerImpl::RadioStateToArgs(const winrt::Windows::Devices::Radios::RadioState &state)
	{
		switch (state)
		{
		case winrt::Windows::Devices::Radios::RadioState::Unknown:
			return BluetoothLowEnergyStateArgs::kUnknown;
		case winrt::Windows::Devices::Radios::RadioState::Disabled:
			return BluetoothLowEnergyStateArgs::kDisabled;
		case winrt::Windows::Devices::Radios::RadioState::Off:
			return BluetoothLowEnergyStateArgs::kOff;
		case winrt::Windows::Devices::Radios::RadioState::On:
			return BluetoothLowEnergyStateArgs::kOn;
		default:
			return BluetoothLowEnergyStateArgs::kUnknown;
		}
	}

	AdvertisementTypeArgs CentralManagerImpl::AdvertisementTypeToArgs(const winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType &type)
	{
		switch (type)
		{
		case winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType::ConnectableUndirected:
			return AdvertisementTypeArgs::kConnectableUndirected;
		case winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType::ConnectableDirected:
			return AdvertisementTypeArgs::kConnectableDirected;
		case winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType::ScannableUndirected:
			return AdvertisementTypeArgs::kScannableUndirected;
		case winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType::NonConnectableUndirected:
			return AdvertisementTypeArgs::kNonConnectableUndirected;
		case winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType::ScanResponse:
			return AdvertisementTypeArgs::kScanResponse;
		case winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementType::Extended:
			return AdvertisementTypeArgs::kExtended;
		default:
			throw std::bad_cast();
		}
	}

	ConnectionStateArgs CentralManagerImpl::ConnectionStatusToArgs(const winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus &status)
	{
		switch (status)
		{
		case winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Disconnected:
			return ConnectionStateArgs::kDisconnected;
		case winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Connected:
			return ConnectionStateArgs::kConnected;
		default:
			throw std::bad_cast();
		}
	}

	ManufacturerSpecificDataArgs CentralManagerImpl::ManufacturerDataToArgs(const winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEManufacturerData &manufacturer_data)
	{
		const auto id = manufacturer_data.CompanyId();
		const auto id_args = static_cast<int64_t>(id);
		const auto data = manufacturer_data.Data();
		const auto data_length = data.Length();
		auto data_args = std::vector<uint8_t>(data_length);
		const auto data_reader = winrt::Windows::Storage::Streams::DataReader::FromBuffer(data);
		data_reader.ReadBytes(data_args);
		return ManufacturerSpecificDataArgs(id_args, data_args);
	}

	AdvertisementArgs CentralManagerImpl::AdvertisementToArgs(const winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisement &advertisement)
	{
		const auto name = advertisement.LocalName();
		const auto name_args = winrt::to_string(name);
		const auto service_uuids = advertisement.ServiceUuids();
		auto service_uuids_args = flutter::EncodableList();
		for (const auto &service_uuid : service_uuids)
		{
			const auto service_uuid_args = GUIDToArgs(service_uuid);
			service_uuids_args.emplace_back(service_uuid_args);
		}
		const auto data_sections = advertisement.DataSections();
		auto service_data_args = flutter::EncodableMap();
		for (const auto data_section : data_sections)
		{
			const auto section_type = data_section.DataType();
			const auto section_buffer = data_section.Data();
			const auto section_data_length = section_buffer.Length();
			const auto type16 = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData16BitUuids();
			const auto type32 = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData32BitUuids();
			const auto type128 = winrt::Windows::Devices::Bluetooth::Advertisement::BluetoothLEAdvertisementDataTypes::ServiceData128BitUuids();
			if (section_type == type16 && section_data_length > 2)
			{
				const auto section_data = section_buffer.data();
				auto data1 = uint16_t();
				std::memcpy(&data1, section_data, 2Ui64);
				const auto uuid_args = std::format("{:04X}", data1);
				const auto data_args = std::vector<uint8_t>(section_data + 2, section_data + section_data_length);
				service_data_args[uuid_args] = data_args;
			}
			else if (section_type == type32 && section_data_length > 4)
			{
				const auto section_data = section_buffer.data();
				auto data1 = uint32_t();
				std::memcpy(&data1, section_data, 4Ui64);
				const auto uuid_args = std::format("{:08X}", data1);
				const auto data_args = std::vector<uint8_t>(section_data + 4, section_data + section_data_length);
				service_data_args[uuid_args] = data_args;
			}
			else if (section_type == type128 && section_data_length > 16)
			{
				const auto section_data = section_buffer.data();
				auto data1 = uint32_t();
				std::memcpy(&data1, section_data, 4Ui64);
				auto data2 = uint16_t();
				std::memcpy(&data2, section_data + 4, 2Ui64);
				auto data3 = uint16_t();
				std::memcpy(&data3, section_data + 6, 2Ui64);
				auto data4 = std::array<uint8_t, 8Ui64>();
				std::memcpy(&data4, section_data + 8, 8Ui64);
				const auto uuid = winrt::guid(data1, data2, data3, data4);
				const auto uuid_args = GUIDToArgs(uuid);
				const auto data_args = std::vector<uint8_t>(section_data + 16, section_data + section_data_length);
				service_data_args[uuid_args] = data_args;
			}
		}
		const auto manufacturer_data = advertisement.ManufacturerData();
		auto manufacturer_specific_data_args = flutter::EncodableList();
		for (const auto &data : manufacturer_data)
		{
			const auto data_args = ManufacturerDataToArgs(data);
			const auto data_args_value = flutter::CustomEncodableValue(data_args);
			manufacturer_specific_data_args.emplace_back(data_args_value);
		}
		return AdvertisementArgs(&name_args, service_uuids_args, service_data_args, manufacturer_specific_data_args);
	}

	GATTServiceArgs CentralManagerImpl::ServiceToArgs(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDeviceService &service)
	{
		const auto handle = service.AttributeHandle();
		const auto handle_args = static_cast<int64_t>(handle);
		const auto uuid = service.Uuid();
		const auto uuid_args = GUIDToArgs(uuid);
		const auto is_primary_args = true;
		const auto included_services_args = flutter::EncodableList();
		const auto characteristics_args = flutter::EncodableList();
		return GATTServiceArgs(handle_args, uuid_args, is_primary_args, included_services_args, characteristics_args);
	}

	GATTCharacteristicArgs CentralManagerImpl::CharacteristicToArgs(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristic &characteristic)
	{
		const auto handle = characteristic.AttributeHandle();
		const auto handle_args = static_cast<int64_t>(handle);
		const auto uuid = characteristic.Uuid();
		const auto uuid_args = GUIDToArgs(uuid);
		const auto properties = characteristic.CharacteristicProperties();
		const auto property_numbers_args = CharacteristicPropertiesToArgs(properties);
		const auto descriptors_args = flutter::EncodableList();
		return GATTCharacteristicArgs(handle_args, uuid_args, property_numbers_args, descriptors_args);
	}

	flutter::EncodableList CentralManagerImpl::CharacteristicPropertiesToArgs(winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties properties)
	{
		const auto readable = static_cast<int>(properties & winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties::Read);
		const auto writable = static_cast<int>(properties & winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties::Write);
		const auto writableWithoutResponse = static_cast<int>(properties & winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties::WriteWithoutResponse);
		const auto notifiable = static_cast<int>(properties & winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties::Notify);
		const auto indicatable = static_cast<int>(properties & winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCharacteristicProperties::Indicate);
		auto property_numbers_args = flutter::EncodableList();
		if (readable)
		{
			const auto property_number_args = static_cast<int>(GATTCharacteristicPropertyArgs::kRead);
			property_numbers_args.emplace_back(property_number_args);
		}
		if (writable)
		{
			const auto property_number_args = static_cast<int>(GATTCharacteristicPropertyArgs::kWrite);
			property_numbers_args.emplace_back(property_number_args);
		}
		if (writableWithoutResponse)
		{
			const auto property_number_args = static_cast<int>(GATTCharacteristicPropertyArgs::kWriteWithoutResponse);
			property_numbers_args.emplace_back(property_number_args);
		}
		if (notifiable)
		{
			const auto property_number_args = static_cast<int>(GATTCharacteristicPropertyArgs::kNotify);
			property_numbers_args.emplace_back(property_number_args);
		}
		if (indicatable)
		{
			const auto property_number_args = static_cast<int>(GATTCharacteristicPropertyArgs::kIndicate);
			property_numbers_args.emplace_back(property_number_args);
		}
		return property_numbers_args;
	}

	GATTDescriptorArgs CentralManagerImpl::DescriptorToArgs(const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattDescriptor &descriptor)
	{
		const auto handle = descriptor.AttributeHandle();
		const auto handle_args = static_cast<int64_t>(handle);
		const auto uuid = descriptor.Uuid();
		const auto uuid_args = GUIDToArgs(uuid);
		return GATTDescriptorArgs(handle_args, uuid_args);
	}

	std::string CentralManagerImpl::GUIDToArgs(const winrt::guid &guid)
	{
		// const auto uuid_value = winrt::to_hstring(uuid);
		// return winrt::to_string(uuid_value);
		return std::format("{}", guid);
	}

	winrt::Windows::Devices::Bluetooth::BluetoothCacheMode CentralManagerImpl::ArgsToCacheMode(const CacheModeArgs &mode_args)
	{
		switch (mode_args)
		{
		case CacheModeArgs::kCached:
			return winrt::Windows::Devices::Bluetooth::BluetoothCacheMode::Cached;
		case CacheModeArgs::kUncached:
			return winrt::Windows::Devices::Bluetooth::BluetoothCacheMode::Uncached;
		default:
			throw std::bad_cast();
		}
	}

	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption CentralManagerImpl::ArgsToWriteOption(const GATTCharacteristicWriteTypeArgs &type_args)
	{
		switch (type_args)
		{
		case GATTCharacteristicWriteTypeArgs::kWithoutResponse:
			return winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption::WriteWithoutResponse;
		case GATTCharacteristicWriteTypeArgs::kWithResponse:
			return winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattWriteOption::WriteWithResponse;
		default:
			throw std::bad_cast();
		}
	}

	winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue CentralManagerImpl::ArgsToCCCDescriptorValue(const GATTCharacteristicNotifyStateArgs &state_args)
	{
		switch (state_args)
		{
		case GATTCharacteristicNotifyStateArgs::kNone:
			return winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue::None;
		case GATTCharacteristicNotifyStateArgs::kNotify:
			return winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue::Notify;
		case GATTCharacteristicNotifyStateArgs::kIndicate:
			return winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattClientCharacteristicConfigurationDescriptorValue::Indicate;
		default:
			throw std::bad_cast();
		}
	}

	// ── ペアリング(暗号化リンクの確立)─────────────────────────────
	//
	// 保護された属性は、リンクが暗号化されていないと読み書きできない。
	// ボンディングしない装置では鍵が保存されないため、接続ごとにここを
	// 通してから初期読み出しへ進む必要がある。

	void CentralManagerImpl::Pair(int64_t address_args, const DevicePairingProtectionLevelArgs &protection_level_args, const DevicePairingConsentArgs &consent_args, std::function<void(ErrorOr<DevicePairingResultStatusArgs> reply)> result)
	{
		PairAsync(address_args, protection_level_args, consent_args, std::move(result));
	}

	void CentralManagerImpl::Unpair(int64_t address_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		UnpairAsync(address_args, std::move(result));
	}

	void CentralManagerImpl::IsPaired(int64_t address_args, std::function<void(ErrorOr<bool> reply)> result)
	{
		IsPairedAsync(address_args, std::move(result));
	}

	// WinRT はプラットフォームスレッド(STA)でのブロック待ちを禁じている。
	// `.get()` で待つと `!is_sta_thread()` の表明で落ちるため、コルーチンにする。
	winrt::fire_and_forget CentralManagerImpl::IsPairedAsync(int64_t address_args, std::function<void(ErrorOr<bool> reply)> result)
	{
		try
		{
			const auto address = static_cast<uint64_t>(address_args);
			const auto &device = co_await winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(address);
			if (device == nullptr)
			{
				result(FlutterError("IllegalArgument", "Device not found."));
				co_return;
			}
			result(device.DeviceInformation().Pairing().IsPaired());
		}
		catch (const winrt::hresult_error &ex)
		{
			result(FlutterError("winrt::hresult_error", winrt::to_string(ex.message())));
		}
		catch (const std::exception &ex)
		{
			result(FlutterError("std::exception", ex.what()));
		}
	}

	// ConfirmOnly の同意を代行する(同意の主体が app のときだけ登録される)。
	// これを登録して Accept() すると Windows の同意 UI は出ない。
	void CentralManagerImpl::OnPairingRequested(const winrt::Windows::Devices::Enumeration::DeviceInformationCustomPairing &sender, const winrt::Windows::Devices::Enumeration::DevicePairingRequestedEventArgs &args)
	{
		const auto kind = args.PairingKind();
		// PIN の入力・表示が要るセレモニーは代行できない。OS/利用者に委ねる。
		if (kind == winrt::Windows::Devices::Enumeration::DevicePairingKinds::ConfirmOnly)
		{
			args.Accept();
		}
	}

	// pigeon の enum → WinRT の保護レベル。
	static winrt::Windows::Devices::Enumeration::DevicePairingProtectionLevel ProtectionLevelFromArgs(const DevicePairingProtectionLevelArgs &args)
	{
		using winrt::Windows::Devices::Enumeration::DevicePairingProtectionLevel;
		switch (args)
		{
		case DevicePairingProtectionLevelArgs::kNone:
			return DevicePairingProtectionLevel::None;
		case DevicePairingProtectionLevelArgs::kEncryption:
			return DevicePairingProtectionLevel::Encryption;
		case DevicePairingProtectionLevelArgs::kEncryptionAndAuthentication:
			return DevicePairingProtectionLevel::EncryptionAndAuthentication;
		case DevicePairingProtectionLevelArgs::kDefaultLevel:
		default:
			return DevicePairingProtectionLevel::Default;
		}
	}

	winrt::fire_and_forget CentralManagerImpl::PairAsync(int64_t address_args, DevicePairingProtectionLevelArgs protection_level_args, DevicePairingConsentArgs consent_args, std::function<void(ErrorOr<DevicePairingResultStatusArgs> reply)> result)
	{
		try
		{
			const auto address = static_cast<uint64_t>(address_args);
			const auto &device = co_await winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(address);
			if (device == nullptr)
			{
				result(FlutterError("IllegalArgument", "Device not found."));
				co_return;
			}
			// BluetoothLEDevice.DeviceInformation() は関連付けの情報が
			// 揃っておらず、そのまま PairAsync すると即 Failed になることが
			// ある。Id から取り直したものを使う。
			const auto &device_information = co_await winrt::Windows::Devices::Enumeration::DeviceInformation::CreateFromIdAsync(device.DeviceInformation().Id());
			const auto &pairing = device_information.Pairing();
			if (pairing.IsPaired())
			{
				result(DevicePairingResultStatusArgs::kAlreadyPaired);
				co_return;
			}
			if (!pairing.CanPair())
			{
				result(DevicePairingResultStatusArgs::kNotReadyToPair);
				co_return;
			}

			// 保護レベルは呼び出し側が装置のセキュリティ要求に合わせて指定
			// する。かつては `pairing.ProtectionLevel()` を渡していたが、
			// これは未ペアリング機器では「現在の水準 = None」を返すだけで
			// 要求水準ではなく、OS の解釈が走行ごとに揺れる(Encryption に
			// 昇格することも None のまま登録することもある)ことが実測で
			// 確認されたため、明示指定に改めた。
			const auto protection_level = ProtectionLevelFromArgs(protection_level_args);
			// 同意の主体で経路が分かれる。
			// - system: 既定の PairAsync。セレモニーと同意の UI は OS が担う
			//   (アプリは同意に関与しない)。
			// - app: Custom ペアリング。ConfirmOnly は PairingRequested ハンドラが
			//   Accept() で代行し、OS の同意 UI は出ない。ハンドラを登録しないと
			//   RequiredHandlerNotRegistered / RejectedByHandler で失敗する。
			winrt::Windows::Devices::Enumeration::DevicePairingResult pair_result{nullptr};
			if (consent_args == DevicePairingConsentArgs::kSystem)
			{
				pair_result = co_await pairing.PairAsync(protection_level);
			}
			else
			{
				const auto &custom = pairing.Custom();
				const auto token = custom.PairingRequested({this, &CentralManagerImpl::OnPairingRequested});
				pair_result = co_await custom.PairAsync(
					winrt::Windows::Devices::Enumeration::DevicePairingKinds::ConfirmOnly |
						winrt::Windows::Devices::Enumeration::DevicePairingKinds::ProvidePin,
					protection_level);
				custom.PairingRequested(token);
			}
			// スレッド切替(co_await)前に結果値をすべて値で確定させる。
			const auto pair_status = PairingStatusToArgs(pair_result.Status());
			const auto used_level = pair_result.ProtectionLevelUsed();
			// 実際に使われた保護レベルは要求と食い違うことがあるため、
			// 操作単位のログに残す。
			Log(LogLevelArgs::kFine,
				"pair: consent=" + std::string(consent_args == DevicePairingConsentArgs::kSystem ? "system" : "app") +
					", requested protectionLevel=" + std::to_string(static_cast<int32_t>(protection_level)) +
					", used=" + std::to_string(static_cast<int32_t>(used_level)));
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			result(pair_status);
		}
		catch (const winrt::hresult_error &ex)
		{
			result(FlutterError("winrt::hresult_error", winrt::to_string(ex.message())));
		}
		catch (const std::exception &ex)
		{
			result(FlutterError("std::exception", ex.what()));
		}
	}

	winrt::fire_and_forget CentralManagerImpl::UnpairAsync(int64_t address_args, std::function<void(std::optional<FlutterError> reply)> result)
	{
		try
		{
			const auto address = static_cast<uint64_t>(address_args);
			const auto &device = co_await winrt::Windows::Devices::Bluetooth::BluetoothLEDevice::FromBluetoothAddressAsync(address);
			if (device == nullptr)
			{
				result(FlutterError("IllegalArgument", "Device not found."));
				co_return;
			}
			const auto &pairing = device.DeviceInformation().Pairing();
			if (!pairing.IsPaired())
			{
				// 既に解除済み。成功として返す。
				result(std::nullopt);
				co_return;
			}
			const auto &unpair_result = co_await pairing.UnpairAsync();
			const auto status = unpair_result.Status();
			if (status != winrt::Windows::Devices::Enumeration::DeviceUnpairingResultStatus::Unpaired &&
				status != winrt::Windows::Devices::Enumeration::DeviceUnpairingResultStatus::AlreadyUnpaired)
			{
				const auto message = "Unpair failed with status: " + std::to_string(static_cast<int>(status));
				result(FlutterError("std::exception", message));
				co_return;
			}
			result(std::nullopt);
		}
		catch (const winrt::hresult_error &ex)
		{
			result(FlutterError("winrt::hresult_error", winrt::to_string(ex.message())));
		}
		catch (const std::exception &ex)
		{
			result(FlutterError("std::exception", ex.what()));
		}
	}

	DevicePairingResultStatusArgs CentralManagerImpl::PairingStatusToArgs(const winrt::Windows::Devices::Enumeration::DevicePairingResultStatus &status)
	{
		using winrt::Windows::Devices::Enumeration::DevicePairingResultStatus;
		switch (status)
		{
		case DevicePairingResultStatus::Paired:
			return DevicePairingResultStatusArgs::kPaired;
		case DevicePairingResultStatus::NotReadyToPair:
			return DevicePairingResultStatusArgs::kNotReadyToPair;
		case DevicePairingResultStatus::NotPaired:
			return DevicePairingResultStatusArgs::kNotPaired;
		case DevicePairingResultStatus::AlreadyPaired:
			return DevicePairingResultStatusArgs::kAlreadyPaired;
		case DevicePairingResultStatus::ConnectionRejected:
			return DevicePairingResultStatusArgs::kConnectionRejected;
		case DevicePairingResultStatus::TooManyConnections:
			return DevicePairingResultStatusArgs::kTooManyConnections;
		case DevicePairingResultStatus::HardwareFailure:
			return DevicePairingResultStatusArgs::kHardwareFailure;
		case DevicePairingResultStatus::AuthenticationTimeout:
			return DevicePairingResultStatusArgs::kAuthenticationTimeout;
		case DevicePairingResultStatus::AuthenticationNotAllowed:
			return DevicePairingResultStatusArgs::kAuthenticationNotAllowed;
		case DevicePairingResultStatus::AuthenticationFailure:
			return DevicePairingResultStatusArgs::kAuthenticationFailure;
		case DevicePairingResultStatus::NoSupportedProfiles:
			return DevicePairingResultStatusArgs::kNoSupportedProfiles;
		case DevicePairingResultStatus::ProtectionLevelCouldNotBeMet:
			return DevicePairingResultStatusArgs::kProtectionLevelCouldNotBeMet;
		case DevicePairingResultStatus::AccessDenied:
			return DevicePairingResultStatusArgs::kAccessDenied;
		case DevicePairingResultStatus::InvalidCeremonyData:
			return DevicePairingResultStatusArgs::kInvalidCeremonyData;
		case DevicePairingResultStatus::PairingCanceled:
			return DevicePairingResultStatusArgs::kPairingCanceled;
		case DevicePairingResultStatus::OperationAlreadyInProgress:
			return DevicePairingResultStatusArgs::kOperationAlreadyInProgress;
		case DevicePairingResultStatus::RequiredHandlerNotRegistered:
			return DevicePairingResultStatusArgs::kRequiredHandlerNotRegistered;
		case DevicePairingResultStatus::RejectedByHandler:
			return DevicePairingResultStatusArgs::kRejectedByHandler;
		case DevicePairingResultStatus::RemoteDeviceHasAssociation:
			return DevicePairingResultStatusArgs::kRemoteDeviceHasAssociation;
		default:
			return DevicePairingResultStatusArgs::kFailed;
		}
	}


	// GATT の失敗を、ATT のエラーコードを保ったまま FlutterError にする。
	//
	// GattCommunicationStatus だけでは「装置が拒否した」ことしか分からない。
	// ProtocolError(ATT のエラーコード)まで返すと、0x05(認証不足)と
	// 0x0F(暗号化不足)のような区別がつき、呼び出し側が対処を選べる。
	FlutterError CentralManagerImpl::GattError(const std::string &operation, const winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus &status, const winrt::Windows::Foundation::IReference<uint8_t> &protocol_error)
	{
		using winrt::Windows::Devices::Bluetooth::GenericAttributeProfile::GattCommunicationStatus;
		std::string code;
		switch (status)
		{
		case GattCommunicationStatus::Unreachable:
			code = "Unreachable";
			break;
		case GattCommunicationStatus::ProtocolError:
			code = "ProtocolError";
			break;
		case GattCommunicationStatus::AccessDenied:
			code = "AccessDenied";
			break;
		default:
			code = "Unknown";
			break;
		}
		auto message = operation + " failed with status: " + code;
		auto details = flutter::EncodableMap{
			{flutter::EncodableValue("status"), flutter::EncodableValue(static_cast<int32_t>(status))},
		};
		if (protocol_error != nullptr)
		{
			const auto att = protocol_error.Value();
			std::stringstream stream;
			stream << "0x" << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(att);
			message += ", protocolError: " + stream.str();
			details.insert({flutter::EncodableValue("protocolError"), flutter::EncodableValue(static_cast<int32_t>(att))});
		}
		return FlutterError(code, message, flutter::EncodableValue(details));
	}

	// ── WinRT イベント → Flutter の中継 ──
	//
	// WinRT のイベントはスレッドプール上で発火するが、Flutter のチャネル
	// 送信はプラットフォームスレッドからしか行えない(他スレッドからの
	// 送信はエンジンが検出してエラーを出し、欠落やクラッシュの原因に
	// なる)。生成時に捕捉した apartment(m_ui_thread)へ co_await で
	// 戻してから送る。
	// fire_and_forget から例外が漏れると std::terminate になるため、
	// 中継の失敗は握りつぶす(イベント通知は元々ベストエフォート)。

	winrt::fire_and_forget CentralManagerImpl::Log(LogLevelArgs level_args, std::string message_args)
	{
		try
		{
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			auto &api = m_api.value();
			api.OnLogged(level_args, message_args, [] {}, [](auto error) {});
		}
		catch (...)
		{
		}
	}

	winrt::fire_and_forget CentralManagerImpl::NotifyStateChanged(BluetoothLowEnergyStateArgs state_args)
	{
		try
		{
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			auto &api = m_api.value();
			api.OnStateChanged(state_args, [] {}, [](auto error) {});
		}
		catch (...)
		{
		}
	}

	winrt::fire_and_forget CentralManagerImpl::NotifyDiscovered(PeripheralArgs peripheral_args, int64_t rssi_args, int64_t timestamp_args, AdvertisementTypeArgs type_args, AdvertisementArgs advertisement_args)
	{
		try
		{
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			auto &api = m_api.value();
			api.OnDiscovered(peripheral_args, rssi_args, timestamp_args, type_args, advertisement_args, [] {}, [](auto error) {});
		}
		catch (...)
		{
		}
	}

	winrt::fire_and_forget CentralManagerImpl::NotifyMTUChanged(PeripheralArgs peripheral_args, int64_t mtu_args)
	{
		try
		{
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			auto &api = m_api.value();
			api.OnMTUChanged(peripheral_args, mtu_args, [] {}, [](auto error) {});
		}
		catch (...)
		{
		}
	}

	winrt::fire_and_forget CentralManagerImpl::NotifyCharacteristicNotified(PeripheralArgs peripheral_args, GATTCharacteristicArgs characteristic_args, std::vector<uint8_t> value_args)
	{
		try
		{
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			auto &api = m_api.value();
			api.OnCharacteristicNotified(peripheral_args, characteristic_args, value_args, []() {}, [](const auto &error) {});
		}
		catch (...)
		{
		}
	}

	winrt::fire_and_forget CentralManagerImpl::HandleConnectionStatusChanged(int64_t address_args, winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus status)
	{
		try
		{
			const auto ui_thread = m_ui_thread;
			co_await ui_thread;
			// 切断時の後片付け(内部マップの変更)もプラットフォーム
			// スレッドで行い、マップの変更を 1 スレッドに寄せる。
			auto &api = m_api.value();
			// 調査用: 状態が変わるたびにセッションの様子を記録する。
			if (status == winrt::Windows::Devices::Bluetooth::BluetoothConnectionStatus::Disconnected)
			{
				// 維持依頼(MaintainConnection)が生きているなら、装置と
				// セッションは手放さない。手放すと依頼ごと消えて OS による
				// 再確立が起きなくなる。リンクに紐づく GATT オブジェクトだけ
				// 捨てる。
				const auto it = m_sessions.find(address_args);
				const auto maintained = it != m_sessions.end() && it->second.has_value() && it->second.value().MaintainConnection();
				if (maintained)
				{
					// GATT オブジェクトも捨てない。呼び出し側が持っている
					// ハンドルは再確立後もそのまま使えるべきで、捨てると
					// 参照が宙に浮く。CCCD は装置側が切断で落とすため、
					// 通知の張り直しは呼び出し側の責務。
				}
				else
				{
					OnDisconnected(address_args);
				}
			}
			const auto peripheral_args = PeripheralArgs(address_args);
			const auto state_args = ConnectionStatusToArgs(status);
			api.OnConnectionStateChanged(peripheral_args, state_args, [] {}, [](auto error) {});
		}
		catch (...)
		{
		}
	}

}
