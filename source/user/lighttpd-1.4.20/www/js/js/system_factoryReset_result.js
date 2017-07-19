$(function () {
    $.ajax({
        type: "POST",
        url: "/cgi-bin/sys_setting.cgi",
        data: "reboot_sys=reboot",
        success: function (response) {
            console.log(response);
        },
        error: function (res) {
            console.log(res);
        }
    });
});
