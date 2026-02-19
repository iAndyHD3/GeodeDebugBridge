geode build -p android64 --config Release -- -DANDROID_STL=c++_shared
adb -s 192.168.0.156:44631 push build-android64\iandyhd3.geode-debug-bridge.geode /storage/emulated/0/Android/media/com.geode.launcher/game/geode/mods
adb -s 192.168.0.156:44631 shell "am force-stop com.geode.launcher && am start -n com.geode.launcher/com.geode.launcher.GeometryDashActivity"

ADB QR: connected to 192.168.0.156:44631
