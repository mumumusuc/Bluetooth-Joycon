# BT-NS-ProController

### HidClient
    
[hidclient](https://github.com/benizi/hidclient)

[BTGamepad](https://github.com/007durgesh219/BTGamepad)

1. 设置bluez
    - 编辑 `/etc/bluetooth/main.conf`
    
        ```
        @ /etc/bluetooth/main.conf  
        
        // Set ProController VID=0x057e & PID=0x2009
        + DeviceID = bluetooth:057e:2009:0100
        
        // Disable input
        + DisablePlugins = input
        ```
        
    - 重启bluetoothd
        
        ```
        @ /etc/systemd/system/dbus-org.bluez.service 
        
        - ExecStart=/usr/lib/bluetooth/bluetoothd
        + ExecStart=/usr/lib/bluetooth/bluetoothd -C
        
        // restart bluetooth
        # sudo systemctl daemon-reload
        # sudo systemctl restart bluetooth.service
        ```
        
 2. 注册SDP Hid Service
 
    [Nintendo_Switch_Bluetooth_Stuff](https://github.com/qsypoq/Nintendo_Switch_Bluetooth_Stuff)
    
    >   Service Name: Wireless Gamepad  
        Service Description: Gamepad  
        Service Provider: Nintendo  
        Service RecHandle: 0x10000  
        Service Class ID List:  
        &emsp;"Human Interface Device" (0x1124)  
        Protocol Descriptor List:  
        &emsp;"L2CAP" (0x0100)  
        &emsp;&emsp;PSM: 17  
        &emsp;"HIDP" (0x0011)  
        Language Base Attr List:  
        &emsp;code_ISO639: 0x656e  
        &emsp;encoding:    0x6a  
        &emsp;base_offset: 0x100  
        Profile Descriptor List:  
        &emsp;"Human Interface Device" (0x1124)  
        &emsp;&emsp;Version: 0x0101 

### Ninetndo Switch Pro(BT)
 
[libjoycon]()     

### Connect

![bluez连接NintendoSwitch](https://github.com/mumumusuc/BT-NS-ProController/blob/master/images/bt-pro.png)

![识别为蓝牙Pro手柄](https://github.com/mumumusuc/BT-NS-ProController/blob/master/images/sample.jpg)



