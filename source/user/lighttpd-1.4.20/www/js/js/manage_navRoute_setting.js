$(function () {
    initialFun();
    function initialFun() {
        //返回按钮
        $("header>.nav-back").on("click", function () {
            window.history.go(-1);
        });

        $.ajax({
            url: "/cgi-bin/wifi_setting.cgi",
            type: "POST",
            data: "wifiScan=Scan",//这里改为请求wifisetting中的已经被选中的wifi的数据
            async: false,//false代表只有在等待ajax执行完毕后才执行window.location.href = "wifi_setting.html";语句
            success: function (responseText) {
                console.log("管理界面的首页跳转到路由设置界面成功");
                console.log(responseText);

                localStorage.setItem("scanWifi", responseText);
            },
            error: function (response) {
                console.log("管理界面的首页跳转到路由设置界面失败");
            },
        });

        // var responseText = '{"Scan":[{"Channel":"3","ssid":"TPLink-EF-23"},{"Channel":"4","ssid":"TPLink-EF-24"},{"Channel":"5","ssid":"TPLink-EF-25"},{"Channel":"6","ssid":"TPLink-EF-26"},{"Channel":"7","ssid":"TPLink-EF-27"},{"Channel":"8","ssid":"TPLink-EF-28"},{"Channel":"9","ssid":"TPLink-EF-29"}]}';
        // localStorage.setItem("scanWifi", responseText);

        //获取上个页面已经发送请求服务端返回的保存在本地的wifi列表信息
        var scanJsonStr = localStorage.getItem('scanWifi');
        console.log(scanJsonStr);
        scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        console.log(scanJsonStr);
        var scan = scanEntity.Scan;

        //将信号的数值转换成中文的优良中差
        $.each(scan, function (index_, value_) {
            $.each(scan[index_], function (name, value) {
                if (name == "signal") {
                    switch (true) {
                        case value >= -80 :
                            scan[index_][name] = "优";
                            break;
                        case value >= -85 :
                            scan[index_][name] = "良";
                            break;
                        case value >= -90 :
                            scan[index_][name] = "中";
                            break;
                        case value >= -110 :
                            scan[index_][name] = "差";
                            break;
                        default:
                            console.log("信号强度不在以上范围内");
                    }
                }
            })
        })
        console.log(scan);

        //判断OtherChosen是否存在，存在的情况下，加载最后按钮提交的数据作为首次展现的数据；否则，拿到wifi_setting的数据加载进来。
        var isChosen = $.isEmptyObject(JSON.parse(localStorage.getItem('OtherChosen')));//OtherChosen不存在的时候也是true
        console.log(isChosen);
        if (isChosen) {
            //拿到配置界面保存的被选中的wifi信息列表
            var objStr = localStorage.getItem("ChosenWifi");
            var obj = JSON.parse(objStr);//此处向服务端请求来的是被选中的wifi的所有信息，是一个对象格式的。
            console.log(obj);
            //用户没有做选择，使用默认值的情况或者用户做了选择点击，但是没有选中哪一个
            var mac = obj.mac;
            var wifiPwd = obj.wifiPwd;

            $.each(scan, function (index_, value_) {
                $.each(scan[index_], function (name, value) {
                    if (name == "bssid" && value == mac) {
                        wifiObj = scan[index_];
                        //初始化路由设置界面的路由名称(Chosen是wifi_setting页面提交过来的，包括配置界面的主路由的所有信息和用户输入的正确密码)
                        $("#wifiName>.wifi-l").text(scan[index_].ssid);
                        //初始化路由设置界面的路由密码（wifiPwd是输入的主路由的密码）
                        $("#wifiPwd").val(wifiPwd);
                        //初始化信号强度
                        $("#signalStrength").val(scan[index_].signal);
                        //加密方式
                        $("#encrypt").val(scan[index_].security);
                        //MAC地址
                        $("#mac").val(mac);
                    }
                })
            })
        } else {
            //非首次进入路由设置界面情况

            //要在每次提交当前被选中的wifi信息的时候setItem一下（otherChosenWifi保存的是当前路由设置界面的被选中的wifi的信息，包括密码）
            var OtherChosenStr = localStorage.getItem('OtherChosen');
            var OtherChosenEntity = JSON.parse(OtherChosenStr);
            console.log(OtherChosenEntity);
            //用户没有做选择，使用默认值的情况或者用户做了选择点击，但是没有选中哪一个
            wifiObj = OtherChosenEntity;
            console.log(wifiObj);
            //初始化路由设置界面的路由名称
            $("#wifiName>.wifi-l").text(OtherChosenEntity.ssid);
            //初始化路由设置界面的路由密码
            $("#wifiPwd").val(OtherChosenEntity.wifiPwd);
            //初始化信号强度
            $("#signalStrength").val(OtherChosenEntity.signal);
            //加密方式
            $("#encrypt").val(OtherChosenEntity.security);
            //MAC地址
            $("#mac").val(OtherChosenEntity.bssid);
        }

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

        //初始化信号强度
        $("#signalStrength").val(wifiObj.signal);
        //加密方式
        $("#encrypt").val(wifiObj.security);
        //MAC地址
        $("#mac").val(wifiObj.bssid);
    });

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
                console.log("response");

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

    //写校验规则
    $("#main>form").validate({
        rules: {
            wifiPwd: {
                emptyCheck: true,
            }
        },
        messages: {
            wifiPwd: {
                //这种验证是有顺序的，写在前面的先验证
                emptyCheck: "输入不能为空",
            }
        },
        //成功验证，label指的是发生错误时那个标签，就是上面例子中的span（element），是p的子元素
        success: function (label) {
            label.parent().parent().css({"border-color": "#ddd"});
        },
        //其中error是字符串，保存了messages中返回的错误信息，
        // element是验证失败的input元素。
        errorPlacement: function (error, element) {
            console.log(element);
            console.log(error);
            error.appendTo(element.siblings("p"));
            element.parent().css({"border-color": "#EE2222"});
        },
        // onfocusout:false
    });

    //确定按钮
    var $btn = $("#main>form>button>a");
    $btn.on("click", function () {
        var wifiPwd = $("#wifiPwd").val();

        // 点击下一步按钮，表单验证通过后，提交数据并且跳转到下一页
        var isBoolean = $("#main>form").valid();
        console.log(isBoolean);
        if (isBoolean) {
            //提交当前的路由信息和密码
            $.ajax({
                type: "POST",
                url: "/cgi-bin/wifi_setting.cgi",//需要服务端的请求的地址
                data: "station_commit=commit" + "&" + "routePwd=" + wifiPwd + "&" + wifiData,//需要哪些数据
                error: function (response) {
                    console.log("提交当前路由设置信息失败");
                },
                success: function (response) {
                    //需要返回所有的信息列表
                    console.log(response);//请返回rootSuccess
                    console.log("提交当前路由设置信息成功");

                    //当前信息提交到服务器成功以后，然后缓存到本地，方便再次进入页面的时候调取。
                    var obj = wifiObj;
                    obj['wifiPwd'] = wifiPwd;
                    console.log(obj);

                    var objstr = JSON.stringify(obj);
                    localStorage.setItem('OtherChosen', objstr);

                    if (response == "routeSuccess") {
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
                                window.location.href = "manage_navWiFi_setting.html";
                            }
                            , no: function (index) {
                                //点击取消按钮
                                // layer.open({content: '执行取消操作'})

                                //执行取消操作，进去重启放大器的过程
                                $.ajax({
                                    type: "POST",
                                    url: "/cgi-bin/sys_setting.cgi",//请求的接口数据，拿到上网的人的信息
                                    data: "reboot_sys=reboot",
                                    error: function () {
                                        console.log("开始重启系统......失败！");
                                    },
                                    success: function (response) {
                                        console.log("开始重启系统......成功！");
                                        console.log(response);
                                        if (response == "reboot") {
                                            window.location.href = "syetem_reboot_progress.html";
                                        }
                                    }
                                });
                            }
                            , success: function (elem) {
                                //这里是打开页面成功
                                //elem:该回调参数返回一个参数为当前层元素对象
                                console.log(elem);

                                //显示弹窗以后，增加一个元素到弹窗中
                                $(".layui-m-layersection>.layui-m-layerchild").append("<div class='popup-close'></div>");

                                $(".layui-m-layersection>.layui-m-layerchild>.popup-close").on("click", function () {
                                    $(elem).css({"display": "none"});
                                });
                            }
                        });
                    }
                }
            })
        }
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