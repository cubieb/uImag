$(function () {
    initialFun();
    function initialFun() {
        //返回按钮
        $("header>.nav-back").on("click", function () {
            window.history.go(-1);
        });

        //我的代码中有模板渲染的问题，在点击下拉按钮之前渲染模板，+才能实现toggleClass，为了解决点击出现下拉不能收缩的bug，必须在初始化的时候刷新
        var scanJsonStr = localStorage.getItem('scanWifi');
        scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        console.log(scanJsonStr);
        console.log(scanEntity);
        var scan = scanEntity.Scan;

        //按照当前信号前强度重新排序数组中的对象
        // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
        // console.log(scan);
        // scan.splice(-1, 1);
        // scan.pop();
        // console.log(scan);
        var sortObj = scan.sort(compare("signal"));//会改变scan的排序
        console.log(scan);
        console.log(scanEntity.Scan);
        console.log(scanEntity);

        //直接拿到localStorage中的数据以后，模板渲染到页面的ul中。
        var html = template("listTmp", scanEntity);
        $('#selectCon').html(html);
        console.log("点击下拉按钮之前渲染本地数据到页面成功");
        console.log($('#selectCon').html());

        //判断当前扫描的wifi是否有锁
        //点击下拉按钮以后需要遍历一遍看是否都有锁
        $.each(scan, function (index_, value_) {
            //给信号强度不同的wifi加上不同的信号强度图标
            // console.log(scan[index_]);
            // console.log((scan[index_].hasOwnProperty("ssid")));

            // console.log((scan[index_]));
            // if(!(scan[index_].hasOwnProperty("ssid"))){
            //     //console.log("不合法的wifi信息数据-------w");
            //     console.log(index_);
            //     console.log(scan.length);
            //     console.log(Array.isArray(scan));
            //     scan.splice(index_,1);
            //     console.log(scan);
            // }

            $.each(scan[index_], function (name, value) {

                // console.log(Array.isArray(scan));//scan是array类型的。打印出true

                //可以检测ssid名为空的情况
                // if (name === "ssid" && value === "") {
                //     console.log("不合法的wifi信息数据1");
                // }
                //
                // console.log(scan[index_].name);

                //删除过滤掉ssid名为空的情况
                //name === "undefined"这样写是对的
                // (name === "ssid" && value === "") ||

                // console.log(scan[index_][name]);

                if ((scan[index_][name] === "")) {
                    console.log("不合法的wifi信息数据-------d");
                    console.log(index_);
                    console.log(scan);
                    // scan.splice(1,1);
                    // console.log(scan[9].name);
                    //
                    // //回去使用apply的方法借用试一下
                    // var arr = [];
                    // $.each(scan, function (index_, value_) {
                    //     arr.push(scan[index_]);
                    // })
                    // arr.splice(index_, 1);
                    // console.log(arr);


                    // scan = [];
                    // $.each(arr, function (index_, value_) {
                    //     scan.push(arr[index_]);
                    // })
                    //
                    // console.log(scan);

                    // $.each(arr, function (index_, value_) {
                    //     arr.push(scan[index_]);
                    // })

                    // console.log("我是做测试用的");
                    // console.log(scan.length);
                    // console.log(scan);
                    // console.log(scan[0]);
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

    var $wifiPassword = $("#wifi-list>.wifi-password>input[type='text']");
    var $newWifiName = $("#wifi-list>.wifi-amplifier>#newWifiName");
    var $newPassword = $("#wifi-list>.amplifier-password>#newPassword");

    //初始化wifi_setting页面的数据
    $("#wifi-list>.wifi-name>.wifi-l").text(scanEntity.Scan[0].ssid);
    //主路由placeholder的提示与所选主路由的wifi同步
    $wifiPassword.attr("placeholder", "请输入" + '"' + scanEntity.Scan[0].ssid + '"' + "的密码");
    //用户没有做选择，使用默认值的情况或者用户做了选择点击，但是没有选中哪一个
    wifiObj = scanEntity.Scan[0];
    wifiData = "Channel=" + wifiObj.Channel + "&" +
        "ssid=" + wifiObj.ssid + "&" +
        "bssid=" + wifiObj.bssid + "&" +
        "security=" + wifiObj.security + "&" +
        "signal=" + wifiObj.signal + "&" +
        "mode=" + wifiObj.mode + "&" +
        "ext_ch=" + wifiObj.ext_ch + "&" +
        "net_type=" + wifiObj.net_type + "&" +
        "wps=" + wifiObj.wps;
    // localStorage.setItem("wifiData", wifiData);
    console.log("成功获取被点击的wifi的相关信息：" + wifiData);

    //将wifi放大器的wifi名和密码设置得与主路由关联
    ssidName = wifiObj.ssid;
    console.log("获取了被点击的wifi名:" + ssidName);
    $newWifiName.val(ssidName + "-Plus");

    //设置新Wi-Fi的名称与新Wi-Fi密码保持同步设定初始值
    $newPassword.attr("placeholder", "请输入" + ssidName + "-Plus" + "的密码");

    //主路由的密码默认与放大器的密码是在用户输入的时候保持同步一致的
    $wifiPassword.on("input", function () {
        $newPassword.val($wifiPassword.val());
    });

    //设置新Wi-Fi的名称与新Wi-Fi密码保持同步
    $newWifiName.on("input", function () {
        $newPassword.attr("placeholder", "请输入" + $newWifiName.val() + "的密码");
    });

    //扫描获取wifi列表信息，并渲染到前端页面
    //wifi列表拿到的只是刷新缓存到本地的数据
    //点击模板的时候需要进行刷新是因为，那个刷新按钮的数据会产生变化，wifi列表要把这种保存到本地的wifi列表数据同步到新的列表中。
    var listBtn = $("#wifi-list>.wifi-name>.wifi-r>.list-btn");
    listBtn.on("click", function (e) {
        //将第一页跳转过程中加载的数据从HTML5的localStorage中取出来
        // 取值时：把获取到的Json字符串转换回对象
        var scanJsonStr = localStorage.getItem('scanWifi');
        scanEntity = JSON.parse(scanJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        console.log(scanEntity);
        var scan = scanEntity.Scan;
        //
        // // $.each(scan, function (index_, value_) {
        // //     $.each(scan[index_], function (name, value) {})
        // // });
        //
        // //按照当前信号前强度重新排序数组中的对象
        // // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
        // var sortObj = scan.sort(compare("signal"));//会改变scan的排序
        // console.log(scan);
        // console.log(scanEntity.Scan);
        // console.log(scanEntity);
        //
        // //直接拿到localStorage中的数据以后，模板渲染到页面的ul中。
        // var html = template("listTmp", scanEntity);
        // $('#selectCon').html(html);//注意这种重新渲染会将上一次渲染设置的样式等等抹掉。
        // console.log(html);
        // console.log("将模板数据渲染到页面成功");

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

        //js动态将最后的“其它”插入第四个元素之后
        // $(".selectContainer>ul>li:last-of-type").insertAfter($(".selectContainer>ul>li:nth-of-type(4)"));
        e.stopPropagation();//不出现事件的原因是浏览器还有默认事件(跟select相关的)。

        $("#selectCon").select();//在selectContainer.js中的一个封装的方法
    });

    //刷新按钮
    //刷新时发送post请求是为了从服务器获取新的wifi列表数据
    //点击刷新按钮，在向服务器请求失败的情况下，虽然不能让获取新的wifi信息，但是呢，初始化的时候，已经渲染过ul的li的wifi列表信息了
    //而且zhixingajax以后还会执行 $(".selectContainer").select();，所以就让初始化的wifi信息列表显示了出来。
    $("#wifi-list>.wifi-name>.wifi-r>.refresh-btn").on("click", function (e) {
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

    //冒泡比较
    //降序排序
    function bubbling(arra) {
        var temp;
        for (var i = 0; i < arra.length; i++) { //比较多少趟，从第一趟开始

            for (var j = 0; j < arra.length - i - 1; j++) { //每一趟比较多少次数

                if (arra[j] < arra[j + 1]) {   //arra[j] > arra[j + 1] 代表升序排序
                    temp = arra[j];
                    arra[j] = arra[j + 1];
                    arra[j + 1] = temp;
                }
            }
        }
        ;
        return arra;
    }

    // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
    function compare(property) {
        return function (obj1, obj2) {
            var value1 = obj1[property];
            var value2 = obj2[property];
            return value2 - value1;     // 降序
        }
    }

    //发送ajax请求，失去焦点时检测用户输入的密码是否与主路由的密码一致
    // $("#wifiPassword").on("blur", function () {
    //     // console.log(this);
    //     console.log($(this).val());
    //     // var wifiPassword = $
    //     // console.log("我失去焦点了");
    //     var wifiPInput = $(this).val();
    //
    //     $.ajax({
    //         url: "/cgi-bin/wifi_setting.cgi",
    //         type: "POST",
    //         ansy: true,
    //         data: "mainpasswd=checkmainpasswd" + "&" + "wifiPassword=" + wifiPInput,
    //         success: function (response) {
    //             if (response === "mainpasswd_success") {
    //
    //             }
    //
    //             if (response === "mainpasswd_failure") {
    //                 var wifiErr = '<label id="wifiPassword-error" class="error" for="wifiPassword">你输入的路由密码错误，请重新输入</label>';
    //                 $("#wifi-list>.wifi-password>p").html(wifiErr);
    //             }
    //         }
    //     });
    //
    //     // if (1) {
    //     //     var wifiErr = '<label id="wifiPassword-error" class="error" for="wifiPassword">你输入的路由密码错误，请重新输入</label>';
    //     //     $("#wifi-list>.wifi-password>p").html(wifiErr);
    //     // }
    // });

    //设置放大器的密码与主路由的密码是否一致（默认勾选上，默认一致）
    //监听checkbox
    $("#wifi-form>#wifi-list>.same-password>.cBox").on("click", function () {
        // console.log($("#wifiPassword").val());
        // console.log(wifiData);
        var isBoolean = $(this).find("input[type='checkbox']").prop("checked");
        if (isBoolean) {
            $("#wifi-list>.password-management").hide(300);
        } else {
            $("#wifi-list>.password-management").show(300);
        }
    });

    //写校验规则
    $("#wifi-form").validate({
        rules: {
            wifiPassword: {
                emptyCheck: true,
                // manageCodeCheck: true,
                // remote: {  //返回的结果只能是true或者false
                //     url: "/cgi-bin/wifi_setting.cgi",
                //     type: "POST",
                //     // data: "mainpasswd=ckpwd" + "&" + "wifiPassword=" + $("#wifiPassword").val() + "&" + wifiData
                //     data: {
                //         mainpasswd: "ckpwd",
                //         wifiPassword: function () {
                //             return $("#wifiPassword").val();
                //         },
                //         Channel: function () {
                //             return wifiObj.Channel;
                //         },
                //         ssid: function () {
                //             return wifiObj.ssid;
                //         },
                //         security: function () {
                //             return wifiObj.security;
                //         }
                //     }
                // }
            },
            newWifiName: {
                emptyCheck: true,
                minlength: 1,
                maxlength: 26
            },
            newPassword: {
                emptyCheck: true,
                manageCodeCheck: true,
            },
            managePassword: {
                emptyCheck: true,
                manageCodeCheck: true,
            }
        },
        messages: {
            wifiPassword: {
                //这种验证是有顺序的，写在前面的先验证
                emptyCheck: "输入不能为空",
                // manageCodeCheck: "有效密码为8-64位字符",
                // remote: "密码输入不正确"
            },
            newWifiName: {
                emptyCheck: "输入不能为空",
            },
            newPassword: {
                emptyCheck: "输入不能为空",
                manageCodeCheck: "有效密码为8-64位字符",
            },
            managePassword: {
                emptyCheck: "输入不能为空",
                manageCodeCheck: "有效密码为8-64位字符",
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

    // 点击下一步按钮，表单验证通过后，提交数据并且跳转到下一页
    $('#wifi-form>#wifi-list>#next-bnt').click(function () {
        var isBoolean = $("#wifi-form").valid();
        console.log(isBoolean);
        if (isBoolean) {
            var obj = {};
            obj["mac"] = wifiObj.bssid;
            obj["wifiName"] = wifiObj.ssid;
            obj["wifiPwd"] = $wifiPassword.val();
            obj["newWifiName"] = $newWifiName.val();
            obj["newPassword"] = $newPassword.val();

            var isManaPwd = $("#samePassword").prop("checked");//wifi密码与管理密码是否相同
            if (isManaPwd) {
                obj["wmcheckedVal"] = true;
                obj["managePassword"] = $newPassword.val();
            } else {
                obj["wmcheckedVal"] = false;
                obj["managePassword"] = $("managePassword").val();
            }

            if ($wifiPassword.val() == $newPassword.val()) {//wifi密码与主路由密码是否相同
                obj["wzcheckedVal"] = true;
            } else {
                obj["wzcheckedVal"] = false;
            }

            // var objStr = JSON.stringify(obj);
            // localStorage.setItem("ChosenWifi", objStr);

            //提交与管理界面相关关联的数据
            $.ajax({
                type: "POST",
                url: "/cgi-bin/wifi_setting.cgi",
                // wifiData是指被选中的wifi的相关信息
                data: {
                    data_commit: "data",
                    mac: function () {
                        return obj["mac"];
                    },
                    wifiName: function () {
                        return obj["wifiName"];
                    },
                    wifiPwd: function () {
                        return obj["wifiPwd"];
                    },
                    newWifiName: function () {
                        return obj["newWifiName"];
                    },
                    newPassword: function () {
                        return  obj["newPassword"];
                    },
                    wmcheckedVal: function () {
                        return   obj["wmcheckedVal"];
                    },
                    wzcheckedVal: function () {
                        return  obj["wzcheckedVal"];
                    },
                    managePassword: function () {
                        return  obj["managePassword"];
                    }
                },
                error: function (xhr, textStatus) {
                    console.log("data_commit---------失败");
                },
                success: function (msg) {
                    console.log("data_commit------------成功");
                }
            });

            // wifiData是指被选中的wifi的相关信息
            $.ajax({
                // type: "POST",
                // url: "/cgi-bin/wifi_setting.cgi",
                // data: "wifiCommit=commit" + "&" + wifiData + "&" + $("#wifi-form").serialize(),  //这里的作用是请求服务端返回一个连接成功的标志status
                // timeout: 10000,
                // async: true,
                // complete: function (XMLHttpRequest, status) { //请求完成后最终执行参数
                //     console.log("提交表单数据成功后服务端返回的数据------------" + XMLHttpRequest);
                //     console.log("提交表单数据成功后服务端返回的数据------------" + status);
                //     //超时的情况下代表着与服务端密码验证成功
                //     //输入正确的主路由的密码进入statue为timeout的情况
                //     if (status == 'timeout') {//超时,status还有success,error等值的情况
                //         console.log("已经超时，表明已经与主路由连接上了");
                //     }
                //     //在10秒钟内有反应
                //     if (status == 'error') {
                //         console.log("error");
                //     }
                //     //输入错误的主路由的密码进入statue为success的情况
                //     if (status == 'success') {
                //         console.log("success");
                //         window.location.href = "configuration_fail.html";
                //     }
                // }

                type: "POST",
                url: "/cgi-bin/wifi_setting.cgi",
                // wifiData是指被选中的wifi的相关信息
                data: "wifiCommit=commit" + "&" + wifiData + "&" + $("#wifi-form").serialize(),
                error: function (xhr, textStatus) {
                    console.log("提交表单失败服务端返回的数据---------" + textStatus);
                    console.log('error:' + textStatus);
                },
                success: function (msg) {
                    console.log("提交表单数据成功后服务端返回的数据------------" + msg);
                    console.log("提交表单数据成功");
                    if (msg == "wifiSettingSuccess") {
                        window.location.href = "configuration.html";
                    }
                }


                // complete : function(XMLHttpRequest,status){ //请求完成后最终执行参数
                //     console.log("提交表单数据成功后服务端返回的数据------------" + XMLHttpRequest);
                //     console.log("提交表单数据成功后服务端返回的数据------------" + status);
                //     //超时的情况下代表着与服务端密码验证成功
                //     //输入正确的主路由的密码进入statue为timeout的情况
                //     if(status=='timeout'){//超时,status还有success,error等值的情况
                //         console.log("已经超时，表明已经与主路由连接上了");
                //     }
                //     //在10秒钟内有反应
                //     if(status=='error'){
                //         console.log("error");
                //     }
                //     //输入错误的主路由的密码进入statue为success的情况
                //     if(status=='success'){
                //         console.log("success");
                //         window.location.href = "configuration_fail.html";
                //     }
                // }
            });

            // window.location.href = "configuration.html";
        }
    });
});
