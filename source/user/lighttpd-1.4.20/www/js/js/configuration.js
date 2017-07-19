$(function () {
    //返回按钮
    $("header>.nav-back").on("click", function () {
        window.history.go(-1);
    });

    // $.ajax({
    //     type: "POST",
    //     url: "/cgi-bin/wifi_setting.cgi",
    //     data: "if_success=success",//只验证了与主路由是否一致，没有重启
    //     success: function (response) {
    //         console.log(response);
    //         console.log(typeof response);
    //         if (response == "conntrue") {
    //             console.log("进入了success函数");
    //             window.location.href = "configuration_success.html";
    //         }
    //         if (response == "connfalse") {
    //             console.log("进入了false函数");
    //             window.location.href = "configuration_fail.html";
    //         }
    //     },
    //     error: function (res) {
    //         console.log(res);
    //         console.log(typeof res);
    //         if (res == "conntrue") {
    //             console.log("进入了success函数");
    //             window.location.href = "configuration_success.html";
    //         }
    //         if (res == "connfalse") {
    //             console.log("进入了false函数");
    //             window.location.href = "configuration_fail.html";
    //         }
    //     },
    // });

    setTimeout(function () {
        window.location.href = "configuration_success.html";
    },15000);

});