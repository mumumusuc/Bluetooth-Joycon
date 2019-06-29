# Bluetooth Joycon

## Android(Need Root!)

*需要Android9-Pie，tested on my Nexus5x with PixelExperience9.0*

1. 设置蓝牙PID、VID
    ```
    // remount /system as "rw"
    # adb shell
    bullhead:$ su
    bullhead:# mount -o remount,rw /system
    
    // pull file
    bullhead:# cp /etc/bluetooth/bt_did.conf /sdcard
    # adb pull /sdcard/bt_did.conf .
    
    // modify file
    @ bt_did.conf
    - productId = 0x1200
    + productId = 0x2009
    + vendorId = 0x057E
    
    // push file
    # adb push bt_did.conf /sdcard
    bullhead:# cp /sdcard/bt_did.conf /etc/bluetooth/ 
    ```
    
2. 安装Xposed

    Android9.0可用的[Xposed](https://github.com/ElderDrivers/EdXposed)。
    
    因为这个项目会使用Xposed禁用除了Hid-Host之外的所有蓝牙服务，所以若不能安装Xposed框架则需修改编译你的系统源码。源码非相关修改将在下节进行说明。
    
    
3. NFC功能&蓝牙服务

    因为Pro手柄的nfc数据需要362字节的InputReport，而在AOSP9.0-Pie的蓝牙源码中将HID_DEV的Report数据大小设置为了64字节。如果需要实现这一功能，你需要修改编译AOSP9.0-Pie源码。
    
    *建议修改device项目，而非直接修改AOSP代码*
    
    ```
    @ device/lge/bullhead/bluetooth/conf/bt_did.conf
    - productId = 0x1200
    + productId = 0x2009
    + vendorId = 0x057E
    
    @ device/lge/bullhead/bluetooth/bdroid_buildcfg.h
    + #define HID_DEV_MTU_SIZE 512 // 362+ is OK
    
    @ device/lge/bullhead/aosp_bullhead.mk
    + PRODUCT_COPY_FILES += device/lge/bullhead/bluetooth/conf/bt_did.conf:system/etc/bluetooth/bt_did.conf
    ```

## Linux


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



