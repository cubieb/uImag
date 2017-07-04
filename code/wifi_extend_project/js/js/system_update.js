$(function () {
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
                        if(response == "recover"){
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

                $(".layui-m-layersection>.layui-m-layerchild>.popup-close").on("click",function () {
                    $(elem).css({"display":"none"});
                });
            }
        });
    });
});