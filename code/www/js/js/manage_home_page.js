$(function () {

    //重新启动
    $("#nav>a:nth-child(6)").on("click", function () {
        layer.open({
            title: [
                '恢复出厂设置',
                'background-color:#438cff; color:#fff;'
            ]
            , anim: 'up'
            , content: '重启需要50s，确定重启WiFi放大器？'
            , btn: [
                '确定',
                '取消',
            ]
            , yes: function (index) {
                //index为当前层的索引
                // console.log(index);//0
                // window.location.href = "syetem_reboot_progress.html";

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
            , no: function (index) {
                //index为当前层的索引
                // console.log(index);//undefined

                //这里不用做任何处理，弹出层会消失
            }
            , success: function (elem) {
                //elem:该回调参数返回一个参数为当前层元素对象
                // console.log(elem);

                //显示弹窗以后，增加一个元素到弹窗中
                $(".layui-m-layersection>.layui-m-layerchild").append("<div class='popup-close'></div>");

                $(".layui-m-layersection>.layui-m-layerchild>.popup-close").on("click", function () {
                    $(elem).css({"display": "none"});
                });
            }
        });

        return false;//禁止a标签的默认事件
    });

    // window.location.href = "manage_nav_online.html";
    // $("body>.nobody-online").css("background-color","red");

    //谁在上网
    $("#nav>a:nth-child(1)").on("click", function () {
        $.ajax({
            type: "POST",
            url: "/cgi-bin/client_info.cgi",//请求的接口数据，拿到上网的人的信息
            data: "Client_Info=wholine",
            error: function () {
                console.log("请求谁在上网信息失败");
            },
            success: function (response) {
                console.log("请求谁在上网信息成功");
                console.log(response);

                var onlineEntity = JSON.parse(response);

                //存储数据，等到进入上网详情页的时候取出
                localStorage.setItem('onlinePep', response);

                var isBooleanFromInfo = $.isEmptyObject(onlineEntity);//json对象为空时返回true
                console.log(isBooleanFromInfo);

                window.location.href = encodeURI("manage_nav_online.html?isBoolean=" + isBooleanFromInfo);

                //点击的时候通过加载进来的数据判断数据是否为空跳转进入不同的页面。
                // if(isBoolean){
                //     //服务器返回数据为空的情况
                //     window.location.href = "manage_nav_online.html";
                //     $("body>.nobody-online").css("display","block");
                // }else{
                //     //服务器返回数据
                //     window.location.href = "manage_nav_online.html";
                // }
            }
        });

        // var responseText = '{"Client_Info":[{"HostName":"3MIX2-xiaomishouj","Mac":"AC:F7:F3:1D:57:6E","Msg_os":"xiaomi Android","Signal":"-99","ConnectedTime":"800"},{"HostName":"hp-PC","Mac":"D1:F7:F3:B1:57:4A","Msg_os":"Microsoft Windows Vista/7 or Server 2008","Signal":"-87","ConnectedTime":"1300"},{"HostName":"飞奔的蜗牛-TV","Mac":"C5:F9:F3:B3:23:2C","Msg_os":"android TV","Signal":"-82","ConnectedTime":"4000"},{"HostName":"你猜我是谁","Mac":"A5:F2:C6:B9:23:1B","Msg_os":"android TV","Signal":"-30","ConnectedTime":"3000"}]}';
        // localStorage.setItem('onlinePep', responseText);
        // window.location.href = "manage_nav_online.html";

        return false;
    })

    //路由设置
    $("#nav>a:nth-child(2)").on("click", function () {

        window.location.href = "manage_navRoute_setting.html";

        return false;
    })

    //wifi设置
    $("#nav>a:nth-child(3)").on("click", function () {
        window.location.href = "manage_navWiFi_setting.html";
        return false;
    })
});