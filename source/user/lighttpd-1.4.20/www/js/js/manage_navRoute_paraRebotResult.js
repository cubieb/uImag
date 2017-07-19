$(function () {
    $.ajax({
        type: "POST",
        url: "/cgi-bin/wifi_setting.cgi",
        data: "init_restart=restart", //单纯只做重启
        success: function (response) {
            console.log(response);
        },
        error: function (res) {
            console.log(res);
        }
    });
});