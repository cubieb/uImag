$(function () {
    initialFun();
    function initialFun() {
        //进入路由设置界面的次数
        var JRCount = localStorage.getItem("Count");
        console.log(JRCount);
        console.log(typeof JRCount);
        if (JRCount == 1) {
            //首次进入路由设置界面情况
            $.ajax({
                // url: "/cgi-bin/wifi_setting.cgi",
                type: "POST",
                // data: "wifiChosen=Chosen",//这里改为请求wifisetting中的已经被选中的wifi的数据,其中还包括密码
                async: false,
                error: function (response) {
                    console.log("首次进入路由设置界面失败");
                },
                success: function (responseText) {
                    //所有扫描出的wifi列表的数据
                    console.log("首次进入路由设置界面成功");

                    var chosenEntity = JSON.parse(responseText);
                    //用户没有做选择，使用默认值的情况或者用户做了选择点击，但是没有选中哪一个
                    wifiObj = chosenEntity.Chosen;
                    //初始化路由设置界面的路由名称(Chosen是wifi_setting页面提交过来的，包括配置界面的主路由的所有信息和用户输入的正确密码)
                    $("#wifiName>.title-name").text(chosenEntity.Chosen.ssid);
                    //初始化路由设置界面的路由密码（wifiPassword是输入的主路由的密码）
                    $("#wifiPwd").val(chosenEntity.Chosen.wifiPassword);
                    //初始化信号强度
                    $("#signalStrength").val(chosenEntity.Chosen.signal);
                    //加密方式
                    $("#encrypt").val(chosenEntity.Chosen.security);
                    //MAC地址
                    $("#mac").val(chosenEntity.Chosen.mac);
                }
            });
        } else {
            //非首次进入路由设置界面情况

            //要在每次提交当前被选中的wifi信息的时候setItem一下（otherChosenWifi保存的是当前路由设置界面的被选中的wifi的信息，包括密码）
            var OtherChosen = localStorage.getItem('OtherChosen');
            //用户没有做选择，使用默认值的情况或者用户做了选择点击，但是没有选中哪一个
            wifiObj = OtherChosen;
            //初始化路由设置界面的路由名称
            $("#wifiName>.title-name").text(OtherChosen.ssid);
            //初始化路由设置界面的路由密码
            $("#wifiPwd").val(OtherChosen.wifiPwd);
            //初始化信号强度
            $("#signalStrength").val(OtherChosen.signal);
            //加密方式
            $("#encrypt").val(OtherChosen.security);
            //MAC地址
            $("#mac").val(OtherChosen.mac);
        }

        if (JRCount > 1) {
            JRCount = 2;
        }
        localStorage.setItem("Count", JRCount);

        //获取上个页面已经发送请求服务端返回的保存在本地的wifi列表信息
        var scanJsonStr = localStorage.getItem('scanWifi');
        scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        console.log(scanJsonStr);
        var scan = scanEntity.Scan;

        //按照当前信号前强度重新排序数组中的对象
        // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
        var sortObj = scan.sort(compare("signal"));//会改变scan的排序
        console.log(scanEntity.Scan);

        //直接拿到localStorage中的数据以后，模板渲染到页面的ul中。
        var html = template("listTmp", scanEntity);
        $('#selectCon').html(html);
        console.log("点击下拉按钮之前渲染本地数据到页面成功");
        console.log($('#selectCon').html());

        //判断当前扫描的wifi是否有锁
        //点击下拉按钮以后需要遍历一遍看是否都有锁
        $.each(scan, function (index_, value_) {
            $.each(scan[index_], function (name, value) {
                if ((scan[index_][name] === "")) {
                    console.log("不合法的wifi信息数据-------d");
                    console.log(index_);
                    console.log(scan);
                }

                //给security不同的wifi加锁和不加锁
                // console.log(index_);
                if (name === "security" && value === "OPEN" || name === "security" && value === "NONE") {
                    $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(1)").css("display", "none");
                    // console.log("加锁图标的显示隐藏");
                }

                //给信号强度不同的wifi加上不同的信号强度图标
                if (name === "signal") {
                    // console.log(value);
                    switch (true) {
                        case value >= 75 :
                            // console.log("满格信号");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_01.png");
                            return false;//$.each的退出循环,此处跳出的是数组中对对象的遍历循环，不是跳出的对对象外层数组的遍历循环。
                        // break;
                        case value >= 50 :
                            // console.log("信号较强");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_02.png");
                            return false;
                        // break;
                        case value >= 25 :
                            // console.log("信号一般");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_03.png");
                            return false;
                        // break;
                        case value >= 0 :
                            // console.log("信号很弱");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_04.png");
                            return false;
                        // break;
                    }
                }
            });
        });

        //存入的是重新排序以后的结果
        var responseStr = JSON.stringify(scanEntity);
        localStorage.setItem('scanWifi', responseStr);
        console.log(localStorage);
    };

    //被选中的wifi信息
    wifiData = "Channel=" + wifiObj.Channel + "&" +
        "ssid=" + wifiObj.ssid + "&" +
        "bssid=" + wifiObj.bssid + "&" +
        "security=" + wifiObj.security + "&" +
        "signal=" + wifiObj.signal + "&" +
        "mode=" + wifiObj.mode + "&" +
        "ext_ch=" + wifiObj.ext_ch + "&" +
        "net_type=" + wifiObj.net_type + "&" +
        "wps=" + wifiObj.wps;
    console.log("成功获取被点击的wifi的相关信息：" + wifiData);

    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    //扫描获取wifi列表信息
    //wifi列表拿到的只是刷新缓存到本地的数据
    var listBtn = $("#wifiName>.wifi-r>.list-btn");
    listBtn.on("click", function (e) {
        //将第一页跳转过程中加载的数据从HTML5的localStorage中取出来
        // 取值时：把获取到的Json字符串转换回对象
        var scanJsonStr = localStorage.getItem('scanWifi');
        scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        console.log(scanEntity);
        var scan = scanEntity.Scan;

        //判断当前扫描的wifi是否有锁
        //点击下拉按钮以后需要遍历一遍看是都有锁
        $.each(scan, function (index_, value_) {
            //给信号强度不同的wifi加上不同的信号强度图标
            $.each(scan[index_], function (name, value) {
                if (name === "signal") {
                    console.log(value);
                    switch (true) {
                        case value >= 75 :
                            // console.log("满格信号");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_01.png");
                            return false;//$.each的退出循环,此处跳出的是数组中对对象的遍历循环，不是跳出的对对象外层数组的遍历循环。
                        // break;
                        case value >= 50 :
                            // console.log("信号较强");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_02.png");
                            return false;
                        // break;
                        case value >= 25 :
                            // console.log("信号一般");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_03.png");
                            return false;
                        // break;
                        case value >= 0 :
                            // console.log("信号很弱");
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_04.png");
                            return false;
                        // break;
                    }
                }

                //给security不同的wifi加锁和不加锁
                // console.log(index_);
                if (name === "security" && value === "OPEN" || name === "security" && value === "NONE") {
                    $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(1)").css("display", "none");
                    // console.log("加锁图标的显示隐藏");
                }
            });
        })

        e.stopPropagation();//不出现事件的原因是浏览器还有默认事件(跟select相关的)。

        $("#selectCon").select();//在selectContainer.js中的一个封装的方法
    });
    //这是测试用的
    //这是测试用的
    // listBtn.on("click", function (e) {
    //     //将第一页跳转过程中加载的数据从HTML5的localStorage中取出来
    //     // 取值时：把获取到的Json字符串转换回对象
    //     var scanJsonStr = localStorage.getItem('scanWifi');
    //     scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
    //     console.log(scanEntity);
    //     var scan = scanEntity.Scan;
    //
    //     //判断当前扫描的wifi是否有锁
    //     //点击下拉按钮以后需要遍历一遍看是都有锁
    //     $.each(scan, function (index_, value_) {
    //         //给信号强度不同的wifi加上不同的信号强度图标
    //         $.each(scan[index_], function (name, value) {
    //             if (name === "signal") {
    //                 console.log(value);
    //                 switch (true) {
    //                     case value >= 75 :
    //                         // console.log("满格信号");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_01.png");
    //                         return false;//$.each的退出循环,此处跳出的是数组中对对象的遍历循环，不是跳出的对对象外层数组的遍历循环。
    //                     // break;
    //                     case value >= 50 :
    //                         // console.log("信号较强");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_02.png");
    //                         return false;
    //                     // break;
    //                     case value >= 25 :
    //                         // console.log("信号一般");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_03.png");
    //                         return false;
    //                     // break;
    //                     case value >= 0 :
    //                         // console.log("信号很弱");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_04.png");
    //                         return false;
    //                     // break;
    //                 }
    //             }
    //
    //             //给security不同的wifi加锁和不加锁
    //             // console.log(index_);
    //             if (name === "security" && value === "OPEN" || name === "security" && value === "NONE") {
    //                 $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(1)").css("display", "none");
    //                 // console.log("加锁图标的显示隐藏");
    //             }
    //         });
    //     })
    //
    //     e.stopPropagation();//不出现事件的原因是浏览器还有默认事件(跟select相关的)。
    //
    //     $("#selectCon").select();//在selectContainer.js中的一个封装的方法
    // });

    //刷新按钮
    var $refreshBtn = $("#wifiName>.wifi-r>.refresh-btn");
    $refreshBtn.on("click", function (e) {
        $.ajax({
            url: "/cgi-bin/wifi_setting.cgi",
            type: "POST",
            data: "wifiScan=Scan",
            async: false,
            success: function (responseText) {
                response = JSON.parse(responseText);
                console.log("11111111111111111");

                //按照当前信号前强度重新排序数组中的对象
                // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
                var scan = response.Scan;
                var sortObj = scan.sort(compare("signal"));//会改变scan的排序
                console.log(scan);
                console.log(response.Scan);
                console.log(response);

                //直接拿到localStorage中的数据以后，模板渲染到页面的ul中。
                var html = template("listTmp", response);
                $('#selectCon').html(html);//注意这种重新渲染会将上一次渲染设置的样式等等抹掉。
                console.log(html);
                console.log("将模板数据渲染到页面成功");

                //判断当前扫描的wifi是否有锁
                //点击下拉按钮以后需要遍历一遍看是都有锁
                $.each(scan, function (index_, value_) {
                    $.each(scan[index_], function (name, value) {
                        //给信号强度不同的wifi加上不同的信号强度图标
                        if (name === "signal") {
                            console.log(value);
                            switch (true) {
                                case value >= 75 :
                                    // console.log("满格信号");
                                    $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_01.png");
                                    return false;//$.each的退出循环,此处跳出的是数组中对对象的遍历循环，不是跳出的对对象外层数组的遍历循环。
                                // break;
                                case value >= 50 :
                                    // console.log("信号较强");
                                    $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_02.png");
                                    return false;
                                // break;
                                case value >= 25 :
                                    // console.log("信号一般");
                                    $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_03.png");
                                    return false;
                                // break;
                                case value >= 0 :
                                    // console.log("信号很弱");
                                    $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_04.png");
                                    return false;
                                // break;
                            }
                        }

                        //给security不同的wifi加锁和不加锁
                        // console.log(index_);
                        if (name === "security" && value === "OPEN" || name === "security" && value === "NONE") {
                            $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(1)").css("display", "none");
                            // console.log("加锁图标的显示隐藏");
                        }
                    });
                })

                //注意：存进localStorage里面的数据只能是字符串形式的，取也只能是字符串形式取出来
                var responseStr = JSON.stringify(response);
                localStorage.setItem('scanWifi', responseStr);
                console.log(localStorage);
                console.log("刷新按钮获取服务器的wifi列表数据成功");
            }
        });
        e.stopPropagation();//不出现事件的原因是浏览器还有默认事件。
        $(".selectContainer").select();
    });
    //这是测试用的
    //这是测试用的
    // $refreshBtn.on("click", function (e) {
    //     var responseText = '{"Scan":[{"Channel":"1","ssid":"CMCC-2ACK","bssid":"e0:1c:ee:08:2a:cc","security":"WPA1PSKWPA2PSK/AES","signal":"59","mode":"11b/g/n","ext_ch":"ABOVE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"MT7628_SKY","bssid":"00:0c:43:e1:76:28","security":"NONE","signal":"79","mode":"11b/g/n","ext_ch":"ABOVE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"BLAZ4G","bssid":"b8:f8:83:c2:97:b3","security":"WPA1PSKWPA2PSK/TKIPAES","signal":"15","mode":"11b/g/n","ext_ch":"ABOVE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Guest","bssid":"70:3d:15:f1:3d:71","security":"OPEN","signal":"40","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Smart","bssid":"70:3d:15:f1:54:10","security":"WPA2PSK/AES","signal":"89","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Guest","bssid":"70:3d:15:f1:54:11","security":"WPA1PSKWPA2PSK/AES","signal":"89","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Smart","bssid":"70:3d:15:f1:89:50","security":"WPA2PSK/AES","signal":"11","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"CMCC~Guest","bssid":"70:3d:15:f1:89:51","security":"WPA1PSKWPA2PSK/AES","signal":"59","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{"Channel":"1","ssid":"","bssid":"70:3d:15:f1:89:8111111111","security":"WPA1PSKWPA2PSK/AES","signal":"23","mode":"11b/g/n","ext_ch":"NONE","net_type":"In","wps":"NO"},{}]}';
    //     localStorage.setItem('scanWifi', responseText);
    //
    //     var scanJsonStr = localStorage.getItem('scanWifi');
    //     scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
    //     console.log(scanEntity);
    //     var scan = scanEntity.Scan;
    //
    //     //按照当前信号前强度重新排序数组中的对象
    //     // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
    //     var sortObj = scan.sort(compare("signal"));//会改变scan的排序
    //     console.log(scan);
    //
    //     //直接拿到localStorage中的数据以后，模板渲染到页面的ul中。
    //     var html = template("listTmp", scanEntity);
    //     $('#selectCon').html(html);//注意这种重新渲染会将上一次渲染设置的样式等等抹掉。
    //     console.log(html);
    //     console.log("将模板数据渲染到页面成功");
    //
    //     //判断当前扫描的wifi是否有锁
    //     //点击下拉按钮以后需要遍历一遍看是都有锁
    //     $.each(scan, function (index_, value_) {
    //         $.each(scan[index_], function (name, value) {
    //             //给信号强度不同的wifi加上不同的信号强度图标
    //             if (name === "signal") {
    //                 console.log(value);
    //                 switch (true) {
    //                     case value >= 75 :
    //                         // console.log("满格信号");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_01.png");
    //                         return false;//$.each的退出循环,此处跳出的是数组中对对象的遍历循环，不是跳出的对对象外层数组的遍历循环。
    //                     // break;
    //                     case value >= 50 :
    //                         // console.log("信号较强");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_02.png");
    //                         return false;
    //                     // break;
    //                     case value >= 25 :
    //                         // console.log("信号一般");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_03.png");
    //                         return false;
    //                     // break;
    //                     case value >= 0 :
    //                         // console.log("信号很弱");
    //                         $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(2)").attr("src", "images/02_icon03_04.png");
    //                         return false;
    //                     // break;
    //                 }
    //             }
    //
    //             //给security不同的wifi加锁和不加锁
    //             // console.log(index_);
    //             if (name === "security" && value === "OPEN" || name === "security" && value === "NONE") {
    //                 $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(1)").css("display", "none");
    //                 // console.log("加锁图标的显示隐藏");
    //             }
    //         });
    //     })
    //
    //     //注意：存进localStorage里面的数据只能是字符串形式的，取也只能是字符串形式取出来
    //     var responseStr = JSON.stringify(scanEntity);
    //     localStorage.setItem('scanWifi', responseStr);
    //     console.log(localStorage);
    //     console.log("刷新按钮获取服务器的wifi列表数据成功");
    //
    //     e.stopPropagation();//不出现事件的原因是浏览器还有默认事件。
    //     $(".selectContainer").select();
    // });

    //确定按钮
    var $btn = $("#main>form>button>a");
    $btn.on("click", function () {
        layer.open({
            title: [
                '提示',
                'background-color:#438cff; color:#fff;'
            ]
            , anim: 'up'
            , content: '是否前往修改扩展WiFi信息？'
            , btn: [
                '确定',
                '取消',
            ]
            , yes: function (index) { //点击确定按钮
                var wifiPwd = $("#wifiPwd").val();
                //提交当前的路由信息和密码
                $.ajax({
                    type: "POST",
                    // url: "",//需要服务端的请求的地址
                    data: "routeInfo=route" + "&" + "routePwd=" + routePwd + "&" + wifiData,//需要哪些数据
                    error: function (response) {
                        console.log("提交当前路由设置信息失败");
                    },
                    success: function (response) {
                        //随便返回什么，然后进入成功界面
                        //需要返回所有的信息列表
                        console.log(response);
                        console.log("提交当前路由设置信息成功");

                        //当前信息提交到服务器成功以后，然后缓存到本地，方便再次进入页面的时候调取。
                        var obj = wifiData;
                        obj['wifiPwd'] = wifiPwd;
                        localStorage.setItem('OtherChosen', obj);

                        //返回字符串为routeSucess时进入下一个页面
                        if (response == "routeSucess") {
                            window.location.href = "manage_WiFi_setting.html";
                        }
                    }
                })

                // window.location.href = "manage_nav_online.html";
            }
            , no: function (index) { //点击取消按钮
                // layer.open({content: '执行取消操作'})

                //执行取消操作，进去重启放大器的过程
                window.location.href = "syetem_reboot_progress.html";
            }
            , success: function (elem) { //这里是打开页面成功
                //elem:该回调参数返回一个参数为当前层元素对象
                console.log(elem);

                //显示弹窗以后，增加一个元素到弹窗中
                $(".layui-m-layersection>.layui-m-layerchild").append("<div class='popup-close'></div>");

                $(".layui-m-layersection>.layui-m-layerchild>.popup-close").on("click", function () {
                    $(elem).css({"display": "none"});
                });
            }
        });
        return false;
    })

    function compare(property) {
        return function (obj1, obj2) {
            var value1 = obj1[property];
            var value2 = obj2[property];
            return value2 - value1;     // 降序
        }
    }
})