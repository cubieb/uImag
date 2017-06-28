$(function () {
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
                $.ajax({
                    type: "POST",
                    // url: "",//需要服务端的请求的地址
                    // data: "blacklist=black" + "&" + "mac=" + mac,//需要哪些数据
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

                        var isBoolean = $.isEmptyObject(onlineEntity);//json对象为空时返回true

                        //点击的时候通过加载进来的数据判断数据是否为空跳转进入不同的页面。
                        if (isBoolean) {
                            //返回数据为空的情况
                            window.location.href = "manage_navOnline_nobody.html";
                        } else {
                            window.location.href = "manage_nav_online.html";
                        }
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