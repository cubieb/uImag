$(function () {
    initialFun();

    function initialFun() {
        //进入界面的时候拿到初始的version版本信息
        $.ajax({
            type: "POST",
            url: "/cgi-bin/sys_setting.cgi",//请求的接口数据，拿到上网的人的信息
            data: "get_version=version",
            error: function () {
                console.log("请求在线升级版本信息......失败！");
            },
            success: function (res) {
                console.log(res);
                console.log("请求在线升级版本信息......成功！");

                var isUpdateRes = JSON.parse(res);
                console.log(isUpdateRes);

                //是否更新
                var updateInfo = isUpdateRes.update;

                //进入界面以后加载到当前版本信息
                var version = isUpdateRes.version;

                var $updateText =  $("#main>p>span:nth-child(3)");
                var $version =  $("#main>p>span:nth-child(2)");

                if (updateInfo == "1") {
                    console.log("updateInfo=1的情况");

                    //从服务器拿到的永远是当前的版本号，而不是最新的版本号
                    $version.text(version);
                    $updateText.get(0).style.display = "inline";
                }

                if (updateInfo == "0") {
                    console.log("updateInfo=0的情况");

                    $version.text(version);
                    if($updateText.get(0)){
                        $updateText.get(0).style.display = "none";
                    }
                }
            }
        });
    }


    //恢复出厂设置并重启放大器
    $("#footer>.factory-reset").on("click", function () {
        layer.open({
            title: [
                '恢复出厂设置',
                'background-color:#438cff; color:#fff;'
            ]
            , anim: 'up'
            , content: '立即恢复出厂设置并重启？'
            , btn: [
                '确定',
                '取消',
            ]
            , yes: function (index) {
                // window.location.href = "system_factoryReset_progress.html";
                $.ajax({
                    type: "POST",
                    url: "/cgi-bin/sys_setting.cgi",//请求的接口数据，拿到上网的人的信息
                    data: "recover_sys=recover",
                    error: function () {
                        console.log("开始恢复出厂设置......失败！");
                    },
                    success: function (response) {
                        console.log("开始恢复出厂设置......成功！");
                        console.log(response);
                        if (response == "recover") {
                            window.location.href = "system_factoryReset_progress.html";
                        }
                    }
                });
            }
            , no: function (index) {
                // layer.open({content: '执行取消操作'})
            }
            , success: function (elem) {
                //elem:该回调参数返回一个参数为当前层元素对象
                console.log(elem);

                //显示弹窗以后，增加一个元素到弹窗中
                $(".layui-m-layersection>.layui-m-layerchild").append("<div class='popup-close'></div>");

                $(".layui-m-layersection>.layui-m-layerchild>.popup-close").on("click", function () {
                    $(elem).css({"display": "none"});
                });
            }
        });
    });

    //在线升级
    $("#main>.online-btn").on("click", function () {
        //直接进入到下一个界面，进行倒计时升级
        window.location.href = "system_update_progress.html";
    })
});