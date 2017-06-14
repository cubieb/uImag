$(function () {
    // 发送ajax请求的目的是为了获取在跳转到下一页的时候获取wifi列表的数据
    //一进入界面的时候就进行ajax请求数据并且缓存到本地
    $.ajax({
        url: "/cgi-bin/wifi_setting.cgi",
        type: "POST",
        data: "wifiScan=Scan",
        async:false,//false代表只有在等待ajax执行完毕后才执行window.location.href = "wifi_setting.html";语句
        success: function (responseText) {
            //所有扫描出的wifi列表的数据
            localStorage.setItem('scanWifi', responseText);
            console.log(localStorage);
            console.log("首页跳转到wifi_setting页时获取wifi列表数据成功");
        },
        error: function(response) {
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
    });
})
