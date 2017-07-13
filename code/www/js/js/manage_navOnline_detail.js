$(function () {
    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    //传参数，接收参数，获取url，解决中文乱码问题的最佳方案。
    //获取url
    var url = window.location.href;
    var data = decodeURI(getUrlVar("data"));
    console.log(typeof data);
    console.log(data);

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

    var arrInfo = data.split(",");
    console.log(arrInfo);

    var $btn = $("#main>button[type='submit']");
    var $a = $("#main>button[type='submit']>a");

    $("#hostname").val(arrInfo[0]);
    var oldvalue = $("#hostname").val();

    //将信号的数值转换成中文的优良中差
    switch (true) {
        case arrInfo[1] >= -80 :
            $("#signal").text("优");
            break;
        case arrInfo[1] >= -85 :
            $("#signal").text("良");
            break;
        case arrInfo[1] >= -90 :
            $("#signal").text("中");
            break;
        case arrInfo[1] >= -110 :
            $("#signal").text("差");
            break;
        default:
            console.log("信号强度不在以上范围内");
    }


    $("#time>span").text(arrInfo[2]);
    $("#mac").text(arrInfo[3]);
    var mac = $("#mac").text();


    // var urlinfo = window.location.href;//获取url
    // var userName = getUrlVar('userName');//拆分url得到userName參數值
    // $("#userName").html(decodeURI(userName));

    //监听input的变化，做相应的处理
    // $("#managePassword").on("input", function () {
    //     // console.log("input变化了");
    //     var newVaule = $(this).val();
    //     // console.log("newVaule===="+newVaule);
    //     // console.log("oldValue==="+oldvalue);
    //     // console.log(typeof newVaule);
    //     // console.log(typeof oldvalue);
    //     if(newVaule == oldvalue){
    //         console.log("2");
    //         $btn.css("background-color", "green");
    //         $a.text("确定");
    //     }
    //     $a.text("保存修改");
    //     $btn.css("background-color", "red");
    // });

    //异步提交表单
    // $("#formId").submit(function () {
    //     $.ajax({
    //         type: "POST",
    //         url: "/cgi-bin/client_info.cgi",//请求的接口数据，拿到上网的人的信息
    //         data: "changename=changehost" + $("#formId").serialize() + mac,
    //         error: function () {
    //             console.log("修改hostname失败");
    //         },
    //         success: function (response) {
    //             console.log(response);
    //             console.log("修改hostname成功");
    //         }
    //     });
    //     return false;
    // });

    //监听移动端软键盘的回车/完成/搜索按钮
    $("#hostname").keypress(function (e) {
        if (e.keyCode === 13) {
            // 处理相关逻辑
            $.ajax({
                type: "POST",
                url: "/cgi-bin/client_info.cgi",//请求的接口数据，拿到上网的人的信息
                data: "changename=changehost" + "&" + $("#formId").serialize() + "&" + "mac=" + mac,
                error: function () {
                    console.log("修改hostname失败");
                },
                success: function (response) {
                    console.log(response);
                    console.log("修改hostname成功");
                }
            });
            return false;
        }
    })

    //加入黑名单功能
    $a.on("click", function () {
        layer.open({
            title: [
                '加入黑名单',
                'background-color:#438cff; color:#fff;'
            ]
            , anim: 'up'
            , content: '是否立即加入黑名单？'
            , btn: [
                '确定',
                '取消',
            ]
            , yes: function (index) { //点击确定按钮
                $.ajax({
                    type: "POST",
                    url: "/cgi-bin/client_info.cgi",//需要服务端的请求的地址
                    data: "blacklist=black" + "&" + "mac=" + mac,
                    error: function (response) {
                        console.log("当前设备加入黑名单失败");
                    },
                    success: function (response) {
                        //提交数据成功以后需要服务器返回除了黑名单以外的所有设备信息列表
                        console.log(response);
                        console.log("当前设备加入黑名单成功");

                        var onlineEntity = JSON.parse(response);

                        //存储除了黑名单以外的数据，等到进入上网详情页的时候取出
                        localStorage.setItem('onlinePep', response);

                        var isBooleanFromInfo = $.isEmptyObject(onlineEntity);//json对象为空时返回true
                        console.log(isBooleanFromInfo);

                        window.location.href = encodeURI("manage_nav_online.html?isBoolean=" + isBooleanFromInfo);

                        //点击的时候通过加载进来的数据判断数据是否为空跳转进入不同的页面。
                        // if (isBoolean) {
                        //     //返回数据为空的情况
                        //     // window.location.href = "manage_navOnline_nobody.html";
                        //     window.location.href = "manage_nav_online.html";
                        //     $("body>.nobody-online").css("display","block");
                        // } else {
                        //     window.location.href = "manage_nav_online.html";
                        // }
                    }
                })

                // window.location.href = "manage_nav_online.html";
            }
            , no: function (index) { //点击取消按钮
                // layer.open({content: '执行取消操作'})
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
})