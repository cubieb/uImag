$(function () {

    // $("body>.nobody-online").css("display","block");
    // $("body>.nobody-online").css("background-color","red");

    //解析通过url传过来的参数boolean
    var url = window.location.href;
    var isBoolean = decodeURI(getUrlVar("isBoolean"));
    console.log(typeof isBoolean);
    console.log(isBoolean);

    // 获取请求url参数数组
    function getUrlVars() {
        var vars = [], hash;
        var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
        for (var i = 0; i < hashes.length; i++) {
            hash = hashes[i].split('=');
            vars.push(hash[0]);
            vars[hash[0]] = hash[1];
        }
        return vars;
    }

    function getUrlVar(name) {
        return getUrlVars()[name];
    }

    //点击的时候通过加载进来的数据判断数据是否为空跳转进入不同的页面。
    //在编码和解码传参的过程中，boolean类型转换为string类型。
    if (isBoolean == "true") {
        //返回数据为空的情况
        $("body>.nobody-online").css("display","block");
    }else{
        initialFun();
        //给每一个上网列表项增加一个点击监听事件
        $("#main>.online-info").on("click", function () {
            var msg_os = $(this).find(".tv-info").children("p:nth-of-type(3)").text();
            console.log(msg_os);
            var hostname = $(this).find(".tv-info").children("h3").text();
            console.log("被点击上网设备的hostname----------" + hostname);

            var signal = $(this).find(".tv-info").children("p:nth-of-type(4)").text();
            console.log(signal);

            var time = $(this).find(".tv-info").children("p:nth-of-type(5)").text();
            console.log(time);
            var mac = $(this).find(".tv-info").children("p:nth-of-type(2)").text();
            console.log(mac);
            var dataFromInfo = [hostname, signal, time, mac];
            console.log(dataFromInfo);
            // window.location.href = "manage_navOnline_detail.html?data="+dataFromInfo;
            //做编码处理，防止中文字符乱码
            window.location.href = encodeURI("manage_navOnline_detail.html?data=" + dataFromInfo);
        });
    }

    function initialFun() {
        //按照当前信号前强度重新排序数组中的对象
        // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
        // console.log(scan);
        // scan.splice(-1, 1);
        // scan.pop();
        // console.log(scan);
        // var sortObj = scan.sort(compare("signal"));//会改变scan的排序
        // console.log(scan);
        // console.log(scanEntity.Scan);
        // console.log(scanEntity);

        //我的代码中有模板渲染的问题，在点击下拉按钮之前渲染模板，+才能实现toggleClass，为了解决点击出现下拉不能收缩的bug，必须在初始化的时候刷新
        var onlineJsonStr = localStorage.getItem('onlinePep');
        onlineEntity = JSON.parse(onlineJsonStr);//变量被放到了全局环境中(这样写安全不安全？)
        console.log(onlineEntity);

        var onlineP = onlineEntity.Client_Info;


        //信号按照强弱排序
        onlineP.sort(compare("Signal"));
        console.log(onlineP);

        //将服务端发送过来的“连接时间”的秒数，换算为分钟显示。
        $.each(onlineP, function (index_, value_) {
            $.each(onlineP[index_], function (name, value) {
                if (name == "ConnectedTime") {
                    var time = (onlineP[index_].ConnectedTime);
                    console.log(time);
                    onlineP[index_].ConnectedTime = parseInt(time / 60);
                }
            })
        })

        var html = template("onlineTmp", onlineEntity);
        $('#main').html(html);

        //给信号强度不同的wifi加上不同的信号强度图标
        $.each(onlineP, function (index_, value_) {
            $.each(onlineP[index_], function (name, value) {
                if (name === "Signal") {
                    console.log(value);
                    var $signalImg = $("#main>.online-info:nth-child(" + (index_ + 1) + ")>.fr>img:nth-child(1)");
                    switch (true) {
                        case value >= -80 :
                            // console.log("满格信号");
                            $signalImg.attr("src", "images/02_icon03_01.png");
                            return false;//$.each的退出循环,此处跳出的是数组中对对象的遍历循环，不是跳出的对对象外层数组的遍历循环。
                        // break;
                        case value >= -85 :
                            // console.log("信号较强");
                            $signalImg.attr("src", "images/02_icon03_02.png");
                            return false;
                        // break;
                        case value >= -90 :
                            // console.log("信号一般");
                            $signalImg.attr("src", "images/02_icon03_03.png");
                            return false;
                        // break;
                        case value >= -110 :
                            // console.log("信号很弱");
                            $signalImg.attr("src", "images/02_icon03_04.png");
                            return false;
                        default:
                            console.log("信号强度不在以上范围内");
                    }
                }

                //给security不同的wifi加锁和不加锁
                // console.log(index_);
                // if (name === "security" && value === "OPEN" || name === "security" && value === "NONE") {
                //     $("#selectCon>ul>li:nth-child(" + (index_ + 1) + ")>img:nth-child(1)").css("display", "none");
                //     // console.log("加锁图标的显示隐藏");
                // }
            });
        })
    }

    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    //进入黑名单
    $("header>.blacklist").on("click", function () {
        // window.location.href = "manage_blacklist.html";

        $.ajax({
            type: "POST",
            url: "/cgi-bin/Client_Info.cgi",//需要服务端的请求的地址
            data: "showblacklist=show",
            error: function (response) {
                console.log("请求黑名单数据失败");
            },
            success: function (response) {
                console.log(response);
                console.log("请求黑名单数据成功");

                var blackonlineEntity = JSON.parse(response);

                //存储除了黑名单以外的数据，等到进入上网详情页的时候取出
                localStorage.setItem('blackList', response);

                var isBooleanFromInfo = $.isEmptyObject(blackonlineEntity);//json对象为空时返回true
                console.log(isBooleanFromInfo);

                window.location.href = encodeURI("manage_blacklist.html?isBoolean=" + isBooleanFromInfo);

                //点击的时候通过加载进来的数据判断数据是否为空跳转进入不同的页面。
                // if (isBoolean) {
                //     //返回数据为空的情况
                //     window.location.href = "manage_blacklist.html";
                //     $("body>.blacklist-info").css("display", "block");
                // } else {
                //     window.location.href = "manage_blacklist.html";
                // }
            }
        })

        // window.location.href = "manage_blacklist.html";
        // window.location.href = "manage_blacklist_nobody.html";
    });

    // 对象数组通过对象的属性进行排序(scan通过数组中对象的signal信号强弱进行排序，排序完了以后再渲染到页面中)
    function compare(property) {
        return function (obj1, obj2) {
            var value1 = obj1[property];
            var value2 = obj2[property];
            return value2 - value1;     // 降序
        }
    }
})