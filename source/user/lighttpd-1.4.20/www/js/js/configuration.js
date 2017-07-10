$(function () {
    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    $.ajax({
        type: "POST",
        url: "/cgi-bin/wifi_setting.cgi",
        data: "if_success=success",
        success: function (response) {
            console.log(response);
            console.log(typeof response);
            if (response == "conntrue") {
                console.log("进入了success函数");
                window.location.href = "configuration_success.html";
            }
            if (response == "connfalse") {
                console.log("进入了false函数");
                window.location.href = "configuration_fail.html";
            }
        },
        error: function (res) {
            console.log(res);
            console.log(typeof res);
            if (res == "conntrue") {
                console.log("进入了success函数");
                window.location.href = "configuration_success.html";
            }
            if (res == "connfalse") {
                console.log("进入了false函数");
                window.location.href = "configuration_fail.html";
            }
        },
        // complete: function (XMLHttpRequest, status) { //请求完成后最终执行参数
        //     console.log("提交表单数据成功后服务端返回的数据------------" + XMLHttpRequest);
        //     console.log("提交表单数据成功后服务端返回的数据------------" + status);
        //     //超时的情况下代表着与服务端密码验证成功
        //     //输入正确的主路由的密码进入statue为timeout的情况
        //     if (status == 'timeout') {//超时,status还有success,error等值的情况
        //         console.log("已经超时，表明已经与主路由连接上了");
        //         window.location.href = "configuration_success.html";
        //     }
        //
        //     //在10秒钟以后断网，代表连接成功
        //     if(status=='error'){
        //         console.log("error");
        //         window.location.href = "configuration_success.html";
        //     }
        //
        //     //输入错误的主路由的密码进入statue为success的情况
        //     if(status=='success'){
        //         console.log("success");
        //         window.location.href = "configuration_fail.html";
        //     }
        // },
    });
    // setTimeout(function () {
    //     window.location.href = "configuration_success.html";
    // },30000);
});
