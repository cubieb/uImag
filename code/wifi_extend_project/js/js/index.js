$(function () {
    // 发送ajax请求的目的是为了获取在跳转到下一页的时候获取wifi列表的数据
    //一进入界面的时候就进行ajax请求数据并且缓存到本地
    $.ajax({
        url: "/cgi-bin/wifi_setting.cgi",
        type: "POST",
        data: "wifiScan=Scan",
        async: false,//false代表只有在等待ajax执行完毕后才执行window.location.href = "wifi_setting.html";语句
        success: function (responseText) {
            //所有扫描出的wifi列表的数据
            localStorage.setItem('scanWifi', responseText);
            console.log(localStorage);
            console.log("首页跳转到wifi_setting页时获取wifi列表数据成功");
        },
        error: function (response) {
            console.log(response);
            console.log("Connection error");
            console.log("首页跳转到wifi_setting页时获取wifi列表数据失败");
        },
    });

    $("#footer>.setting").on("click", function () {
        //wifi列表全部加载完成以后，再跳转到下一个wifi_setting.html页面
        window.location.href = "wifi_setting.html";

        //测试用的本地的假数据
        // var responseText = '{"Scan":[{"Channel":"3","ssid":"TPLink-EF-23"},{"Channel":"4","ssid":"TPLink-EF-24"},{"Channel":"5","ssid":"TPLink-EF-25"},{"Channel":"6","ssid":"TPLink-EF-26"},{"Channel":"7","ssid":"TPLink-EF-27"},{"Channel":"8","ssid":"TPLink-EF-28"},{"Channel":"9","ssid":"TPLink-EF-29"}]}';
        // localStorage.setItem('scanWifi', responseText);
        // console.log(localStorage);


        // 数组中一共有10个元素
        // var responseText = '{"Scan":[{"Channel":"1","ssid":"CMCC-2ACK","bssid":"e0:1c:ee:08:2a:cc","security":"WPA1PSKWPA2PSK/AES","signal":"59","mode":"11b/g/n","ext_ch":"ABOVE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"MT7628_SKY","bssid":"00:0c:43:e1:76:28","security":"NONE","signal":"79","mode":"11b/g/n","ext_ch":"ABOVE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"BLAZ4G","bssid":"b8:f8:83:c2:97:b3","security":"WPA1PSKWPA2PSK/TKIPAES","signal":"15","mode":"11b/g/n","ext_ch":"ABOVE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Guest","bssid":"70:3d:15:f1:3d:71","security":"OPEN","signal":"40","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Smart","bssid":"70:3d:15:f1:54:10","security":"WPA2PSK/AES","signal":"89","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Guest","bssid":"70:3d:15:f1:54:11","security":"WPA1PSKWPA2PSK/AES","signal":"89","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Smart","bssid":"70:3d:15:f1:89:50","security":"WPA2PSK/AES","signal":"11","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Guest","bssid":"70:3d:15:f1:89:51","security":"WPA1PSKWPA2PSK/AES","signal":"59","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"","bssid":"70:3d:15:f1:89:8111111111","security":"WPA1PSKWPA2PSK/AES","signal":"23","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{}]}';
        // localStorage.setItem('scanWifi', responseText);
        // console.log(localStorage);
    });
})
