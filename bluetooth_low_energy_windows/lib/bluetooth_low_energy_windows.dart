import 'package:bluetooth_low_energy_platform_interface/bluetooth_low_energy_platform_interface.dart';

import 'src/central_manager_impl.dart';
import 'src/peripheral_manager_impl.dart';

// Windows 固有の追加 API(ペアリング)。共有インターフェースには足さず、
// この実装パッケージの公開面にだけ出す。
export 'src/central_manager_impl.dart' show CentralManagerImpl;
export 'src/pairing.dart'
    show
        DevicePairingProtectionLevel,
        DevicePairingResultStatus,
        GATTProtectionLevel;

abstract class BluetoothLowEnergyWindowsPlugin {
  static void registerWith() {
    CentralManagerChannel.instance = CentralManagerChannelImpl();
    PeripheralManagerChannel.instance = PeripheralManagerChannelImpl();
  }
}
